/**
# -*- coding:UTF-8 -*-
*/

#include "dynamic_array.h"
#include <assert.h>

void dynamic_array_initialize(dynamic_array_t* self, uint32_t element_size)
{
	assert(element_size != 0);
	self->capacity = 0;
	self->data = NULL;
	self->element_size = element_size;
	self->size = 0;
}

void dynamic_array_finalize(dynamic_array_t* self)
{
	free(self->data);
	self->capacity = 0;
	self->data = NULL;
	self->element_size = 0;
	self->size = 0;
}

uint32_t dynamic_array_size(dynamic_array_t* self)
{
	return self->size;
}

int32_t dynamic_array_empty(dynamic_array_t* self)
{
	return self->size == 0;
}

static int32_t dynamic_array_enlarge(dynamic_array_t* self, uint32_t new_size)
{
	char* p = (char*)realloc(self->data, new_size*self->element_size);
	assert(self->element_size != 0);
	if (p != NULL)
	{
		self->data = p;
		self->capacity = new_size;
		return 0;
	}
	else
	{
		assert(0);
		return ERR_ETM_OUT_OF_MEMORY;
	}
}

int32_t dynamic_array_resize(dynamic_array_t* self, uint32_t new_size)
{
	if (self->capacity < new_size)
	{
		int32_t ret = dynamic_array_enlarge(self, new_size + 1);
		if (ret != 0)
		{
			return ret;
		}
	}
	self->size = new_size;
	return 0;
}

int32_t dynamic_array_reserve(dynamic_array_t* self, uint32_t new_size)
{
	if (self->capacity < new_size)
	{
		return dynamic_array_enlarge(self, new_size + 1);
	}
	else
	{
		return 0;
	}
}

void* dynamic_array_at(dynamic_array_t* self, uint32_t index)
{
	if (index < self->size)
	{
		return self->data + index * self->element_size;
	}
	else return NULL;
}

void* dynamic_array_data(dynamic_array_t* self)
{
	return self->data;
}

void dynamic_array_attach_buffer(dynamic_array_t* self, void* buffer, uint32_t buffer_size)
{
	dynamic_array_clear(self);
	free(self->data);
	self->size = buffer_size;
	self->data = (char*)buffer;
	self->capacity = buffer_size;
}

void* dynamic_array_detach_buffer(dynamic_array_t* self)
{
	void* buf = self->data;
	self->capacity = 0;
	self->data = NULL;
	self->size = 0;
	return buf;
}

void dynamic_array_swap(dynamic_array_t* self, dynamic_array_t* other)
{
	dynamic_array_t tmp;
	tmp = *self;
	*self = *other;
	*other = tmp;
}

void dynamic_array_copy(dynamic_array_t* self, const dynamic_array_t* other)
{
	dynamic_array_finalize(self);
	*self = *other;
}

int32_t dynamic_array_clone(dynamic_array_t* self, const dynamic_array_t* other)
{
	//char* buf;
	//uint32_t buf_size;

	dynamic_array_clear(self);
	//assert(self->element_size == other->element_size);
	//self->capacity = other->size;
	if(other->size == 0) return 0;

	if(other->size > self->capacity)
	{
		int32_t ret = dynamic_array_enlarge(self, other->size);
		if(ret != 0) return ret;
	}

	self->size = other->size;
	memcpy(self->data, other->data, self->size*self->element_size);
	return 0;
}

void dynamic_array_clear(dynamic_array_t* self)
{
	self->size = 0;
}

int32_t dynamic_array_push_back(dynamic_array_t* self, const void* p_target)
{
	if (self->size == self->capacity)
	{
		int32_t ret = dynamic_array_enlarge(self, (self->size + 1) * 2);
		if (ret != 0)
		{
			return ret;
		}
	}
	memcpy(self->data + self->size * self->element_size, p_target, self->element_size);
	++self->size;
	return 0;
}

void dynamic_array_pop_back(dynamic_array_t* self)
{
	if (self->size != 0)
	{
		--self->size;
	}
}

