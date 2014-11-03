/**
# -*- coding:UTF-8 -*-
*/

#ifndef _SET_H_
#define _SET_H_

#include "rb_tree.h"

#ifdef __cplusplus
extern "C" 
{
#endif

typedef rb_tree_iterator_t ordered_set_iterator_t;

typedef struct tag_ordered_set_t{
	rb_tree_t tree;
}ordered_set_t;

void ordered_set_init(ordered_set_t *p_set, element_type_t elem_type);
#define ordered_set_size(p_set) rb_tree_size(p_set->tree)

ordered_set_iterator_t ordered_set_find(ordered_set_t *p_set, const void *p_data);
const void *   ordered_set_iterator_value(ordered_set_iterator_t iter);

void ordered_set_insert(ordered_set_t *p_set, const void *p_data);
void ordered_set_erase(ordered_set_t *p_set, const void *p_data);

ordered_set_iterator_t ordered_set_erase_iterator(ordered_set_t *set, ordered_set_iterator_t iterator);

void ordered_set_clear(ordered_set_t *set);

#define ordered_set_begin(p_set) rb_tree_begin(&p_set->tree)
#define ordered_set_end(p_set)   rb_tree_end(&p_set->tree)
#define ordered_set_iterator_next(p_iter) rb_tree_iterator_next(p_iter)
#define ordered_set_iterator_equal(iter1,iter2) rb_tree_iterator_equal(iter1,iter2)


#ifdef __cplusplus
}
#endif

#endif
