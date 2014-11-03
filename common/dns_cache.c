/**
# -*- coding:UTF-8 -*-
*/

#include "dns_cache.h" 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "define.h"
#include "string_utility.h"
#include "logger.h"

#define dns_copy_ipstr(dst, src) \
	do \
	{\
		int32_t __o;\
		for (__o = 0; __o < MAX_IP && (src)[__o]; ++__o) (dst)[__o] = (src)[__o];\
		(dst)[__o] = '\0'; \
	} while (0)

int32_t net_addr_initialize(NetAddr* self)
{
	self->host[0] = 0;
	self->ip[0] = 0;
	self->result = EAGAIN;
	return 0;
}

int32_t net_addr_finalize(void* self)
{
	return 0;
}
int32_t net_addr_set_host(NetAddr* self, const char* host)
{
	strncpy(self->host, host, sizeof(self->host));
	return 0;
}
int32_t net_addr_copy(NetAddr* self, const NetAddr* other)
{
	if(self == other) return 0;
	memcpy(self->host, other->host, MAX_HOST);
	dns_copy_ipstr(self->ip, other->ip);
	return 0;
}

int32_t net_addr_clone(NetAddr* self, const NetAddr* other)
{
	return net_addr_copy(self, other);
}
static inline int32_t net_addr_cmp_host(const void* l, const void* r)
{
	return strcmp(((NetAddr*)l)->host, ((NetAddr*)r)->host);
}

int32_t dns_cache_initialize(DnsCache* self, int32_t max_cache_number, uint32_t dns_expire_time)
{
	lru_array_initialize(&self->lru_cache, max_cache_number, sizeof(NetAddr), dns_expire_time, net_addr_finalize);
	return 0;
}

int32_t dns_cache_finalize(DnsCache* self)
{
	lru_array_finalize(&self->lru_cache);
	return 0;
}

struct s_host_info
{
	NetAddr* addr;
	volatile int32_t is_available;
};

static int32_t query_from_dns_server(struct s_host_info* host_info)
{
	NetAddr* addr = host_info->addr;
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	struct sockaddr_in  *sin_p = NULL;
	int32_t ret = 0;
	char ipstr[MAX_IP];

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; //IPv4
	hints.ai_socktype = 0; //any address type
	//hints.ai_flags = AI_CANONNAME;
	hints.ai_protocol = 0; //any protocol;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	ret = getaddrinfo(addr->host, NULL, &hints, &result);
	if (!host_info->is_available) goto L_END;

	if (0 != ret)
	{
		logger_error(default_logger, "getaddrinfo of %s fail:%d[%s], errno:%d[%s], result: %p",
			addr->host, ret, gai_strerror(ret), errno, strerror(errno), result);
		freeaddrinfo(result);
		addr->result = ret;
		return ret;
	}

	sin_p = (struct sockaddr_in *)result->ai_addr;
	const char* p = inet_ntop(AF_INET, &sin_p->sin_addr, ipstr, MAX_IP);
	if (NULL == p)
	{
		logger_debug(default_logger, "inet_ntop fail:%s", strerror(errno));
	}
	else
	{
		dns_copy_ipstr(addr->ip, p);
	}
	addr->result = ret;
L_END:
	free(host_info);
	freeaddrinfo(result);
	return ret;
}

const char* query_addr(DnsCache* self, const char* host, char* ip)
{
	int32_t ret;
	NetAddr addr;
	NetAddr* paddr;
	net_addr_initialize(&addr);
	net_addr_set_host(&addr, host);
	paddr = lru_array_get(&self->lru_cache, (void*)&addr, net_addr_cmp_host);
	if (paddr)
	{
		// 同步查询时可能已经有一个异步查询将该host加入lru，并且有可能没有完成
		while (EAGAIN == paddr->result) usleep(1000); // 1ms
		ret = paddr->result;
		if (0 != ret)
		{
			struct s_host_info* host_info = (struct s_host_info*)malloc(sizeof*host_info);
			host_info->addr = paddr;
			host_info->is_available = 1;
			ret = query_from_dns_server(host_info);
			if (0 == ret) dns_copy_ipstr(ip, paddr->ip);
		}
	}
	else
	{
		// lru须保证存储地址不变，以及有效时间内不可被替换
		paddr = (NetAddr*)lru_array_put(&self->lru_cache, (void*)&addr, net_addr_cmp_host);
		if (NULL == paddr)
		{
			struct s_host_info* host_info = (struct s_host_info*)malloc(sizeof*host_info);
			host_info->addr = &addr;
			host_info->is_available = 1;
			ret = query_from_dns_server(host_info);
			if (0 == ret) dns_copy_ipstr(ip, addr.ip);
		}
		else
		{
			struct s_host_info* host_info = (struct s_host_info*)malloc(sizeof*host_info);
			host_info->addr = paddr;
			host_info->is_available = 1;
			ret = query_from_dns_server(host_info);
			if (0 == ret) dns_copy_ipstr(ip, paddr->ip);
		}
	}
	
	net_addr_finalize(&addr);
	if (0 == ret)
	{
		return ip;
	}
	else
	{
		return NULL;
	}
}

