/**
# -*- coding:UTF-8 -*-
*/

#include "http_utility.h"
#include "error_code.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/*************************** hash set implement ****************************/

static const unsigned int NIL = 0;

void linear_probing_hash_set_reset(LinearProbingHashSet* self)
{
	int32_t i = 0;
	for( ; i < MAX_HASH_SET_SIZE; ++i)
		self->key[i] = NIL;
	//memset(self->key, NIL, sizeof self->key);
	self->size = 0;
}

int32_t linear_probing_hash_set_insert(LinearProbingHashSet* self, uint32_t k, char* val, uint32_t val_len)
{
	uint32_t i = k % MAX_HASH_SET_SIZE;
	do{
		if(self->key[i] == NIL)
		{
			if(self->size == MAX_HASH_SET_SIZE-1) return ERR_NOT_ENOUGH_BUFFER;
			++self->size;
			self->key[i] = k;
			self->data[i] = val;
			self->data_len[i] = val_len;
			return 0;
		}
		if(self->key[i] == k)
		{
			//新的同名参数覆盖旧的
			self->data[i] = val;
			self->data_len[i] = val_len;
			return 0;
		}
	}while(++i<MAX_HASH_SET_SIZE);

	for(i=0; self->key[i]!=NIL; ++i)
	{
		if(self->key[i] == k)
		{
			self->data[i] = val;
			self->data_len[i] = val_len;
			return 0;
		}
	}

	if(self->size == MAX_HASH_SET_SIZE-1) return ERR_NOT_ENOUGH_BUFFER;
	++self->size;
	self->key[i] = k;
	self->data[i] = val;
	self->data_len[i] = val_len;
	return 0;
}

char* linear_probing_hash_set_get_val_len(const LinearProbingHashSet* self, uint32_t k, uint32_t* len)
{
	uint32_t i = k % MAX_HASH_SET_SIZE;
	do{
		if(self->key[i]==k)
		{
			*len = self->data_len[i];
			return self->data[i];
		}
		if(self->key[i]==NIL)
		{
			*len = 0;
			return NULL;
		}
	}while(++i<MAX_HASH_SET_SIZE);

	for(i=0; ; ++i)
	{
		if(self->key[i]==k)
		{
			*len = self->data_len[i];
			return self->data[i];
		}
		if(self->key[i]==NIL)
		{
			*len = 0;
			return NULL;
		}
	}
}

char* linear_probing_hash_set_get_val(const LinearProbingHashSet* self, uint32_t k)
{
	uint32_t len;
	return linear_probing_hash_set_get_val_len(self, k, &len);
}

/*************************** http query string parser implement ****************************/

static int32_t http_query_string_insert_param(HttpQueryStringParser* self, uint32_t k, char* val, uint32_t val_len)
{
	return linear_probing_hash_set_insert(&self->hash_set, k, val, val_len);
}

uint32_t http_query_string_get_cmd_value(const HttpQueryStringParser* self)
{
	return self->cmd_value;
}

const char* http_query_string_get_cmd_name(const HttpQueryStringParser* self)
{
	return self->cmd_name;
}

char* http_query_string_get_param_by_hash_value(const HttpQueryStringParser* self, uint32_t k)
{
	return linear_probing_hash_set_get_val(&self->hash_set, k);
}

char* http_query_string_get_param_by_name(const HttpQueryStringParser* self, const char* const name)
{
	return http_query_string_get_param_by_hash_value(self, time33_hash(name));
}

char* http_query_string_get_param_len_by_hash_value(const HttpQueryStringParser* self, uint32_t k, uint32_t* param_len)
{
	return linear_probing_hash_set_get_val_len(&self->hash_set, k, param_len);
}

char* http_query_string_get_param_len_by_name(const HttpQueryStringParser* self, const char* const name, uint32_t* param_len)
{
	return linear_probing_hash_set_get_val_len(&self->hash_set, time33_hash(name), param_len);
}

int32_t http_query_string_parse_url_inplace(HttpQueryStringParser* self, char* url, uint32_t len)
{
	char* p_param;
	char* p_val = url;
	char* p_cur = url;

	linear_probing_hash_set_reset(&self->hash_set);

	//cmd
	url[len] = '?';
	while(*p_cur!='?'){
		if(*p_cur=='/')p_val = ++p_cur;
		else ++p_cur;
	}
	*p_cur = 0;
	self->cmd_name = p_val;
	self->cmd_value = time33_hash(self->cmd_name);

	if (p_cur - url == len) return 0;

	//param
	url[len] = '&';
	url[len+1] = 0;
	p_param = ++p_cur;
	while(*p_cur){
		while(*p_cur!='='){
			if(*p_cur=='&')
				goto LABLE_NEXT_PARAM;
			++p_cur;
		}
		*p_cur = 0;
		p_val = ++p_cur;
		while(*p_cur!='&')++p_cur;
		*p_cur = 0;
		if(http_query_string_insert_param(self, time33_hash(p_param), p_val, p_cur-p_val) != 0)
			return ERR_TOO_MANY_PARAMETERS;
LABLE_NEXT_PARAM:
		p_param = ++p_cur;
	}

	url[len] = 0;
	return 0;
}

