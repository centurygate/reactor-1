#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "string_utility.h"
#include "reactor.h"
#include "socket_wrapper.h"
#include "error_code.h"
#include "fixed_queue.h"
#include "logger.h"

int32_t socket_buf_init(socket_buf_t* p_buf, uint32_t size) {
	p_buf->data_size = 0;
	p_buf->buf_size = size;
	p_buf->p_buf = (char*)malloc(size);
	if(p_buf->p_buf == NULL) {
		return EMEMOUT;
	}
	return 0;
}

void socket_buf_finalize(socket_buf_t* p_buf) {
	if(p_buf->p_buf) {
		free(p_buf->p_buf);
	}
}

int32_t socket_buf_resize(socket_buf_t* p_buf, uint32_t new_size) {
	void* p = malloc(new_size);
	if(p == NULL) {
		return EMEMOUT;
	}
	memcpy(p, p_buf->p_buf, 
		p_buf->data_size < new_size ? p_buf->data_size : new_size);
	free(p_buf->p_buf);
	p_buf->buf_size = new_size;
	p_buf->p_buf = (char*)p;
	return 0;
}

void free_socket(socket_t *p_socket) {
	if (p_socket->p_send_queue) {
		while (!fixed_queue_isempty(p_socket->p_send_queue)) {
			socket_buf_t send_buf = *(socket_buf_t*)fixed_queue_out(p_socket->p_send_queue);
			socket_buf_finalize(&send_buf);
		}
	}

	if (p_socket->p_send_queue) {
		fixed_queue_destory(p_socket->p_send_queue);
	}

	socket_buf_finalize(&p_socket->recv_buf);

	if (p_socket->p_reactor) {
		reactor_deregister(p_socket->p_reactor, p_socket);
	}

	if (p_socket->p_handle) {
		close_handle(p_socket->p_handle);
	}
	STRING_FINALIZE(p_socket->host_str, p_socket->host_str_len);
	free(p_socket);
}

void do_close_socket(socket_t *p_socket) {
	if (p_socket->msg_processor.close_handler) {
		p_socket->msg_processor.close_handler(p_socket);
	}
	logger_info(default_logger, "socket_wrapper=%p, totally sent %u bytes, recv %u bytes",
		p_socket, p_socket->sent_bytes, p_socket->recv_bytes);
	free_socket(p_socket);
}

static int32_t _socket_do_accept(socket_t *p_socket) {
	int ret;
	handle_t *p_handle_new;
	handle_t *p_handle = p_socket->p_handle;
	socket_t *p_socket_new;
	uint16_t peerport;
	logger_debug(default_logger, "try to accept conn on sock %p", p_socket);
	p_socket_new = (socket_t*)malloc(sizeof(*p_socket_new));
	if(p_socket_new ==  NULL) {
		p_socket->errcode = -errno;
		return -errno;
	}
	memset(p_socket_new, 0, sizeof(*p_socket_new));
	p_socket_new->unique_tag = -1;
	p_handle_new = handle_accept(p_handle, 
			&p_socket_new->peerip,
			&peerport);
	if(p_handle_new == NULL) {
		free(p_socket_new);
		p_socket->errcode = -errno;
		return -errno;
	}
	
	handle_set_nonblocking(p_handle_new);
	
	p_socket_new->stat = EM_SOCK_STAT_CONNECTED;
	p_socket_new->protocol = p_socket->protocol;
	p_socket_new->p_handle = p_handle_new;
	p_socket_new->msg_factory = p_socket->msg_factory;
	p_socket_new->msg_processor = p_socket->msg_processor;
	p_socket_new->p_reactor = p_socket->p_reactor;
	p_socket_new->last_active_time = time(NULL);
	p_socket_new->idle_timeout = p_socket->idle_timeout;
	p_socket_new->peerport = be16toh(peerport);

	if(p_socket->protocol == EM_SOCK_TCP) {
		handle_settcpnodelay(p_handle);
	}

	p_socket_new->p_send_queue = fixed_queue_create(sizeof(socket_buf_t), SND_QUEUE_SIZE);
	if(p_socket_new->p_send_queue == NULL) {
		close_handle(p_handle_new);
		free(p_socket_new);
		p_socket->errcode = -errno;
		return -errno;
	}
	ret = socket_buf_init(&p_socket_new->recv_buf, RECVBUF_INIT_SIZE);
	if(ret < 0) {
		fixed_queue_destory(p_socket_new->p_send_queue);
		close_handle(p_handle_new);
		free(p_socket_new);
		p_socket->errcode = ret;
		return ret;
	}

	ret = reactor_register(p_socket->p_reactor, p_socket_new);
	if(ret < 0) {
		logger_error(default_logger, "register new connection to reactor failed[%d]", ret);
		socket_buf_finalize(&p_socket_new->recv_buf);
		fixed_queue_destory(p_socket_new->p_send_queue);
		close_handle(p_handle_new);
		free(p_socket_new);
		p_socket->errcode = ret;
		return ret;
	}
	logger_debug(default_logger, "accepted conn %p on sock %p", p_socket_new, p_socket);

	if (p_socket_new->msg_processor.connect_handler != NULL) {
		p_socket_new->msg_processor.connect_handler(p_socket_new);
	}
	return 0;
}