void* dynamic_array_back(dynamic_array_t* self)
{
	if (self->size != 0)
	{
		return self->data + (self->size - 1) * self->element_size;
	}
	else return NULL;
}

void* dynamic_array_front(dynamic_array_t* self)
{
	if (self->size != 0)
	{
		return self->data;
	}
	else return NULL;
}

int32_t dynamic_array_append(dynamic_array_t* self, const void* p_target, uint32_t n)
{
	if (self->size + n >= self->capacity)
	{
		int32_t ret = dynamic_array_enlarge(self, (self->size + n + 1) * 2);
		if (ret != 0)
		{
			return ret;
		}
	}
	memcpy(self->data + self->size * self->element_size, p_target, n*self->element_size);
	self->size += n;
	return 0;
}

int32_t dynamic_array_insert(dynamic_array_t* self, uint32_t index, const void* p_target, uint32_t n)
{
	if (self->size + n >= self->capacity)
	{
		int32_t ret = dynamic_array_enlarge(self, (self->size + n + 1) * 2);
		if (ret != 0)
		{
			return ret;
		}
	}
	memmove(self->data + (index + n)*self->element_size,
			self->data + index*self->element_size,
			(self->size-index)*self->element_size);
	memcpy(self->data + index*self->element_size, p_target, n*self->element_size);
	self->size += n;
	return 0;
}

/* cmp(self[i], p_target) */
static int32_t dynamic_array_bsearch_index(dynamic_array_t* self, const void* p_target, comparator_t cmp, int32_t* p_result)
{
	int32_t mid = 0;
	int32_t start = 0;
	int32_t end = (int32_t)self->size - 1;
	while (start <= end)
	{
		int32_t ret;
		mid = (start + end) / 2;
		ret = cmp(self->data + mid*self->element_size, p_target);
		if (ret < 0)
		{
			start = mid + 1;
		}
		else if (ret > 0)
		{
			end = mid - 1;
		}
		else
		{
			do ++mid;
			while (mid < self->size && cmp(self->data + mid*self->element_size, p_target) == 0);
			*p_result = mid - 1;
			return 0;
		}
	}
	*p_result = end;
	return 1;
}

/* cmp(p_target, self[i]) */
static int32_t dynamic_array_bsearch_index_reverse(dynamic_array_t* self, const void* p_target, comparator_t cmp, int32_t* p_result)
{
	int32_t mid = 0;
	int32_t start = 0;
	int32_t end = (int32_t)self->size - 1;
	while (start <= end)
	{
		int32_t ret;
		mid = (start + end) / 2;
		ret = cmp(p_target, self->data + mid*self->element_size);
		if (ret < 0)
		{
			end = mid - 1;
		}
		else if (ret > 0)
		{
			start = mid + 1;
		}
		else
		{
			do ++mid;
			while (mid < self->size && cmp(p_target, self->data + mid*self->element_size) == 0);
			*p_result = mid - 1;
			return 0;
		}
	}
	*p_result = end;
	return 1;
}

int32_t dynamic_array_insert_by_comparator(dynamic_array_t* self, const void* p_target, comparator_t cmp)
{
	int32_t index;
	dynamic_array_bsearch_index(self, p_target, cmp, &index);
	return dynamic_array_insert(self, index+1, p_target, 1);
}

void dynamic_array_sort(dynamic_array_t* self, comparator_t cmp)
{
	qsort(self->data, self->size, self->element_size, cmp);
}

void* dynamic_array_search(dynamic_array_t* self, void* p_target, comparator_t cmp)
{
	char* p = self->data;
	uint32_t i = 0;
	for (; i < self->size; ++i)
	{
		if (cmp(p, p_target) == 0)
		{
			return p;
		}
		p += self->element_size;
	}
	return NULL;
}

/* cmp(self[i], p_target) */
void* dynamic_array_bsearch(dynamic_array_t* self, void* p_target, comparator_t cmp)
{
	int32_t index;
	int32_t ret = dynamic_array_bsearch_index(self, p_target, cmp, &index);
	if (ret == 0)
	{
		return self->data + index*self->element_size;
	}
	else
	{
		return NULL;
	}
}

