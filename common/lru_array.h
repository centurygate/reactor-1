/**
# -*- coding:UTF-8 -*-
*/

#ifndef _LRU_ARRAY_H_
#define _LRU_ARRAY_H_

#include "define.h"

#ifdef __cplusplus
extern "C" 
{
#endif

typedef struct s_lru_array_t
{
	char* data;
	uint32_t* timestamp;
	unary_function_t finalizer;
	uint32_t element_size;
	uint32_t capacity;
	uint32_t expire_time;
	uint32_t hit_count;
	uint32_t miss_count;
} lru_array_t;

/**
 *构造一个lru缓存
 *在缓存满时向缓存中新增一项会淘汰最旧的过期元素
 @param max_size, 缓存的最大大小
 @param element_size, 缓存元素的大小，单位为字节
 @expire_time, 缓存项的过期时间
			   如所有元素都未过期，则不能插入
			   可以为0
 @param finalizer, 缓存元素的析构函数，为NULL表示不需要析构
 */
void lru_array_initialize(lru_array_t* self, uint32_t capacity,
						uint32_t element_size, uint32_t expire_time, unary_function_t finalizer);

/**
 *析构一个lru缓存，并回收内存
 *缓存元素如果是个对象，需要者自行析构
 */
void lru_array_finalize(lru_array_t* self);

/**
 *对缓存中的每个有效元素执行unary_function
 */
void lru_array_for_each(lru_array_t* self, unary_function_t unary_function);

/**
 *获取缓存容量
 @return 缓存中有效元素的个数
 */
uint32_t lru_array_capacity(lru_array_t* self);

/**
 *添加/更新一个元素
 @param p_target, 新元素的指针，该对象将被复制到缓存中(浅拷贝)
 @param compar, compar(&self[j], p_target)
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
void* lru_array_put(lru_array_t* self, const void* p_target, comparator_t compar);
//添加/更新一个元素，该元素将被保留在缓存中，直至变为普通元素或被删除
void* lru_array_put_reserved(lru_array_t* self, const void* p_target, comparator_t compar);

/**
 *查找一个元素，本操作不会更新该元素的访问时间
 @param key, 要查找的元素
 @param compar, compar(&self[i], key)
 @return 成功返回该元素的指针，失败返回NULL
 */
void* lru_array_get(lru_array_t* self, const void* key, comparator_t compar);

/**
 *刷新一个元素的访问时间
 @param key, 要刷新的元素
 @param compar, compar(&self[i], key)
 */
void lru_array_refresh(lru_array_t* self, const void* key, comparator_t compar);

/**
 *将一个元素保留在缓存中
 @param key, 要保留的元素
 @param compar, compar(&self[i], key)
 */
void lru_array_reserve(lru_array_t* self, const void* key, comparator_t compar);

/**
 *刷新一个元素的访问时间，reserved项将变为普通项
 @param key, 要刷新的元素
 @param compar, compar(&self[i], key)
 */
void lru_array_reset(lru_array_t* self, const void* key, comparator_t compar);

/**
 *移除一个元素
 @param key, 要删除的元素
 @param compar, compar(&self[j], key)
 @return 成功返回0
 */
int32_t lru_array_remove(lru_array_t* self, const void* key, comparator_t compar);

#ifdef __cplusplus
}
#endif

#endif