int32_t http_package_parse_inplace(char* package, uint32_t package_len,
									char** p_head, uint32_t* p_head_len,
									char** p_body, uint32_t* p_body_len)
{
	char* p = package + 32; //按照规范，http头一般应大于32个字节
	char* end = package + package_len - 4;
	for( ; p<=end; ++p)
	{
		if(p[0]=='\r' && p[1]=='\n' &&
			p[2]=='\r' && p[3]=='\n')
		{
			if (p_head != NULL)
			{
				*p_head = package;
				*p_head_len = p-package;
			}

			if (p_body != NULL)
			{
				*p_body = p + 4;
				*p_body_len = package_len - (p - package) - 4;
			}

			//*p = 0;
			return 0;
		}
	}
	return ERR_PROTOCOL_ERR;
}

static char* http_move_to_next_line(char* cur_line, char* package, uint32_t package_len)
{
	char* p = cur_line;
	char* end = package + package_len;// - 1;
	for( ; p<end; ++p)
	{
		if(/*p[0]=='\r' && p[1]=='\n'*/ *p == '\n')
		{
			++p; //p += 2;
			return p < end ? p : NULL;
		}
	}
	return NULL;
}

int32_t http_get_request_header(char* header, uint32_t header_len,
								const char* name, uint32_t name_len,
								char** p_value, uint32_t* p_value_len)
{
	char* line = header;
	char* end = header + header_len;
	do
	{
		if(memcmp(name, line, name_len) == 0)
		{
			char* p = line + 1;
			while(*p != ':')
			{
				if(*p=='\r' || p>=end)
					return ERR_PROTOCOL_ERR;
				++p;
			}
			++p;
			skip_space(p);
			*p_value = p;
			while(*p!='\r' && p<end) ++p;
			*p_value_len = p - *p_value;
			return 0;
		}
		line = http_move_to_next_line(line, header, header_len);
	}while(line);

	return ERR_PARAM_NOT_FOUND;
}

static char* http_cookie_move_to_next_item(char* cur_item, char* cookie, uint32_t cookie_len)
{
	char* p = cur_item + 3; //按照规范，cookie项至少大于3字节
	char* end = cookie + cookie_len;
	for( ; p<end; ++p)
	{
		if(*p == ';')
		{
			++p;
			skip_space(p);
			return p;
		}
	}
	return NULL;
}

int32_t http_get_cookie_item(char* cookie, uint32_t cookie_len,
							 const char* name, uint32_t name_len,
							 char** p_value, uint32_t* p_value_len)
{
	char* item = cookie;
	char* end = cookie + cookie_len;
	do
	{
		if(item[name_len]=='=' && memcmp(name, item, name_len)==0)
		{
			char* p = item + name_len + 1;
			*p_value = p;
			while (p < end)
			{
				if (*p == ';')
				{
					break;
				}
				++p;
			}
			*p_value_len = p - *p_value;
			return 0;
		}
		item = http_cookie_move_to_next_item(item, cookie, cookie_len);
	}while(item);

	return ERR_PARAM_NOT_FOUND;
}

