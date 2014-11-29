#ifndef SELECTOR_20130716
#define SELECTOR_20130716

#include "define.h"
#include "handle.h"

#define SELECTORIN  0x1
#define SELECTOROUT 0x2
#define SELECTORERR 0x4

typedef struct selector_event{
	handle_t*			p_handle;
	uint16_t 				events;
	uint16_t 				revents;
} selector_event_t;

typedef struct selector {
	handle_t*			p_selector_handle;
	int32_t 				max_size;
	selector_event_t*	p_events;
	void**					pp_data;
	int32_t					free_idx;
	int32_t					tail_idx;
} selector_t;

selector_t *create_selector(int32_t selector_size);

void destroy_selector(selector_t *p_selector);

int32_t selector_add(struct selector *p_selector, handle_t *p_handle, 
		int32_t expected_events, void *p_data);

int32_t selector_mod(struct selector *p_selector, handle_t *p_handle, 
		int32_t expected_events, void *p_data);

int32_t selector_del(struct selector *p_selector, handle_t *p_handle);

int32_t selector_wait(struct selector *p_selector, int32_t timeout /* ms */);

int32_t selector_next(struct selector *p_selector, int32_t idx); // return idx

int32_t selector_isvalid(struct selector *p_selector, int32_t idx);

int32_t selector_iserror(struct selector *p_selector, int32_t idx);

int32_t selector_isreadable(struct selector *p_selector, int32_t idx);

int32_t selector_iswriteable(struct selector *p_selector, int32_t idx);

void *selector_data(struct selector *p_selector, int32_t idx);

#endif //SELECTOR_20130716
