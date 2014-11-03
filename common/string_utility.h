/**
# -*- coding:UTF-8 -*-
*/

#ifndef _STRING_UTILITY_H_
#define _STRING_UTILITY_H_

#ifdef __GNUC__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include "define.h"

#include <stdlib.h>
#include <ctype.h>

#ifdef __GNUC__

#include <strings.h>

#ifndef stricmp
#define stricmp(s1,s2) strcasecmp(s1,s2)
#endif
#ifndef strnicmp
#define strnicmp(s1,s2,n) strncasecmp(s1,s2,n)
#endif
//#define strtou64(nptr,endptr,base) strtoull(nptr,endptr,base) //暂不使用标准库
//#define strtoi64(nptr,endptr,base) strtoll(nptr,endptr,base)
#elif defined(_WIN32)
#define stricmp(a,b) _stricmp(a,b)
#define strnicmp(a,b,n) _strnicmp(a,b,n)
#define strtolower(s) _strlwr(s)
#define strtoupper(s) _strupr(s)
#else
#error os not support
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef __GNUC__
char* strtolower(char* s);
char* strtoupper(char* s);
#elif defined(_WIN32)
//uint64_t strtou64(const char *nptr, const char **endptr, int base);
#endif

/**
 *将10进制字符串转换为数字
 */
int64_t atoi64(const char *nptr);
uint64_t atou64(const char *nptr);
int32_t atoi32(const char *nptr);
uint32_t atou32(const char *nptr);
/**
 *将base进制字符串转换为数字，*endptr返回第一个不识别字符的地址（如果非空）
 */
int64_t strtoi64(const char *nptr, const char **endptr, int32_t base);
uint64_t strtou64(const char *nptr, const char **endptr, int32_t base);
int32_t strtoi32(const char *nptr, const char **endptr, int32_t base);
uint32_t strtou32(const char *nptr, const char **endptr, int32_t base);

#define CHAR_TO_INT(c) ((int32_t)(uint32_t)(uint8_t)(c))
#define skip_space(str) while(isspace(CHAR_TO_INT(*str)))++str
#define skip_graph(str) while(isgraph(CHAR_TO_INT(*str)))++str
int32_t is_string_integer(const char* str);
int32_t is_string_unsigned_integer(const char* str);
uint32_t get_integer_count_from_string(const char* str);

int32_t strtoi64list(const char* str, int64_t** p_arr, uint32_t* p_arr_len);
int32_t strtou64list(const char* str, uint64_t** p_arr, uint32_t* p_arr_len);
int32_t strtoi32list(const char* str, int32_t** p_arr, uint32_t* p_arr_len);
int32_t strtou32list(const char* str, uint32_t** p_arr, uint32_t* p_arr_len);

uint32_t strchrcount(const char* str, uint32_t str_len, char chr);

/**
 *将数字转换为10进制字符串，result缓冲区应包括结尾的0
 @return result_len(不包括结尾的0)
 */
uint32_t i64toa(int64_t n, char* result);
uint32_t u64toa(uint64_t n, char* result);
uint32_t i32toa(int32_t n, char* result);
uint32_t u32toa(uint32_t n, char* result);
/**
 *将数字转换为base进制字符串，result缓冲区应包括结尾的0
 @return result_len(不包括结尾的0)
 */
uint32_t i64tostr(int64_t n, char* result, int32_t base);
uint32_t u64tostr(uint64_t n, char* result, int32_t base);
uint32_t i32tostr(int32_t n, char* result, int32_t base);
uint32_t u32tostr(uint32_t n, char* result, int32_t base);

/**
 *计算字符串的行数
 *支持\r,\n,\r\n,\n\r的任意一种，混合出现可能会导致计算错误
 @return 字符串的行数，即使最后一行不含有末尾的换行符，也算作一行
 */
uint32_t string_line_number(const char* str, uint32_t str_len);

/**
 *去除字符串前面的空白符
 @return 去除空白符后的字符串长度
 */
uint32_t strltrim(char* s, uint32_t n);
/**
 *去除字符串后面的空白符
 @return 去除空白符后的字符串长度
 */
uint32_t strrtrim(char* s, uint32_t n);
/**
 *去除字符串前后的空白符
 @return 去除空白符后的字符串长度
 */
uint32_t strtrim(char* s, uint32_t n);

