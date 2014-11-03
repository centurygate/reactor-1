/**
# -*- coding:UTF-8 -*-
*/

#ifndef _DNS_CACHE_H_
#define _DNS_CACHE_H_

#include <time.h>
#include "define.h"
#include "lru_array.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define MAX_HOST 64
#define MAX_IPV4 16
#define MAX_IPV6 40
#define MAX_IP MAX_IPV6

typedef struct s_net_addr_t
{
	char host[MAX_HOST];
	volatile char ip[MAX_IP];
	volatile int32_t result;
} NetAddr;

typedef struct s_dns_cache_t
{
	lru_array_t lru_cache;
} DnsCache;

/**
 *初始化DNS缓存大小为max_cache_number个, dns过期时间为dns_expire_time秒
 *如果缓存满，移除最旧的域名
 *内部采用遍历方式查询，建议max_cache_number<40
 */
int32_t dns_cache_initialize(DnsCache* self, int32_t max_cache_number, uint32_t dns_expire_time);
int32_t dns_cache_finalize(DnsCache* self);

/**
 *同步查询dns
 *阻塞，直至获得结果或dns查询出错
 */
const char* query_addr(DnsCache* self, const char* host, char* ip);

/**
 *异步查询dns
 *如果超过wait_timeout毫秒还没有结果，返回EAGAIN
 *调用者重复调用该接口，直至其返回值不等于EAGAIN
 *在缓存满时, 如无缓存项过期，重复查询响应时间可能变慢
 */
int32_t query_addr_async(DnsCache* self, const char* host, char* ip, uint32_t wait_timeout);

#ifdef __cplusplus
}
#endif

#endif
