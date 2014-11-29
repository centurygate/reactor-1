#ifdef LINUX

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "handle.h"

handle_t* create_file_handle(const char *file_path, int mode) {
	///TODO: implement
	return NULL;
}

handle_t* create_sock_handle(int protocol) {
	handle_t *p_handle = NULL;
	int sock_type;
	
	p_handle = (handle_t*)malloc(sizeof(*p_handle));

	if(p_handle == NULL) return NULL;

	memset(p_handle, 0, sizeof(*p_handle));
	p_handle->id = -1;
	if(protocol == EM_SOCK_TCP) {
		sock_type = SOCK_STREAM;
		p_handle->type = EM_SOCK_TCP_HANDLE;
	}
	else if(protocol == EM_SOCK_UDP) {
		sock_type = SOCK_DGRAM;
		p_handle->type = EM_SOCK_UDP_HANDLE;
	}
	else {
		return NULL;
	}
	p_handle->os_handle.fd = socket(AF_INET, sock_type, 0);
	if(p_handle->os_handle.fd == -1) {
		return NULL;
	}

	return p_handle;
}

void close_handle(handle_t *p_handle) {
	close(p_handle->os_handle.fd);
	free(p_handle);
}

int32_t handle_read(handle_t *p_handle, char *buffer, int32_t expected_len) {
	return  0;
}

int32_t handle_recv(handle_t *p_handle, char *buffer, int32_t expected_len) {
	int ret;
	do {
		ret = recv(p_handle->os_handle.fd, buffer, expected_len, 0);
	} while(ret < 0 && errno == EINTR);
	
	if(ret == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return EHANDLESHOULDRETRY;
		}
		return -errno;
	}

	return ret;
}

int32_t handle_recvfrom (struct handle *p_handle, char *buffer, int32_t expected_len,
		uint32_t *p_ip, /* network byte ordered */                         
		uint16_t *p_port /* network byte ordered */) {
	struct sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(peer_addr);
	int ret;

	memset(&peer_addr, 0, sizeof(peer_addr));
	do {
		ret = recvfrom(p_handle->os_handle.fd, buffer, expected_len, 0,
				(struct sockaddr*)&peer_addr, &addr_len);
	} while (ret < 0 && errno == EINTR);

	if(ret > 0) {
		if(p_ip) {
			*p_ip = peer_addr.sin_addr.s_addr;
		}
		if(p_port) {
			*p_port = peer_addr.sin_port;
		}
		return ret;
	}

	return -errno;
}

int32_t handle_write(handle_t *p_handle, const char *buffer, int32_t expected_len) {
	return 0;
}

int32_t handle_send(handle_t *p_handle, const char *buffer, int32_t expected_len) {
	int ret;

	do {
		ret = send(p_handle->os_handle.fd, buffer, expected_len, 0);
	} while(ret < 0 && errno == EINTR);

	if(ret > 0) {
		return ret;
	}
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
		return EHANDLESHOULDRETRY;
	}
	return -errno;
}

int32_t handle_sendto(handle_t *p_handle, const char *buffer, int32_t expected_len,
		uint32_t ip, /* network byte ordered */    
		uint16_t port /* network byte ordered */) {
	int ret;
	int addr_len;
	struct sockaddr_in peer_addr;

	memset(&peer_addr, 0, sizeof(peer_addr));
	addr_len = sizeof(peer_addr);
	
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = port;
	peer_addr.sin_addr.s_addr = ip;
	
	do {
		ret = sendto(p_handle->os_handle.fd, buffer, expected_len, 0,
				(const struct sockaddr*)&peer_addr, addr_len);
	} while(ret < 0 && errno == EINTR);

	if(ret > 0) {
		return ret;
	}
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
		return EHANDLESHOULDRETRY;
	}
	return -errno;
}

//int32_t handle_set_blocking(struct handle *p_handle) {
//	int flags;
//	flags = fcntl(p_handle->os_handle.fd, F_GETFL, 0);
//	if (flags < 0) {
//		return -errno;
//	}
//
//	if (fcntl(p_handle->os_handle.fd, F_SETFL, flags & (~O_NONBLOCK)) < 0) {
//		return -errno;
//	}
//	return 0;
//}

