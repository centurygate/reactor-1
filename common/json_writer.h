/**
# -*- coding:UTF-8 -*-
*/

#ifndef _JSON_WRITER_H_
#define _JSON_WRITER_H_

#include <stdio.h>
#include "string_utility.h"
#include "dynamic_array.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*json_object_to_string_func_t)(char** p_json_string, void* json_obj, ...);

/******************************json writer use dynamic_array*********************************/

typedef dynamic_array_t jo_writer;
void jo_writer_initialize(jo_writer* self, uint32_t init_buffer_size);
#define jo_writer_finalize(self) dynamic_array_finalize(self)
#define jo_writer_get_string(self) (char*)dynamic_array_data(self)
#define jo_writer_get_length(self)  dynamic_array_size(self)
#define jo_writer_attach_buffer(self, buffer, buffer_size) dynamic_array_attach_buffer(self, buffer, buffer_size)
#define jo_writer_detach_buffer(self) (char*)dynamic_array_detach_buffer(self)
#define jo_writer_append(self, str, str_len) dynamic_array_append(self, str, str_len)

void jo_writer_object_begin(jo_writer* self);
void jo_writer_object_add_string(jo_writer* self, const char* name, const char* s, uint32_t len);
void jo_writer_object_add_int32(jo_writer* self, const char* name, int32_t n);
void jo_writer_object_add_uint32(jo_writer* self, const char* name, uint32_t n);
void jo_writer_object_add_int64(jo_writer* self, const char* name, int64_t n);
void jo_writer_object_add_uint64(jo_writer* self, const char* name, uint64_t n);
void jo_writer_object_add_bool(jo_writer* self, const char* name, int32_t n);
void jo_writer_object_end(jo_writer* self);

void jo_writer_array_begin(jo_writer* self);
void jo_writer_array_add_string(jo_writer* self, const char* s, uint32_t len);
void jo_writer_array_add_int32(jo_writer* self, int32_t n);
void jo_writer_array_add_uint32(jo_writer* self, uint32_t n);
void jo_writer_array_add_int64(jo_writer* self, int64_t n);
void jo_writer_array_add_uint64(jo_writer* self, uint64_t n);
void jo_writer_array_add_bool(jo_writer* self, int32_t n);
void jo_writer_array_end(jo_writer* self);

void jo_writer_object_add_name(jo_writer* self, const char* s, uint32_t len);

#define JO_OBJECT_ADD_OBJECT(self, name, json_obj, to_json_string_func, ...) \
{\
	jo_writer_object_add_name(self, name, strlen(name)); \
	to_json_string_func(self, json_obj, ##__VA_ARGS__); \
}

#define JO_OBJECT_ADD_ARRAY(self, name, json_obj, to_json_string_func, ...) JO_OBJECT_ADD_OBJECT(self, name, json_obj, to_json_string_func, ##__VA_ARGS__)

#define JO_ARRAY_ADD_OBJECT(self, json_obj, to_json_string_func, ...)  to_json_string_func(self, json_obj, ##__VA_ARGS__)
#define JO_ARRAY_ADD_ARRAY(self, name, json_obj, to_json_string_func, ...) JO_ARRAY_ADD_OBJECT(self, name, json_obj, to_json_string_func, ##__VA_ARGS__)


/******************************json writer use fixed-length buffer*********************************/

extern const char json_writer_escape_table[256];
//a json string is ANY UNICODE character except " or \ or control character
#define JSON_ESCAPE_STRING(json_string, str) \
{\
	const uint8_t* p = (const uint8_t*)str;\
	while(*p){\
		if(json_writer_escape_table[*p]!=0)\
		{\
			*json_string++ = '\\';\
			*json_string++ = json_writer_escape_table[*p];\
		}\
		else *json_string++ = *p;\
		++p;\
	}\
}

#define JSON_OBJECT_ADD_NAME(json_string, name) \
{\
	*json_string++ = '"';\
	memcpy(json_string, name, strlen(name));\
	json_string += strlen(name);\
	*json_string++ = '"';\
	*json_string++ = ':';\
}

#define JSON_ADD_VALUE_STRING(json_string, val) \
{\
	*json_string++ = '"';\
	JSON_ESCAPE_STRING(json_string, val)\
	*json_string++ = '"';\
	*json_string++ = ',';\
}

#define JSON_ADD_VALUE_UINT32(json_string, val) \
{\
	int32_t l = u32toa(val, json_string);\
	json_string += l;\
	*json_string++ = ',';\
}

#define JSON_ADD_VALUE_INT32(json_string, val) \
{\
	int32_t l = i32toa(val, json_string);\
	json_string += l;\
	*json_string++ = ',';\
}

#define JSON_ADD_VALUE_UINT64(json_string, val) \
{\
	*json_string++ = '"';\
	int32_t l = u64toa(val, json_string);\
	json_string += l;\
	*json_string++ = '"';\
	*json_string++ = ',';\
}

#define JSON_OBJECT_BEGIN(json_string) *json_string++='{';

#define JSON_OBJECT_ADD_STRING(json_string, name, val) \
{\
	JSON_OBJECT_ADD_NAME(json_string, name)\
	JSON_ADD_VALUE_STRING(json_string, val)\
}

#define JSON_OBJECT_ADD_UINT32(json_string, name, val) \
{\
	JSON_OBJECT_ADD_NAME(json_string, name)\
	JSON_ADD_VALUE_UINT32(json_string, val)\
}

#define JSON_OBJECT_ADD_INT32(json_string, name, val) \
{\
	JSON_OBJECT_ADD_NAME(json_string, name)\
	JSON_ADD_VALUE_INT32(json_string, val)\
}

#define JSON_OBJECT_ADD_UINT64(json_string, name, val) \
{\
	JSON_OBJECT_ADD_NAME(json_string, name)\
	JSON_ADD_VALUE_UINT64(json_string, val)\
}

#define JSON_OBJECT_ADD_OBJECT(json_string, name, json_obj, to_json_string_func, ...) \
{\
	JSON_OBJECT_ADD_NAME(json_string, name)\
	to_json_string_func(&json_string, json_obj, ##__VA_ARGS__);\
	*json_string++ = ',';\
}

#define JSON_OBJECT_END(json_string) \
{\
	if(json_string[-1]==',')json_string[-1] = '}';\
	else *json_string++ = '}';\
}

#define JSON_ARRAY_BEGIN(json_string) *json_string++='[';


#define JSON_ARRAY_ADD_OBJECT(json_string, json_obj, to_json_string_func, ...)  \
{\
	to_json_string_func(&json_string, json_obj, ##__VA_ARGS__);\
	*json_string++ = ',';\
}

#define JSON_ARRAY_END(json_string) \
{\
	if(json_string[-1]==',')json_string[-1] = ']';\
	else *json_string++ = ']';\
}

#ifdef __cplusplus
}
#endif

#endif