static int32_t http_get_body_item_multipart(char* body, uint32_t body_len,
											char* boundary, uint32_t boundary_len,
											char* name, uint32_t name_len,
											char** p_value, uint32_t* p_value_len)
{
	char* p_sub_datagram = body;
	uint32_t start_boundary_len = 2;
	char start_boundary[128] = "--";
	uint32_t end_boundary_len;
	char end_boundary[128];

	memcpy(start_boundary+start_boundary_len, boundary, boundary_len);
	start_boundary_len += boundary_len;

	memcpy(end_boundary, start_boundary, start_boundary_len);
	memcpy(end_boundary+start_boundary_len, "--", 2);
	end_boundary_len = start_boundary_len+2;

	do
	{
		if(p_sub_datagram[start_boundary_len] == '\r'
			&& memcmp(p_sub_datagram, start_boundary, start_boundary_len) == 0)
		{
			char* p_name;
			p_sub_datagram = http_move_to_next_line(p_sub_datagram, body, body_len);
			if(p_sub_datagram == NULL) return ERR_PROTOCOL_ERR;
			p_name = p_sub_datagram;
			while(*p_name!='\r')
			{
				if(memcmp("name=\"", p_name, strlen("name=\"")) == 0)
				{
					char* p = p_name + strlen("name=\"");
					if(p[name_len]=='"' && memcmp(p, name, name_len)==0)
					{
						do
						{
							p_sub_datagram = http_move_to_next_line(p_sub_datagram, body, body_len);
							if(p_sub_datagram == NULL) return ERR_PROTOCOL_ERR;
						}while(*p_sub_datagram != '\r'); //发现空行
						p_sub_datagram = http_move_to_next_line(p_sub_datagram, body, body_len);
						if(p_sub_datagram == NULL) return ERR_PROTOCOL_ERR;
						*p_value = p_sub_datagram;

						do
						{
							p_sub_datagram = http_move_to_next_line(p_sub_datagram, body, body_len);
							if(p_sub_datagram == NULL) return ERR_PROTOCOL_ERR;
						}while(memcmp(p_sub_datagram, end_boundary, end_boundary_len) != 0);

						*p_value_len = p_sub_datagram - *p_value - strlen("\r\n");
						return 0;
					}
					else goto LABEL_NEXT;
				}
				++p_name;
			}
			return ERR_PROTOCOL_ERR;
		}
LABEL_NEXT:
		p_sub_datagram = http_move_to_next_line(p_sub_datagram, body, body_len);
	}while(p_sub_datagram);

	return ERR_PARAM_NOT_FOUND;
}

static char* http_query_string_move_to_next_item(char* cur_item, char* query_string, uint32_t query_string_len)
{
	char* p = cur_item + 2; //按照规范，query_string项至少大于2字节
	char* end = query_string + query_string_len;
	for( ; p<end; ++p)
	{
		if(*p == '&')
		{
			do
			{
				++p;
				skip_space(p);
			}while(*p == '&');
			return p;
		}
	}
	return NULL;
}

static int32_t http_get_body_item_urlencoded(char* body, uint32_t body_len,
											 char* name, uint32_t name_len,
											 char** p_value, uint32_t* p_value_len)
{
	char* item = body;
	char* end = body + body_len;
	do
	{
		char* p = item + name_len;
		if (p >= end) break;
		if(item[name_len]=='=' && memcmp(name, item, name_len)==0)
		{
			*p_value = ++p;
			while(p<end && *p!='&') ++p;
			*p_value_len = p - *p_value;
			return 0;
		}
		item = http_query_string_move_to_next_item(item, body, body_len);
	}while(item);

	return ERR_PARAM_NOT_FOUND;
}

int32_t http_get_body_item(char* header, uint32_t header_len,
						   char* body, uint32_t body_len,
						   char* name, uint32_t name_len,
						   char** p_value, uint32_t* p_value_len)
{
	char* content_type;
	uint32_t content_type_len;
	int32_t ret;
	ret = http_get_request_header(header, header_len, "Content-Type", strlen("Content-Type"), &content_type, &content_type_len);
	if(ret != 0 || content_type_len == 0) return ERR_PROTOCOL_ERR;
	if(memcmp("multipart/form-data", content_type, strlen("multipart/form-data")) == 0)
	{
		char* p_boundary = content_type + strlen("multipart/form-data") + 1;
		skip_space(p_boundary);
		if(memcmp("boundary=", p_boundary, strlen("boundary=")) == 0)
		{
			p_boundary += strlen("boundary=");
			return http_get_body_item_multipart(body, body_len, p_boundary, content_type_len - (p_boundary-content_type), name, name_len, p_value, p_value_len);
		}
		else return ERR_PROTOCOL_ERR;
	}
	else if(memcmp("application/x-www-form-urlencoded", content_type, strlen("application/x-www-form-urlencoded")) == 0)
	{
		return http_get_body_item_urlencoded(body, body_len, name, name_len, p_value, p_value_len);
	}
	else return ERR_PROTOCOL_ERR;
}

#ifdef __cplusplus
}
#endif

#if 0
int main()
{
	HttpQueryStringParser parser;
	char url[1024] = "http://yuancheng.xunlei.com/test?p1=v1%202+2&xxx&&p2=123+321";
	uint32_t ret = http_query_string_parse_url_inplace(&parser ,url, strlen(url));
	printf("ret:%d\n", ret);

	URL_GET_INT32_PARAM(parser, "p2", ret, "main test 1", (void)0);
	printf("p2:%d\n", ret);

	URL_GET_INT32_PARAM(parser, "p3", ret, "main test 2", (void)0);
	printf("p3:%d\n", ret);

	URL_GET_DECODED_STRING_PARAM(parser, "p1", url, ret, "main test 3", return -1);
	printf("p1:%d, %s\n", ret, url);

	return 0;
}
#endif