static int32_t _socket_do_recv(socket_t* p_socket) {
	int32_t ret;
	int32_t len;
	
	ret = handle_recv(p_socket->p_handle, 
		p_socket->recv_buf.p_buf + p_socket->recv_buf.data_size, 
		p_socket->recv_buf.buf_size - p_socket->recv_buf.data_size);
	if(ret < 0) {
		if (ret == EHANDLESHOULDRETRY) {
			return 0;
		}
		return ret;
	}
	p_socket->recv_bytes += ret;
	if(ret == 0) {
		logger_debug(default_logger, "socket_wrapper=%p closed by peer!!!", p_socket);
		do_close_socket(p_socket);
		return SOCKCLOSED;
	}
	p_socket->recv_buf.data_size += ret;
label_try_to_handle_msg:
	ret = p_socket->msg_factory.validator(p_socket->recv_buf.p_buf, p_socket->recv_buf.data_size);
	if(ret < 0) {
		p_socket->msg_processor.error_handler(p_socket, ret);
		p_socket->recv_buf.data_size = 0; // clear data
		return 0;
	}
	len = ret;
	if(len <= p_socket->recv_buf.data_size) {
		if(p_socket->msg_factory.decode) {
			char *msg_decoded = NULL;
			ret = p_socket->msg_factory.decode(p_socket->recv_buf.p_buf, ret, &msg_decoded);
			if(ret < 0) {
				return ret;
			}
			p_socket->msg_processor.msg_handler(p_socket, msg_decoded, ret);
			free(msg_decoded);
		}
		else {
			p_socket->msg_processor.msg_handler(p_socket, p_socket->recv_buf.p_buf, ret);
		}
		memmove(p_socket->recv_buf.p_buf, p_socket->recv_buf.p_buf + len, p_socket->recv_buf.data_size - len);
		p_socket->recv_buf.data_size -= len;

		if(p_socket->recv_buf.data_size > 0) {
			goto label_try_to_handle_msg;
		}
	}
	
	if(p_socket->stat == EM_SOCK_STAT_CLOSING) {
		if(	p_socket->p_send_queue == NULL ||
			fixed_queue_isempty(p_socket->p_send_queue)) {
			do_close_socket(p_socket);
			return SOCKCLOSED;
		}
		return 0;
	}

	if(len > p_socket->recv_buf.buf_size) {
		ret = socket_buf_resize(&p_socket->recv_buf, ret);
		if(ret < 0) {
			return ret;
		}
	}
	return 0;
}

