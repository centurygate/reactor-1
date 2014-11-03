/**
# -*- coding:UTF-8 -*-
*/

#ifndef _HTTP_UTILITY_H_
#define _HTTP_UTILITY_H_

#include "string_utility.h"
#include "logger.h"

#include <stdlib.h>

#define MAX_QUERY_STRING_PARAMETER_NUMBER 37
#define MAX_HASH_SET_SIZE MAX_QUERY_STRING_PARAMETER_NUMBER

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct s_linear_probing_hash_set_t
{
	char* data[MAX_HASH_SET_SIZE];
	uint32_t data_len[MAX_HASH_SET_SIZE];
	uint32_t key[MAX_HASH_SET_SIZE];
	uint32_t size;
	//uint32_t capacity; //currently fixed size
}LinearProbingHashSet;

typedef struct s_http_query_string_parser_t
{
	LinearProbingHashSet hash_set;
	const char* cmd_name;
	uint32_t cmd_value;
}HttpQueryStringParser;

/**
 *获取URL中的path(URL格式为：http://host:port/path?param_list)
 *cmd_value = time33_hash(cmd_name)
 *仅在parse_url调用成功后有效
 */
uint32_t http_query_string_get_cmd_value(const HttpQueryStringParser* self);
const char* http_query_string_get_cmd_name(const HttpQueryStringParser* self);

/**
 *获取URL参数，其中哈希值 k = time33_hash(name)
 *@return 如果该参数不存在，返回NULL。返回结果是C风格字符串。
 *仅在parse_url调用成功后有效
 */
char* http_query_string_get_param_by_hash_value(const HttpQueryStringParser* self, uint32_t k);
char* http_query_string_get_param_by_name(const HttpQueryStringParser* self, const char* const name);
char* http_query_string_get_param_len_by_hash_value(const HttpQueryStringParser* self, uint32_t k, uint32_t* param_len);
char* http_query_string_get_param_len_by_name(const HttpQueryStringParser* self, const char* const name, uint32_t* param_len);

/**
 *原地解析URL中的参数列表，传入的URL将会被改变，需保证URL缓冲区长度至少为 url_len+1
 *URL的内存将用于随后的get_param调用中，调用者需保证其生命周期足够长
 *@return 0表示成功
 */
int32_t http_query_string_parse_url_inplace(HttpQueryStringParser* self, char* url, uint32_t url_len);


#define URL_GET_PARAM(url_parser, name, val, val_len, err_msg, err_handle) \
val = http_query_string_get_param_len_by_hash_value(url_parser, time33_hash(name), &val_len);\
if(val==NULL)\
{\
	logger_debug(default_logger,err_msg" parse url error, %s param missed.", name);\
	err_handle;\
}\
else

/**
 * 如果该参数存在，val直接引用url_parser内部内存，否则val=0
 */
#define URL_GET_CSTR_PARAM(url_parser, name, val, err_msg, err_handle) \
{\
	uint32_t len;\
	URL_GET_PARAM(url_parser, name, val, len, err_msg, err_handle);\
}

/**
 * 获取URL中的特定参数
 * 如果该参数为字符串，同时获取字符串的长度(不包括结尾的0)。
 * 如果该参数不存在，输出包含err_msg的日志，并执行语句err_handle，不对val或val_len作任何改变。
 */
#define URL_GET_INT32_PARAM(url_parser, name, val, err_msg, err_handle) \
{\
	const char* p;\
	uint32_t len;\
	URL_GET_PARAM(url_parser, name, p, len, err_msg, err_handle)\
	if(*p != 0) val = atoi32(p);\
}

#define URL_GET_INT64_PARAM(url_parser, name, val, err_msg, err_handle) \
{\
	const char* p;\
	uint32_t len;\
	URL_GET_PARAM(url_parser, name, p, len, err_msg, err_handle)\
	if (*p != 0) val = atoi64(p); \
}

#define URL_GET_STRING_PARAM(url_parser, name, val, val_len, err_msg, err_handle) \
{\
	char *p;\
	char *pv;\
	uint32_t len;\
	URL_GET_PARAM(url_parser, name, p, len, err_msg, err_handle)\
	{\
		pv = (char*)realloc(val, len+1);\
		if(pv)\
		{\
			val = pv;\
			memcpy(val, p, len+1);\
			val_len = len;\
		}\
		else\
		{\
			logger_debug(default_logger,err_msg" parse url, %s param out of memory.", name);\
			err_handle;\
		}\
	}\
}

/**
 * 获取字符串参数，并对其进行URL解码
 */
#define URL_GET_DECODED_STRING_PARAM(url_parser, name, val, val_len, err_msg, err_handle) \
{\
	char *p;\
	char *pv;\
	uint32_t len;\
	URL_GET_PARAM(url_parser, name, p, len, err_msg, err_handle)\
	{\
		pv = (char*)realloc(val, len+1);\
		if(pv)\
		{\
			val = pv;\
			val_len = decode_url(p, len, val);\
		}\
		else\
		{\
			logger_debug(default_logger,err_msg" parse url, %s param out of memory.", name);\
			err_handle;\
		}\
	}\
}

/**
 * 当参数的第一个字符是't'或'1'时，val=1，否则val=0
 */
#define URL_GET_BOOL_PARAM(url_parser, name, val, err_msg, err_handle) \
{\
	const char* p;\
	uint32_t len;\
	URL_GET_PARAM(url_parser, name, p, len, err_msg, err_handle)\
	val = *p=='t'||*p=='1';\
}

/**
 *以下http相关接口均不会申请、释放内存，也不会改变传入内存
 */
int32_t http_package_parse_inplace(char* package, uint32_t package_len,
									char** p_head, uint32_t* p_head_len,
									char** p_body, uint32_t* p_body_len);

int32_t http_get_request_header(char* header, uint32_t header_len,
								const char* name, uint32_t name_len,
								char** p_value, uint32_t* p_value_len);

#define http_get_request_cookie(header, header_len, cookie, cookie_len) \
	http_get_request_header(header, header_len, "Cookie", strlen("Cookie"), &(cookie), &(cookie_len))

int32_t http_get_cookie_item(char* cookie, uint32_t cookie_len,
							 const char* name, uint32_t name_len,
							 char** p_value, uint32_t* p_value_len);

int32_t http_get_body_item(char* header, uint32_t header_len,
						   char* body, uint32_t body_len,
						   char* name, uint32_t name_len,
						   char** p_value, uint32_t* p_value_len);

#ifdef __cplusplus
}
#endif


#endif