/* cmp(p_target, self[i]) */
void* dynamic_array_bsearch_reverse(dynamic_array_t* self, void* p_target, comparator_t cmp)
{
	int32_t index;
	int32_t ret = dynamic_array_bsearch_index_reverse(self, p_target, cmp, &index);
	if (ret == 0)
	{
		return self->data + index*self->element_size;
	}
	else
	{
		return NULL;
	}
}

void dynamic_array_erase(dynamic_array_t* self, uint32_t index)
{
	if (index < self->size)
	{
		memmove(self->data + index*self->element_size,
				self->data + (index + 1)*self->element_size,
				(self->size-index-1)*self->element_size);
		--self->size;
	}
}

void dynamic_array_erase_addr(dynamic_array_t* self, void* target)
{
	char* val = (char*)target;
	char* end = self->data + self->size*self->element_size;
	if (val >= self->data && val < end)
	{
		assert((val - self->data) % self->element_size == 0);
		memmove(val, val + self->element_size, end - val - self->element_size);
		--self->size;
	}
}

void dynamic_array_erase_multi(dynamic_array_t* self, uint32_t index, uint32_t n)
{
	if (index < self->size)
	{
		if (index + n > self->size) n = self->size - index;
		memmove(self->data + index*self->element_size,
				self->data + (index + n)*self->element_size,
				(self->size - index - n)*self->element_size);
		self->size -= n;
	}
}

void dynamic_array_erase_addr_multi(dynamic_array_t* self, void* target, uint32_t n)
{
	char* val = (char*)target;
	char* end = self->data + self->size*self->element_size;
	if (val >= self->data && val < end)
	{
		const char* val_end = val + self->element_size * n;
		assert((val - self->data) % self->element_size == 0);
		if (val_end > end)
		{
			val_end = end;
			n = (val_end - val) / self->element_size;
		}
		memmove(val, val_end, end - val_end);
		self->size -= n;
	}
}

void dynamic_array_remove(dynamic_array_t* self, const void* p_target, comparator_t cmp)
{
	int32_t index;
	int32_t ret = dynamic_array_bsearch_index(self, p_target, cmp, &index);
	if (ret == 0)
	{
		int32_t i = index - 1;
		while (i >= 0 && cmp(self->data + i*self->element_size, p_target) == 0)
		{
			--i;
		}
		dynamic_array_erase_multi(self, index, index-i);
	}
}

void dynamic_array_remove_multi(dynamic_array_t* self, dynamic_array_t* targets, comparator_t cmp)
{
	uint32_t i = 0;
	char* p = self->data;
	char* result = self->data;
	for (; i < self->size; ++i)
	{
		if (dynamic_array_bsearch_reverse(targets, p, cmp) == 0)
		{
			memcpy(result, p, self->element_size);
			result += self->element_size;
		}
		p += self->element_size;
	}
	self->size = (result - self->data) / self->element_size;
}

void dynamic_array_unique(dynamic_array_t* self, comparator_t cmp)
{
	char* p = self->data;
	char* q = p + self->element_size;
	char* end = p + self->size * self->element_size;
	while (q < end)
	{
		if (cmp(p, q) != 0)
		{
			p += self->element_size;
			if (p != q)
			{
				memcpy(p, q, self->element_size);
			}
		}
		else
		{
			--self->size;
		}
		q += self->element_size;
	}
}

void* dynamic_array_find_first_if(dynamic_array_t* self, unary_function_t unary_predicate)
{
	char* p = self->data;
	char* end = p + self->size*self->element_size;
	while (p < end)
	{
		if (unary_predicate(p))
		{
			return p;
		}
		p += self->element_size;
	}
	return NULL;
}