int32_t socket_on_readable(socket_t* p_socket) {
	int ret;
	logger_assert(p_socket != NULL);

	switch(p_socket->stat) {
		case EM_SOCK_STAT_LISTENING:
			ret = _socket_do_accept(p_socket);
			if(ret < 0) {
				socket_on_error(p_socket, ret);
			}
			break;
		case EM_SOCK_STAT_CONNECTING:
			break;
		case EM_SOCK_STAT_CONNECTED:
			p_socket->stat = EM_SOCK_STAT_WORKING;
		case EM_SOCK_STAT_WORKING:
			ret = _socket_do_recv(p_socket);
			if (ret == SOCKCLOSED) {
				return SOCKCLOSED;
			}
			if(ret < 0) {
				socket_on_error(p_socket, ret);
				do_close_socket(p_socket);
				return SOCKCLOSED;
			}
			break;
		case EM_SOCK_STAT_ERROR:
			socket_on_error(p_socket, p_socket->errcode);
			do_close_socket(p_socket);
			return SOCKCLOSED;
			break;
		case EM_SOCK_STAT_CLOSING:
			if (fixed_queue_isempty(p_socket->p_send_queue)
				|| p_socket->errcode != 0)
			{
				do_close_socket(p_socket);
				return SOCKCLOSED;
			}
			break;
		default:
			logger_assert_false();
	}
	return 0;
}

static int32_t _socket_do_send(socket_t* p_socket) {
	int32_t ret;
	socket_buf_t* send_buf = NULL;
	
	send_buf = (socket_buf_t*)fixed_queue_head(p_socket->p_send_queue);
    if (send_buf == NULL)
    {
        return 0;
    }
    
	ret = handle_send(p_socket->p_handle, send_buf->p_buf, send_buf->data_size);
	if(ret < 0) {
		if (ret == EHANDLESHOULDRETRY) {
			return 0;
		}
		return ret;
	}
	else if(ret < send_buf->data_size) {
		memmove(send_buf->p_buf, send_buf->p_buf + ret, send_buf->data_size - ret);
		send_buf->data_size -= ret;
	}
	else if(ret == send_buf->data_size) {
		free(send_buf->p_buf);
		fixed_queue_out(p_socket->p_send_queue);
	}
	p_socket->sent_bytes += ret;
	return ret;
}

int32_t socket_on_writable(socket_t* p_socket) {
	int ret;
	logger_assert(p_socket != NULL);

	switch(p_socket->stat) {
		case EM_SOCK_STAT_LISTENING:
			// ignore
			break;
		case EM_SOCK_STAT_DNSPARSING:
			break;
		case EM_SOCK_STAT_CONNECTING:
			ret = handle_getsockerr(p_socket->p_handle);
			if(ret < 0) {
				socket_on_error(p_socket, ret);
				do_close_socket(p_socket);
				return SOCKCLOSED;
			}
			else {
				uint16_t peerport;
				handle_getpeername(p_socket->p_handle, &p_socket->peerip, &peerport);
				p_socket->peerport = be16toh(peerport);
				p_socket->stat = EM_SOCK_STAT_CONNECTED;
				if(p_socket->msg_processor.connect_handler) {
					p_socket->msg_processor.connect_handler(p_socket);
				}
			}
			break;
		case EM_SOCK_STAT_CONNECTED:
			p_socket->stat = EM_SOCK_STAT_WORKING;
		case EM_SOCK_STAT_WORKING:
			ret = _socket_do_send(p_socket);
			if(ret < 0) {
				socket_on_error(p_socket, ret);
				do_close_socket(p_socket);
				return SOCKCLOSED;
			}
			if(fixed_queue_isempty(p_socket->p_send_queue)) {
				return selector_mod(p_socket->p_reactor->p_selector, p_socket->p_handle, 
					EM_SELECTORIN, (void*)p_socket->id);
			}
			break;
		case EM_SOCK_STAT_ERROR:
			socket_on_error(p_socket, p_socket->errcode);
			do_close_socket(p_socket);
			return SOCKCLOSED;
			break;
		case EM_SOCK_STAT_CLOSING:
			if(!fixed_queue_isempty(p_socket->p_send_queue)) {
				ret = _socket_do_send(p_socket);
				if(ret < 0) {
					do_close_socket(p_socket);
					return SOCKCLOSED;
				}
			}
			else {
				do_close_socket(p_socket);
				return SOCKCLOSED;
			}
			break;
		default:
			logger_assert_false();
	}
	return 0;
}

