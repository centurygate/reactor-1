/**
# -*- coding:UTF-8 -*-
*/

#ifndef _DYNAMIC_ARRAY_H_
#define _DYNAMIC_ARRAY_H_

#include "define.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/**
 *动态数组
 *仅适用于POD数据，如果元素是个对象，或包含指针，请谨慎使用
*/
typedef struct s_dynamic_array_t
{
	char* data;
	uint32_t size;
	uint32_t element_size;
	uint32_t capacity;
} dynamic_array_t;

/**
 *构造一个动态数组
 @param element_size, 数组元素的大小，单位为字节
 */
void dynamic_array_initialize(dynamic_array_t* self, uint32_t element_size);

/**
 *析构一个动态数组，并回收内存
 *数组元素如果是个对象，需要调用者自行析构
 */
void dynamic_array_finalize(dynamic_array_t* self);

/**
 *获取数组当前元素的个数
 @return 数组中有效元素的个数
 */
uint32_t dynamic_array_size(dynamic_array_t* self);

/**
 *测试当前数组是否是空数组
 @return 如果数组为空返回true，否则返回false
 */
int32_t dynamic_array_empty(dynamic_array_t* self);

/**
 *改变一个数组的大小
 *如果内存不足，不执行任何操作
 @param new_size, 期望的新大小，单位为数组元素的个数
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_resize(dynamic_array_t* self, uint32_t new_size);

/**
 *改变一个数组的容量
 *只能扩大数组的容量，如果内存不足，不执行任何操作
 @param new_size, 期望的新大小，单位为数组元素的个数
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_reserve(dynamic_array_t* self, uint32_t new_size);

/**
 *获取指定下标的元素的指针
 @param index, 元素的下标，从0开始
 @return 成功返回元素的指针，下标越界返回NULL
 */
void* dynamic_array_at(dynamic_array_t* self, uint32_t index);

/**
 *获取数组的内部数据
 @return 数组内部内存，如果数组尚未添加过任何元素，返回NULL
 */
void* dynamic_array_data(dynamic_array_t* self);

/**
 *设置数组所使用的内存空间
 *本操作会导致当前数组析构，重新绑定到buffer所指向的内存，并将size改为buffer_size
 @param buffer, 新的内存，必须满足：足够大 或 可被realloc使用
 @param buffer_size, 新的内存大小，单位为元素个数
 */
void dynamic_array_attach_buffer(dynamic_array_t* self, void* buffer, uint32_t buffer_size);

/**
 *解绑并获取数组内存，该操作会使数组变回初始状态
 *将数组内存交给调用者管理，该内存可用于free或realloc
 @return 数组内部内存，如果数组尚未添加过任何元素，返回NULL
 */
void* dynamic_array_detach_buffer(dynamic_array_t* self);

/**
 *交换两个数组的内存
 */
void dynamic_array_swap(dynamic_array_t* self, dynamic_array_t* other);

/**
 *复制数组，浅copy，self将直接使用other的数据
 */
void dynamic_array_copy(dynamic_array_t* self, const dynamic_array_t* other);

/**
 *复制数组，深copy(对数组元素是浅copy)
 *内存不足不会执行任何操作
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_clone(dynamic_array_t* self, const dynamic_array_t* other);

/**
 *将数组大小清0，数组元素需由调用者析构
 */
void dynamic_array_clear(dynamic_array_t* self);

/**
 *向数组末尾添加一个元素
 @param p_target, 新元素的指针
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_push_back(dynamic_array_t* self, const void* p_target);

/**
 *删除数组末尾的元素
 */
void dynamic_array_pop_back(dynamic_array_t* self);

/**
 *获取数组末尾的元素
 @return 数组为空返回NULL
 */
void* dynamic_array_back(dynamic_array_t* self);

/**
 *获取数组的首个元素
 @return 数组为空返回NULL
 */
void* dynamic_array_front(dynamic_array_t* self);

/**
 *向数组末尾添加一批元素
 @param p_target, 新元素的指针
 @param n, 要添加的元素的个数
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_append(dynamic_array_t* self, const void* p_target, uint32_t n);

/**
 *向数组的指定位置添加一批新元素
 @param index, 将新元素添加到这个位置，下标从0开始
 @param p_target, 新元素的指针
 @param n, 要添加的元素的个数
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_insert(dynamic_array_t* self, uint32_t index, const void* p_target, uint32_t n);

/**
 *向数组的指定位置添加一个新元素，并保持数组按照cmp顺序不变
 *self必须是有序的，如果self中已经存在与p_target相等的元素，p_target将被添加到那些元素的后面
 *时间复杂度：线性, 空间复杂度：常数
 @param p_target, 要添加的元素
 @param cmp, cmp(&self[j], p_target)
 @return 成功返回0，失败返回ERR_ETM_OUT_OF_MEMORY
 */
