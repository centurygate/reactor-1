#ifndef _QUEUE_H_00138F8F2E70_200806260949
#define _QUEUE_H_00138F8F2E70_200806260949

#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct fixed_queue {
	int32_t unit_size;
	int32_t max_units;
	int32_t head_idx;
	int32_t tail_idx;
	void* data;
} fixed_queue_t;

fixed_queue_t* fixed_queue_create(uint32_t unit_size, uint32_t max_unit_num);

void fixed_queue_destory(fixed_queue_t* p_queue);

int32_t fixed_queue_isempty(fixed_queue_t* p_queue);

int32_t fixed_queue_isfull(fixed_queue_t* p_queue);

void* fixed_queue_head(fixed_queue_t* p_queue);

int32_t fixed_queue_in(fixed_queue_t* p_queue, void *data);

void* fixed_queue_out(fixed_queue_t* p_queue);


#ifdef __cplusplus
}
#endif

#endif
