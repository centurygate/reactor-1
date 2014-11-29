/**
# -*- coding:UTF-8 -*-
*/

#ifndef _DEFINE_H_
#define _DEFINE_H_

#include <stdlib.h>

#ifdef __GNUC__
#include <alloca.h>
#elif defined(_WIN32)
#include <malloc.h>
#else
#error os not support
#endif

/*------- integer type define --------*/
#include <stdint.h>

/*------------ error code ------------*/
#include "error_code.h"

/*------- string ---------*/
#include <string.h>


/**
 *如下几个宏提供“字符串”操作的部分支持
 *“字符串”是指一个char*指针和一个uint32_t表示长度
 *该“字符串”同时是C风格字符串，长度不包括结尾的0
 */
 
/**
 *字符串构造
 */
#define STRING_INITIALIZE(self_ptr, self_len) \
{\
	self_ptr = 0;\
	self_len = 0;\
}

/**
 *内部用途，请勿调用
 *执行完后更新self_len为new_len
 */
#define STRING_RESIZE(self_ptr, self_len, new_len, result) \
{\
	uint32_t _sr_new_len_ = new_len;\
	char* _sr_p_ = (char*)realloc(self_ptr, _sr_new_len_+1);\
	if (_sr_p_)\
	{\
		(self_ptr) = _sr_p_; \
		if((self_len)<_sr_new_len_)(self_ptr)[self_len]=0;\
		else (self_ptr)[_sr_new_len_]=0;\
		self_len=_sr_new_len_;\
		result=0;\
	}\
	else result=ERR_ETM_OUT_OF_MEMORY;\
}

/**
 *字符串赋值，自动为self_ptr分配内存，如果成功，result=0
 */
#define STRING_ASSIGN(self_ptr, self_len, other_ptr, other_len, result) \
{\
	if ((self_ptr) != (other_ptr) && (other_len) >= 0)\
	{\
		STRING_RESIZE(self_ptr, self_len, other_len, result)\
		if(result==0)\
		{\
			memcpy(self_ptr, other_ptr, other_len);\
			(self_ptr)[self_len]=0;\
		}\
	}\
}

/**
 *字符串赋值，保证self_ptr的内存比other_ptr大
 */
#define STRING_COPY(self_ptr, self_len, other_ptr, other_len) \
{\
	if((self_ptr)!=(other_ptr))\
	{\
		memcpy(self_ptr, other_ptr, other_len);\
		self_len=other_len;\
		(self_ptr)[self_len]=0;\
	}\
}

/**
 *字符串赋值，不对内存进行复制
 */
#define STRING_DIRECT_COPY(self_ptr, self_len, other_ptr, other_len) \
{\
	self_ptr = other_ptr;\
	self_len = other_len;\
}

/**
 *字符串拼接，自动为self_ptr分配内存，如果成功，result=0
 */
#define STRING_APPEND(self_ptr, self_len, other_ptr, other_len, result) \
{\
	uint32_t _sa_old_len_ = self_len;\
	STRING_RESIZE(self_ptr, self_len, (self_len) + (other_len), result)\
	if(result==0)\
	{\
		memcpy(self_ptr + _sa_old_len_, other_ptr, other_len); \
		(self_ptr)[self_len]=0;\
	}\
}

/**
 *字符串析构，回收内存
 */
#define STRING_FINALIZE(self_ptr, self_len) \
{\
	free(self_ptr);\
	self_ptr = 0;\
	self_len = 0;\
}

/**
 *如下几个宏提供“数组”操作的部分支持
 *“数组”是指一个数据指针和一个uint32_t表示元素个数
 *另可参阅dynamic_array
 */
 
#define INITIALIZER_NULL(pe)
#define INITIALIZER_SET_ZERO(pe) memset(pe, 0, sizeof(*(pe)))

#define FINALIZER_NULL(pe)
#define FINALIZER_DIRECT_FREE(pe) free(*(pe))

#define DUPLICATOR_NULL(self_ptr, other_ptr)
#define DUPLICATOR_DIRECT_COPY(self_ptr, other_ptr) *(self_ptr)=*(other_ptr)

/**
 *数组构造
 */
#define ARRAY_INITIALIZE(self_ptr, self_len) \
{\
	self_ptr = 0;\
	self_len = 0;\
}

