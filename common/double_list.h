/**
# -*- coding:UTF-8 -*-
*/

#ifndef _LIST_H_
#define _LIST_H_

#include "define.h"

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct tag_double_list_node_t{
	void *_data;
	struct tag_double_list_node_t *_prev;
	struct tag_double_list_node_t *_next;
}double_list_node_t;

typedef struct tag_double_list_t{
	double_list_node_t *_head;
	double_list_node_t *_tail;
	uint32_t _list_size;
} double_list_t;


typedef struct tag_double_list_iterator_t{
	void *list_data;
}double_list_iterator_t;


void double_list_init(double_list_t *p_list);


void double_list_insert_before_iterator(double_list_t *p_list, void *data, double_list_iterator_t *iter);

void double_list_erase(double_list_t *p_list, double_list_iterator_t erase_node);
void double_list_erase_by_comparator(double_list_t *p_list, comparator_t cmp_fun, const void *data);


uint32_t double_list_size(const double_list_t *list);


void* double_list_pop_front(double_list_t *list);
void* double_list_pop_back(double_list_t *list);

void double_list_push_front(double_list_t *p_list, void *data);
void double_list_push_back(double_list_t *p_list, void *data);



void double_list_clear(double_list_t *p_list);


double_list_iterator_t double_list_begin(double_list_t *p_list);
double_list_iterator_t double_list_end(double_list_t *p_list);
void double_list_iterator_next(double_list_iterator_t *iter);

void*	double_list_iterator_value(double_list_iterator_t iter);
int32_t	double_list_iterator_equal(double_list_iterator_t iter1,double_list_iterator_t iter2);

#ifdef __cplusplus
}
#endif

#endif
