#ifndef _REACTOR_20130716
#define _REACTOR_20130716
#include "define.h"
#include "selector.h"
#include "socket_wrapper.h"
#include "msg_factory.h"
#include "msg_processor.h"
#include "dns_cache.h"

typedef struct reactor {
	int32_t 		max_socket_num;
	int32_t 		free_sock_idx;
	int32_t			errcode;
	selector_t*	p_selector;
	socket_t**	pp_socket;
	uint32_t		no_event_tick;
	DnsCache		dns_cache;
	int32_t			wait_dns_sock_count;
	uint32_t		socket_unique_tag_pool;
} reactor_t;

reactor_t* reactor_create(int32_t max_socket_num);

void reactor_destroy(reactor_t *p_reactor);

int32_t reactor_register(reactor_t *p_reactor, socket_t *p_socket);

void reactor_deregister(reactor_t *p_reactor, socket_t *p_socket);

// @return :
// valid connection info pointer on success, 
// NULL on error
conn_info_t* reactor_add_connector(
	reactor_t*		p_reactor, 
	int32_t 			protocol, 		// SOCK_TCP or SOCK_UDP
	const char*			host, 			// c style string. e.g. "192.168.8.8\0"
	uint16_t 			port, 			// host byte ordered
	msg_factory_t*	p_msg_factory,
	msg_processor_t*	p_msg_processor);

// @return :
//	valid connection info pointer on success, 
//	NULL on error
conn_info_t* reactor_add_acceptor(
	reactor_t*		p_reactor,
	int32_t 			protocol, 		// SOCK_TCP or SOCK_UDP
	const char*			ip, 			// c style string. ip string on interface. e.g. "192.168.8.8\0"
										// pre-defined macros:
										// 		INADDR_ANY_STR for any interface
										// 		INADDR_LOCAL_STR for loopback interface only

	uint16_t 			port, 			// host byte ordered
	msg_factory_t*	p_msg_factory,
	msg_processor_t*	p_msg_processor);

void reactor_handle_events(reactor_t *p_reactor, int32_t timeout /* ms */);

int32_t reactor_get_last_err(reactor_t *p_reactor);

socket_t* reactor_find_conn_by_unique_tag(reactor_t *p_reactor, uint32_t unique_tag);

#endif //_REACTOR_20130716
