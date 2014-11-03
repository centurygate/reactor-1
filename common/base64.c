/**
# -*- coding:UTF-8 -*-

simple base64 encoder and decoder.

    Copyright (c) 2012, liuxuan - liu20076@sina.com

This code may be freely used for any purpose, either personal
or commercial, provided the authors copyright notice remains
intact.
*/

#include "base64.h"

static const char base64_fillchar = '=';

static const char* base64_encode_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t* base64_decode_table = (const uint8_t*)
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x3E\x00\x00\x00\x3F"
	"\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E"
	"\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x00\x00\x00\x00\x00"
	"\x00\x1A\x1B\x1C\x1D\x1E\x1F\x20\x21\x22\x23\x24\x25\x26\x27\x28"
	"\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

uint32_t base64_encode(const char* data, uint32_t data_len, char* out){
	const uint8_t* in = (const uint8_t*)data;
	char* p = out;
	uint32_t l = data_len/3;
	uint32_t r = data_len%3;
	uint32_t i = 0;

	for( ; i<l; ++i)
	{
		*p++ = base64_encode_table[*in>>2];
		*p++ = base64_encode_table[((*in&0x3)<<4)|(*(in+1)>>4)];
		++in;
		*p++ = base64_encode_table[((*in&0xf)<<2)|(*(in+1)>>6)];
		*p++ = base64_encode_table[*++in&0x3f];
		++in;
	}
	if(r==1){
		*p++ = base64_encode_table[*in>>2];
		*p++ = base64_encode_table[(*in&0x3)<<4];
		*p++ = base64_fillchar;
		*p++ = base64_fillchar;
	}
	else if(r!=0){
		*p++ = base64_encode_table[*in>>2];
		*p++ = base64_encode_table[((*in&0x3)<<4)|(*(in+1)>>4)];
		*p++ = base64_encode_table[(*++in&0xf)<<2];
		*p++ = base64_fillchar;
	}
	*p = 0;
	return p-out;
}

/**
可以缺失一个= ; 如果剩余1个或2个字符，将被忽略
TODO: 忽略data中出现的 \r,\n
*/
uint32_t base64_decode(const char* data, uint32_t data_len, char* out){
	const uint8_t* in = (const uint8_t*)data;
	char* p = out;
	for( ; data_len>2 ; data_len-=4){
		*p = base64_decode_table[*in++] << 2;
		*p++ |= base64_decode_table[*in] >> 4;
		*p = base64_decode_table[*in++] << 4;
		if(*in == base64_fillchar) break;
		*p++ |= base64_decode_table[*in] >> 2;
		*p = base64_decode_table[*in++] << 6;
		if(*in == base64_fillchar || *in==0) break;
		*p++ |= base64_decode_table[*in++];
	}
	//*p = 0; //optional

	return p - out;
}

static const char base32_fillchar = '=';

static const char* base32_encode_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static const uint8_t* base32_decode_table = (const uint8_t*)
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x1A\x1B\x1C\x1D\x1E\x1F\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E"
"\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x00\x00\x00\x00\x00"
"\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E"
"\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

