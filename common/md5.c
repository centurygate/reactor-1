/**
# -*- coding:UTF-8 -*-
copyright liuxuan - 2012 , liu20076@sina.com
*/

#include "md5.h"
#include "define.h"
#include <memory.h>

static inline uint32_t rol32(uint32_t n,uint32_t bits){
	return ((n<<bits) | (n>>(32-bits)));
}
static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z){
	return (x&y) | (~x&z);
}
static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z){
	return (x&z) | (y&~z);
}
static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z){
	return x^y^z;
}
static inline uint32_t I(uint32_t x, uint32_t y, uint32_t z){
	return y^(x|~z);
}

#define FF(a, b, c, d,mk, s, ti) a =  b + rol32(a+F(b,c,d)+le32toh(mk)+ti, s)
#define HH(a, b, c, d,mk, s, ti) a =  b + rol32(a+H(b,c,d)+le32toh(mk)+ti, s)
#define GG(a, b, c, d, mk, s, ti) a =  b + rol32(a+G(b,c,d)+le32toh(mk)+ti, s)
#define II(a, b, c, d,mk, s, ti) a =  b + rol32(a+I(b,c,d)+le32toh(mk)+ti, s)

void md5_hash_data(const char* data, int data_len, char* val){
	static const uint32_t block_size = 64; //64B = 512b
	char buf[8 * 1024];

	memcpy(buf, data, data_len);
	uint32_t blen;
	buf[data_len] = (char)0x80;
	int padding_len = (56 - data_len%block_size - 1 + block_size)%block_size; //padding 0 len
	memset(buf+data_len+1, 0, padding_len);
	*(uint64_t*)(buf + data_len+1+padding_len) = htole64((uint64_t)data_len<<3);
	blen = (data_len+1+padding_len+8) / block_size;

	uint32_t A = 0x67452301, B = 0xEFCDAB89;
	uint32_t C = 0x98BADCFE, D = 0x10325476;
	uint32_t* M = (uint32_t*)buf;

	for( ; blen; --blen,M+=block_size/sizeof*M){
		uint32_t A1 = A, B1 = B, C1 = C, D1 = D;

		FF(A,B,C,D,M[0],7,0xD76AA478);
		FF(D,A,B,C,M[1],12,0xE8C7B756);
		FF(C,D,A,B,M[2],17,0x242070DB);
		FF(B,C,D,A,M[3],22,0xC1BDCEEE);
		FF(A,B,C,D,M[4],7,0xF57C0FAF);
		FF(D,A,B,C,M[5],12,0x4787C62A);
		FF(C,D,A,B,M[6],17,0xA8304613);
		FF(B,C,D,A,M[7],22,0xFD469501);
		FF(A,B,C,D,M[8],7,0x698098D8);
		FF(D,A,B,C,M[9],12,0x8B44F7AF);
		FF(C,D,A,B,M[10],17,0xFFFF5BB1);
		FF(B,C,D,A,M[11],22,0x895CD7BE);
		FF(A,B,C,D,M[12],7,0x6B901122);
		FF(D,A,B,C,M[13],12,0xFD987193);
		FF(C,D,A,B,M[14],17,0xA679438E);
		FF(B,C,D,A,M[15],22,0x49B40821);

		GG(A,B,C,D,M[1],5,0xF61E2562);
		GG(D,A,B,C,M[6],9,0xC040B340);
		GG(C,D,A,B,M[11],14,0x265E5A51);
		GG(B,C,D,A,M[0],20,0xE9B6C7AA);
		GG(A,B,C,D,M[5],5,0xD62F105D);
		GG(D,A,B,C,M[10],9,0x02441453);
		GG(C,D,A,B,M[15],14,0xD8A1E681);
		GG(B,C,D,A,M[4],20,0xE7D3FBC8);
		GG(A,B,C,D,M[9],5,0x21E1CDE6);
		GG(D,A,B,C,M[14],9,0xC33707D6);
		GG(C,D,A,B,M[3],14,0xF4D50D87);
		GG(B,C,D,A,M[8],20,0x455A14ED);
		GG(A,B,C,D,M[13],5,0xA9E3E905);
		GG(D,A,B,C,M[2],9,0xFCEFA3F8);
		GG(C,D,A,B,M[7],14,0x676F02D9);
		GG(B,C,D,A,M[12],20,0x8D2A4C8A);

		HH(A,B,C,D,M[5],4,0xFFFA3942);
		HH(D,A,B,C,M[8],11,0x8771F681);
		HH(C,D,A,B,M[11],16,0x6D9D6122);
		HH(B,C,D,A,M[14],23,0xFDE5380C);
		HH(A,B,C,D,M[1],4,0xA4BEEA44);
		HH(D,A,B,C,M[4],11,0x4BDECFA9);
		HH(C,D,A,B,M[7],16,0xF6BB4B60);
		HH(B,C,D,A,M[10],23,0xBEBFBC70);
		HH(A,B,C,D,M[13],4,0x289B7EC6);
		HH(D,A,B,C,M[0],11,0xEAA127FA);
		HH(C,D,A,B,M[3],16,0xD4EF3085);
		HH(B,C,D,A,M[6],23,0x04881D05);
		HH(A,B,C,D,M[9],4,0xD9D4D039);
		HH(D,A,B,C,M[12],11,0xE6DB99E5);
		HH(C,D,A,B,M[15],16,0x1FA27CF8);
		HH(B,C,D,A,M[2],23,0xC4AC5665);

		II(A,B,C,D,M[0],6,0xF4292244);
		II(D,A,B,C,M[7],10,0x432AFF97);
		II(C,D,A,B,M[14],15,0xAB9423A7);
		II(B,C,D,A,M[5],21,0xFC93A039);
		II(A,B,C,D,M[12],6,0x655B59C3);
		II(D,A,B,C,M[3],10,0x8F0CCC92);
		II(C,D,A,B,M[10],15,0xFFEFF47D);
		II(B,C,D,A,M[1],21,0x85845DD1);
		II(A,B,C,D,M[8],6,0x6FA87E4F);
		II(D,A,B,C,M[15],10,0xFE2CE6E0);
		II(C,D,A,B,M[6],15,0xA3014314);
		II(B,C,D,A,M[13],21,0x4E0811A1);
		II(A,B,C,D,M[4],6,0xF7537E82);
		II(D,A,B,C,M[11],10,0xBD3AF235);
		II(C,D,A,B,M[2],15,0x2AD7D2BB);
		II(B,C,D,A,M[9],21,0xEB86D391);

		A = A+A1;
		B = B+B1;
		C = C+C1;
		D = D+D1;
	}

	((uint32_t*)val)[0] = htole32(A);
	((uint32_t*)val)[1] = htole32(B);
	((uint32_t*)val)[2] = htole32(C);
	((uint32_t*)val)[3] = htole32(D);
}

#if 0
/*
 * RFC 1321 test vectors
 */
static const char* md5_test_str[7] = {
    "",
    "a",
    "abc",
    "message digest",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
};

static const unsigned char md5_test_sum[7][16] = {
    {
        0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
        0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E
    },
    {
        0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
        0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61
    },
    {
        0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
        0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72
    },
    {
        0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
        0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0
    },
    {
        0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
        0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B
    },
    {
        0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
        0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F
    },
    {
        0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
        0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A
    }
};
#endif