uint32_t dynamic_array_find_first_n_if(dynamic_array_t* self, unary_function_t unary_predicate, void* buf, uint32_t n)
{
	uint32_t i = 0;
	char* p = self->data;
	char* end = p + self->size*self->element_size;
	while (p < end && i < n)
	{
		if (unary_predicate(p))
		{
			memcpy((char*)buf + i*self->element_size, p, self->element_size);
			++i;
		}
		p += self->element_size;
	}
	return i;
}

void dynamic_array_remove_if(dynamic_array_t* self, unary_function_t unary_predicate)
{
	char* p = self->data;
	char* q = p;
	char* end = p + self->size*self->element_size;
	while (q < end)
	{
		if (unary_predicate(q))
		{
			--self->size;
		}
		else
		{
			if (p != q)
			{
				memcpy(p, q, self->element_size);
			}
			p += self->element_size;
		}
		q += self->element_size;
	}
}

void dynamic_array_remove_if_2(dynamic_array_t* self, binary_function_t binary_predicate, void* arg2)
{
	char* p = self->data;
	char* q = p;
	char* end = p + self->size*self->element_size;
	while (q < end)
	{
		if (binary_predicate(q, arg2))
		{
			--self->size;
		}
		else
		{
			if (p != q)
			{
				memcpy(p, q, self->element_size);
			}
			p += self->element_size;
		}
		q += self->element_size;
	}
}

void dynamic_array_remove_if_3(dynamic_array_t* self, ternary_function_t ternary_predicate, void* arg2, void* arg3)
{
	char* p = self->data;
	char* q = p;
	char* end = p + self->size*self->element_size;
	while (q < end)
	{
		if (ternary_predicate(q, arg2, arg3))
		{
			--self->size;
		}
		else
		{
			if (p != q)
			{
				memcpy(p, q, self->element_size);
			}
			p += self->element_size;
		}
		q += self->element_size;
	}
}

void dynamic_array_for_each(dynamic_array_t* self, unary_function_t unary_function)
{
	char* p = self->data;
	char* end = p + self->size*self->element_size;
	while (p < end)
	{
		unary_function(p);
		p += self->element_size;
	}
}

void dynamic_array_for_each_2(dynamic_array_t* self, binary_function_t binary_function, void* arg2)
{
	char* p = self->data;
	char* end = p + self->size*self->element_size;
	while (p < end)
	{
		binary_function(p, arg2);
		p += self->element_size;
	}
}

void dynamic_array_for_each_3(dynamic_array_t* self, ternary_function_t ternary_function, void* arg2, void* arg3)
{
	char* p = self->data;
	char* end = p + self->size*self->element_size;
	while (p < end)
	{
		ternary_function(p, arg2, arg3);
		p += self->element_size;
	}
}

void dynamic_array_union(const dynamic_array_t* a, const dynamic_array_t* b, dynamic_array_t* result, comparator_t cmp)
{
	dynamic_array_reserve(result, a->size + b->size);
	dynamic_array_append(result, a->data, a->size);
	dynamic_array_append(result, b->data, b->size);
	dynamic_array_sort(result, cmp);
	dynamic_array_unique(result, cmp);
}

void dynamic_array_intersection(const dynamic_array_t* a, dynamic_array_t* b, dynamic_array_t* result, comparator_t cmp)
{
	uint32_t i = 0;
	char* p = a->data;

	dynamic_array_sort(b, cmp);
	for (; i < a->size; ++i)
	{
		if (dynamic_array_bsearch(b, p, cmp) != NULL)
		{
			dynamic_array_push_back(result, p);
		}
		p += a->element_size;
	}
	dynamic_array_sort(result, cmp);
	dynamic_array_unique(result, cmp);
}

void dynamic_array_subtract(const dynamic_array_t* a, dynamic_array_t* b, dynamic_array_t* result, comparator_t cmp)
{
	uint32_t i = 0;
	char* p = a->data;

	dynamic_array_sort(b, cmp);
	for (; i < a->size; ++i)
	{
		if (dynamic_array_bsearch(b, p, cmp) == NULL)
		{
			dynamic_array_push_back(result, p);
		}
		p += a->element_size;
	}
	dynamic_array_sort(result, cmp);
	dynamic_array_unique(result, cmp);
}

