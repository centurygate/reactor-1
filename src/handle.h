#ifndef HANDLE_20130716
#define HANDLE_20130716

#include "define.h"

#define SOCK_TCP 0x1
#define SOCK_UDP 0x2

typedef enum {
	FILE_HANDLE, 
	SOCK_TCP_HANDLE,
	SOCK_UDP_HANDLE
} handle_type;

typedef union {
	int fd;
	void *p_handle;
} os_handle_t;

typedef struct handle {
	int32_t			id;
	handle_type 	type;
	os_handle_t 	os_handle;
} handle_t;

handle_t* create_file_handle(const char *file_path, int mode);
handle_t* create_sock_handle(int protocol);

void close_handle(handle_t *p_handle);

int32_t handle_read(handle_t *p_handle, char *buffer, int32_t expected_len);

int32_t handle_recv(handle_t *p_handle, char *buffer, int32_t expected_len);

int32_t handle_recvfrom (struct handle *p_handle, char *buffer, 
		int32_t expected_len,
		uint32_t *p_ip, /* network byte ordered */                         
		uint16_t *p_port /* network byte ordered */);

int32_t handle_write(handle_t *p_handle, const char *buffer, int32_t expected_len);
int32_t handle_send(handle_t *p_handle, const char *buffer, int32_t expected_len);
int32_t handle_sendto(handle_t *p_handle, const char *buffer, int32_t expected_len,
		uint32_t ip, /* network byte ordered */    
		uint16_t port /* network byte ordered */);
int32_t handle_set_blocking(struct handle *p_handle);
int32_t handle_set_nonblocking(struct handle *p_handle);
int32_t handle_set_reuseaddr(struct handle *p_handle);
int32_t handle_getpeername(handle_t *p_handle, uint32_t *p_ip, uint16_t *p_port);
int32_t handle_bind(handle_t *p_handle, uint32_t ip, uint16_t port);
int32_t handle_listen(handle_t *p_handle, int32_t backlog);
handle_t *handle_accept(handle_t *p_handle, uint32_t *p_ip, uint16_t *p_port);
int32_t handle_connect(handle_t *p_handle, uint32_t ip, uint16_t port);
int32_t handle_getsockerr(handle_t *p_handle);
int32_t handle_settcpnodelay(handle_t *p_handle);

//int32_t handle_read(handle_t *p_handle, char *buffer, int32_t expected_len);
//int32_t handle_write(handle_t *p_handle, const char *buffer, int32_t expected_len);
//int32_t handle_set_nonblocking(struct handle *p_handle);

#endif //HANDLE_20130716
