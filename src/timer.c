#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "timer.h"
#include "error_code.h"
#include "logger.h"

// to implement this via min heap

timer_container_t* create_timer_container(int32_t max_timer_num) {
	int i;

	timer_container_t* p_timer_container = (timer_container_t*)malloc(sizeof(*p_timer_container));
	if(p_timer_container == NULL) {
		abort();
	}
	p_timer_container->size = max_timer_num;

	p_timer_container->expire_times = (uint32_t*)malloc(sizeof(uint32_t)*max_timer_num);
	if(p_timer_container->expire_times == NULL) {
		abort();
	}
	memset(p_timer_container->expire_times, 0, sizeof(uint32_t)*max_timer_num);

	p_timer_container->data = (void**)malloc(sizeof(void*)*max_timer_num);
	if(p_timer_container->data == NULL) {
		abort();
	}
	memset(p_timer_container->data, 0, sizeof(void*)*max_timer_num);

	p_timer_container->lock_by_inner = (int32_t*)malloc(sizeof(int32_t)*max_timer_num);
	if(p_timer_container->lock_by_inner == NULL) {
		abort();
	}
	memset(p_timer_container->lock_by_inner, 0, sizeof(int32_t)*max_timer_num);
	
	p_timer_container->cbs = (timer_cb_t*)malloc(sizeof(timer_cb_t)*max_timer_num);
	if(p_timer_container->cbs == NULL) {
		abort();
	}
	memset(p_timer_container->cbs, 0, sizeof(timer_cb_t)*max_timer_num);

	for(i = 0; i < max_timer_num - 1; i++) {
		p_timer_container->data[i] = (void*)(i + 1);
	}
	p_timer_container->data[i] = (void*)-1;

	p_timer_container->free_idx = 0;

	return p_timer_container;
}

void destroy_timer_container(timer_container_t* p_timer_container) {
	free(p_timer_container->expire_times);
	free(p_timer_container->data);
	free(p_timer_container->lock_by_inner);
	free(p_timer_container->cbs);
	free(p_timer_container);
}

int32_t __add_timer(timer_container_t* p_timer_container, timer_cb_t cb, void *args, uint32_t timeout /* s */, 
	const char* caller, int32_t line) {
	uint32_t now_time_s;
	int32_t free_idx = p_timer_container->free_idx;
	int32_t next_free_idx;

	if(free_idx == -1) {
		logger_error(default_logger, "%s:%d add timer=%p error, %p full", caller, line, cb, p_timer_container);
		return ETIMERFULL;
	}
	
	next_free_idx = (int32_t)p_timer_container->data[free_idx];
	p_timer_container->cbs[free_idx] = cb;
	p_timer_container->data[free_idx] = args;

	now_time_s = time(NULL);
	p_timer_container->expire_times[free_idx] = now_time_s + timeout;
	logger_debug(default_logger, "%s:%d add timer[%d]=%p into %p, timeout=%u, will execute at %u", 
		caller, line, free_idx, cb, p_timer_container, timeout, now_time_s + timeout);

	p_timer_container->free_idx = next_free_idx;
	
	return free_idx;
}

int32_t __del_timer(timer_container_t* p_timer_container, int32_t timerid, const char* caller, int32_t line) {
	logger_assert(timerid >= -1);
	if(timerid < 0) {
		logger_error(default_logger, "%s:%d del timer[%d] from %p error", caller, line, timerid, p_timer_container);
		return ETIMERINVALID;
	}
	
	logger_assert(timerid < p_timer_container->size);
	if(timerid >= p_timer_container->size) {
		logger_error(default_logger, "%s:%d del timer[%d] from %p error", caller, line, timerid, p_timer_container);
		return ETIMERINVALID;
	}
	
	if(p_timer_container->cbs[timerid] == NULL) {
		logger_error(default_logger, "%s:%d del timer[%d] from %p error ETIMERNOENT", caller, line, timerid, p_timer_container);
		return ETIMERNOENT;
	}

	if(p_timer_container->lock_by_inner[timerid] == 1) {
		logger_error(default_logger, "%s:%d del timer[%d] from %p error IGNORE", caller, line, timerid, p_timer_container);
		return 0;
	}
	
	logger_debug(default_logger, "%s:%d del timer[%d] from %p", caller, line, timerid, p_timer_container);
	p_timer_container->cbs[timerid] = NULL;

	p_timer_container->data[timerid] = (void*)p_timer_container->free_idx;

	p_timer_container->free_idx = timerid;

	return 0;
}

void timer_container_handle_events(timer_container_t* p_timer_container) {
	uint32_t now_time_s;
	int32_t i;

	for(i = 0; i < p_timer_container->size; i++) {
		if(p_timer_container->cbs[i] == NULL) {
			continue;
		}
		now_time_s = time(NULL);
		if(p_timer_container->expire_times[i] <= now_time_s) {
			p_timer_container->lock_by_inner[i] = 1;
			p_timer_container->cbs[i](p_timer_container, i, p_timer_container->data[i]);
			p_timer_container->lock_by_inner[i] = 0;
			logger_debug(default_logger, "timer[%d]:%p, executed, execute at now=%u, expected time:%u",
				i, p_timer_container->cbs[i], now_time_s, p_timer_container->expire_times[i]);
			del_timer(p_timer_container, i);
		}
	}
}

void* timer_get_args(timer_container_t* p_timer_container, int32_t timerid)
{
	return p_timer_container->data[timerid];
}

