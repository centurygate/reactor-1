/**
# -*- coding:UTF-8 -*-
*/

#include "forward_list.h"
#include <stdlib.h>

int32_t forward_list_node_initialize(forward_list_node_t* self)
{
	self->data = NULL;
	self->next = NULL;
	return 0;
}

int32_t forward_list_node_finalize(forward_list_node_t* self)
{
	return 0;
}

void forward_list_initialize(forward_list_t* self)
{
	self->head = NULL;
	self->tail = NULL;
}

void forward_list_finalize(forward_list_t* self)
{
	while (self->head != NULL)
	{
		forward_list_node_t* n = self->head;
		self->head = self->head->next;
		forward_list_node_finalize(n);
		free(n);
	}
	self->tail = NULL;
}

int32_t forward_list_empty(forward_list_t* self)
{
	return self->head == NULL;
}

uint32_t forward_list_size(forward_list_t* self)
{
	if (forward_list_empty(self))
	{
		return 0;
	}
	else
	{
		uint32_t cnt = 1;
		forward_list_node_t* n = self->head;
		while (n != self->tail)
		{
			++cnt;
			n = n->next;
		}
		return cnt;
	}
}

void forward_list_push_front(forward_list_t* self, void* data)
{
	forward_list_node_t* n = (forward_list_node_t*)malloc(sizeof*n);
	forward_list_node_initialize(n);
	n->data = data;
	n->next = self->head;
	self->head = n;
	if (self->tail == NULL)
	{
		self->tail = n;
	}
}

void forward_list_push_back(forward_list_t* self, void* data)
{
	forward_list_node_t* n = (forward_list_node_t*)malloc(sizeof*n);
	forward_list_node_initialize(n);
	n->data = data;
	if (self->tail != NULL)
	{
		self->tail->next = n;
		self->tail = n;
	}
	else
	{
		self->head = n;
		self->tail = n;
	}
}

void forward_list_erase_front(forward_list_t* self)
{
	if (self->head != NULL)
	{
		forward_list_node_t* n = self->head;
		self->head = n->next;
		forward_list_node_finalize(n);
		free(n);
		if (self->head == NULL)
		{
			self->tail = NULL;
		}
	}
}

void forward_list_erase(forward_list_t* self, forward_list_node_t* before_target, forward_list_node_t* target)
{
	if (target != self->head)
	{
		if (before_target != NULL && target != NULL)
		{
			before_target->next = target->next;
			if (target == self->tail)
			{
				self->tail = before_target;
			}
			forward_list_node_finalize(target);
			free(target);
		}
	}
	else
	{
		forward_list_erase_front(self);
	}
}

forward_list_node_t* forward_list_begin(forward_list_t* self)
{
	return self->head;
}

forward_list_node_t* forward_list_end(forward_list_t* self)
{
	return NULL;
}

forward_list_node_t* forward_list_next(forward_list_node_t* it)
{
	return it->next;
}