int32_t dynamic_array_insert_by_comparator(dynamic_array_t* self, const void* p_target, comparator_t cmp);

/**
 *将self按照cmp排序
 *时间复杂度为：o((self->size)*lg(self->size)), 空间复杂度为常数
 @param cmp, cmp(&self[j], &self[i])
 */
void dynamic_array_sort(dynamic_array_t* self, comparator_t cmp);

/**
 在self中顺序查找一个元素
 *时间复杂度：线性, 空间复杂度：常数
 @param p_target, 要查找的元素
 @param cmp, cmp(&self[j], p_target)
 @return 成功返回该元素的地址，找不到返回NULL
 */
void* dynamic_array_search(dynamic_array_t* self, void* p_target, comparator_t cmp);

/**
 *在self中查找一个元素，self必须是有序的
 *时间复杂度：o(lg(self->size)), 空间复杂度:常数
 @param p_target, 要查找的元素
 @param cmp, cmp(&self[j], p_target)
 @return 成功返回该元素的地址，找不到返回NULL
 */
void* dynamic_array_bsearch(dynamic_array_t* self, void* p_target, comparator_t cmp);

/**
 *删除self中指定下标的元素
 @param index, 要删除的元素的下标，从0开始
 */
void dynamic_array_erase(dynamic_array_t* self, uint32_t index);

/**
 *删除self中指定地址的元素
 *数组内存会随着数据的增加而变化
 @param target, 要删除的元素的地址
 */
void dynamic_array_erase_addr(dynamic_array_t* self, void* target);

/**
 *批量删除self中指定下标的元素
 *从下标index开始，删除n个，最多不会超过数组剩余元素的个数
 @param index, 要删除的元素的下标，从0开始
 @param n, 要删除的元素的个数
 */
void dynamic_array_erase_multi(dynamic_array_t* self, uint32_t index, uint32_t n);

/**
 *批量删除self中指定地址的元素
 *数组内存会随着数据的增加而变化
 @param target, 要删除的元素的地址
 @param n, 要删除的元素的个数
 */
void dynamic_array_erase_addr_multi(dynamic_array_t* self, void* target, uint32_t n);

/**
 *删除self中所有与p_target相等的元素，self必须是有序的
 *时间复杂度：线性, 空间复杂度:常数
 @param p_target, 要删除的元素
 @param cmp, cmp(&self[j], p_target)
 */
void dynamic_array_remove(dynamic_array_t* self, const void* p_target, comparator_t cmp);

/**
 *删除self中所有出现在target中的元素，targets必须是有序的
 *时间复杂度：o(self->size * lg(targets->size)), 空间复杂度:常数
 @param targets, 要删除的元素列表
 @param cmp, cmp(&self[j], &targets[i])
 */
void dynamic_array_remove_multi(dynamic_array_t* self, dynamic_array_t* targets, comparator_t cmp);

/**
 *使self中没有重复元素，self必须是有序的，重复元素只保留第一个
 *时间复杂度：线性, 空间复杂度：常数
 @param cmp, cmp(&self[i], &self[j])
 */
void dynamic_array_unique(dynamic_array_t* self, comparator_t cmp);

/**
 *获取self中第一个unary_predicate返回true的元素
 *时间复杂度：线性, 空间复杂度：常数
 @param unary_predicate, unary_predicate(&self[i])
 @return 第一个满足条件的元素地址，找不到返回NULL
 */
void* dynamic_array_find_first_if(dynamic_array_t* self, unary_function_t unary_predicate);

/**
 *获取self中前n个unary_predicate返回true的元素
 *时间复杂度：线性, 空间复杂度：常数
 @param unary_predicate, unary_predicate(&self[i])
 @param buf, 用于接收结果的缓冲区，这是原数据的二进制copy，请勿析构
 @param n, 期望得到n个元素
 @return 实际找到的元素个数
 */
uint32_t dynamic_array_find_first_n_if(dynamic_array_t* self, unary_function_t unary_predicate, void* buf, uint32_t n);

/**
 *删除self中所有unary_predicate返回true的元素
 *时间复杂度：线性, 空间复杂度：常数
 @param unary_predicate, unary_predicate(&self[i])
 */
void dynamic_array_remove_if(dynamic_array_t* self, unary_function_t unary_predicate);

