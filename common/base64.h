/**
# -*- coding:UTF-8 -*-
*/

#ifndef _BASE64_H_
#define _BASE64_H_

#include "define.h"

#ifdef __cplusplus
extern "C" 
{
#endif

//@return out length, out_buffer_length >= (data_len*4/3 + 1)
//@see RFC 1421,rfc2045,rfc3548
uint32_t base64_encode(const char* data, uint32_t data_len, char* out);

//@return out length, out_buffer_length >= data_len*3/4
//@see RFC 1421,rfc2045,rfc3548
uint32_t base64_decode(const char* data, uint32_t data_len, char* out);

//@return out length, out_buffer_length >= (data_len*8/5 + 1)
//@see rfc3548,rfc4648
uint32_t base32_encode(const char* data, uint32_t data_len, char* out);

//@return out length, out_buffer_length >= data_len*5/8
//@see rfc3548,rfc4648
uint32_t base32_decode(const char* data, uint32_t data_len, char* out);

#ifdef __cplusplus
}
#endif

#endif
