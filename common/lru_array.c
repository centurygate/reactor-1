/**
# -*- coding:UTF-8 -*-
*/

#include <time.h>
#include "lru_array.h"
#include "logger.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define RESERVED_TIMESTAMP 0xff000000u //year 2105
#define EMPTY_TIMESTAMP 0x00ff0000u

void lru_array_initialize(lru_array_t* self, uint32_t capacity,
						uint32_t element_size, uint32_t expire_time, unary_function_t finalizer)
{
	uint32_t i;
	//uint32_t timestamp;

	self->data = (char*)calloc(capacity, element_size);

	self->timestamp = (uint32_t*)malloc(capacity*sizeof*self->timestamp);
	for (i = 0; i < capacity; ++i)
	{
		self->timestamp[i] = EMPTY_TIMESTAMP;
	}

	self->finalizer = finalizer;
	self->element_size = element_size;
	self->capacity = capacity;
	self->expire_time = expire_time;
	self->hit_count = 0;
	self->miss_count = 0;
}

void lru_array_finalize(lru_array_t* self)
{
	int32_t i;

	logger_debug(default_logger, "lru capacity = %u, lur expire_time = %u, lru hit_count = %u, lru miss_count = %u",
		self->capacity, self->expire_time, self->hit_count, self->miss_count);
	
	if (self->finalizer != NULL)
	{
		for (i = 0; i < self->capacity; ++i)
		{
			if (self->timestamp[i] != EMPTY_TIMESTAMP)
			{
				self->finalizer(self->data + i*self->element_size);
			}
		}
	}

	free(self->data);
	self->data = NULL;
	free(self->timestamp);
	self->timestamp = NULL;
}

void lru_array_for_each(lru_array_t* self, unary_function_t unary_function)
{
	int32_t i;
	uint32_t cur_time = time(NULL);

	for (i = 0; i < self->element_size; ++i)
	{
		if (self->timestamp[i] != EMPTY_TIMESTAMP)
		{
			if (self->expire_time == 0
				|| cur_time <= self->expire_time + self->timestamp[i])
			{
				unary_function(self->data + i*self->element_size);
			}
		}
	}
}

uint32_t lru_array_capacity(lru_array_t* self)
{
	return self->capacity;
}

static inline void lru_array_set_timestamp(uint32_t* timestamp, uint32_t cur_time, int32_t is_reserved)
{
	if (*timestamp == RESERVED_TIMESTAMP) //already reserved
	{
	}
	else
	{
		if (is_reserved)
		{
			*timestamp = RESERVED_TIMESTAMP;
		}
		else
		{
			*timestamp = cur_time;
		}
	}
}

static inline void lru_array_force_set_timestamp(uint32_t* timestamp, uint32_t cur_time, int32_t is_reserved)
{
	if (is_reserved)
	{
		*timestamp = RESERVED_TIMESTAMP;
	}
	else
	{
		*timestamp = cur_time;
	}
}

static inline int32_t lru_array_is_expired(lru_array_t* self, uint32_t timestamp, uint32_t cur_time)
{
	//if (timestamp == RESERVED_TIMESTAMP) return 0;
	return timestamp + self->expire_time <= cur_time;
}

static void* lru_array_put_impl(lru_array_t* self, const void* p_target, comparator_t compar, int32_t is_reserved)
{
	char* p_element;
	uint32_t i = 0;
	uint32_t oldest = 0;
	uint32_t cur_time = time(NULL);

	for (i = 0; i < self->capacity; ++i)
	{
		if (self->timestamp[i] != EMPTY_TIMESTAMP)
		{
			p_element = self->data + i*self->element_size;
			if (compar(p_element, p_target) == 0)
			{
				oldest = i;
				if (self->finalizer != NULL) self->finalizer(p_element);
				goto L_FOUND;
			}

			if (self->timestamp[oldest] != EMPTY_TIMESTAMP
				&& self->timestamp[oldest] > self->timestamp[i])
			{
				oldest = i;
			}
		}
		else
		{
			if (self->timestamp[oldest] != EMPTY_TIMESTAMP) oldest = i;
		}
	}

	p_element = self->data + oldest*self->element_size; //remove the oldest element
	if (self->timestamp[oldest] != EMPTY_TIMESTAMP)
	{
		if (!lru_array_is_expired(self, self->timestamp[oldest], cur_time))
		{
			// special requirement: do not remove anyone if not expired,
			// return NULL for cache full
			return NULL;
		}
		else
		{
			if (self->finalizer != NULL) self->finalizer(p_element);
		}
	}

L_FOUND:
	lru_array_set_timestamp(&self->timestamp[oldest], cur_time, is_reserved);
	memcpy(p_element, p_target, self->element_size);
	return (void*)p_element;
}

void* lru_array_put(lru_array_t* self, const void* p_target, comparator_t compar)
{
	return lru_array_put_impl(self, p_target, compar, 0);
}

void* lru_array_put_reserved(lru_array_t* self, const void* p_target, comparator_t compar)
{
	return lru_array_put_impl(self, p_target, compar, 1);
}

void* lru_array_get(lru_array_t* self, const void* key, comparator_t compar)
{
	char* p_element;
	uint32_t i = 0;
	uint32_t cur_time = time(NULL);

	for (; i < self->capacity; ++i)
	{
		p_element = self->data + i*self->element_size;
		if (self->timestamp[i] != EMPTY_TIMESTAMP
			&& compar(p_element, key) == 0)
		{
			if (self->expire_time == 0
				|| !lru_array_is_expired(self, self->timestamp[i], cur_time))
			{
				lru_array_set_timestamp(&self->timestamp[i], cur_time, 0);
				++self->hit_count;
				return p_element;
			}
			else
			{
				++self->miss_count;
				return NULL;
			}
		}
	}
	++self->miss_count;
	return NULL;
}

void lru_array_refresh(lru_array_t* self, const void* key, comparator_t compar)
{
	uint32_t i = 0;

	for (; i < self->capacity; ++i)
	{
		if (self->timestamp[i] != EMPTY_TIMESTAMP
			&& compar(self->data + i*self->element_size, key) == 0)
		{
			self->timestamp[i] = time(NULL);
			break;
		}
	}
}

void lru_array_reserve(lru_array_t* self, const void* key, comparator_t compar)
{
	uint32_t i = 0;

	for (; i < self->capacity; ++i)
	{
		if (self->timestamp[i] != EMPTY_TIMESTAMP
			&& compar(self->data + i*self->element_size, key) == 0)
		{
			self->timestamp[i] = RESERVED_TIMESTAMP;
			break;
		}
	}
}

void lru_array_reset(lru_array_t* self, const void* key, comparator_t compar)
{
	uint32_t i = 0;

	for (; i < self->capacity; ++i)
	{
		if (self->timestamp[i] != EMPTY_TIMESTAMP
			&& compar(self->data + i*self->element_size, key) == 0)
		{
			lru_array_force_set_timestamp(&self->timestamp[i], time(NULL), 0);
			break;
		}
	}
}

int32_t lru_array_remove(lru_array_t* self, const void* key, comparator_t compar)
{
	char* p_element;
	uint32_t i = 0;

	for (; i < self->capacity; ++i)
	{
		p_element = self->data + i*self->element_size;
		if (self->timestamp[i] != EMPTY_TIMESTAMP
			&& compar(p_element, key) == 0)
		{
			if (self->finalizer != NULL)
			{
				self->finalizer(p_element);
			}
			self->timestamp[i] = EMPTY_TIMESTAMP;
			return 0;
		}
	}
	return ERR_NOT_FOUND;
}

#ifdef __cplusplus
}
#endif