/**
 *删除self中所有binary_predicate返回true的元素
 *时间复杂度：线性, 空间复杂度：常数
 @param binary_predicate, binary_predicate(&self[i], arg2)
 @param arg2, 传递给binary_predicate的第二个参数
 */
void dynamic_array_remove_if_2(dynamic_array_t* self, binary_function_t binary_predicate, void* arg2);

/**
 *删除self中所有ternary_predicate返回true的元素
 *时间复杂度：线性, 空间复杂度：常数
 @param binary_predicate, ternary_predicate(&self[i], arg2, arg3)
 @param arg2, 传递给ternary_predicate的第二个参数
 @param arg3, 传递给ternary_predicate的第三个参数
 */
void dynamic_array_remove_if_3(dynamic_array_t* self, ternary_function_t ternary_predicate, void* arg2, void* arg3);

/**
 *对self中的元素依次执行unary_function
 *时间复杂度：线性, 空间复杂度：常数
 @param unary_function, unary_function(&self[i])
 */
void dynamic_array_for_each(dynamic_array_t* self, unary_function_t unary_function);

/**
 *对self中的元素依次执行binary_function
 *时间复杂度：线性, 空间复杂度：常数
 @param binary_function, binary_function(&self[i], arg2)
 @param arg2, 传递给binary_function的第二个参数
 */
void dynamic_array_for_each_2(dynamic_array_t* self, binary_function_t binary_function, void* arg2);

/**
 *对self中的元素依次执行ternary_function
 *时间复杂度：线性, 空间复杂度：常数
 @param binary_function, ternary_function(&self[i], arg2, arg3)
 @param arg2, 传递给ternary_function的第二个参数
 @param arg3, 传递给ternary_function的第三个参数
 */
void dynamic_array_for_each_3(dynamic_array_t* self, ternary_function_t ternary_function, void* arg2, void* arg3);

/**
 *计算集合a和集合b的并集
 *时间复杂度：n*lg(n), 空间复杂度：线性
 *结果集result中的元素来自a或者来自b是未定义的（当有重复元素时），按照cmp排序、去重
 @param cmp, cmp(&a[i], &a[j]), cmp(&b[i], &b[j]), cmp(&a[i], &b[j]), cmp(&b[i], &a[j])
 */
void dynamic_array_union(const dynamic_array_t* a, const dynamic_array_t* b, dynamic_array_t* result, comparator_t cmp);

/**
 *计算集合a和集合b的交集
 *时间复杂度：n*lg(n), 空间复杂度：线性
 *集合b会被排序
 *结果集result的元素来自集合a，按照cmp排序、去重
 @param cmp, cmp(&b[i], &b[j]), cmp(&b[i], &a[j]), cmp(&result[i], &result[j])
 */
void dynamic_array_intersection(const dynamic_array_t* a, dynamic_array_t* b, dynamic_array_t* result, comparator_t cmp);

/**
 *计算集合b在集合a中的相对补集(a-b)
 *时间复杂度：n*lg(n), 空间复杂度：线性
 *集合b会被排序
 *结果集result的元素来自集合a，按照cmp排序、去重
 @param cmp, cmp(&b[i], &b[j]), cmp(&b[i], &a[j]), cmp(&result[i], &result[j])
 */
void dynamic_array_subtract(const dynamic_array_t* a, dynamic_array_t* b, dynamic_array_t* result, comparator_t cmp);

/**
 *将数组转换为最小堆
 *时间复杂度：n*lg(n), 空间复杂度：常数
 @param cmp, cmp(&self[i], &self[j])
 */
void dynamic_array_make_min_heap(dynamic_array_t* self, comparator_t cmp);

/**
 *判断数组是否为最小堆
 *时间复杂度：线性, 空间复杂度：常数
 @param cmp, cmp(&self[i], &self[j])
 @return true/false，如果数组为空，返回true
 */
int32_t dynamic_array_is_min_heap(dynamic_array_t* self, comparator_t cmp);

/**
 *向最小堆添加一个新的元素
 *时间复杂度：lg(n), 空间复杂度：常数
 @param p_target, 新元素
 @param cmp, cmp(&self[i], &self[j])
 */
void dynamic_array_push_min_heap(dynamic_array_t* self, const void* p_target, comparator_t cmp);

/**
 *取出最小堆第一个元素
 *时间复杂度：lg(n), 空间复杂度：常数
 @param p_result, 用于接收结果
                  如果数组为空，该内存不会被改变
				  p_result = NULL 表示不需要结果
 @param cmp, cmp(&self[i], &self[j])
 */
void dynamic_array_pop_min_heap(dynamic_array_t* self, void* p_result, comparator_t cmp);


#ifdef __cplusplus
}
#endif

#endif