static void* query_dns_async_thread(void* args)
{
	struct s_host_info* p_host_info = (struct s_host_info*)args;
	query_from_dns_server(p_host_info);
	return NULL;
}

static int32_t query_from_dns_server_async(NetAddr* addr, int32_t in_cache, char* ip_out, uint32_t wait_timeout)
{
	pthread_t tid;
	pthread_attr_t attr;
	uint32_t wait_time = 1;
	struct s_host_info* host_info = (struct s_host_info*)malloc(sizeof*host_info);

	host_info->addr = addr;
	host_info->is_available = 1;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 64 * 1024);
	pthread_create(&tid, &attr, query_dns_async_thread, host_info);
	pthread_attr_destroy(&attr);
	for (; wait_time <= wait_timeout; ++wait_time)
	{
		usleep(1 * 1000); //wait 1ms
		if (addr->result != EAGAIN) //query dns ok
		{
			logger_info(default_logger, "query dns %s[%s] return:%d, cost time:%ums",
				addr->host, addr->ip, addr->result, wait_time);
			dns_copy_ipstr(ip_out, addr->ip);
			break;
		}
		if (!in_cache) host_info->is_available = 0;
	}
	return addr->result;
}

int32_t query_addr_async(DnsCache* self, const char* host, char* ip, uint32_t wait_timeout)
{
	int32_t ret;
	NetAddr addr;
	NetAddr* paddr;
	net_addr_initialize(&addr);
	net_addr_set_host(&addr, host);
	ip[0] = '\0';
	paddr = lru_array_get(&self->lru_cache, (void*)&addr, net_addr_cmp_host);
	if (paddr)
	{
		if (paddr->result == 0)
		{
			dns_copy_ipstr(ip, paddr->ip);
			logger_info(default_logger, "get host[%s]'s ip[%s] from cache.", host, ip);
			return 0;
		}
		else if (paddr->result == EAGAIN) return EAGAIN;
		else
		{
			logger_info(default_logger, "detect host[%s] in cache error:%d, try query from server.", host, paddr->result);
			ret = query_from_dns_server_async(paddr, 1, ip, wait_timeout);
		}
	}
	else
	{
		// lru须保证存储地址不变，以及有效时间内不可被替换
		paddr = (NetAddr*)lru_array_put(&self->lru_cache, (void*)&addr, net_addr_cmp_host);
		if (NULL == paddr)
		{
			ret = query_from_dns_server_async(&addr, 0, ip, wait_timeout);
		}
		else
		{
			ret = query_from_dns_server_async(paddr, 1, ip, wait_timeout);
		}
	}
	net_addr_finalize(&addr);
	// 多线程访问可能导致信息不一致 —— result被赋值为0，ip的赋值可能该线程还未得到结果
	if (ret == 0 && ip[0] == '\0') ret = EAGAIN;
	return ret;
}

#if 0
int main()
{
	int32_t ret;
	char* host;
	char* ip;
	char buf[MAX_IP] = {0};
	DnsCache dns_cache;
	dns_cache_initialize(&dns_cache, 6, 1*60*60);
	host = "baidu.com";
	ip = (char*)query_addr(&dns_cache, host, buf);
	printf("%s : %s\n", host, ip);
	int i = 0;
	while (i++ < 10)
	{
		host = "map.baidu.com";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
			ret = query_addr_async(&dns_cache, host, buf, 10);
			printf("wait dns:%s\n", host);
			usleep(1 * 1000);
		}*/
		if (ret == 0)printf("%s : %s\n", host, buf);

		host = "baidu.com";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
			printf("wait dns:%s\n", host);
			usleep(1 * 1000);
			ret = query_addr_async(&dns_cache, host, buf, 10);
		}*/
		if (ret == 0)printf("%s : %s\n", host, buf);
		//==========================================
		host = "map.qq.com";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
			ret = query_addr_async(&dns_cache, host, buf, 10);
			printf("wait dns:%s\n", host);
			usleep(1 * 1000);
		}*/
		if (ret == 0)printf("%s : %s\n", host, buf);

		host = "qq.com";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
			printf("wait dns:%s\n", host);
			usleep(1 * 1000);
			ret = query_addr_async(&dns_cache, host, buf, 10);
		}*/
		if (ret == 0)printf("%s : %s\n", host, buf);
		//==========================================
		host = "163.com";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
			ret = query_addr_async(&dns_cache, host, buf, 10);
			printf("wait dns:%s\n", host);
			usleep(1 * 1000);
		}*/
		if (ret == 0)printf("%s : %s\n", host, buf);

		host = "www.google.de";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
			printf("wait dns:%s\n", host);
			usleep(1 * 1000);
			ret = query_addr_async(&dns_cache, host, buf, 10);
		}*/
		if(ret==0)printf("%s : %s\n", host, buf);

		host = "www.google.com";
		ret = query_addr_async(&dns_cache, host, buf, 10);
		/*while (ret == EAGAIN)
		{
		printf("wait dns:%s\n", host);
		usleep(1 * 1000);
		ret = query_addr_async(&dns_cache, host, buf, 10);
		}*/
		if (ret == 0)printf("%s : %s\n", host, buf);
	}
	dns_cache_finalize(&dns_cache);
	return 0;
}
#endif