int32_t handle_set_nonblocking(struct handle *p_handle) {
	int flags;
	flags = fcntl(p_handle->os_handle.fd, F_GETFL, 0);
	if(flags < 0) {
		return -errno;
	}

	if(fcntl(p_handle->os_handle.fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		return -errno;
	}
	return 0;
}

int32_t handle_set_reuseaddr(struct handle *p_handle) {
	int flag = 1;
	int len = sizeof(flag);
	int ret;

	ret = setsockopt(p_handle->os_handle.fd, SOL_SOCKET, SO_REUSEADDR, &flag, len);
	if(ret == -1) {
		return -errno;
	}

	return 0;
}

int32_t handle_bind(handle_t *p_handle, uint32_t ip, uint16_t port) {
	int ret;
	int addr_len;
	struct sockaddr_in peer_addr;

	memset(&peer_addr, 0, sizeof(peer_addr));
	addr_len = sizeof(peer_addr);
	
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = port;
	peer_addr.sin_addr.s_addr = ip;

	ret = bind(p_handle->os_handle.fd, (const struct sockaddr*)&peer_addr, addr_len); 
	if(ret == -1) {
		return -errno;
	}
	return 0;
}

int32_t handle_listen(handle_t *p_handle, int32_t backlog) {
	int ret;

	ret = listen(p_handle->os_handle.fd, backlog);

	if(ret == -1) {
		return -errno;
	}

	return 0;
}

int32_t handle_getpeername(handle_t *p_handle, uint32_t *p_ip, uint16_t *p_port) {
	int ret;
	struct sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(peer_addr);

	ret = getpeername(p_handle->os_handle.fd, (struct sockaddr*)&peer_addr, &addr_len);
	if(ret < 0) {
		return -errno;
	}
	if(p_ip) *p_ip = peer_addr.sin_addr.s_addr;
	if(p_port) *p_port = peer_addr.sin_port;

	return 0;
}

handle_t *handle_accept(handle_t *p_handle, uint32_t *p_ip, uint16_t *p_port) {
	int ret;
	struct sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(peer_addr);            
	handle_t *p_new_handle = (handle_t*)malloc(sizeof(*p_new_handle));
	if(p_new_handle == NULL) {
		return NULL;
	}
	memset(&peer_addr, 0, sizeof(peer_addr));
	memset(p_new_handle, 0, sizeof(*p_new_handle));
	p_new_handle->type = EM_SOCK_TCP_HANDLE;
	do {
		ret = accept(p_handle->os_handle.fd, (struct sockaddr*)&peer_addr, &addr_len);
	} while(ret < 0 && errno == EINTR);
	if(ret == -1) {
		free(p_new_handle);
		return NULL;
	}
	p_new_handle->os_handle.fd = ret;

	if(p_ip) *p_ip = peer_addr.sin_addr.s_addr;
	if(p_port) *p_port = peer_addr.sin_port;

	return p_new_handle;
}

int32_t handle_connect(handle_t *p_handle, uint32_t ip, uint16_t port) {
	int ret;
	int addr_len;
	struct sockaddr_in peer_addr;

	memset(&peer_addr, 0, sizeof(peer_addr));
	addr_len = sizeof(peer_addr);
	
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = port;
	peer_addr.sin_addr.s_addr = ip;

	do {
		ret = connect(p_handle->os_handle.fd, (const struct sockaddr*)&peer_addr, addr_len);
	} while(ret < 0 && errno == EINTR);
	if(ret == -1) {
		return -errno;
	}
	return 0;
}

int32_t handle_getsockerr(handle_t *p_handle) {
	int error = 0;
	socklen_t len = sizeof(error);
	int ret = getsockopt(p_handle->os_handle.fd, SOL_SOCKET, SO_ERROR, (void *) &error, &len);

	if(ret == -1) {
		return -errno;
	}

	return -error;
}

int32_t handle_settcpnodelay(handle_t *p_handle) {
	const int on = 1;
	int ret = setsockopt(p_handle->os_handle.fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
	if(ret == -1) {
		return -errno;
	}
	return 0;
}

#endif //LINUX