void socket_on_error(socket_t* p_socket, int32_t errcode) {
	logger_assert(p_socket != NULL);

	p_socket->errcode = errcode;

	p_socket->stat = EM_SOCK_STAT_ERROR;

	if(p_socket->msg_processor.error_handler) {
		p_socket->msg_processor.error_handler(p_socket, errcode);
	}
}

socket_t *create_socket(int protocol) {
	socket_t *p_socket = NULL;
	handle_t *p_handle = NULL;

	p_handle = create_sock_handle(protocol);
	if(p_handle == NULL) {
		return NULL;
	}
	handle_set_nonblocking(p_handle);
	p_socket = (socket_t*)malloc(sizeof(*p_socket));
	if(p_socket == NULL) {
		logger_error(default_logger, "create socket_wrapper struct error");
		close_handle(p_handle);
		return NULL;
	}
	memset(p_socket, 0, sizeof(*p_socket));
	STRING_INITIALIZE(p_socket->host_str, p_socket->host_str_len);
	p_socket->stat = EM_SOCK_STAT_INIT;
	p_socket->protocol = protocol;
	p_socket->p_handle = p_handle;
	p_socket->last_active_time = time(NULL);
	p_socket->idle_timeout = (time_t)EM_SOCKET_DEFAULT_IDLE_TIMEOUT;
	p_socket->dns_timeout = (time_t)EM_SOCKET_DEFAULT_DNS_TIMEOUT;
	p_socket->connect_timeout = (time_t)EM_SOCKET_DEFAULT_CONNECT_TIMEOUT;
	p_socket->unique_tag = -1;

	if(protocol == EM_SOCK_TCP) {
		handle_settcpnodelay(p_handle);
	}
	return p_socket;
}

void close_socket(socket_t *p_socket) {
	p_socket->stat = EM_SOCK_STAT_CLOSING;
	socket_set_idle_timeout(p_socket, (time_t)3);
	if(NULL == p_socket->p_reactor) {
		do_close_socket(p_socket);
	}
}

int32_t socket_bind(socket_t *p_socket, 
		const char *addr, // host in string format. e.g. "0.0.0.0" 
		uint16_t port) { // host byte ordered
	int ret;
	uint32_t ip;
	ip = ipv4_aton(addr);

	ret = handle_set_reuseaddr(p_socket->p_handle);
	if(ret < 0) {
		p_socket->stat = EM_SOCK_STAT_ERROR;
		p_socket->errcode = ret;
		return ret;
	}

	return handle_bind(p_socket->p_handle, ip, htobe16(port));
}

int32_t socket_listen(socket_t *p_socket, int32_t backlog) {
	int ret;

	ret = handle_listen(p_socket->p_handle, backlog);
	if(ret < 0) {
		p_socket->stat = EM_SOCK_STAT_ERROR;
		p_socket->errcode = ret;
		return ret;
	}

	p_socket->stat = EM_SOCK_STAT_LISTENING;

	return 0;
}

int32_t socket_connect(socket_t *p_socket, const char *ipstr, uint16_t port /* host byte ordered */) {
	uint32_t ip;
	int ret;

	ip = ipv4_aton(ipstr);

	p_socket->peerip = ip;
	ret = handle_connect(p_socket->p_handle, ip, htobe16(port));
	if(ret < 0) {
		if(ret != -EINPROGRESS) {
			return ret;
		}
	}

	p_socket->stat = EM_SOCK_STAT_CONNECTING;
	
	return 0;
}

