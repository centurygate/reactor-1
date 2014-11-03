/**
# -*- coding:UTF-8 -*-
*/

#ifndef _FORWARD_LIST_H_
#define _FORWARD_LIST_H_

#include "define.h"

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct s_forward_list_node_t
{
	void* data;
	struct s_forward_list_node_t* next;
}forward_list_node_t;

int32_t forward_list_node_initialize(forward_list_node_t* self);
int32_t forward_list_node_finalize(forward_list_node_t* self);

typedef struct s_forward_list_t
{
	forward_list_node_t* head;
	forward_list_node_t* tail;
} forward_list_t;

void forward_list_initialize(forward_list_t* self);
void forward_list_finalize(forward_list_t* self);

int32_t forward_list_empty(forward_list_t* self);
uint32_t forward_list_size(forward_list_t* self);
void forward_list_push_front(forward_list_t* self, void* data);
void forward_list_push_back(forward_list_t* self, void* data);
void forward_list_erase_front(forward_list_t* self);
void forward_list_erase(forward_list_t* self, forward_list_node_t* before_target, forward_list_node_t* target);
forward_list_node_t* forward_list_begin(forward_list_t* self);
forward_list_node_t* forward_list_end(forward_list_t* self);
forward_list_node_t* forward_list_next(forward_list_node_t* it);

#ifdef __cplusplus
}
#endif

#endif
