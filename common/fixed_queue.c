#include <stdlib.h>
#include <string.h>
#include "fixed_queue.h"
#include "error_code.h"

fixed_queue_t* fixed_queue_create(uint32_t unit_size, uint32_t max_unit_num) {
	fixed_queue_t *p_queue;

	void *ptr = malloc(sizeof(*p_queue) + unit_size*(max_unit_num + 1));
	if(ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, sizeof(*p_queue) + unit_size*(max_unit_num + 1));
	p_queue = (fixed_queue_t *)ptr;
	p_queue->unit_size = unit_size;
	p_queue->max_units = max_unit_num;
	p_queue->head_idx = 0;
	p_queue->tail_idx = 0;
	p_queue->data = (char*)ptr + sizeof(*p_queue);

	return p_queue;
}

void fixed_queue_destory(fixed_queue_t* p_queue) {
	if(p_queue) { 
		free(p_queue);
	}
}

int32_t fixed_queue_isempty(fixed_queue_t* p_queue) {
	return p_queue->head_idx == p_queue->tail_idx;
}

int32_t fixed_queue_isfull(fixed_queue_t* p_queue) {
	return p_queue->head_idx == (p_queue->tail_idx + 1) % (p_queue->max_units + 1);
}

int32_t fixed_queue_in(fixed_queue_t* p_queue, void *data) {
	if(fixed_queue_isfull(p_queue)) {
		return EQUEUEFULL;
	}

	memcpy((char*)p_queue->data + p_queue->tail_idx * p_queue->unit_size, 
		data, p_queue->unit_size);
	
	p_queue->tail_idx++;
	p_queue->tail_idx %= (p_queue->max_units + 1);

	return 0;
}

void* fixed_queue_head(fixed_queue_t* p_queue) {
	if(fixed_queue_isempty(p_queue)) {
		return NULL;
	}
	return (char*)p_queue->data + p_queue->head_idx * p_queue->unit_size;
}

void* fixed_queue_out(fixed_queue_t* p_queue) {
	void *ptr;
	
	if(fixed_queue_isempty(p_queue)) {
		return NULL;
	}

	ptr = (char*)p_queue->data + p_queue->head_idx * p_queue->unit_size;
	p_queue->head_idx++;
	p_queue->head_idx %= (p_queue->max_units + 1);

	return ptr;
}

