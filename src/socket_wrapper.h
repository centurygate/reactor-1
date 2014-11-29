#ifndef _SOCKET_20130716
#define _SOCKET_20130716

#include <time.h>
#include "handle.h"
#include "msg_factory.h"
#include "msg_processor.h"
#include "fixed_queue.h"
#include "dynamic_array.h"

#define SND_QUEUE_SIZE 20
#define RECVBUF_INIT_SIZE 1024
#define INADDR_ANY_STR "0.0.0.0"
#define INADDR_LOCAL_STR "127.0.0.1"
#define SOCKET_DEFAULT_IDLE_TIMEOUT 300 //seconds
#define SOCKET_DEFAULT_DNS_TIMEOUT 120
#define SOCKET_DEFAULT_CONNECT_TIMEOUT 30

typedef enum {
	SOCK_STAT_INIT, 
	SOCK_STAT_LISTENING,
	SOCK_STAT_DNSPARSING, 
	SOCK_STAT_CONNECTING,
	SOCK_STAT_CONNECTED,
	SOCK_STAT_WORKING,
	SOCK_STAT_ERROR,
	SOCK_STAT_CLOSING
} socket_stat_t;

typedef struct socket_buf {
	uint32_t			data_size;
	uint32_t			buf_size;
	char*				p_buf;
} socket_buf_t;

int32_t socket_buf_init(socket_buf_t* p_buf, uint32_t size);
void socket_buf_finalize(socket_buf_t* p_buf);
int32_t socket_buf_resize(socket_buf_t* p_buf, uint32_t new_size);

struct reactor;
typedef struct socket_wrapper {
	int32_t 			id;
	struct reactor*	p_reactor;
	socket_stat_t 	stat;
	int 				protocol;
	int32_t 			errcode;
	char*				host_str;
	uint32_t			host_str_len;
	uint32_t 			peerip; 			// network byte ordered
	uint16_t 			peerport; 			// host byte ordered 
	handle_t*		p_handle;
	msg_processor_t	msg_processor;
	msg_factory_t	msg_factory;
	fixed_queue_t*	p_send_queue;		// elements' type: socket_buf_t
	socket_buf_t		recv_buf;
	time_t				last_active_time;
	time_t				idle_timeout;
	time_t				dns_timeout;
	time_t				connect_timeout;
	uint32_t			unique_tag;
	uint32_t			sent_bytes;
	uint32_t			recv_bytes;
} socket_t;

typedef socket_t conn_info_t;

socket_t *create_socket(int protocol);

// try to close socket_wrapper, action may take effect later.
// @Note: this function will set idle timeout to 3 sec.
void close_socket(socket_t *p_socket);

// only free resources related to socket_wrapper, no callback;
void free_socket(socket_t *p_socket);

// callback first, then free resources related to socket_wrapper;
void do_close_socket(socket_t *p_socket);

int32_t socket_bind(socket_t *p_socket, 
		const char *addr, 	// host in string format. e.g "0.0.0.0" 
		uint16_t port); 	// host byte ordered

int32_t socket_listen(socket_t *p_socket, int32_t backlog); 

int32_t socket_connect(socket_t *p_socket,        
		const char *ipstr, uint16_t port /* host byte ordered */);

enum e_socket_send_flag {
	SOCKET_COPY_MSG_BUFFER,   // -caller have to manage msg buffer itself
	SOCKET_DETACH_MSG_BUFFER, // -msg buffer will be freed when sent
};
int32_t socket_send(socket_t* p_socket, char *buffer, int32_t size, enum e_socket_send_flag flag);

int32_t socket_on_readable(socket_t* p_socket);

int32_t socket_on_writable(socket_t* p_socket);

void socket_on_error(socket_t* p_socket, int32_t errcode);

void socket_set_idle_timeout(socket_t* p_socket, time_t timeout);

int32_t conn_send(conn_info_t* p_conn_info, char *msg, uint32_t msg_len, enum e_socket_send_flag flag);

void conn_close(conn_info_t* p_conn_info);

// @return:
//  1 on connected
//  0 on other status. e.g. init, connecting, error...
int32_t conn_is_connected(conn_info_t* p_conn_info); 

// @return: ipv4 in network byte ordered
uint32_t conn_get_peerip(conn_info_t* p_conn_info);

// @return: port in host byte ordered
uint16_t conn_get_peerport(conn_info_t* p_conn_info);

// @return: peer's host domain name
const char* conn_get_peerhost(conn_info_t* p_conn_info);

void conn_set_dns_timeout(conn_info_t* p_conn_info, time_t timeout /* second */);
void conn_set_connect_timeout(conn_info_t* p_conn_info, time_t timeout /* second */);
void conn_set_idle_timeout(conn_info_t* p_conn_info, time_t timeout /* second */);

// @return:
// -1 for invalid
uint32_t conn_get_unique_tag(conn_info_t* p_conn_info);

// send data immediately.
// @return: 
//  positive number for sent bytes
//  negative number on error
int32_t conn_flush(conn_info_t* p_conn_info);

#endif //_SOCKET_20130716