#if defined(__GNUC__)
#define CAST_VOID_PTR_TO_ARRAY_PTR(array_ptr, void_ptr) array_ptr=(typeof(array_ptr))(void_ptr)
#else
//#define CAST_VOID_PTR_TO_ARRAY_PTR(array_ptr, void_ptr) array_ptr=void_ptr //cast void* to self_ptr may cause compile error in C++ compiler
#define CAST_VOID_PTR_TO_ARRAY_PTR(array_ptr, void_ptr) memcpy(&(array_ptr), &(void_ptr), sizeof(array_ptr))
#endif

/**
 *调整数组大小，element_initializer和element_finalizer分别是数组元素的构造和析构函数
 *如果成功，result=0
 */
#define ARRAY_RESIZE(self_ptr, self_len, new_len, element_initializer, element_finalizer, result) \
{\
	uint32_t _ar_idx_;\
	uint32_t _ar_new_len_ = new_len;\
	if(self_len<_ar_new_len_)\
	{\
		void* _p_ = realloc(self_ptr, (_ar_new_len_+1)*sizeof*self_ptr);\
		if(_p_)\
		{\
			CAST_VOID_PTR_TO_ARRAY_PTR(self_ptr, _p_);\
			for(_ar_idx_=self_len; _ar_idx_<_ar_new_len_; ++_ar_idx_)\
			{\
				element_initializer(&self_ptr[_ar_idx_]);\
			}\
			self_len=_ar_new_len_;\
			result=0;\
		}\
		else result=ERR_ETM_OUT_OF_MEMORY;\
	}\
	else if(self_len>_ar_new_len_)\
	{\
		for(_ar_idx_=_ar_new_len_; _ar_idx_<self_len; ++_ar_idx_)\
		{\
			element_finalizer(&self_ptr[_ar_idx_]);\
		}\
		self_len=_ar_new_len_;\
		result=0;\
	}\
	else\
	{\
		result=0;\
	}\
}

/**
 *数组析构，回收内存
 */
#define ARRAY_FINALIZE(self_ptr, self_len, element_finalizer) \
{\
	if (self_ptr != NULL)\
	{\
		uint32_t _af_idx_=0;\
		for (; _af_idx_<self_len; ++_af_idx_)\
		{\
			element_finalizer(&self_ptr[_af_idx_]); \
		}\
		free(self_ptr);\
		self_ptr = 0;\
		self_len = 0;\
	}\
}

/**
 *数组赋值，element_duplicator是数据成员的copy constructor
 *如果成功，result=0
 */
#define ARRAY_ASSIGN(self_ptr, self_len, other_ptr, other_len, element_initializer, element_finalizer, element_duplicator, result) \
{\
	if(self_ptr!=other_ptr)\
	{\
		ARRAY_RESIZE(self_ptr, self_len, other_len, element_initializer, element_finalizer, ret)\
		if(ret == 0)\
		{\
			uint32_t _aa_idx_;\
			for(_aa_idx_=0; _aa_idx_<self_len; ++_aa_idx_)\
			{\
				element_duplicator(&self_ptr[_aa_idx_], &other_ptr[_aa_idx_]);\
			}\
		}\
	}\
}

/**
 *数组赋值，浅复制
 */
#define ARRAY_DIRECT_COPY(self_ptr, self_len, other_ptr, other_len) \
{\
	self_ptr = other_ptr;\
	self_len = other_len;\
}

/*----------- byte order -------------*/
/*仅提供编译时的字节序支持*/
#define htobe8(x) (x)
#define htole8(x) (x)
#define be8toh(x) (x)
#define le8toh(x) (x)

#ifdef __GNUC__

#include <endian.h>

#ifndef htobe16
#include <byteswap.h>
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define htobe16(x) __bswap_16 (x)
#  define htole16(x) (x)
#  define be16toh(x) __bswap_16 (x)
#  define le16toh(x) (x)

#  define htobe32(x) __bswap_32 (x)
#  define htole32(x) (x)
#  define be32toh(x) __bswap_32 (x)
#  define le32toh(x) (x)

//#  if __GLIBC_HAVE_LONG_LONG
#   define htobe64(x) __bswap_64 (x)
#   define htole64(x) (x)
#   define be64toh(x) __bswap_64 (x)
#   define le64toh(x) (x)
//#  endif

# else
#  define htobe16(x) (x)
#  define htole16(x) __bswap_16 (x)
#  define be16toh(x) (x)
#  define le16toh(x) __bswap_16 (x)

