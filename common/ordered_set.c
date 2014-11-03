/**
# -*- coding:UTF-8 -*-
*/

#include "ordered_set.h"

//implement

int64_t ordered_set_cmpfunc(void *p_container, const void* former,const void* latter)
{
	ordered_set_t *p_set = (ordered_set_t *)p_container;
	if(p_set->tree._key_t == STRING_ELEMENT_TYPE){
		return string_comparator(former,latter);
	}
	else{

		return *(int64_t *)former - *(int64_t *)latter;
	}
}

void ordered_set_init(ordered_set_t *p_set, element_type_t elem_type)
{
	rb_tree_init(&p_set->tree, elem_type, ordered_set_cmpfunc);
}

ordered_set_iterator_t ordered_set_erase_iterator(ordered_set_t *set, ordered_set_iterator_t iter)
{
	ordered_set_iterator_t ret = iter;
	rb_tree_iterator_next(&ret);
	rb_tree_erase(&set->tree,iter.data);
	return ret;
}

void ordered_set_clear(ordered_set_t *set)
{
	rb_recursive_delete(set->tree.root);
	set->tree._size= 0;
}

ordered_set_iterator_t ordered_set_find(ordered_set_t *p_set, const void *p_data)
{
	ordered_set_iterator_t ret;
	ret.data = rb_tree_find(&p_set->tree,p_data);
	return ret;
}

const void* ordered_set_iterator_value(ordered_set_iterator_t iter)
{
	rb_tree_node_t *node = (rb_tree_node_t *)iter.data;
	return node->data;
}

void ordered_set_insert(ordered_set_t *p_set, const void *p_data)
{
	if(p_set->tree._key_t == STRING_ELEMENT_TYPE){
		char *p_str = (char *)malloc(strlen((const char *)p_data)+1);
		strcpy(p_str,(const char *)p_data);
		rb_tree_insert(&p_set->tree,p_str);
	}
	else{
		int64_t *p_int = (int64_t *)malloc(sizeof(int64_t));
		*p_int = *(int64_t *)p_data;
		rb_tree_insert(&p_set->tree,p_int);
	}
}

void ordered_set_erase(ordered_set_t *p_set, const void *p_data)
{
	rb_tree_erase(&p_set->tree,p_data);
}