/**
 *将字符串转换为16进制字符串，out缓冲区需要in_len*2+1
 @return out_len(不包括结尾的0)
 */
uint32_t string2hex(const char* in, uint32_t in_len, char* out);

/**
 *获取字符串表示的数字字符的数值
 *如果该字符不是正确的36进制字符，则返回-1
 */
extern const char* const tbl_hexvalue;
static inline char get_hexvalue(char v)
{
	return tbl_hexvalue[(int32_t)(uint32_t)(uint8_t)v];
}

/**
*获取36进制数值的字符
*如果v不是正确的36进制数值，结果是未定义的
*/
extern const char* const tbl_int2hex_lower;
static inline char int2hex_lower(int32_t v)
{
	return tbl_int2hex_lower[v];
}
extern const char* const tbl_int2hex_upper;
static inline char int2hex_upper(int32_t v)
{
	return tbl_int2hex_upper[v];
}
#define int2hex(v) int2hex_upper(v)

/**
 *将16进制字符串还原，out缓冲区需要in_len/2
 @return out_len
 */
uint32_t hex2string(const char* in, uint32_t len, char* out);

/**
 *将编码过的URL还原(%xx -> char, + -> ' ')，out缓冲区最多需要in_len+1
 @return out_len
 */
uint32_t decode_url(const char* in, uint32_t in_len, char* out);
#define decode_url_inplace(url, url_len) decode_url(url, url_len, url)

/**
 *对字符串编码
 *保留ASCII字母、数字以及*@-_+./，其余转换为%xx
 *out缓冲区最多需要in_len*3 + 1
 @return out_len
 */
uint32_t escape_uri(const char* in, uint32_t in_len, char* out);

/**
 *对URI编码
 *保留ASCII字母、数字以及-_.!~*'();/?:@&=+$,#，其余转换为%xx
 *out缓冲区最多需要in_len*3 + 1
 @return out_len
 */
uint32_t encode_uri(const char* in, uint32_t in_len, char* out);

/**
 *对URI组件编码
 *保留ASCII字母、数字以及-_.!~*'()，其余转换为%xx
 *out缓冲区最多需要in_len*3 + 1
 @return out_len
 */
uint32_t encode_uri_component(const char* in, uint32_t in_len, char* out);

/**
 *将字符串in以chr字符分隔，并将结果存储于sub_str中
 @param sub_str, [in]每个元素及数组必须能够被free等函数接受
                 [out]每个元素及数组本身使用free删除
 @return 成功返回0
 */
int32_t string_split(const char* in, uint32_t in_len, char chr, char*** sub_str, uint32_t* sub_str_number);

/**
 *计算字符串 hash
 */
uint32_t time33_hash(const char* s);
uint32_t time31_hash(const char* s);

uint32_t time33_hash_bin(const void* data, uint32_t length);
uint32_t time31_hash_bin(const void* data, uint32_t length);
uint32_t one_at_a_time_hash_bin(const void* data, uint32_t length);
uint32_t bob_hash_bin(const void* data, uint32_t length);

/**
 *分别使用time31和bob计算hash值，并将之合成一个64位的结果
 */
uint64_t time31_bob_mixed_hash_bin(const void* data, uint32_t length);

/**
 *将点分10进制IPv4串（\0结尾C风格字符串）转换为网络字节序IP值
 @return 0-无效IP, 其它值-有效IP
*/
uint32_t ipv4_aton(const char* ipstr);

/**
 *判断给定的字符串是不是ipv4
 @return true/false
 */
int32_t is_str_ipv4(const char* ipv4str);

/**
 *判断给定的文件名是否含有非法字符
 *非法字符包括 \/:*?"<>|
 @return true/false
 */
int32_t is_file_name_valid(const char* name);

/**
 *判断给定的文件路径是否含有非法字符
 *非法字符包括 :*?"<>|
 @return true/false
 */
int32_t is_file_path_valid(const char* path);

/**
 *将给定的文件名中的非法字符替换成target_char
 *非法字符包括 \/:*?"<>|
 @return 替换字符的数量
 */
int32_t replace_illegal_char_in_file_name(char* name, char target_char);

/**
 *将给定的文件路径中的非法字符替换成target_char
 *非法字符包括 :*?"<>|
 @return 替换字符的数量
 */
int32_t replace_illegal_char_in_file_path(char* path, char target_char);

#ifdef __cplusplus
}
#endif

#endif
