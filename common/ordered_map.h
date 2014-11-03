/**
# -*- coding:UTF-8 -*-
*/

#ifndef _MAP_H_
#define _MAP_H_

#include "rb_tree.h"

#ifdef __cplusplus
extern "C" 
{
#endif


typedef struct tag_ordered_map_iterator_t{
	void *data;
	element_type_t key_t;
}ordered_map_iterator_t;


typedef struct tag_ordered_map_t {
	rb_tree_t m_tree;
}ordered_map_t;

typedef union  tag_union_key_t{
	int64_t i64_key;
	char * str_key;
}union_key_t;

typedef struct tag_pair_t{
    union_key_t  _key;
	void*      _value;
}pair_t;

void ordered_map_init(ordered_map_t *p_map,element_type_t elem_type);
/**
 * Clear all the nodes
 */
void ordered_map_destroy(ordered_map_t *p_map);

uint32_t ordered_map_size(ordered_map_t *p_map);

#define ordered_map_iterator_second(iter) ((pair_t *)(((rb_tree_node_t *)iter.data)->data))->_value
const void *ordered_map_iterator_first(ordered_map_iterator_t iter);

ordered_map_iterator_t    ordered_map_find(ordered_map_t *p_map, const void *p_key);

/**
 * If an element with the same key already exists in the map,
 * invoking ordered_map_insert_pair takes no effect.
 */
void ordered_map_insert_pair(ordered_map_t *p_map,pair_t *p_pair);

/**
 * If an element with the same key already exists in the map,
 * invoking ordered_map_insert takes no effect.
 */
void ordered_map_insert(ordered_map_t *p_map, const void *key, void *value);


void ordered_map_erase(ordered_map_t *p_map, const void *p_key);

ordered_map_iterator_t ordered_map_erase_iterator(ordered_map_t *p_map, ordered_map_iterator_t iter);


ordered_map_iterator_t ordered_map_begin(ordered_map_t *p_map);

ordered_map_iterator_t ordered_map_end(ordered_map_t *p_map);

int ordered_map_iterator_equal(ordered_map_iterator_t iter1, ordered_map_iterator_t iter2);

void ordered_map_iterator_next(ordered_map_iterator_t *p_iter);

#ifdef __cplusplus
}
#endif

#endif
