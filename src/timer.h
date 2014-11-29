#ifndef _TIMER_20130718
#define _TIMER_20130718

#include "define.h"

struct timer_container;

typedef void (*timer_cb_t)(struct timer_container* p_timer_container, int32_t timerid, void *args);

typedef struct timer_container {
	int32_t size;
	int32_t free_idx;
	uint32_t *expire_times; // s
	void** data;
	int32_t *lock_by_inner;
	timer_cb_t *cbs; // callbacks
} timer_container_t;

timer_container_t* create_timer_container(int32_t max_timer_num);

void destroy_timer_container(timer_container_t* p_timer_container);

#define add_timer(a, b, c, d) __add_timer( (a), (b), (c), (d), __func__, __LINE__)
int32_t __add_timer(timer_container_t* p_timer_container, timer_cb_t cb, void *args, uint32_t timeout /* s */,
	const char* caller, int32_t line);

#define del_timer(a, b) __del_timer( (a), (b), __func__, __LINE__)
int32_t __del_timer(timer_container_t* p_timer_container, int32_t timerid,
	const char* caller, int32_t line);

void timer_container_handle_events(timer_container_t* p_timer_container);

void* timer_get_args(timer_container_t* p_timer_container, int32_t timerid);

#endif //_TIMER_20130718