#  define htobe32(x) (x)
#  define htole32(x) __bswap_32 (x)
#  define be32toh(x) (x)
#  define le32toh(x) __bswap_32 (x)

//#  if __GLIBC_HAVE_LONG_LONG
#   define htobe64(x) (x)
#   define htole64(x) __bswap_64 (x)
#   define be64toh(x) (x)
#   define le64toh(x) __bswap_64 (x)
//#  endif
# endif
#endif

#elif defined(_WIN32)

/*windows is little endian only*/
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __FLOAT_WORD_ORDER __BYTE_ORDER 

#define htobe16(host_16bits) htons(host_16bits)
#define htole16(host_16bits) (host_16bits)
#define be16toh(big_endian_16bits) ntohs(big_endian_16bits)
#define le16toh(little_endian_16bits) (little_endian_16bits)

#define htobe32(host_32bits) htonl(host_32bits)
#define htole32(host_32bits) (host_32bits)
#define be32toh(big_endian_32bits) ntohl(big_endian_32bits)
#define le32toh(little_endian_32bits) (little_endian_32bits)

#define htobe64(host_64bits)((((uint64_t)htobe32((uint32_t)host_64bits))<<32) | htobe32((uint32_t)(host_64bits>>32)))
#define htole64(host_64bits) (host_64bits)
#define be64toh(big_endian_64bits)((((uint64_t)be32toh((uint32_t)big_endian_64bits))<<32) | be32toh((uint32_t)(big_endian_64bits>>32)))
#define le64toh(little_endian_64bits) (little_endian_64bits)

#else
#error os not support
#endif

/*-------- inline ----------*/
#ifndef __cplusplus

#if defined(__STDC_VERSION__) && __STDC_VERSION__ > 199901L
/* it's a keyword */
#else

#ifdef __GNUC__
#define inline __inline__
#elif defined(WIN32)
#define inline __inline
#else
#error os not support
#endif

#endif

#endif

/*------ integer <----> pointor*/
#if defined(__x86_64__) || defined(_WIN64)
#define POINTER_TO_INT32(pointor) (uint32_t)(uint64_t)(pointor)
#define INT32_TO_POINTER(integer) (void*)(uint64_t)(integer)
#define POINTER_TO_INT64(pointor) (uint64_t)(pointor)
#define INT64_TO_POINTER(integer) (void*)(integer)
#else
#define POINTER_TO_INT32(pointor) (uint32_t)(pointor)
#define INT32_TO_POINTER(integer) (void*)(integer)
//#define POINTER_TO_INT64(pointor) (uint32_t)(pointor)
//#define INT64_TO_POINTER(integer) (void*)(uint32_t)(integer)
#endif

/*------ others -------*/
#define CID_BIN_SIZE 20
#define CID_HEX_SIZE 40
#define CID_BASE32_SIZE 32
#define CID_SIZE CID_HEX_SIZE

#define LICENSE_SIZE 42
#define LICENSE_PREFIX_LEN 23
#define MAX_PEERID_SIZE 255
#define MAX_DEVICEID_LEN (MAX_PEERID_SIZE-15)
#define ET_PEERID_SIZE 16

#define MAC_BIN_SIZE 6
#define MAC_HEX_SIZE 12
#define MAC_SIZE MAC_HEX_SIZE

#define MAX_USERNAME_SIZE 32
#define MAX_JUMPKEY_SIZE 256
#define MAX_SYSTPATH 80
#define MAX_DEFAULT_DL_PATH 128

typedef int32_t(*unary_function_t)(void*);
typedef int32_t(*const_unary_function_t)(const void*);

typedef int32_t(*binary_function_t)(void*, void*);
typedef int32_t(*const_binary_function_t)(const void*, const void*);

typedef int32_t(*ternary_function_t)(void*, void*, void*);
typedef int32_t(*const_ternary_function_t)(const void*, const void*, const void*);

/*--------- comparator_t --------*/
/*下面这些比较器的参数是数据的指针，可用于qsort函数*/
typedef const_binary_function_t comparator_t;

static inline int32_t int8p_comparator(const void* pl, const void* pr)
{
	return *(const int8_t*)pl - *(const int8_t*)pr;
}

static inline int32_t int16p_comparator(const void* pl, const void* pr)
{
	return *(const int16_t*)pl - *(const int16_t*)pr;
}

