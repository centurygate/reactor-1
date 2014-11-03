/**
# -*- coding:UTF-8 -*-
*/

#include "json_parser.h"

//目前只考虑下面几个控制字符
static const char json_parser_escape_table[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, '"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '/', //不转义/
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0,
	0, 0, 0x08, 0, 0, 0, 0x0c, 0, 0, 0, 0, 0, 0, 0, 0x0a, 0,
	0, 0, 0x0d, 0, 0x09, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

jo_t* jo_forward_iterator_value(jo_forward_iterator self)
{
	return (jo_t*)self->data;
}

#define MAX_JSON_LEN (1*1024*1024*1024)
static int32_t expect_token(char** p_cursor, uint32_t remain_len, char token)
{
	if (remain_len > 0)
	{
		if (**p_cursor == token)
		{
			++*p_cursor;
			return 0;
		}
		else return -1;
	}
	else return -1;
}

static int32_t jo_parse_string(char** s, char** p_cursor, uint32_t remain_len, char quote);

static int32_t jo_parse_key(jo_t* self, char** p_cursor, uint32_t remain_len);
static int32_t jo_parse_value_string(jo_t* self, char** p_cursor, uint32_t remain_len, char quote);
static int32_t jo_parse_value_number(jo_t* self, char** p_cursor, uint32_t remain_len);
static int32_t jo_parse_value_bool(jo_t* self, char** p_cursor, uint32_t remain_len);
static int32_t jo_parse_value_object(jo_t* self, char** p_cursor, uint32_t remain_len);
static int32_t jo_parse_value_array(jo_t* self, char** p_cursor, uint32_t remain_len);

static void transfer_char(char** s, char c)
{
	if (c != 0) //暂不过滤其他字符
	{
		**s = c;
		++*s;
	}
}

static int32_t transfer_ucs2_to_utf8(char** s, char** p_cursor, uint32_t remain_len, char quote)
{
	if (remain_len < 5 || remain_len > MAX_JSON_LEN) return -1;
	if (isxdigit((*p_cursor)[0]) && isxdigit((*p_cursor)[1])
		&& isxdigit((*p_cursor)[2]) && isxdigit((*p_cursor)[3]))
	{
		uint8_t hi = (get_hexvalue((*p_cursor)[0]) << 4) | get_hexvalue((*p_cursor)[1]);
		uint8_t lo = (get_hexvalue((*p_cursor)[2]) << 4) | get_hexvalue((*p_cursor)[3]);
		
		if (hi&0xf8) //3B
		{
			*(*s)++ = 0xE0 | (hi >> 4);
			*(*s)++ = 0x80 | ((hi & 0xf) << 2) | (lo >> 6);
			*(*s)++ = 0x80 | (lo & 0x3f);
		}
		else if (hi) //2B
		{
			*(*s)++ = 0xC0 | ((hi & 0x07) << 2) | (lo >> 6);
			*(*s)++ = 0x80 | (lo & 0x3f);
		}
		else  //1B
		{
			transfer_char(s, lo);
		}
	}
	else return -1;
	*p_cursor += 4;
	return 0;
}

static int32_t jo_parse_string(char** s, char** p_cursor, uint32_t remain_len, char quote)
{
	int32_t ret = 0;
	char* cursor = *p_cursor;
	char* end = cursor + remain_len;

	if (remain_len > MAX_JSON_LEN) return -1;

	while (*cursor != quote && cursor < end)
	{
		if (*cursor != '\\')
		{
			transfer_char(s, *cursor);
			++cursor;
		}
		else
		{
			++cursor;
			if (*cursor == 'u')
			{
				++cursor;
				ret = transfer_ucs2_to_utf8(s, &cursor, end - cursor, quote);
				if (ret != 0)
				{
					*p_cursor = cursor;
					return ret;
				}
			}
			else if (json_parser_escape_table[CHAR_TO_INT(*cursor)])
			{
				*(*s)++ = json_parser_escape_table[CHAR_TO_INT(*cursor++)];
			}
			else if (*cursor == 'x')
			{
				++cursor;
				if (isxdigit(cursor[0]) && isxdigit(cursor[1]))
				{
					transfer_char(s, (get_hexvalue(cursor[0]) << 4) | get_hexvalue(cursor[1]));
					cursor += 2;
				}
				else
				{
					*p_cursor = cursor;
					return -1;
				}
			}
			else
			{
				*(*s)++ = cursor[-1];
				*(*s)++ = cursor[0];
				++cursor;
			}
		}
	}

	if (cursor < end)
	{
		**s = 0;
		++cursor;
		*p_cursor = cursor;
		return 0;
	}
	else return -1;
}

static int32_t jo_parse_key(jo_t* self, char** p_cursor, uint32_t remain_len)
{
	char* s;
	char quote;

	if (remain_len > MAX_JSON_LEN) return -1;

	skip_space(*p_cursor);
	if (**p_cursor != '"' && **p_cursor != '\'')
	{
		return -1;
	}

	quote = **p_cursor;
	s = ++*p_cursor;
	self->key = s;

	return jo_parse_string(&s, p_cursor, remain_len, quote);
}

static int32_t jo_parse_value_string(jo_t* self, char** p_cursor, uint32_t remain_len, char quote)
{
	char* s = *p_cursor;

	if (remain_len > MAX_JSON_LEN) return -1;

	self->o_type = jo_type_string;
	self->jo_value = s;

	return jo_parse_string(&s, p_cursor, remain_len, quote);
}

static int32_t jo_parse_value_bool(jo_t* self, char** p_cursor, uint32_t remain_len)
{
	skip_space(*p_cursor);

	self->o_type = jo_type_boolean;
	self->jo_value = *p_cursor;
	if (memcmp(*p_cursor, "true", 4) == 0)
	{
		*p_cursor += 4;
	}
	else if (memcmp(*p_cursor, "false", 5) == 0)
	{
		*p_cursor += 5;
	}
	else return -1;
	return 0;
}

static int32_t jo_parse_value_number(jo_t* self, char** p_cursor, uint32_t remain_len)
{
	if (remain_len > MAX_JSON_LEN) return -1;

	self->o_type = jo_type_number;
	self->jo_value = *p_cursor;
	strtod(*p_cursor, p_cursor); //暂不将数字结尾置0
	return 0;
}

static int32_t jo_parse_value_array(jo_t* self, char** p_cursor, uint32_t remain_len)
{
	jo_t* jo = NULL;
	int32_t ret = -1;
	char* cursor = *p_cursor;
	char* end = cursor + remain_len;

	if (remain_len > MAX_JSON_LEN) return -1;

	self->o_type = jo_type_array;
	dynamic_array_initialize(&self->jo_array_elements, sizeof(jo_t*));

	while (cursor <= end)
	{

		skip_space(cursor);
		if (*cursor == ']')
		{
			++cursor;
			ret = 0;
			break;
		}

		jo = (jo_t*)malloc(sizeof*jo);
		jo_initialize(jo);

		if (*cursor == '"' || *cursor == '\'') //string
		{
			char quote = *cursor;
			++cursor;
			ret = jo_parse_value_string(jo, &cursor, end - cursor, quote);
			if (ret != 0) break;
		}
		else if (*cursor == '{') //object
		{
			++cursor;
			ret = jo_parse_value_object(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (*cursor == '[') //array
		{
			++cursor;
			ret = jo_parse_value_array(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (isdigit(*cursor) || *cursor == '-') //number
		{
			ret = jo_parse_value_number(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (*cursor == 't' || *cursor == 'f') //true,false
		{
			ret = jo_parse_value_bool(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (*cursor == 'n') //null
		{
			if (cursor[1] == 'u' && cursor[2] == 'l' && cursor[3] == 'l')
			{
				cursor += 4;
				ret = 0;
				jo->o_type = jo_type_object;
			}
			else
			{
				ret = -1;
				break;
			}
		}
		else
		{
			ret = -1;
			break;
		}

		skip_space(cursor);
		if (*cursor != ']')
		{
			ret = expect_token(&cursor, end - cursor, ',');
			if (ret != 0) break;
		}

		dynamic_array_push_back(&self->jo_array_elements, &jo);
	}
	*p_cursor = cursor;
	if (ret != 0)
	{
		jo_finalize_inplace(jo);
		free(jo);
	}
	return ret;
}

static int32_t jo_parse_value_object(jo_t* self, char** p_cursor, uint32_t remain_len)
{
	jo_t* jo = NULL;
	int32_t ret = -1;
	char* cursor = *p_cursor;
	char* end = cursor + remain_len;

	if (remain_len > MAX_JSON_LEN) return -1;

	self->o_type = jo_type_object;
	forward_list_initialize(&self->jo_object_members);

	while (cursor <= end)
	{
		skip_space(cursor);
		if (*cursor == '}')
		{ //object end
			++cursor;
			ret = 0;
			break;
		}

		jo = (jo_t*)malloc(sizeof*jo);
		jo_initialize(jo);

		ret = jo_parse_key(jo, &cursor, end - cursor);
		if (ret != 0) break;

		skip_space(cursor);
		ret = expect_token(&cursor, end - cursor, ':');
		if (ret != 0) break;

		skip_space(cursor);
		if (*cursor == '"' || *cursor == '\'') //string
		{
			char quote = *cursor;
			++cursor;
			ret = jo_parse_value_string(jo, &cursor, end - cursor, quote);
			if (ret != 0) break;
		}
		else if (*cursor == '{') //object
		{
			++cursor;
			ret = jo_parse_value_object(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (*cursor == '[') //array
		{
			++cursor;
			ret = jo_parse_value_array(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (isdigit(*cursor) || *cursor == '-') //number
		{
			ret = jo_parse_value_number(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (*cursor == 't' || *cursor == 'f') //true,false
		{
			ret = jo_parse_value_bool(jo, &cursor, end - cursor);
			if (ret != 0) break;
		}
		else if (*cursor == 'n') //null
		{
			if (cursor[1] == 'u' && cursor[2] == 'l' && cursor[3] == 'l')
			{
				cursor += 4;
				ret = 0;
				jo->o_type = jo_type_object;
			}
			else
			{
				ret = -1;
				break;
			}
		}
		else
		{
			ret = -1;
			break;
		}

		skip_space(cursor);
		if (*cursor != '}')
		{
			ret = expect_token(&cursor, end - cursor, ',');
			if (ret != 0) break;
		}

		forward_list_push_back(&self->jo_object_members, jo);
	}
	*p_cursor = cursor;
	if (ret != 0)
	{
		jo_finalize_inplace(jo);
		free(jo);
	}
	return ret;
}

jo_t* jo_parse_inplace(char** json, uint32_t remain_len)
{
	jo_t* jo;
	int32_t ret = -1;
	char* cursor = *json;
	char* end = cursor + remain_len;

	if (remain_len > MAX_JSON_LEN) return NULL;

	jo = (jo_t*)malloc(sizeof*jo);
	jo_initialize(jo);
	skip_space(cursor);
	if (*cursor == '{')
	{
		++cursor;
		ret = jo_parse_value_object(jo, &cursor, end-cursor);
	}
	else if (*cursor == '[')
	{
		++cursor;
		ret = jo_parse_value_array(jo, &cursor, end - cursor);
	}
	else
	{
		ret = -1;
	}
	*json = cursor;
	if (ret != 0)
	{
		jo_finalize_inplace(jo);
		return NULL;
	}
	else return jo;
}

void jo_release_inplace(jo_t* jo)
{
	if (jo != NULL)
	{
		jo_finalize_inplace(jo);
		free(jo);
	}
}

jo_t* jo_parse(const char** json_string, uint32_t json_len)
{
	return NULL;
}

void jo_release(jo_t* jo)
{
}

void jo_initialize(jo_t* self)
{
	memset(self, 0, sizeof*self);
}

void jo_finalize(jo_t* self)
{
}

void jo_finalize_inplace(jo_t* self)
{
	if (self->o_type == jo_type_object)
	{
		jo_forward_iterator it = forward_list_begin(&self->jo_object_members);
		jo_forward_iterator end = forward_list_end(&self->jo_object_members);
		while (it != end)
		{
			jo_t* jo = jo_forward_iterator_value(it);
			jo_finalize_inplace(jo);
			free(jo);
			it = jo_iterator_next(it);
		}
		forward_list_finalize(&self->jo_object_members);
	}
	else if (self->o_type == jo_type_array)
	{
		uint32_t it = 0;
		uint32_t end = dynamic_array_size(&self->jo_array_elements);
		while (it != end)
		{
			jo_t* jo = *(jo_t**)dynamic_array_at(&self->jo_array_elements, it);
			jo_finalize_inplace(jo);
			free(jo);
			++it;
		}
		dynamic_array_finalize(&self->jo_array_elements);
	}
}

jo_type_t jo_get_type(jo_t* jo)
{
	return jo->o_type;
}

const char* jo_get_key(jo_t* jo)
{
	return jo->key;
}

int32_t jo_get_int32(jo_t* jo)
{
	if (jo->o_type == jo_type_number || jo->o_type == jo_type_string)
	{
		return atoi(jo->jo_value);
	}
	else if (jo->o_type == jo_type_boolean)
	{
		return *jo->jo_value == 't' ? 1 : 0;
	}
	else return 0;
}

int64_t jo_get_int64(jo_t* jo)
{
	if (jo->o_type == jo_type_number || jo->o_type == jo_type_string)
	{
		return atoll(jo->jo_value);
	}
	else if (jo->o_type == jo_type_boolean)
	{
		return *jo->jo_value == 't' ? 1 : 0;
	}
	else return 0;
}

double jo_get_double(jo_t* jo)
{
	if (jo->o_type == jo_type_number || jo->o_type == jo_type_string)
	{
		return atof(jo->jo_value);
	}
	else if (jo->o_type == jo_type_boolean)
	{
		return *jo->jo_value == 't' ? 1 : 0;
	}
	else return 0;
}

char* jo_get_string(jo_t* jo)
{
	if (jo->o_type == jo_type_string )
	{
		return jo->jo_value;
	}
	else return NULL;
}

int32_t jo_members_find_int32(jo_t* jo, const char* key)
{
	jo_t* m = jo_members_find_object(jo, key);
	if (m != NULL) return jo_get_int32(m);
	else return 0;
}

int64_t jo_members_find_int64(jo_t* jo, const char* key)
{
	jo_t* m = jo_members_find_object(jo, key);
	if (m != NULL) return jo_get_int64(m);
	else return 0;
}

double jo_members_find_double(jo_t* jo, const char* key)
{
	jo_t* m = jo_members_find_object(jo, key);
	if (m != NULL) return jo_get_double(m);
	else return 0;
}

char* jo_members_find_string(jo_t* jo, const char* key)
{
	jo_t* m = jo_members_find_object(jo, key);
	if (m != NULL) return jo_get_string(m);
	else return NULL;
}

jo_t* jo_members_find_object(jo_t* jo, const char* key)
{
	jo_forward_iterator it = jo_members_find_iterator(jo, key);
	if (it != NULL) return jo_forward_iterator_value(it);
	else return NULL;
}


uint32_t jo_members_size(jo_t* jo)
{
	if (jo->o_type == jo_type_object)
	{
		return forward_list_size(&jo->jo_object_members);
	}
	else
	{
		return 0;
	}
}

int32_t jo_members_empty(jo_t* jo)
{
	if (jo->o_type == jo_type_object)
	{
		return forward_list_empty(&jo->jo_object_members);
	}
	else
	{
		return 1;
	}
}

jo_forward_iterator jo_members_begin(jo_t* jo)
{
	return forward_list_begin(&jo->jo_object_members);
}

jo_forward_iterator jo_members_end(jo_t* jo)
{
	return forward_list_end(&jo->jo_object_members);
}

jo_forward_iterator jo_iterator_next(jo_forward_iterator it)
{
	return forward_list_next(it);
}

static jo_forward_iterator jo_find_iterator(jo_forward_iterator it, const char* key)
{
	while (it != NULL)
	{
		jo_t* m = jo_forward_iterator_value(it);
		if (strcmp(key, m->key) == 0)
		{
			return it;
		}
		it = jo_iterator_next(it);
	}
	return NULL;
}

jo_forward_iterator jo_members_find_iterator(jo_t* jo, const char* key)
{
	if (jo->o_type == jo_type_object)
	{
		return jo_find_iterator(forward_list_begin(&jo->jo_object_members), key);
	}
	else return NULL;
}

jo_forward_iterator jo_iterator_next_key(jo_forward_iterator it, const char* key)
{
	return jo_find_iterator(it, key);
}


uint32_t jo_array_size(jo_t* jo)
{
	if (jo->o_type == jo_type_array)
	{
		return dynamic_array_size(&jo->jo_array_elements);
	}
	else
	{
		return 0;
	}
}

int32_t jo_array_empty(jo_t* jo)
{
	if (jo->o_type == jo_type_array)
	{
		return dynamic_array_empty(&jo->jo_array_elements);
	}
	else
	{
		return 1;
	}
}

int32_t jo_array_get_int32(jo_t* jo, uint32_t index)
{
	if (jo->o_type == jo_type_array)
	{
		return jo_get_int32(*(jo_t**)dynamic_array_at(&jo->jo_array_elements, index));
	}
	else return 0;
}

int64_t jo_array_get_int64(jo_t* jo, uint32_t index)
{
	if (jo->o_type == jo_type_array)
	{
		return jo_get_int64(*(jo_t**)dynamic_array_at(&jo->jo_array_elements, index));
	}
	else return 0;
}

double jo_array_get_double(jo_t* jo, uint32_t index)
{
	if (jo->o_type == jo_type_array)
	{
		return jo_get_double(*(jo_t**)dynamic_array_at(&jo->jo_array_elements, index));
	}
	else return 0;
}

char* jo_array_get_string(jo_t* jo, uint32_t index)
{
	if (jo->o_type == jo_type_array)
	{
		return jo_get_string(*(jo_t**)dynamic_array_at(&jo->jo_array_elements, index));
	}
	else return NULL;
}

jo_t* jo_array_get_object(jo_t* jo, uint32_t index)
{
	if (jo->o_type == jo_type_array)
	{
		return *(jo_t**)dynamic_array_at(&jo->jo_array_elements, index);
	}
	else return NULL;
}

#if 0
int main()
{
	char s[100] = "{\"a\\x20b\":\"123\\u6211abc\\/\x0d\x0avwx\",'b':[1,2,3,{'ba':'aa'}],'c':123.12}";
	char* json = s;
	jo_t* jo = jo_parse_inplace(&json, strlen(json));
	if (jo == NULL)
	{
		printf("json parse fail at: %s\n", json);
		return -1;
	}
	char* s1 = jo_members_find_string(jo, "a b");
	FILE* f;
	fopen_s(&f, "1", "w");
	fprintf(f, "%s\n", s1);
	fclose(f);
	printf("a b : %s\n", s1);
	jo_t* b = jo_members_find_object(jo, "b");
	jo_t* b3 = jo_array_get_object(b, 3);
	printf("b0 : %d\n", jo_get_int32(jo_array_get_object(b, 0)));
	printf("b1 : %d\n", jo_get_int32(jo_array_get_object(b, 1)));
	printf("b2 : %d\n", jo_get_int32(jo_array_get_object(b, 2)));
	printf("ba : %s\n", jo_members_find_string(b3, "ba"));
	printf("c : %f\n", jo_members_find_double(jo, "c"));
	jo_release_inplace(jo);
}
#endif
