/**
# -*- coding:UTF-8 -*-
*/

#include "double_list.h"
#include <malloc.h>

void double_list_init(double_list_t *p_list)
{
	p_list->_head = NULL;
	p_list->_list_size = 0;
	p_list->_tail = NULL;
}

void double_list_insert_before_iterator(double_list_t *p_list, void *p_data, double_list_iterator_t *iter)
{
	double_list_node_t *p_successor = (double_list_node_t *)iter->list_data;

	if(p_successor==NULL){
		double_list_push_back(p_list,p_data);
	}
	else{
		double_list_node_t *p_new = (double_list_node_t *)malloc(sizeof(double_list_node_t));
		p_new->_data = p_data;
		p_new->_next = p_successor;
		p_new->_prev = p_successor->_prev;

		if(p_new->_prev == NULL){
			p_list->_head = p_new;
		}
		else{
			p_new->_prev->_next = p_new;
		}
		
		p_successor->_prev = p_new;
	
		p_list->_list_size+=1;
	}
}

void double_list_erase_node(double_list_t *p_list, double_list_node_t *p_erase)
{
	if(p_erase->_prev==NULL && p_erase->_next==NULL){
 		p_list->_head = NULL;
		p_list->_tail = NULL;
	}
	else if(p_erase->_prev==NULL){
		p_list->_head = p_erase->_next;
		p_list->_head->_prev = NULL;
	}
	else if(p_erase->_next==NULL){
		p_list->_tail = p_erase->_prev;
		p_list->_tail->_next = NULL;
	}
	else{
		p_erase->_prev->_next = p_erase->_next;
		p_erase->_next->_prev = p_erase->_prev;
	}

	free(p_erase);
	p_list->_list_size-=1;
}

void double_list_erase(double_list_t *p_list, double_list_iterator_t erase_node)
{
	double_list_erase_node(p_list,(double_list_node_t *)erase_node.list_data);
}

void double_list_erase_by_comparator(double_list_t *p_list, comparator_t cmp_fun, const void *data)
{
	double_list_node_t* p_node = p_list->_head;

	while(p_node != NULL){
		if((*cmp_fun)(data,p_node->_data)==0){
			double_list_erase_node(p_list,p_node);
			break;
		}

		p_node = p_node->_next;
	}
}


uint32_t double_list_size(const double_list_t *p_list)
{
	return p_list->_list_size;
}


void* double_list_pop_front(double_list_t *p_list)
{
	void *p_data = NULL;

	if(p_list->_head!=NULL){
		p_data = p_list->_head->_data;
		p_list->_head = p_list->_head->_next;
		if(p_list->_head==NULL){
			p_list->_tail = NULL;
		}
		else{
			free(p_list->_head->_prev);
			p_list->_head->_prev = NULL;
		}
		p_list->_list_size-=1;
	}
	
	return p_data;
}

void* list_pop_back(double_list_t *p_list)
{
	void *p_data = NULL;

	if(p_list->_tail!=NULL){
		p_data = p_list->_tail->_data;
		p_list->_tail = p_list->_tail->_prev;
		if(p_list->_tail==NULL){
			p_list->_head = NULL;
		}
		else{
			free(p_list->_tail->_next);
			p_list->_tail->_next= NULL;
		}
		p_list->_list_size-=1;
	}
	
	return p_data;
}


void double_list_push_front(double_list_t *p_list, void *p_data)
{
	double_list_node_t *p_node = (double_list_node_t *)malloc(sizeof(double_list_node_t));
	p_node->_data = p_data;
	p_node->_next = p_list->_head;
	p_node->_prev = NULL;

	if(p_list->_list_size>0){
		p_list->_head->_prev = p_node;
		p_list->_head = p_node;
	}
	else{
		p_list->_head = p_node;
		p_list->_tail = p_node;
	}
	p_list->_list_size+=1;
}

void double_list_push_back(double_list_t *p_list, void *p_data)
{
	double_list_node_t *p_node = (double_list_node_t *)malloc(sizeof(double_list_node_t));
	p_node->_data = p_data;
	p_node->_next = NULL;
	p_node->_prev = p_list->_tail;
	
	if(p_list->_list_size>0){
		p_list->_tail->_next = p_node;
		p_list->_tail = p_node;
	}
	else{
		p_list->_tail = p_node;
		p_list->_head = p_node;
	}
	
	p_list->_list_size+=1;
}



void double_list_clear(double_list_t *p_list)
{
	double_list_node_t *p_node = p_list->_head;
	double_list_node_t *p_del;
	
	while(p_node != NULL){
		p_del = p_node;
		p_node = p_node->_next;
		free(p_del);
	}
	p_list->_head = NULL;
	p_list->_tail = NULL;
	p_list->_list_size = 0;
}


double_list_iterator_t double_list_begin(double_list_t *p_list)
{
	double_list_iterator_t iter;
	iter.list_data = p_list->_head;
	return iter;
}

double_list_iterator_t double_list_end(double_list_t *p_list)
{
	double_list_iterator_t iter;
	iter.list_data = NULL;
	return iter;
}

void double_list_iterator_next(double_list_iterator_t *iter)
{
	iter->list_data = ((double_list_node_t *)iter->list_data)->_next;
}

void *double_list_iterator_value(double_list_iterator_t iter)
{
	return iter.list_data;
}

int32_t double_list_iterator_equal(double_list_iterator_t iter1,double_list_iterator_t iter2)
{
	return iter1.list_data == iter2.list_data;
}