int32_t socket_send(socket_t *p_socket, char *buffer, int32_t size, enum e_socket_send_flag flag) {
	socket_buf_t send_buf = {0};
	int32_t ret;
	if(p_socket->msg_factory.encode) {
		char *out_buf;

		ret = p_socket->msg_factory.encode(buffer, size, &out_buf);
		if(ret < 0) {
			return ret;
		}

		if (flag == EM_SOCKET_DETACH_MSG_BUFFER) {
			free(buffer);
		}

		size = ret;
		send_buf.p_buf = out_buf;
	} else {
		if (flag == EM_SOCKET_DETACH_MSG_BUFFER) {
			send_buf.p_buf = buffer;
		} else {
			char *p = (char*)malloc(size);
			if(p == NULL) {
				return EMEMOUT;
			}
			memcpy(p, buffer, size);
			send_buf.p_buf = p;
		}
	}
	
	send_buf.buf_size = send_buf.data_size = size;

	ret = fixed_queue_in(p_socket->p_send_queue, &send_buf);
	if(ret < 0) {
		free(send_buf.p_buf);
		return ret;
	}
	return size;
}

void socket_set_idle_timeout(socket_t* p_socket, time_t timeout) {
	logger_debug(default_logger, "socket_wrapper[%p] timeout=%ju", p_socket, (uintmax_t)timeout);
	p_socket->idle_timeout = timeout;
}
void socket_set_dns_timeout(socket_t* p_socket, time_t timeout) {
	logger_debug(default_logger, "socket_wrapper[%p] timeout=%ju", p_socket, (uintmax_t)timeout);
	p_socket->dns_timeout = timeout;
}
void socket_set_connect_timeout(socket_t* p_socket, time_t timeout) {
	logger_debug(default_logger, "socket_wrapper[%p] timeout=%ju", p_socket, (uintmax_t)timeout);
	p_socket->connect_timeout = timeout;
}

int32_t conn_send(conn_info_t* p_conn_info, char *msg, uint32_t msg_len, enum e_socket_send_flag flag) {
	int ret, send_len;
	send_len = socket_send(p_conn_info, msg, msg_len, flag);
	if (send_len < 0) {
		return send_len;
	}
	ret = selector_mod(p_conn_info->p_reactor->p_selector, p_conn_info->p_handle,
		EM_SELECTORIN | EM_SELECTOROUT, (void*)p_conn_info->id);
	if (ret < 0) {
		return ret;
	}
	return send_len;
}

void conn_close(conn_info_t* p_conn_info) {
	selector_mod(p_conn_info->p_reactor->p_selector, p_conn_info->p_handle,
		EM_SELECTOROUT, (void*)p_conn_info->id);
	close_socket(p_conn_info);
}

int32_t conn_is_connected(conn_info_t* p_conn_info) {
	if (p_conn_info->stat == EM_SOCK_STAT_CONNECTED ||
		p_conn_info->stat == EM_SOCK_STAT_WORKING) {
		return 1;
	}

	return 0;
}

uint32_t conn_get_peerip(conn_info_t* p_conn_info) {
	return p_conn_info->peerip;
}

uint16_t conn_get_peerport(conn_info_t* p_conn_info) {
	return p_conn_info->peerport;
}

const char* conn_get_peerhost(conn_info_t* p_conn_info) {
	return p_conn_info->host_str;
}

void conn_set_idle_timeout(conn_info_t* p_conn_info, time_t timeout) {
	socket_set_idle_timeout(p_conn_info, timeout);
}
void conn_set_dns_timeout(conn_info_t* p_conn_info, time_t timeout) {
	socket_set_dns_timeout(p_conn_info, timeout);
}
void conn_set_connect_timeout(conn_info_t* p_conn_info, time_t timeout) {
	socket_set_connect_timeout(p_conn_info, timeout);
}

uint32_t conn_get_unique_tag(conn_info_t* p_conn_info)
{
	return p_conn_info->unique_tag;
}

int32_t conn_flush(conn_info_t* p_conn_info) {
	int32_t ret = 0;
	int32_t sent = 0;

	while (!fixed_queue_isempty(p_conn_info->p_send_queue)) {
		ret = _socket_do_send(p_conn_info);
		if (ret >= 0) {
			sent += ret;
		}
		else {
			return ret;
		}
	}

	return sent;
}

