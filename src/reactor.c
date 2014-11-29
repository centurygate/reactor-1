#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "reactor.h"
#include "socket_wrapper.h"
#include "selector.h"
#include "handle.h"
#include "error_code.h"
#include "logger.h"

reactor_t* reactor_create(int32_t max_socket_num) {
	int32_t i;
	reactor_t* p_reactor = NULL;
	
	p_reactor = (reactor_t*)malloc(sizeof(*p_reactor));
	if(p_reactor == NULL) {
		abort();
	}
	p_reactor->max_socket_num = max_socket_num;
	p_reactor->p_selector = create_selector(max_socket_num);
	if(p_reactor->p_selector == NULL) {
		abort();
	}
	p_reactor->pp_socket = (socket_t**)malloc(max_socket_num*sizeof(*p_reactor->pp_socket));
	if(p_reactor->pp_socket == NULL) {
		abort();
	}
	memset(p_reactor->pp_socket, 0, max_socket_num*sizeof(*p_reactor->pp_socket));
	for(i = 0; i < max_socket_num - 1; i++) {
		p_reactor->pp_socket[i] = (socket_t*)(i+1);
	}
	p_reactor->pp_socket[i] = (socket_t*)(-1);
	p_reactor->free_sock_idx = 0;
	dns_cache_initialize(&p_reactor->dns_cache, 30, 3600);
	p_reactor->wait_dns_sock_count = 0;
	p_reactor->socket_unique_tag_pool = 0;
	return p_reactor;
}

void reactor_destroy(reactor_t *p_reactor) {
	int32_t idx = -1;
	socket_t* p_socket;
	for (;;) {
		int32_t sock_idx;
		idx = selector_next(p_reactor->p_selector, idx);
		if (idx == -1) { // end
			break;
		}
		sock_idx = (int32_t)selector_data(p_reactor->p_selector, idx);
		p_socket = p_reactor->pp_socket[sock_idx];
		do_close_socket(p_socket);
	}
	dns_cache_finalize(&p_reactor->dns_cache);
	destroy_selector(p_reactor->p_selector);
	free(p_reactor->pp_socket);
	free(p_reactor);
}

int32_t reactor_register(reactor_t *p_reactor, socket_t *p_socket) {
	int32_t free_idx = p_reactor->free_sock_idx;
	int32_t next_free_idx;
	int32_t ret;

	if(free_idx == -1) {
		return EREACTORFULL;
	}
	next_free_idx = (int32_t)p_reactor->pp_socket[free_idx];

	p_reactor->pp_socket[free_idx] = p_socket;
	p_socket->id = free_idx;
	p_socket->p_reactor = p_reactor;
	p_socket->unique_tag = p_reactor->socket_unique_tag_pool++;

	ret = selector_add(p_reactor->p_selector, p_socket->p_handle, 
		SELECTORIN | SELECTOROUT, (void*)p_socket->id);
	if(ret < 0) {
		p_reactor->pp_socket[free_idx] = (socket_t*)next_free_idx;
		return ret;
	}
	p_reactor->free_sock_idx = next_free_idx;

	return 0;
}

void reactor_deregister(reactor_t *p_reactor, socket_t *p_socket) {
	logger_assert(p_socket->id >= 0);
	logger_assert(p_socket->id < p_reactor->max_socket_num);
	
	if(p_socket->id < 0 || p_socket->id >= p_reactor->max_socket_num) {
		return;
	}
	
	p_reactor->pp_socket[p_socket->id] = (socket_t*)(p_reactor->free_sock_idx);

	p_reactor->free_sock_idx = p_socket->id;

	selector_del(p_reactor->p_selector, p_socket->p_handle);
}

// @return :
// 	valid connection info pointer on success, 
// 	NULL on error
conn_info_t* reactor_add_connector(
	reactor_t*		p_reactor, 
	int32_t 			protocol, 		// SOCK_TCP or SOCK_UDP
	const char*			host, 			// c style string. e.g. "192.168.8.8\0"
	uint16_t 			port, 			// host byte ordered
	msg_factory_t*	p_msg_factory,
	msg_processor_t*	p_msg_processor) {
	int32_t ret;
	char ipstr[MAX_IP];
	socket_t* p_socket = create_socket(protocol);
	if(p_socket == NULL) {
		return NULL;
	}
	p_socket->msg_factory = *p_msg_factory;
	p_socket->msg_processor = *p_msg_processor;
	p_socket->p_send_queue = fixed_queue_create(sizeof(socket_buf_t), SND_QUEUE_SIZE);
	if(p_socket->p_send_queue == NULL) {
		free_socket(p_socket);
		return NULL;
	}
	ret = socket_buf_init(&p_socket->recv_buf, RECVBUF_INIT_SIZE);
	if(ret < 0) {
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}
	STRING_ASSIGN(p_socket->host_str, p_socket->host_str_len, host, strlen(host), ret);
	if (ret != 0){
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}
	p_socket->peerport = port;

	ret = query_addr_async(&p_reactor->dns_cache, host, ipstr, 20);
	if (ret == 0) {
		ret = socket_connect(p_socket, ipstr, port);
		if (ret < 0) {
			free_socket(p_socket);
			p_reactor->errcode = ret;
			return NULL;
		}
	}
	else if (ret == EAGAIN) {
		p_reactor->wait_dns_sock_count++;
		p_socket->stat = SOCK_STAT_DNSPARSING;
	}
	else {
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}

	ret = reactor_register(p_reactor, p_socket);
	if(ret < 0) {
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}
	
	return p_socket;
}

