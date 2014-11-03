/**
# -*- coding:UTF-8 -*-
*/

#include "ordered_map.h"
#include <malloc.h>

int64_t ordered_map_cmpfunc(void *p_container,const void* former, const void* latter)
{
	rb_tree_t* p_tree = (rb_tree_t *)p_container;
	
	const pair_t *pair1 = (const pair_t *)former;
	const pair_t *pair2 = (const pair_t *)latter;
		
	if (p_tree->_key_t == STRING_ELEMENT_TYPE){
		return string_comparator(pair1->_key.str_key,pair2->_key.str_key);
	}
	else{
		return pair1->_key.i64_key - pair2->_key.i64_key;
	}
}

void ordered_map_init(ordered_map_t *p_map,element_type_t key_type)
{
	rb_tree_init(&p_map->m_tree, key_type, ordered_map_cmpfunc);
}

ordered_map_iterator_t    ordered_map_find(ordered_map_t *p_map, const void *p_key)
{
	ordered_map_iterator_t iter;
	pair_t pair;

	if(p_map->m_tree._key_t == STRING_ELEMENT_TYPE){
		pair._key.str_key = (char *)p_key;
	}
	else{
		pair._key.i64_key = *(int64_t *)p_key;
	}
	iter.data = rb_tree_find(&p_map->m_tree,&pair);
	return iter;
}

void ordered_map_erase(ordered_map_t *p_map,const void *p_key)
{
	pair_t pair;

	if(p_map->m_tree._key_t == STRING_ELEMENT_TYPE){
		pair._key.str_key = (char *)p_key;
	}
	else{
		pair._key.i64_key = *(int64_t *)p_key;
	}

	rb_tree_erase(&p_map->m_tree,&pair);
}

void ordered_map_destroy(ordered_map_t *p_map)
{
	if(p_map->m_tree._key_t == STRING_ELEMENT_TYPE){
		ordered_map_iterator_t iter = ordered_map_begin(p_map);
		ordered_map_iterator_t end = ordered_map_end(p_map);

		for(;!ordered_map_iterator_equal(iter,end);ordered_map_iterator_next(&iter)){
			free((char *)ordered_map_iterator_first(iter));
		}
	}
	
	rb_recursive_delete(p_map->m_tree.root);
	p_map->m_tree._size = 0;
}

void ordered_map_insert(ordered_map_t *p_map,const void *p_key, void *p_value)
{
	pair_t* pair = (pair_t *)malloc(sizeof(pair_t));
	pair->_value = p_value;

	if(p_map->m_tree._key_t == STRING_ELEMENT_TYPE){
		pair->_key.str_key = (char *)malloc(strlen(p_key)+1);
		strcpy(pair->_key.str_key,p_key);
	}
	else{
		pair->_key.i64_key = *(int64_t *)p_key;
	}
	rb_tree_insert(&p_map->m_tree,pair);
}


ordered_map_iterator_t ordered_map_erase_iterator(ordered_map_t *p_map, ordered_map_iterator_t iter)
{
	ordered_map_iterator_t ret = iter;
	ordered_map_iterator_next(&ret);
	rb_tree_erase_node(&p_map->m_tree,(rb_tree_node_t *)iter.data);
	return ret;
}


ordered_map_iterator_t ordered_map_begin(ordered_map_t *p_map)
{
	ordered_map_iterator_t ret;
	ret.key_t = p_map->m_tree._key_t;
	ret.data = rb_tree_begin(&p_map->m_tree).data;
	return ret;
}

ordered_map_iterator_t ordered_map_end(ordered_map_t *p_map)
{
	ordered_map_iterator_t ret;
	ret.key_t = p_map->m_tree._key_t;
	ret.data = NULL;
	return ret;
}

int ordered_map_iterator_equal(ordered_map_iterator_t iter1, ordered_map_iterator_t iter2)
{
	return iter1.data == iter2.data;
}

void ordered_map_iterator_next(ordered_map_iterator_t *p_iter)
{
	rb_tree_iterator_next((rb_tree_iterator_t *)p_iter);
}

const void *ordered_map_iterator_first(ordered_map_iterator_t iter){
	rb_tree_node_t *node = (rb_tree_node_t *)iter.data;
	pair_t *pair = (pair_t *)node->data;
	if(iter.key_t == STRING_ELEMENT_TYPE){
		return pair->_key.str_key;
	}
	else{
		return &pair->_key.i64_key;
	}
}