uint32_t base32_encode(const char* data, uint32_t data_len, char* out){
	const uint8_t* in = (const uint8_t*)data;
	char* p = out;
	uint32_t l = data_len / 5;
	uint32_t r = data_len % 5;
	uint32_t i = 0;

	for (; i < l; ++i)
	{
		*p++ = base32_encode_table[*in >> 3];
		*p++ = base32_encode_table[((*in & 0x7) << 2) | (*(in + 1) >> 6)]; ++in;
		*p++ = base32_encode_table[(*in >> 1) & 0x1f];
		*p++ = base32_encode_table[((*in & 0x1) << 4) | (*(in + 1) >> 4)]; ++in;
		*p++ = base32_encode_table[((*in & 0xf) << 1) | (*(in + 1) >> 7)]; ++in;
		*p++ = base32_encode_table[(*in >> 2) & 0x1f];
		*p++ = base32_encode_table[((*in & 0x3) << 3) | (*(in + 1) >> 5)]; ++in;
		*p++ = base32_encode_table[*in & 0x1f]; ++in;
	}
	if (r == 1){
		*p++ = base32_encode_table[*in >> 3];
		*p++ = base32_encode_table[((*in & 0x7) << 2)]; //++in;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
	}
	else if (r == 2){
		*p++ = base32_encode_table[*in >> 3];
		*p++ = base32_encode_table[((*in & 0x7) << 2) | (*(in + 1) >> 6)]; ++in;
		*p++ = base32_encode_table[(*in >> 1) & 0x1f];
		*p++ = base32_encode_table[((*in & 0x1) << 4)]; //++in;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
	}
	else if (r == 3){
		*p++ = base32_encode_table[*in >> 3];
		*p++ = base32_encode_table[((*in & 0x7) << 2) | (*(in + 1) >> 6)]; ++in;
		*p++ = base32_encode_table[(*in >> 1) & 0x1f];
		*p++ = base32_encode_table[((*in & 0x1) << 4) | (*(in + 1) >> 4)]; ++in;
		*p++ = base32_encode_table[((*in & 0xf) << 1)]; //++in;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
		*p++ = base32_fillchar;
	}
	else if (r != 0){ // r == 4
		*p++ = base32_encode_table[*in >> 3];
		*p++ = base32_encode_table[((*in & 0x7) << 2) | (*(in + 1) >> 6)]; ++in;
		*p++ = base32_encode_table[(*in >> 1) & 0x1f];
		*p++ = base32_encode_table[((*in & 0x1) << 4) | (*(in + 1) >> 4)]; ++in;
		*p++ = base32_encode_table[((*in & 0xf) << 1) | (*(in + 1) >> 7)]; ++in;
		*p++ = base32_encode_table[(*in >> 2) & 0x1f];
		*p++ = base32_encode_table[((*in & 0x3) << 3)]; //++in;
		*p++ = base32_fillchar;
	}
	*p = 0;
	return p - out;
}

/**
忽略第一个=之后的内容
TODO: 忽略data中出现的 \r,\n
*/
uint32_t base32_decode(const char* data, uint32_t data_len, char* out){
	const uint8_t* in = (const uint8_t*)data;
	//const uint8_t* end = in + data_len;
	char* p = out;

	if (data_len & 0x7) return 0;

	for (; data_len>0; data_len -= 8){
		/*decode the first byte*/
		*p = base32_decode_table[*in++] << 3;
		if (*in == base32_fillchar/* || *in == 0*/) break;
		*p++ |= base32_decode_table[*in] >> 2;

		/*decode the second byte*/
		*p = (base32_decode_table[*in++] & 0x3) << 6;
		*p |= base32_decode_table[*in++] << 1;
		if (*in == base32_fillchar/* || *in == 0*/) break;
		*p++ |= base32_decode_table[*in] >> 4;

		/*decode the third byte*/
		*p = (base32_decode_table[*in++] & 0xf) << 4;
		if (*in == base32_fillchar/* || *in == 0*/) break;
		*p++ |= base32_decode_table[*in] >> 1;

		/*decode the fourth byte*/
		*p = (base32_decode_table[*in++] & 0x1) << 7;
		*p |= base32_decode_table[*in++] << 2;
		if (*in == base32_fillchar/* || *in == 0*/) break;
		*p++ |= base32_decode_table[*in] >> 3;

		/*decode the fifth byte*/
		*p = (base32_decode_table[*in++] & 0x7) << 5;
		*p++ |= base32_decode_table[*in++];
	}
	//*p = 0; //optional

	return p - out;
}

#if 0
/* {"plain text", "base32 encode text"} */
char* base32_test_vector[][2] = 
{
	{"", ""},
	{"0", "GA======"},
	{"12345", "GEZDGNBV"},
	{"123456", "GEZDGNBVGY======" },
	{"1234567", "GEZDGNBVGY3Q====" },
	{"12345678", "GEZDGNBVGY3TQ===" },
	{"123456789", "GEZDGNBVGY3TQOI=" },
	{"1234567890", "GEZDGNBVGY3TQOJQ" },
};
#endif