// @return :
// 	valid connection info pointer on success, 
// 	NULL on error
conn_info_t* reactor_add_acceptor(
	reactor_t*		p_reactor,
	int32_t 			protocol,
	const char*			ip,
	uint16_t 			port,
	msg_factory_t*	p_msg_factory,
	msg_processor_t*	p_msg_processor) {
	int32_t ret;
	socket_t* p_socket = create_socket(protocol);
	if(p_socket == NULL) {
		return NULL;
	}
	p_socket->msg_factory = *p_msg_factory;
	p_socket->msg_processor = *p_msg_processor;

	ret = socket_bind(p_socket, ip, port);
	if(ret < 0) {
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}

	ret = socket_listen(p_socket, 10);
	if(ret < 0) {
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}

	ret = reactor_register(p_reactor, p_socket);
	if(ret < 0) {
		free_socket(p_socket);
		p_reactor->errcode = ret;
		return NULL;
	}

	return p_socket;
}

void reactor_handle_events(reactor_t *p_reactor, int32_t timeout /* ms */) {
	int32_t ret;
	int32_t idx = -1;
	int errcode;

	ret = selector_wait(p_reactor->p_selector, timeout);
	if(ret < 0) {
		fprintf(stderr, "FATAL ERROR in reactor %p, error=%d\n", p_reactor, ret);
		abort();
	}
	if (ret == 0) p_reactor->no_event_tick++;
	if (ret > 0 || p_reactor->no_event_tick > 10 || p_reactor->wait_dns_sock_count > 0) {
		handle_t* p_handle;
		socket_t* p_socket;
		int32_t sock_idx;
		p_reactor->no_event_tick = 0;
		for(;;) {
			int32_t has_event = 0;
			time_t now = time(NULL);

			ret = 0;
			idx = selector_next(p_reactor->p_selector, idx);
			if(idx == -1) { // end
				break;
			}

			sock_idx = (int32_t)selector_data(p_reactor->p_selector, idx);
			p_socket = p_reactor->pp_socket[sock_idx];

			if (p_socket->stat == SOCK_STAT_DNSPARSING) {
				char ipstr[MAX_IP];
				ret = query_addr_async(&p_reactor->dns_cache, p_socket->host_str, ipstr, 20);
				if (ret == 0) {
					p_socket->last_active_time = now;
					p_reactor->wait_dns_sock_count--;
					ret = socket_connect(p_socket, ipstr, p_socket->peerport);
					if (ret < 0) {
						socket_on_error(p_socket, ret);
						do_close_socket(p_socket);
					}
				}
				else if (ret == EAGAIN) {
					// do nothing
					// p_socket->stat = SOCK_STAT_DNSPARSING;
				}
				else {
					p_reactor->wait_dns_sock_count--;
					socket_on_error(p_socket, ret);
					do_close_socket(p_socket);
				}
				logger_assert(p_reactor->wait_dns_sock_count >= 0);
				continue;
			}

			p_handle = p_socket->p_handle;
			if(selector_iserror(p_reactor->p_selector, idx)) {
				errcode = handle_getsockerr(p_handle);
				socket_on_error(p_socket, errcode);
				do_close_socket(p_socket);
				continue;
			}
			else {

				if(selector_isreadable(p_reactor->p_selector, idx)) {
					has_event = 1;
					if (p_socket->stat != SOCK_STAT_CONNECTING)
						p_socket->last_active_time = now;
					ret = socket_on_readable(p_socket);
				}

				if (ret != SOCKCLOSED && selector_iswriteable(p_reactor->p_selector, idx)) {
					has_event = 1;
					if (p_socket->stat != SOCK_STAT_CONNECTING)
						p_socket->last_active_time = now;
					ret = socket_on_writable(p_socket);
				}

				if ( ret != SOCKCLOSED && !has_event && p_socket->stat != SOCK_STAT_LISTENING) {
					uintmax_t wait_time = (uintmax_t)now - (uintmax_t)p_socket->last_active_time;
					switch (p_socket->stat)
					{
					case SOCK_STAT_WORKING:
					case SOCK_STAT_CONNECTED:
						if (wait_time > (uintmax_t)p_socket->idle_timeout) {
							socket_on_error(p_socket, ESOCKIDLETIMEOUT);
							do_close_socket(p_socket);
						}
						break;
					case SOCK_STAT_DNSPARSING:
						if (wait_time > (uintmax_t)p_socket->dns_timeout) {
							socket_on_error(p_socket, ESOCKDNSTIMEOUT);
							do_close_socket(p_socket);
						}
						break;
					case SOCK_STAT_CONNECTING:
						if (wait_time > (uintmax_t)p_socket->connect_timeout) {
							socket_on_error(p_socket, ESOCKCONNECTTIMEOUT);
							do_close_socket(p_socket);
						}
						break;
					case SOCK_STAT_INIT:
					case SOCK_STAT_LISTENING:
					case SOCK_STAT_ERROR:
					case SOCK_STAT_CLOSING:
						break;
					}
				}
			}
		}
	}
}

int32_t reactor_get_last_err(reactor_t *p_reactor) {
	return p_reactor->errcode;
}

socket_t* reactor_find_conn_by_unique_tag(reactor_t *p_reactor, uint32_t unique_tag) {
	int32_t idx = -1;
	socket_t* p_socket = NULL;
	socket_t* p_socket_target = NULL;
	for (;;) {
		int32_t sock_idx;
		idx = selector_next(p_reactor->p_selector, idx);
		if (idx == -1) { // end
			break;
		}
		sock_idx = (int32_t)selector_data(p_reactor->p_selector, idx);
		p_socket = p_reactor->pp_socket[sock_idx];
		if (p_socket->unique_tag == unique_tag)
		{
			p_socket_target = p_socket;
			break;
		}
	}
	return p_socket_target;
}