static inline int32_t int32p_comparator(const void* pl, const void* pr)
{
	int32_t lv = *(const int32_t*)pl;
	int32_t rv = *(const int32_t*)pr;
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}

static inline int32_t int64p_comparator(const void* pl, const void* pr)
{
	int64_t lv = *(const int64_t*)pl;
	int64_t rv = *(const int64_t*)pr;
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}

static inline int32_t uint8p_comparator(const void* pl, const void* pr)
{
	return *(const uint8_t*)pl - *(const uint8_t*)pr;
}

static inline int32_t uint16p_comparator(const void* pl, const void* pr)
{
	return *(const uint16_t*)pl - *(const uint16_t*)pr;
}

static inline int32_t uint32p_comparator(const void* pl, const void* pr)
{
	uint32_t lv = *(const uint32_t*)pl;
	uint32_t rv = *(const uint32_t*)pr;
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}

static inline int32_t uint64p_comparator(const void* pl, const void* pr)
{
	uint64_t lv = *(const uint64_t*)pl;
	uint64_t rv = *(const uint64_t*)pr;
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}

static inline int32_t stringp_comparator(const void* pl, const void* pr)
{
	return strcmp(*(const char*const*)pl, *(const char*const*)pr);
}

/*下面这些比较器的参数是数据本身，请注意指针与整数转换可能造成的数据丢失问题*/
static inline int32_t int8_comparator(const void* l, const void* r)
{
	return (int8_t)POINTER_TO_INT32(l) - (int8_t)POINTER_TO_INT32(r);
}

static inline int32_t int16_comparator(const void* l, const void* r)
{
	return (int16_t)POINTER_TO_INT32(l) - (int16_t)POINTER_TO_INT32(r);
}

static inline int32_t int32_comparator(const void* l, const void* r)
{
	int32_t lv = (int32_t)POINTER_TO_INT32(l);
	int32_t rv = (int32_t)POINTER_TO_INT32(r);
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}

#ifdef POINTER_TO_INT64
static inline int32_t int64_comparator(const void* l, const void* r)
{
	int64_t lv = (int64_t)POINTER_TO_INT64(l);
	int64_t rv = (int64_t)POINTER_TO_INT64(r);
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}
#endif

static inline int32_t uint8_comparator(const void* l, const void* r)
{
	return (uint8_t)POINTER_TO_INT32(l) - (uint8_t)POINTER_TO_INT32(r);
}

static inline int32_t uint16_comparator(const void* l, const void* r)
{
	return (uint16_t)POINTER_TO_INT32(l) - (uint16_t)POINTER_TO_INT32(r);
}

static inline int32_t uint32_comparator(const void* l, const void* r)
{
	uint32_t lv = (uint32_t)POINTER_TO_INT32(l);
	uint32_t rv = (uint32_t)POINTER_TO_INT32(r);
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}

#ifdef POINTER_TO_INT64
static inline int32_t uint64_comparator(const void* l, const void* r)
{
	uint64_t lv = (uint64_t)POINTER_TO_INT64(l);
	uint64_t rv = (uint64_t)POINTER_TO_INT64(r);
	if(lv < rv) return -1;
	else if(lv == rv) return 0;
	else return 1;
}
#endif

static inline int32_t string_comparator(const void* l, const void* r)
{
	return strcmp((const char*)l, (const char*)r);
}

/*-------------- min/max ---------------*/
static inline int32_t int32min(int32_t a, int32_t b)
{
	return a < b ? a : b;
}
static inline uint32_t uint32min(uint32_t a, uint32_t b)
{
	return a < b ? a : b;
}
static inline int64_t int64min(int64_t a, int64_t b)
{
	return a < b ? a : b;
}
static inline uint64_t uint64min(uint64_t a, uint64_t b)
{
	return a < b ? a : b;
}

static inline int32_t int32max(int32_t a, int32_t b)
{
	return a > b ? a : b;
}
static inline uint32_t uint32max(uint32_t a, uint32_t b)
{
	return a > b ? a : b;
}
static inline int64_t int64max(int64_t a, int64_t b)
{
	return a > b ? a : b;
}
static inline uint64_t uint64max(uint64_t a, uint64_t b)
{
	return a > b ? a : b;
}

/*-------------- encoding ---------------*/
#define UTF8_ENCODING			"UTF-8"
#define BIG5_ENCODING			"BIG5"
#define GBK_ENCODING			"GBK"

#endif
