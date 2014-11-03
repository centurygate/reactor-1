/**
# -*- coding:UTF-8 -*-
*/

#ifndef _RB_TREE_H_
#define _RB_TREE_H_

#include "define.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum tag_element_type_t{
	STRING_ELEMENT_TYPE,
	INT64_ELEMENT_TYPE,
}element_type_t;


struct tag_rb_tree_t;

typedef int64_t (*associative_container_cmp_func) (void *,const void*, const void*);

typedef enum tag_rb_tree_node_color_t{
	RED=0,BLACK=1,
}rb_tree_node_color_t;

typedef struct tag_rb_tree_node_t{
	struct tag_rb_tree_node_t *parent;
	void *data;
	rb_tree_node_color_t color;
	struct tag_rb_tree_node_t *left;
	struct tag_rb_tree_node_t *right;
}rb_tree_node_t;

typedef struct tag_rb_tree_t{
	associative_container_cmp_func cmp_fun;
	uint32_t _size;
	rb_tree_node_t* root;
	element_type_t _key_t;
}rb_tree_t;



typedef struct tag_rb_tree_iterator_t{
	void *data;
}rb_tree_iterator_t;

void rb_tree_init(rb_tree_t *tree, element_type_t elem_type, associative_container_cmp_func cmp_func);

rb_tree_iterator_t    rb_tree_find_by_comparator(rb_tree_t *tree, comparator_t cmp_fun, void *data);
rb_tree_node_t*       rb_tree_find(rb_tree_t* tree,const void * key);

rb_tree_node_t*       rb_tree_insert(rb_tree_t* tree, void *data);

void	rb_tree_erase(rb_tree_t *tree, const void * key) ;
void	rb_tree_erase_iterator(rb_tree_t *p_tree, rb_tree_iterator_t iter);
void	rb_tree_erase_node(rb_tree_t *tree, rb_tree_node_t *node)  ;


rb_tree_iterator_t    rb_tree_begin(rb_tree_t *tree);
rb_tree_iterator_t    rb_tree_end(rb_tree_t *tree);
void                  rb_tree_iterator_next(rb_tree_iterator_t *iter);
void*                 rb_tree_iterator_value(rb_tree_iterator_t iter);


void	rb_recursive_delete(rb_tree_node_t *p_node);

int32_t	rb_tree_iterator_equal(rb_tree_iterator_t former, rb_tree_iterator_t latter);

uint32_t rb_tree_size(rb_tree_t *tree);

#ifdef __cplusplus
}
#endif

#endif
