/**
# -*- coding:UTF-8 -*-
*/

#include "json_writer.h"
#include "encoding_conversion.h"

static void jo_writer_escape_utf8(char** p_dst, const char** p_s)
{
	uint32_t ucs4 = translate_utf8_char_to_ucs4(*p_s, p_s);
	if (ucs4 != 0)
	{
		if (ucs4 & 0xffff0000) //ignore
		{
		}
		else
		{
			*(*p_dst)++ = '\\';
			*(*p_dst)++ = 'u';
			*(*p_dst)++ = int2hex_lower(ucs4 >> 12);
			*(*p_dst)++ = int2hex_lower((ucs4 >> 8) & 0xf);
			*(*p_dst)++ = int2hex_lower((ucs4 >> 4) & 0xf);
			*(*p_dst)++ = int2hex_lower(ucs4 & 0xf);
		}
	}
	else
	{
		*(*p_dst)++ = *(*p_s)++;
	}
}

static int32_t jo_writer_escape_string(char* dst, const char* s, uint32_t len)
{
	const char* in = s;
	const char* end = in + len;
	char* out = dst;
	while (in < end)
	{
		if (*in & 0x80)
		{
			jo_writer_escape_utf8(&out, &in);
		}
		else
		{
			if (*in != 0)
			{
				if (json_writer_escape_table[CHAR_TO_INT(*in)])
				{
					*out++ = '\\';
					*out++ = json_writer_escape_table[CHAR_TO_INT(*in)];
				}
				else *out++ = *in;
			}
			++in;
		}
	}
	return out - dst;
}

void jo_writer_initialize(jo_writer* self, uint32_t init_buffer_size)
{
	dynamic_array_initialize(self, 1);
	dynamic_array_reserve(self, init_buffer_size);
}

static void jo_writer_add_value_string(jo_writer* self, const char* s, uint32_t len)
{
	char* buf = (char*)alloca(len * 3 + 3);
	int32_t esc_len = 0;
	buf[esc_len++] = '"';
	esc_len += jo_writer_escape_string(buf + esc_len, s, len);
	buf[esc_len++] = '"';
	dynamic_array_append(self, buf, esc_len);
}

static void jo_writer_add_value_int32(jo_writer* self, int32_t n)
{
	char buf[16];
	int32_t len = 0;
	len += i32toa(n, buf+len);
	dynamic_array_append(self, buf, len);
}

static void jo_writer_add_value_uint32(jo_writer* self, uint32_t n)
{
	char buf[16];
	int32_t len = 0;
	len += u32toa(n, buf+len);
	dynamic_array_append(self, buf, len);
}

static void jo_writer_add_value_int64(jo_writer* self, int64_t n)
{
	char buf[32];
	int32_t len = 0;
	buf[len++] = '"';
	len += i64toa(n, buf+len);
	buf[len++] = '"';
	dynamic_array_append(self, buf, len);
}

static void jo_writer_add_value_uint64(jo_writer* self, uint64_t n)
{
	char buf[32];
	int32_t len = 0;
	buf[len++] = '"';
	len += u64toa(n, buf+len);
	buf[len++] = '"';
	dynamic_array_append(self, buf, len);
}

static void jo_writer_add_value_bool(jo_writer* self, int32_t n)
{
	if (n)
	{
		dynamic_array_append(self, "true", strlen("true"));
	}
	else
	{
		dynamic_array_append(self, "false", strlen("false"));
	}
}

void jo_writer_object_add_name(jo_writer* self, const char* s, uint32_t len)
{
	char* buf = (char*)alloca(len * 3 + 3);
	int32_t esc_len = 0;
	char* json_string = jo_writer_get_string(self);
	uint32_t json_len = jo_writer_get_length(self);
	if (json_len != 0 && json_string[json_len - 1] != '{')
	{
		buf[esc_len++] = ',';
	}
	buf[esc_len++] = '"';
	esc_len += jo_writer_escape_string(buf + esc_len, s, len);
	buf[esc_len++] = '"';
	buf[esc_len++] = ':';
	dynamic_array_append(self, buf, esc_len);
}

void jo_writer_object_begin(jo_writer* self)
{
	char* json_string = jo_writer_get_string(self);
	uint32_t json_len = jo_writer_get_length(self);
	if (json_len != 0 && json_string[json_len - 1] != '{'
		&& json_string[json_len - 1] != '[' && json_string[json_len - 1] != ':')
	{
		dynamic_array_push_back(self, ",");
	}
	dynamic_array_push_back(self, "{");
}

void jo_writer_object_add_string(jo_writer* self, const char* name, const char* s, uint32_t len)
{
	jo_writer_object_add_name(self, name, strlen(name));
	jo_writer_add_value_string(self, s, len);
}

void jo_writer_object_add_int32(jo_writer* self, const char* name, int32_t n)
{
	jo_writer_object_add_name(self, name, strlen(name));
	jo_writer_add_value_int32(self, n);
}

void jo_writer_object_add_uint32(jo_writer* self, const char* name, uint32_t n)
{
	jo_writer_object_add_name(self, name, strlen(name));
	jo_writer_add_value_uint32(self, n);
}

void jo_writer_object_add_int64(jo_writer* self, const char* name, int64_t n)
{
	jo_writer_object_add_name(self, name, strlen(name));
	jo_writer_add_value_int64(self, n);
}

