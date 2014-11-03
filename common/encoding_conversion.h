#ifndef _ENCODING_CONVERSION_H_
#define _ENCODING_CONVERSION_H_

#include "iconv.h" //depend on libiconv
#include <stdint.h>

//convert charset encoding among utf8, gbk and big5

int32_t iconv_module_initialize();
int32_t iconv_module_finalize();

/**
 *如果不能转换，返回-1，不对参数作任何改变
 *如果in已经是目标字符集，则将in复制到out
 *out不是C字符串(没有结尾的0)，必须保证其内存足够大
 */
//thread unsafe, for etm main thread only
int32_t to_utf8(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= 1.5*in_len
int32_t to_gbk(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= in_len
int32_t to_big5(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= in_len

int32_t gbk_to_utf8(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= 1.5*in_len
int32_t big5_to_utf8(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= 1.5*in_len
int32_t utf8_to_gbk(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= in_len
int32_t big5_to_gbk(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= in_len
int32_t gbk_to_big5(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= in_len
int32_t utf8_to_big5(const char* in, uint32_t in_len, char* out, uint32_t* out_len); //out_len >= in_len

//thread safe
int32_t to_utf8_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t to_gbk_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t to_big5_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);

int32_t gbk_to_utf8_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t big5_to_utf8_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t utf8_to_gbk_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t big5_to_gbk_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t gbk_to_big5_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);
int32_t utf8_to_big5_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len);

int32_t encoding_convertor_copy_input(const char* in, uint32_t in_len, char* out, uint32_t* out_len);

typedef int32_t (*encoding_conversion_func_t)(const char* in, uint32_t in_len, char* out, uint32_t* out_len);

/**
 *将utf8序列转换为本机字节序的ucs4字符
 *如果endptr!=NULL，则指向原序列的下一个字符
 @return 成功返回ucs4字符，失败返回0
 */
uint32_t translate_utf8_char_to_ucs4(const char* s, const char** endptr);

#endif