static void dynamic_array_min_heap_shift_down(dynamic_array_t* self, int32_t heap_index, comparator_t cmp)
{
	int32_t heap_idx = heap_index;
	int32_t last_leaf = self->size - 1;
	int32_t last_non_leaf = (last_leaf - 1) >> 1;
	char* val = (char*)alloca(self->element_size);
	memcpy(val, self->data + heap_idx*self->element_size, self->element_size);
	while (heap_idx <= last_non_leaf)
	{
		int32_t right_child = (heap_idx + 1) << 1;
		int32_t left_child = right_child - 1;
		if (right_child <= last_leaf &&
			cmp(self->data + right_child*self->element_size, self->data + left_child*self->element_size) < 0)
		{
			left_child = right_child;
		}
		if (cmp(self->data + left_child*self->element_size, val) < 0)
		{
			memcpy(self->data + heap_idx*self->element_size, self->data + left_child*self->element_size, self->element_size);
			heap_idx = left_child;
		}
		else break;
	}
	if (heap_index != heap_idx)
	{
		memcpy(self->data + heap_idx*self->element_size, val, self->element_size);
	}
}

static void dynamic_array_min_heap_shift_up(dynamic_array_t* self, int32_t heap_index, comparator_t cmp)
{
	int32_t heap_idx = heap_index;
	int32_t father = (heap_idx - 1) >> 1; // -1/2 = 0; -1>>1 = -1;
	char* val = (char*)alloca(self->element_size);
	memcpy(val, self->data + heap_idx*self->element_size, self->element_size);
	while (father >= 0)
	{
		if (cmp(val, self->data + father*self->element_size) < 0)
		{
			memcpy(self->data + heap_idx*self->element_size, self->data + father*self->element_size, self->element_size);
			heap_idx = father; father = (heap_idx - 1) >> 1;
		}
		else break;
	}
	if (heap_index != heap_idx)
	{
		memcpy(self->data + heap_idx*self->element_size, val, self->element_size);
	}
}

void dynamic_array_make_min_heap(dynamic_array_t* self, comparator_t cmp)
{
	int32_t last_leaf = self->size - 1;
	int32_t last_non_leaf = (last_leaf - 1) >> 1;
	int32_t heap_idx = last_non_leaf;
	for (; heap_idx >= 0; --heap_idx)
	{
		dynamic_array_min_heap_shift_down(self, heap_idx, cmp);
	}
}

int32_t dynamic_array_is_min_heap(dynamic_array_t* self, comparator_t cmp)
{
	int32_t heap_idx = 0;
	int32_t last_leaf = self->size - 1;
	int32_t last_non_leaf = (last_leaf - 1) >> 1;
	for (; heap_idx <= last_non_leaf; ++heap_idx)
	{
		int32_t right_child = (heap_idx + 1) << 1;
		int32_t left_child = right_child - 1;
		if (cmp(self->data + heap_idx*self->element_size, self->data + left_child*self->element_size) > 0)
		{
			return 0;
		}
		if (right_child <= last_leaf &&
			cmp(self->data + heap_idx*self->element_size, self->data + right_child*self->element_size) > 0)
		{
			return 0;
		}
	}
	return 1;
}

void dynamic_array_push_min_heap(dynamic_array_t* self, const void* p_target, comparator_t cmp)
{
	uint32_t size = self->size;
	int32_t ret = dynamic_array_push_back(self, p_target);
	assert(ret == 0);
	if (ret == 0)
	{
		dynamic_array_min_heap_shift_up(self, size, cmp);
	}
}

void dynamic_array_pop_min_heap(dynamic_array_t* self, void* p_result, comparator_t cmp)
{
	if (self->size != 0)
	{
		if (p_result != NULL) memcpy(p_result, self->data, self->element_size);
		--self->size;
		if(self->size != 0)
		{
			memcpy(self->data, self->data + self->size*self->element_size, self->element_size);
			dynamic_array_min_heap_shift_down(self, 0, cmp);
		}
	}
}