void jo_writer_object_add_uint64(jo_writer* self, const char* name, uint64_t n)
{
	jo_writer_object_add_name(self, name, strlen(name));
	jo_writer_add_value_uint64(self, n);
}

void jo_writer_object_add_bool(jo_writer* self, const char* name, int32_t n)
{
	jo_writer_object_add_name(self, name, strlen(name));
	jo_writer_add_value_bool(self, n);
}

void jo_writer_object_end(jo_writer* self)
{
	dynamic_array_push_back(self, "}");
}


static void jo_writer_array_add_comma(jo_writer* self)
{
	char* json_string = jo_writer_get_string(self);
	uint32_t json_len = jo_writer_get_length(self);
	if (json_len != 0 && json_string[json_len - 1] != '[')
	{
		dynamic_array_push_back(self, ",");
	}
}

void jo_writer_array_begin(jo_writer* self)
{
	char* json_string = jo_writer_get_string(self);
	uint32_t json_len = jo_writer_get_length(self);
	if (json_len != 0 && json_string[json_len - 1] != '{'
		&& json_string[json_len - 1] != '[' && json_string[json_len - 1] != ':')
	{
		dynamic_array_push_back(self, ",");
	}
	dynamic_array_push_back(self, "[");
}

void jo_writer_array_add_string(jo_writer* self, const char* s, uint32_t len)
{
	jo_writer_array_add_comma(self);
	jo_writer_add_value_string(self, s, len);
}

void jo_writer_array_add_int32(jo_writer* self, int32_t n)
{
	jo_writer_array_add_comma(self);
	jo_writer_add_value_int32(self, n);
}

void jo_writer_array_add_uint32(jo_writer* self, uint32_t n)
{
	jo_writer_array_add_comma(self);
	jo_writer_add_value_uint32(self, n);
}

void jo_writer_array_add_int64(jo_writer* self, int64_t n)
{
	jo_writer_array_add_comma(self);
	jo_writer_add_value_int64(self, n);
}

void jo_writer_array_add_uint64(jo_writer* self, uint64_t n)
{
	jo_writer_array_add_comma(self);
	jo_writer_add_value_uint64(self, n);
}

void jo_writer_array_add_bool(jo_writer* self, int32_t n)
{
	jo_writer_array_add_comma(self);
	jo_writer_add_value_bool(self, n);
}

void jo_writer_array_end(jo_writer* self)
{
	dynamic_array_push_back(self, "]");
}

//目前只考虑下面几个控制字符
const char json_writer_escape_table[256] =
{
	0,0,0,0,0,0,0,0,'b','t','n',0,'f','r',0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,'"',0,0,0,0,0,0,0,0,0,0,0,0,'/',
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,'\\',0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#if 0
/*********** example ************/
const char* dest_json_string = 
"{"
"	\"task_number\" : 2,"
"	\"task_list\" :"
"	["
"		{"
"			\"name\": \"taks1\","
"			\"size\": 12345,"
"		},"
"		{"
"			\"name\": \"taks2\","
"			\"size\": 12346,"
"		}"
"	]"
"}";

struct task_info
{
	char* name;
	uint64_t size;
}task_list[2];

void ti_to_json_string(char** json_string, const struct task_info* task)
{
	char* p_json = *json_string;
	JSON_OBJECT_BEGIN(p_json);
	JSON_OBJECT_ADD_STRING(p_json, "name", task->name);
	JSON_OBJECT_ADD_UINT64(p_json, "name", task->size);
	JSON_OBJECT_END(p_json);
	*json_string = p_json;
}

void test_json_writer()
{
	int i=0;
	char buf[1024];
	char* p_json = buf;
	task_list[0].name = "task1";
	task_list[0].size = 12345;
	task_list[1].name = "task2";
	task_list[1].size = 12346;
	JSON_OBJECT_BEGIN(p_json);
	JSON_OBJECT_ADD_UINT32(p_json, "task_number", 2);
	JSON_ARRAY_BEGIN(p_json);
	for( ; i<2; ++i)
	{
		JSON_ARRAY_ADD_OBJECT(p_json, &task_list[i], ti_to_json_string);
	}
	JSON_ARRAY_END(p_json);
	JSON_OBJECT_END(p_json);
	*p_json = 0;
	puts(buf);
}

static void test_add_array(jo_writer* jw, int32_t a1, int32_t a2)
{
	jo_writer_array_begin(jw);
	jo_writer_array_add_bool(jw, a1);
	jo_writer_array_begin(jw);
	jo_writer_array_add_int32(jw, a2);
	jo_writer_array_end(jw);
	jo_writer_array_add_string(jw, "a b	cd'\"'ef", 11);
	jo_writer_array_end(jw);
}

void test_jo_writer()
{
	jo_writer jw;
	jo_writer_initialize(&jw);
	jo_writer_object_begin(&jw);
	jo_writer_object_add_int32(&jw, "n1", 1);

	JO_OBJECT_ADD_ARRAY(&jw, "a\\r\t\fr", 1, test_add_array, 2);
	//same as below
	//jo_writer_object_add_name(&jw, "a\\r\t\fr", strlen("a\\r\t\fr"));
	//test_add_array(&jw, 1, 2);

	jo_writer_object_add_int32(&jw, "n2\x0d\x0a", 2);
	jo_writer_object_end(&jw);
	dynamic_array_push_back(&jw, "\x00");
	printf("%s\n", (char*)jo_writer_get_string(&jw));
}

int main()
{
	test_json_writer();
	test_jo_writer();
	return 0;
}

#endif
