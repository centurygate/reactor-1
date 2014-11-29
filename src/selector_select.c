#ifdef LINUX
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "selector.h"
#include "error_code.h"

selector_t *create_selector(int32_t selector_size) {
	int i;
	selector_t *p_selector;

	p_selector = (selector_t*)malloc(sizeof(*p_selector));
	if(p_selector == NULL) {
		return NULL;
	}

	memset(p_selector, 0, sizeof(*p_selector));
	p_selector->max_size = selector_size;
	p_selector->p_events = 
		(selector_event_t*)malloc(selector_size*sizeof(*p_selector->p_events));
	if(p_selector->p_events == NULL) {
		free(p_selector);
		return NULL;
	}
	p_selector->pp_data = (void**)malloc(selector_size*sizeof(void*));
	if(p_selector->pp_data == NULL) {
		free(p_selector->p_events);
		free(p_selector);
		return NULL;
	}
	
	for(i = 0; i < selector_size; i++) {
		p_selector->p_events[i].p_handle = NULL;
		p_selector->p_events[i].events = p_selector->p_events[i].revents = 0;
		if(i == selector_size - 1) {
			p_selector->pp_data[i] = (void*)-1;
		}
		else {
			p_selector->pp_data[i] = (void*)(i + 1);
		}
	}
	p_selector->free_idx = 0;
	p_selector->tail_idx = 0;

	return p_selector;
}

void destroy_selector(selector_t *p_selector) {
	free(p_selector->p_events);
	free(p_selector->pp_data);
	free(p_selector);
}

int32_t selector_add(struct selector *p_selector, handle_t *p_handle, 
		int32_t expected_events, void *p_data) {
	int free_index;
	int32_t next_free_idx;

	if(p_selector->free_idx == -1) {
		return ESELECTORFULL;
	}
	
	free_index = p_selector->free_idx;
	next_free_idx = (int32_t)p_selector->pp_data[free_index];

	p_selector->p_events[free_index].p_handle = p_handle;
	p_selector->p_events[free_index].events = expected_events;
	p_selector->p_events[free_index].revents = 0;
	p_handle->id = free_index;
	p_selector->pp_data[free_index] = p_data;

	p_selector->free_idx = next_free_idx;

	if(free_index > p_selector->tail_idx) {
		p_selector->tail_idx = free_index;
	}
	return 0;
}

int32_t selector_mod(struct selector *p_selector, handle_t *p_handle, 
		int32_t expected_events, void *p_data) {
	int32_t i = p_handle->id;
	
	if(i == -1) {
		return ESELECTORNOENT;
	}

	p_selector->p_events[i].events = expected_events;

	return 0;
}

int32_t selector_del(struct selector *p_selector, handle_t *p_handle) {
	int32_t i = p_handle->id;

	if(i == -1) {
		return ESELECTORNOENT;
	}

	p_selector->p_events[i].p_handle = NULL;
	p_selector->p_events[i].events = 0;
	p_selector->p_events[i].revents = 0;
	p_selector->pp_data[i] = (void*)p_selector->free_idx;
	p_selector->free_idx = i;

	if(p_selector->tail_idx == i) {
		int32_t j;
		for(j = p_selector->tail_idx - 1; j > 0; j--) {
			if(p_selector->p_events[j].p_handle != NULL) {
				break;
			}
		}
		p_selector->tail_idx = j + 1; 
	}

	return 0;
}

int32_t selector_wait(struct selector *p_selector, int32_t timeout /* ms */) {
	int32_t i;
	int ret;
	struct timeval tv;

	fd_set rset;
	fd_set wset;
	fd_set eset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);
	
	p_selector->p_selector_handle = NULL;
	for(i = 0; i <= p_selector->tail_idx; i++) {
		if(p_selector->p_events[i].p_handle == NULL) {
			continue;
		}
		if(p_selector->p_selector_handle == NULL) {
			p_selector->p_selector_handle = p_selector->p_events[i].p_handle;
		}
		if(p_selector->p_events[i].p_handle->os_handle.fd > 
				p_selector->p_selector_handle->os_handle.fd) {
			p_selector->p_selector_handle = p_selector->p_events[i].p_handle;
		}
		if(p_selector->p_events[i].events & SELECTORIN) {
			FD_SET(p_selector->p_events[i].p_handle->os_handle.fd, &rset);
		}
		if(p_selector->p_events[i].events & SELECTOROUT) {
			FD_SET(p_selector->p_events[i].p_handle->os_handle.fd, &wset);
		}
		FD_SET(p_selector->p_events[i].p_handle->os_handle.fd, &eset);
	}

	if(p_selector->p_selector_handle == NULL) {
		usleep(timeout*1000);
		return 0;
	}
	
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	do 
	{
		ret = select(p_selector->p_selector_handle->os_handle.fd + 1,
			&rset, &wset, &eset, &tv);
	} while (ret == -1 && errno == EINTR);

	if(ret == -1) {
		return -errno;
	}

	if(ret > 0) {
		for(i = 0; i <= p_selector->tail_idx; i++) {
			if(p_selector->p_events[i].p_handle == NULL) {
				continue;
			}
			p_selector->p_events[i].revents = 0;
			if(FD_ISSET(p_selector->p_events[i].p_handle->os_handle.fd, &rset)) {
				p_selector->p_events[i].revents |= SELECTORIN;
			}
			if(FD_ISSET(p_selector->p_events[i].p_handle->os_handle.fd, &wset)) {
				p_selector->p_events[i].revents |= SELECTOROUT;
			}
			if(FD_ISSET(p_selector->p_events[i].p_handle->os_handle.fd, &eset)) {
				p_selector->p_events[i].revents |= SELECTORERR;
			}
		}
	}

	return ret;
}

int32_t selector_next(struct selector *p_selector, int32_t idx) { // return idx
	
	idx++;
	while(idx <= p_selector->tail_idx) {
		if(p_selector->p_events[idx].p_handle != NULL) {
			return idx;
		}
		idx++;
	}

	return -1;
}

int32_t selector_isvalid(struct selector *p_selector, int32_t idx) {
	return (p_selector->p_events[idx].p_handle != NULL);
}

int32_t selector_iserror(struct selector *p_selector, int32_t idx) {
	return (int32_t)(p_selector->p_events[idx].revents & SELECTORERR);
}

int32_t selector_isreadable(struct selector *p_selector, int32_t idx) {
	return (int32_t)(p_selector->p_events[idx].revents & SELECTORIN);
}

int32_t selector_iswriteable(struct selector *p_selector, int32_t idx) {
	return (int32_t)(p_selector->p_events[idx].revents & SELECTOROUT);
}

void *selector_data(struct selector *p_selector, int32_t idx) {
	return p_selector->pp_data[idx];
}
#endif //LINUX

