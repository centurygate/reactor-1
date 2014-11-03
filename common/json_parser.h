/**
# -*- coding:UTF-8 -*-
*/

#ifndef _JSON_PARSER_H_
#define _JSON_PARSER_H_

#include "string_utility.h"
#include "forward_list.h"
#include "dynamic_array.h"

#ifdef __cplusplus
extern "C" 
{
#endif

typedef enum e_jo_type_t
{
	jo_type_null,
	jo_type_boolean,
	jo_type_number,
	jo_type_object,
	jo_type_array,
	jo_type_string
} jo_type_t;

typedef struct s_jo_t
{
	char* key;
	union u_jo_data
	{
		char* value;
		forward_list_t object_members;
		dynamic_array_t array_elements;
	}jo_data;
#define jo_object_members jo_data.object_members
#define jo_array_elements jo_data.array_elements
#define jo_value jo_data.value
	jo_type_t o_type;
} jo_t;

typedef forward_list_node_t* jo_forward_iterator;
jo_t* jo_forward_iterator_value(jo_forward_iterator self);

jo_t* jo_parse_inplace(char** json, uint32_t json_len);
void jo_release_inplace(jo_t* jo);

jo_t* jo_parse(const char** self, uint32_t json_len);
void jo_release(jo_t* jo);

void jo_initialize(jo_t* self);
void jo_finalize(jo_t* self);

#define jo_initialize_inplace(self) jo_initialize(self)
void jo_finalize_inplace(jo_t* self);

jo_type_t jo_get_type(jo_t* jo);
const char* jo_get_key(jo_t* jo);
int32_t jo_get_int32(jo_t* jo);
int64_t jo_get_int64(jo_t* jo);
double jo_get_double(jo_t* jo);
char* jo_get_string(jo_t* jo);

int32_t jo_members_find_int32(jo_t* jo, const char* key);
int64_t jo_members_find_int64(jo_t* jo, const char* key);
double jo_members_find_double(jo_t* jo, const char* key);
char* jo_members_find_string(jo_t* jo, const char* key);
jo_t* jo_members_find_object(jo_t* jo, const char* key);

uint32_t jo_members_size(jo_t* jo);
int32_t jo_members_empty(jo_t* jo);
jo_forward_iterator jo_members_begin(jo_t* jo);
jo_forward_iterator jo_members_end(jo_t* jo);
jo_forward_iterator jo_members_find_iterator(jo_t* jo, const char* key);
jo_forward_iterator jo_iterator_next(jo_forward_iterator it);
jo_forward_iterator jo_iterator_next_key(jo_forward_iterator it, const char* key);

uint32_t jo_array_size(jo_t* jo);
int32_t jo_array_empty(jo_t* jo);
int32_t jo_array_get_int32(jo_t* jo, uint32_t index);
int64_t jo_array_get_int64(jo_t* jo, uint32_t index);
double jo_array_get_double(jo_t* jo, uint32_t index);
char* jo_array_get_string(jo_t* jo, uint32_t index);
jo_t* jo_array_get_object(jo_t* jo, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
