/**
** @file		: encoding_conversion.cpp
** @encoding	: ANSI
** @author		: liuxuan, liu20076@sina.com
** @date		: 2012-09-02
** @see			: ASCII, GBK, GB-2312, BIG-5, UTF-8
**/
#include "encoding_conversion.h"
#include <string.h>
#include <stddef.h>
#include <errno.h>

/**
ascii [0x00,0x7f]
*/
int32_t is_str_ascii(const char* str, uint32_t str_len) 
{
	const char* end = str + str_len;
	for( ; str<end; ++str)
	{
		if(*str&0x80) return 0;
	}
	return 1;
}

/** 
UTF-8的编码方式
0xxxxxxx
110xxxxx 10xxxxxx
1110xxxx 10xxxxxx 10xxxxxx
11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
int32_t is_str_utf8(const char* str, uint32_t str_len) 
{
	const uint8_t* p = (const uint8_t*)str;
	const uint8_t* end = p + str_len;
	for( ; p<end; ++p)
	{
		if(*p&0x80){
			if(*p<0xC0) return 0;
			else if(*p<0xE0) goto L_2_BYTES;
			else if(*p<0xF0) goto L_3_BYTES;
			else if(*p<0xF8) goto L_4_BYTES;
			else if(*p<0xFC) goto L_5_BYTES;
			else if(*p<0xFE) goto L_6_BYTES;
			else return 0;

L_6_BYTES:
			if((*++p&0xC0)!=0x80) return 0;
L_5_BYTES:
			if((*++p&0xC0)!=0x80) return 0;
L_4_BYTES:
			if((*++p&0xC0)!=0x80) return 0;
L_3_BYTES:
			if((*++p&0xC0)!=0x80) return 0;
L_2_BYTES:
			if((*++p&0xC0)!=0x80) return 0;
		}
	}
	return 1; 
}

/*
 @return 常用字符的个数。ASCII不参与计算。
*/
uint32_t get_str_utf8_char_number(const char* str, uint32_t str_len,
								uint32_t* p_non_common_utf8_num,
								uint32_t* p_invalid_num)
{
	uint32_t common_utf8_char_num = 0;
	uint32_t non_common_utf8_char_num = 0;
	uint32_t invalid_num = 0;
	const uint8_t* p = (const uint8_t*)str;
	const uint8_t* end = p + str_len;

	while(p<end)
	{
		if(*p&0x80)
		{
			uint32_t* p_num = &non_common_utf8_char_num;
			//首字节,应大于等于 11000000 并且小于等于 11111101
			if (*p < 0xC0){ ++invalid_num; ++p; continue; }
			else if(*p<0xE0) goto L_2_BYTES;
			else if (*p<0xF0){ p_num = &common_utf8_char_num; goto L_3_BYTES; }
			else if(*p<0xF8) goto L_4_BYTES;
			else if(*p<0xFC) goto L_5_BYTES;
			else if(*p<0xFE) goto L_6_BYTES;
			else{ ++invalid_num; ++p; continue; }

			//非首字节,应为 10xxxxxx 
L_6_BYTES:
			if ((*++p & 0xC0) != 0x80){ ++invalid_num; continue; }
L_5_BYTES:
			if ((*++p & 0xC0) != 0x80){ ++invalid_num; continue; }
L_4_BYTES:
			if ((*++p & 0xC0) != 0x80){ ++invalid_num; continue; }
L_3_BYTES:
			if ((*++p & 0xC0) != 0x80){ ++invalid_num; continue; }
L_2_BYTES:
			if ((*++p & 0xC0) != 0x80){ ++invalid_num; continue; }
			++*p_num;
			++p;
		}
		else ++p;
	}
	if (p_non_common_utf8_num != NULL)*p_non_common_utf8_num = non_common_utf8_char_num;
	if (p_invalid_num != NULL)*p_invalid_num = invalid_num;
	return common_utf8_char_num;
}

/*
 @return 当中日韩字符占比大于80%时返回真
*/
int32_t predict_str_utf8(const char* str, uint32_t str_len) 
{
	uint32_t non_common_num, invalid_num;
	uint32_t common_num = get_str_utf8_char_number(str, str_len, &non_common_num, &invalid_num);
	return invalid_num == 0 && common_num > 4 * non_common_num;
}

uint32_t translate_utf8_char_to_ucs4(const char* s, const char** endptr)
{
	uint32_t ucs4 = 0;
	const uint8_t* p = (const uint8_t*)s;
	if (*p < 0xC0)
	{
		++p;
		goto L_END;
	}
	else if (*p < 0xE0)
	{
		ucs4 = *p & 0x1f;
		goto L_2_BYTES;
	}
	else if (*p < 0xF0)
	{
		ucs4 = *p & 0xf;
		goto L_3_BYTES;
	}
	else if (*p < 0xF8)
	{
		ucs4 = *p & 0x7;
		goto L_4_BYTES;
	}
	else if (*p < 0xFC)
	{
		ucs4 = *p & 0x3;
		goto L_5_BYTES;
	}
	else if (*p < 0xFE)
	{
		ucs4 = *p & 0x1;
		goto L_6_BYTES;
	}
	else
	{
		++p;
		goto L_END;
	}

L_6_BYTES:
	if ((*++p & 0xC0) != 0x80) goto L_END;
	ucs4 = (ucs4 << 6) | (*p & 0x3f);
L_5_BYTES:
	if ((*++p & 0xC0) != 0x80) goto L_END;
	ucs4 = (ucs4 << 6) | (*p & 0x3f);
L_4_BYTES:
	if ((*++p & 0xC0) != 0x80) goto L_END;
	ucs4 = (ucs4 << 6) | (*p & 0x3f);
L_3_BYTES:
	if ((*++p & 0xC0) != 0x80) goto L_END;
	ucs4 = (ucs4 << 6) | (*p & 0x3f);
L_2_BYTES:
	if ((*++p & 0xC0) != 0x80) goto L_END;
	ucs4 = (ucs4 << 6) | (*p & 0x3f);

	++p;

L_END:
	if (endptr)
	{
		*endptr = (const char*)p;
	}

	return ucs4;
}

uint32_t utf8_next_char(const char* s, const char** endptr)
{
	if (*s & 0x80) return translate_utf8_char_to_ucs4(s, endptr);
	else
	{
		if (endptr) *endptr = s + 1;
		return (uint8_t)*s;
	}
}

uint32_t utf8_fill_question_mark(char** in)
{
	*(*in)++ = 0xef;
	*(*in)++ = 0xbc;
	*(*in)++ = 0x9f;
	return 3;
}

/**
GBK的编码范围
范围		第1字节	第2字节			编码数	字数	备注
标准GBK/1	A1–A9	A1–FE			846		717		符号717个
标准GBK/2	B0–F7	A1–FE			6768	6763	GB2312汉字6763个(常用字)
标准GBK/3	81–A0	40–FE(7F除外)	6080	6080	GB13000.1中的CJK汉字6080个
标准GBK/4	AA–FE	40–A0(7F除外)	8160	8160	CJK汉字和增补的汉字8160个
标准GBK/5	A8–A9	40–A0(7F除外)	192		166		BIG5非汉字符号、结构符166个
用户定义	AA–AF	A1–FE			564	
用户定义	F8–FE	A1–FE			658	
用户定义	A1–A7	40–A0(7F除外)	672	
合计:								23940	21886
*/
static const char seg12[] =
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00";
static const char seg3[] =
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00";
static const char seg45[] =
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
static const char* const _gb2312_segment[] =
{
#if 0
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
#endif
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,
  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,
  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,
  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,
  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
};
static const char* const * gb2312_segment = _gb2312_segment - 16 * 8;
static const char* const _gbk_other_segment[] =
{
#if 0
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
#endif
   NULL,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,
   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,   seg3,
   seg3,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,
  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,
  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,
  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,
  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,
  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,   NULL,
};
static const char* const * gbk_other_segment = _gbk_other_segment - 16 * 8;
static const char* const _user_segment[] =
{
#if 0
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
#endif
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,  seg45,   NULL,   NULL,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,  seg12,   NULL,
};
static const char* const * user_segment = _user_segment - 16 * 8;
/**
 @return 1 if str match gbk, but str may be another charset encoding like utf8,big5,etc.
*/
int32_t is_str_gbk(const char* str, uint32_t str_len) 
{
	const uint8_t* p = (const uint8_t*)str;
	const uint8_t* end = p + str_len;
	while (p < end)
	{
		if(*p&0x80)
		{
			const char* seg = gb2312_segment[*p];
			if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

			seg = gbk_other_segment[*p];
			if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

			seg = user_segment[*p];
			if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

			return 0; //invalid char

L_NEXT_CHAR:
			p += 2;
		}
		else ++p;
	}
	return 1; 
}

/*
 @return 常用字符的个数。ASCII码不参与计算。
*/
uint32_t get_str_gbk_char_number(const char* str, uint32_t str_len,
								uint32_t* p_non_common_gbk_num,
								uint32_t* p_invalid_num)
{
	uint32_t common_gbk_char_num = 0;
	uint32_t non_common_gbk_char_num = 0;
	uint32_t invalid_num = 0;
	const uint8_t* p = (const uint8_t*)str;
	const uint8_t* end = p + str_len;

	while (p < end)
	{
		if (*p & 0x80)
		{
			const char* seg = gb2312_segment[*p];
			if (seg != NULL && seg[p[1]] != 0)
			{
				if (*p < 0xd8) goto L_GB2312_CHAR; //符号和一级汉字
				else goto L_NEXT_CHAR; //二级汉字
			}

			seg = gbk_other_segment[*p];
			if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

			seg = user_segment[*p];
			if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

			//invalid char
			++invalid_num;
			++p;
			continue;
L_GB2312_CHAR:
			++common_gbk_char_num;
			p += 2;
			continue;
L_NEXT_CHAR:
			++non_common_gbk_char_num;
			p += 2;
			continue;
		}
		else ++p;
	}
	if (p_non_common_gbk_num != NULL)*p_non_common_gbk_num = non_common_gbk_char_num;
	if (p_invalid_num != NULL) *p_invalid_num = invalid_num;
	return common_gbk_char_num;
}

/**
 @return 1 if str match gbk, and the GB2312 character proportion exceeds 80%
*/
int32_t predict_str_gbk(const char* str, uint32_t str_len) 
{
	uint32_t non_common_num, invalid_num;
	uint32_t common_num = get_str_gbk_char_number(str, str_len, &non_common_num, &invalid_num);
	return invalid_num == 0 && common_num > 4 * common_num;
}

uint32_t gbk_next_char(const char* in, const char** endptr)
{
	const uint8_t* p = (const uint8_t*)in;
	if (*p & 0x80)
	{
		const char* seg = gb2312_segment[*p];
		if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

		seg = gbk_other_segment[*p];
		if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

		seg = user_segment[*p];
		if (seg != NULL && seg[p[1]] != 0) goto L_NEXT_CHAR;

		//invalid char
		if (endptr) *endptr = in + 1;
		return 0; 

L_NEXT_CHAR:
		if (endptr) *endptr = in + 2;
		return (*p << 8) | p[1];
	}
	else
	{
		if (endptr) *endptr = in + 1;
		return *p;
	}
}

uint32_t gbk_fill_question_mark(char** in)
{
	*(*in)++ = 0xa3;
	*(*in)++ = 0xbf;
	return 2;
}

/**
BIG5的编码方式
0x8140-0xA0FE	保留给用户自定义字符（造字区），目前未用
0xA140-0xA3BF	标点符号、希腊字母及特殊符号
0xA3C0-0xA3FE	保留。此区没有开放作造字区用
		0xA3C0-0xA3E0	33个控制字符图象，倚天中文系统扩充
		0xA3E1			欧元（€）符号，Windows Code Page 950
0xA440-0xC67E	常用汉字，先按笔划再按部首排序
0xC6A1-0xC8FE	保留给用户自定义字符（造字区）
		0xC6A1-0xC875	章节符号、部首及笔划结构、其他字母符号，倚天中文系统扩充
0xC940-0xF9D5	次常用汉字，先按笔划再按部首排序
0xF9D6-0xFEFE	保留给用户自定义字符（造字区）
		0xF9D6-0xF9FE	扩充字及制表符，倚天中文系统扩充
*/
/**
 @return 1 if str match big5, but str may be another charset encoding like utf8,gbk,etc.
*/
int32_t is_str_big5(const char* str, uint32_t str_len) 
{
	const uint8_t* p = (const uint8_t*)str;
	const uint8_t* end = p + str_len;
	for( ; p<end; ++p)
	{
		if(*p&0x80){
			uint16_t big5_char_value = *p<<8;
			big5_char_value |= *++p;
			if(0xA140<=big5_char_value && big5_char_value<=0xF9FE)
			{
				if(big5_char_value<=0xA3E1)
				{ //标点符号,ok
				}
				else if(big5_char_value<0xA440) return 0;
				else if(big5_char_value<=0xC67E)
				{ //常用汉字,ok
				}
				else if(big5_char_value<0xC6A1) return 0;
				else if(big5_char_value<=0xC875)
				{ //倚天中文系统扩充,ok
				}
				else if(big5_char_value<0xC940) return 0;
				else
				{ //次常用汉字，倚天中文系统扩充,ok
				}
			}
			else
			{
				if (0x8140 <= big5_char_value && big5_char_value <= 0xA0FE)
				{ //造字区
				}
				else if (0xF9FE < big5_char_value && big5_char_value <= 0xFEFE)
				{ //造字区
				}
				else return 0;
			}
		}
	}
	return 1; 
}

/*
 @return 常用字符的个数。ASCII码不参与计算。
*/
uint32_t get_str_big5_char_number(const char* str, uint32_t str_len,
								uint32_t* p_non_common_big5_num,
								uint32_t* p_invalid_num)
{
	uint32_t common_big5_char_num = 0;
	uint32_t non_common_big5_char_num = 0;
	uint32_t invalid_num = 0;
	const uint8_t* p = (const uint8_t*)str;
	const uint8_t* end = p + str_len;

	for (; p<end; ++p)
	{
		if (*p & 0x80){
			uint16_t big5_char_value = *p << 8;
			big5_char_value |= *++p;
			if (0xA140 <= big5_char_value && big5_char_value <= 0xF9FE)
			{
				if (big5_char_value <= 0xA3E1)
				{ //标点符号,ok
					++common_big5_char_num;
				}
				else if (big5_char_value < 0xA440) --p, ++invalid_num;
				else if (big5_char_value <= 0xC67E)
				{ //常用汉字,ok
					++common_big5_char_num;
				}
				else if (big5_char_value<0xC6A1) --p, ++invalid_num;
				else if (big5_char_value <= 0xC875)
				{ //倚天中文系统扩充,ok
					++non_common_big5_char_num;
				}
				else if (big5_char_value<0xC940) --p, ++invalid_num;
				else
				{ //次常用汉字，倚天中文系统扩充,ok
					++non_common_big5_char_num;
				}
			}
			else
			{
				if (0x8140 <= big5_char_value && big5_char_value <= 0xA0FE)
				{ //造字区
					++non_common_big5_char_num;
				}
				else if (0xF9FE < big5_char_value && big5_char_value <= 0xFEFE)
				{ //造字区
					++non_common_big5_char_num;
				}
				else --p, ++invalid_num;
			}
		}
	}
	if (p_non_common_big5_num != NULL) *p_non_common_big5_num = non_common_big5_char_num;
	if (p_invalid_num != NULL) *p_invalid_num = invalid_num;
	return common_big5_char_num;
}

/**
 @return 1 if str match big5, and common character proportion exceeds 80%
*/
int32_t predict_str_big5(const char* str, uint32_t str_len) 
{
	uint32_t non_common_num, invalid_num;
	uint32_t common_num = get_str_big5_char_number(str, str_len, &non_common_num, &invalid_num);
	return invalid_num == 0 && common_num > 4 * non_common_num;
}

uint32_t big5_next_char(const char* in, const char** endptr)
{
	const uint8_t* p = (const uint8_t*)in;
	if (*p & 0x80){
		uint16_t big5_char_value = *p << 8;
		big5_char_value |= *++p;
		if (0xA140 <= big5_char_value && big5_char_value <= 0xF9FE)
		{
			if (big5_char_value <= 0xA3E1)
			{ //标点符号,ok
			}
			else if (big5_char_value<0xA440) big5_char_value = 0;
			else if (big5_char_value <= 0xC67E)
			{ //常用汉字,ok
			}
			else if (big5_char_value<0xC6A1) big5_char_value = 0;
			else if (big5_char_value <= 0xC875)
			{ //倚天中文系统扩充,ok
			}
			else if (big5_char_value<0xC940) big5_char_value = 0;
			else
			{ //次常用汉字，倚天中文系统扩充,ok
			}
		}
		else
		{
			if (0x8140 <= big5_char_value && big5_char_value <= 0xA0FE)
			{ //造字区
			}
			else if (0xF9FE < big5_char_value && big5_char_value <= 0xFEFE)
			{ //造字区
			}
			else big5_char_value = 0;
		}
		if (endptr) *endptr = in + (big5_char_value != 0 ? 2 : 1);
		return big5_char_value;
	}
	else
	{
		if (endptr) *endptr = in + 1;
		return *p;
	}
}

uint32_t big5_fill_question_mark(char** in)
{
	*(*in)++ = 0xa1;
	*(*in)++ = 0x48;
	return 2;
}

typedef uint32_t (*next_char_t)(const char* in, const char** endptr);
typedef uint32_t(*fill_default_char_t)(char** in);

typedef struct s_ecv_ctx_t
{
	const char* encoding_name_from;
	const char* encoding_name_to;
	next_char_t next_char; //获取输入流中的下一个字符，输入指针指向后面的字符，返回读取的字符编码
	fill_default_char_t fill_default_char; //当输入流中存在非法字符时，使用本函数向输出流填充一个默认字符，返回写入的字节数
}ecv_ctx_t;

static ecv_ctx_t gbk_to_utf8_ctx = { "GBK", "UTF-8", gbk_next_char, utf8_fill_question_mark };
static ecv_ctx_t big5_to_utf8_ctx = { "BIG5", "UTF-8", big5_next_char, utf8_fill_question_mark };
static ecv_ctx_t big5_to_gbk_ctx = { "BIG5", "GBK", big5_next_char, gbk_fill_question_mark };
static ecv_ctx_t utf8_to_gbk_ctx = { "UTF-8", "GBK", utf8_next_char, gbk_fill_question_mark };
static ecv_ctx_t gbk_to_big5_ctx = { "GBK", "BIG5", gbk_next_char, big5_fill_question_mark };
static ecv_ctx_t utf8_to_big5_ctx = { "UTF-8","BIG5", utf8_next_char, big5_fill_question_mark };

typedef struct s_encoding_convertor_t
{
	iconv_t gbk_to_utf8;
	iconv_t big5_to_utf8;
	iconv_t big5_to_gbk;
	iconv_t utf8_to_gbk;
	iconv_t gbk_to_big5;
	iconv_t utf8_to_big5;
}EncodingConvertor;

static EncodingConvertor default_encoding_convertor;

static void encoding_convertor_reset(iconv_t cd);
static int32_t encoding_convertor_conv(iconv_t cd, ecv_ctx_t* ctx, const char* in, uint32_t in_len, char* out, uint32_t* out_len);

int32_t encoding_convertor_initialize(EncodingConvertor* self)
{
	if(self->gbk_to_utf8 != NULL) return 0;

	self->gbk_to_utf8 = iconv_open("UTF-8//TRANSLIT", "GBK");
	if(self->gbk_to_utf8==(iconv_t)-1)
	{
		self->gbk_to_utf8 = NULL;
		return errno;
	}

	self->big5_to_utf8 = iconv_open("UTF-8//TRANSLIT", "BIG5");
	if(self->big5_to_utf8==(iconv_t)-1)
	{
		self->big5_to_utf8 = NULL;
		return errno;
	}
	
	self->big5_to_gbk = iconv_open("GBK//TRANSLIT", "BIG5");
	if(self->big5_to_gbk==(iconv_t)-1)
	{
		self->big5_to_gbk = NULL;
		return errno;
	}
	
	self->utf8_to_gbk = iconv_open("GBK//TRANSLIT", "UTF-8");
	if(self->utf8_to_gbk==(iconv_t)-1)
	{
		self->utf8_to_gbk = NULL;
		return errno;
	}

	self->gbk_to_big5 = iconv_open("BIG5//TRANSLIT", "GBK");
	if(self->gbk_to_big5==(iconv_t)-1)
	{
		self->gbk_to_big5 = NULL;
		return errno;
	}

	self->utf8_to_big5 = iconv_open("BIG5//TRANSLIT", "UTF-8");
	if (self->utf8_to_big5 == (iconv_t)-1)
	{
		self->utf8_to_big5 = NULL;
		return errno;
	}

	return 0;
}

int32_t encoding_convertor_finalize(EncodingConvertor* self)
{
	if(self->gbk_to_utf8 == NULL) return 0;

	iconv_close(self->gbk_to_utf8);
	self->gbk_to_utf8 = NULL;

	iconv_close(self->big5_to_utf8);
	self->big5_to_utf8 = NULL;

	iconv_close(self->big5_to_gbk);
	self->big5_to_gbk = NULL;

	iconv_close(self->utf8_to_gbk);
	self->utf8_to_gbk = NULL;

	iconv_close(self->gbk_to_big5);
	self->gbk_to_big5 = NULL;

	iconv_close(self->utf8_to_big5);
	self->utf8_to_big5 = NULL;

	return 0;
}

int32_t encoding_convertor_gbk_to_utf8(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_conv(self->gbk_to_utf8, &gbk_to_utf8_ctx, in, in_len, out, out_len);
}

int32_t encoding_convertor_big5_to_utf8(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_conv(self->big5_to_utf8, &big5_to_utf8_ctx, in, in_len, out, out_len);
}

int32_t encoding_convertor_utf8_to_gbk(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_conv(self->utf8_to_gbk, &utf8_to_gbk_ctx, in, in_len, out, out_len);
}

int32_t encoding_convertor_big5_to_gbk(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_conv(self->big5_to_gbk, &big5_to_gbk_ctx, in, in_len, out, out_len);
}

int32_t encoding_convertor_gbk_to_big5(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_conv(self->gbk_to_big5, &gbk_to_big5_ctx, in, in_len, out, out_len);
}

int32_t encoding_convertor_utf8_to_big5(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_conv(self->utf8_to_big5, &utf8_to_big5_ctx, in, in_len, out, out_len);
}

static void encoding_convertor_reset(iconv_t cd)
{
	iconv(cd, NULL, NULL, NULL, NULL);
}

int32_t fix_encoding(next_char_t next_char, fill_default_char_t fill_default_char, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	if(*out_len >= in_len)
	{
		const char* cur_end;
		char* p_out = out;
		const char* p = in;
		const char* end = p + in_len;
		const char* cur = p;
		for(;;)
		{
			while(next_char(cur, &cur_end) != 0)
			{
				if(cur_end < end) cur = cur_end;
				else if(cur_end == end)
				{
					memcpy(p_out, p, cur_end - p);
					p_out += cur_end - p;
					goto L_END;
				}
				else //if(cur_end > end)
				{
					memcpy(p_out, p, cur - p);
					p_out += cur - p;
					fill_default_char(&p_out);
					goto L_END;
				}
			}
			memcpy(p_out, p, cur - p);
			p_out += cur - p;
			fill_default_char(&p_out);
			p = cur = cur_end;
		}
L_END:
		*out_len = p_out - out;
		return 0;
	}
	else return E2BIG;
}

/**
* @return	0,		success, and write target encoding into in
			-1,		cannot recognize the encoding of in
			errno,	cannot convert in to target encoding
					errno may be EILSEQ, EINVAL, E2BIG
*/
int32_t encoding_convertor_to_utf8(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	uint32_t non_common_utf8_num;
	uint32_t common_utf8_num;
	uint32_t invalid_utf8_num;
	uint32_t common_gbk_num;
	uint32_t non_common_gbk_num;
	uint32_t invalid_gbk_num;
	uint32_t common_big5_num;
	uint32_t non_common_big5_num;
	uint32_t invalid_big5_num;

	/* 查找常用字比例超过80%的编码 */
	common_gbk_num = get_str_gbk_char_number(in, in_len, &non_common_gbk_num, &invalid_gbk_num);
	if (invalid_gbk_num == 0 && common_gbk_num > 4 * non_common_gbk_num)
	{ //gbk
		return encoding_convertor_gbk_to_utf8(self, in, in_len, out, out_len);
	}

	common_utf8_num = get_str_utf8_char_number(in, in_len, &non_common_utf8_num, &invalid_utf8_num);
	if (invalid_utf8_num == 0 && common_utf8_num > 4 * non_common_utf8_num)
	{ //utf8
		return encoding_convertor_copy_input(in, in_len, out, out_len);
	}

	common_big5_num = get_str_big5_char_number(in, in_len, &non_common_big5_num, &invalid_big5_num);
	if (invalid_big5_num == 0 && common_big5_num > 4 * non_common_big5_num)
	{ //big5
		return encoding_convertor_big5_to_utf8(self, in, in_len, out, out_len);
	}

	if (common_gbk_num == 0 && common_utf8_num == 0 && common_big5_num == 0) return -1;

	/* 获取发现有效字符数最多的编码 */
	if (common_gbk_num >= common_big5_num)
	{ //gbk or utf8
		if (common_gbk_num >= common_utf8_num)
		{ //gbk
			return encoding_convertor_gbk_to_utf8(self, in, in_len, out, out_len);
		}
		else
		{ //utf8
			if(invalid_utf8_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(utf8_next_char, utf8_fill_question_mark, in, in_len, out, out_len);
		}
	}
	else
	{ // big5 or utf8
		if (common_utf8_num >= common_big5_num)
		{ //utf8
			if(invalid_utf8_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(utf8_next_char, utf8_fill_question_mark, in, in_len, out, out_len);
		}
		else
		{ //big5
			return encoding_convertor_big5_to_utf8(self, in, in_len, out, out_len);
		}
	}
}

int32_t encoding_convertor_to_gbk(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	uint32_t non_common_utf8_num;
	uint32_t common_utf8_num;
	uint32_t invalid_utf8_num;
	uint32_t common_gbk_num;
	uint32_t non_common_gbk_num;
	uint32_t invalid_gbk_num;
	uint32_t common_big5_num;
	uint32_t non_common_big5_num;
	uint32_t invalid_big5_num;

	/* 查找常用字比例超过80%的编码 */
	common_gbk_num = get_str_gbk_char_number(in, in_len, &non_common_gbk_num, &invalid_gbk_num);
	if (invalid_gbk_num == 0 && common_gbk_num > 4 * non_common_gbk_num)
	{ //gbk
		return encoding_convertor_copy_input(in, in_len, out, out_len);
	}

	common_utf8_num = get_str_utf8_char_number(in, in_len, &non_common_utf8_num, &invalid_utf8_num);
	if (invalid_utf8_num == 0 && common_utf8_num > 4 * non_common_utf8_num)
	{ //utf8
		return encoding_convertor_utf8_to_gbk(self, in, in_len, out, out_len);
	}

	common_big5_num = get_str_big5_char_number(in, in_len, &non_common_big5_num, &invalid_big5_num);
	if (invalid_big5_num == 0 && common_big5_num > 4 * non_common_big5_num)
	{ //big5
		return encoding_convertor_big5_to_gbk(self, in, in_len, out, out_len);
	}

	if (common_gbk_num == 0 && common_utf8_num == 0 && common_big5_num == 0) return -1;

	/* 获取发现有效字符数最多的编码 */
	if (common_gbk_num >= common_big5_num)
	{ //gbk or utf8
		if (common_gbk_num >= common_utf8_num)
		{ //gbk
			if(invalid_gbk_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(gbk_next_char, gbk_fill_question_mark, in, in_len, out, out_len);
		}
		else
		{ //utf8
			return encoding_convertor_utf8_to_gbk(self, in, in_len, out, out_len);
		}
	}
	else
	{ // big5 or utf8
		if (common_utf8_num >= common_big5_num)
		{ //utf8
			return encoding_convertor_utf8_to_gbk(self, in, in_len, out, out_len);
		}
		else
		{ //big5
			return encoding_convertor_big5_to_gbk(self, in, in_len, out, out_len);
		}
	}
}

int32_t encoding_convertor_to_big5(EncodingConvertor* self, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	uint32_t non_common_utf8_num;
	uint32_t common_utf8_num;
	uint32_t invalid_utf8_num;
	uint32_t common_gbk_num;
	uint32_t non_common_gbk_num;
	uint32_t invalid_gbk_num;
	uint32_t common_big5_num;
	uint32_t non_common_big5_num;
	uint32_t invalid_big5_num;

	/* 查找常用字比例超过80%的编码 */
	common_gbk_num = get_str_gbk_char_number(in, in_len, &non_common_gbk_num, &invalid_gbk_num);
	if (invalid_gbk_num == 0 && common_gbk_num > 4 * non_common_gbk_num)
	{ //gbk
		return encoding_convertor_gbk_to_big5(self, in, in_len, out, out_len);
	}

	common_utf8_num = get_str_utf8_char_number(in, in_len, &non_common_utf8_num, &invalid_utf8_num);
	if (invalid_utf8_num == 0 && common_utf8_num > 4 * non_common_utf8_num)
	{ //utf8
		return encoding_convertor_utf8_to_big5(self, in, in_len, out, out_len);
	}

	common_big5_num = get_str_big5_char_number(in, in_len, &non_common_big5_num, &invalid_big5_num);
	if (invalid_big5_num == 0 && common_big5_num > 4 * non_common_big5_num)
	{ //big5
		return encoding_convertor_copy_input(in, in_len, out, out_len);
	}

	if (common_gbk_num == 0 && common_utf8_num == 0 && common_big5_num == 0) return -1;

	/* 获取发现有效字符数最多的编码 */
	if (common_gbk_num >= common_big5_num)
	{ //gbk or utf8
		if (common_gbk_num >= common_utf8_num)
		{ //gbk
			return encoding_convertor_gbk_to_big5(self, in, in_len, out, out_len);
		}
		else
		{ //utf8
			return encoding_convertor_utf8_to_big5(self, in, in_len, out, out_len);
		}
	}
	else
	{ // big5 or utf8
		if (common_utf8_num >= common_big5_num)
		{ //utf8
			return encoding_convertor_utf8_to_big5(self, in, in_len, out, out_len);
		}
		else
		{ //big5
			if(invalid_big5_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(big5_next_char, big5_fill_question_mark, in, in_len, out, out_len);
		}
	}
}

static int32_t encoding_convertor_conv(iconv_t cd, ecv_ctx_t* ctx, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	size_t n;

	size_t outbytesleft = *out_len;
	char* pout = out;

	//convert arg 2 to char* to adapt the problem in the iconv specification
	//actually, iconv will NOT change the in buffer contents
	char* pin = (char*)in;
	size_t inbytesleft = in_len;

	encoding_convertor_reset(cd);

	for (;;)
	{
		n = iconv(cd, (char**)&pin, &inbytesleft, &pout, &outbytesleft);
		if (n != (size_t)(-1))
		{
			*out_len = pout - out;
			return 0;
		}
		else
		{
			if (errno == EILSEQ)
			{
				if(inbytesleft != 0)
				{
					char* p = pin;
					/*uint32_t c =*/ ctx->next_char(pin, (const char**)&pin);
					//if (c != 0)
					//{
						uint32_t l = pin - p;
						outbytesleft -= ctx->fill_default_char(&pout);
						if (inbytesleft > l) inbytesleft -= l;
						else
						{
							*out_len = pout - out;
							return 0;
						}
					//}
					//else return errno;
				}
				else
				{
					*out_len = pout - out;
					return 0;
				}
			}
			else if (errno == EINVAL)
			{
				ctx->fill_default_char(&pout);
				*out_len = pout - out;
				return 0;
			}
			else return errno;
		}
	}
}

int32_t encoding_convertor_copy_input(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	if(*out_len >= in_len)
	{
		memcpy(out, in, in_len);
		*out_len = in_len;
		return 0;
	}
	else return E2BIG;
}

int32_t iconv_module_initialize()
{
	return encoding_convertor_initialize(&default_encoding_convertor);
}

int32_t iconv_module_finalize()
{
	return encoding_convertor_finalize(&default_encoding_convertor);
}

int32_t to_utf8(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_to_utf8(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t to_gbk(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_to_gbk(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t to_big5(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_to_big5(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t gbk_to_utf8(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_gbk_to_utf8(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t big5_to_utf8(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_big5_to_utf8(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t utf8_to_gbk(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_utf8_to_gbk(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t big5_to_gbk(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_big5_to_gbk(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t gbk_to_big5(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_gbk_to_big5(&default_encoding_convertor, in, in_len, out, out_len);
}

int32_t utf8_to_big5(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return encoding_convertor_utf8_to_big5(&default_encoding_convertor, in, in_len, out, out_len);
}


/************** thread safe ****************/
static int32_t iconv_wrapper(ecv_ctx_t* ctx, const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	int32_t ret = 0;
	size_t n;

	size_t outbytesleft = *out_len;
	char* pout = out;

	//convert arg 2 to char* to adapt the problem in the iconv specification
	//actually, iconv will NOT change the in buffer contents
	char* pin = (char*)in;
	size_t inbytesleft = in_len;

	iconv_t cd = iconv_open(ctx->encoding_name_to, ctx->encoding_name_from);
	if(cd == (iconv_t)-1) return errno;

	for (;;)
	{
		n = iconv(cd, (char**)&pin, &inbytesleft, &pout, &outbytesleft);
		if (n != (size_t)(-1))
		{
			*out_len = pout - out;
			break;
		}
		else
		{
			if (errno == EILSEQ)
			{
				if(inbytesleft !=  0)
				{
					char* p = pin;
					/*uint32_t c =*/ ctx->next_char(pin, (const char**)&pin);
					//if (c != 0)
					//{
						uint32_t l = pin - p;
						outbytesleft -= ctx->fill_default_char(&pout);
						if (inbytesleft > l) inbytesleft -= l;
						else
						{
							*out_len = pout - out;
							break;
						}
					/*}
					else
					{
						ret = errno;
						break;
					}*/
				}
				else
				{
					*out_len = pout - out;
					break;
				}
			}
			else if (errno == EINVAL)
			{
				ctx->fill_default_char(&pout);
				*out_len = pout - out;
				break;
			}
			else
			{
				ret = errno;
				break;
			}
		}
	}

	iconv_close(cd);
	return ret;
}

int32_t to_utf8_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	uint32_t non_common_utf8_num;
	uint32_t common_utf8_num;
	uint32_t invalid_utf8_num;
	uint32_t common_gbk_num;
	uint32_t non_common_gbk_num;
	uint32_t invalid_gbk_num;
	uint32_t common_big5_num;
	uint32_t non_common_big5_num;
	uint32_t invalid_big5_num;

	/* 查找常用字比例超过80%的编码 */
	common_gbk_num = get_str_gbk_char_number(in, in_len, &non_common_gbk_num, &invalid_gbk_num);
	if (invalid_gbk_num == 0 && common_gbk_num > 4 * non_common_gbk_num)
	{ //gbk
		return gbk_to_utf8_s(in, in_len, out, out_len);
	}

	common_utf8_num = get_str_utf8_char_number(in, in_len, &non_common_utf8_num, &invalid_utf8_num);
	if (invalid_utf8_num == 0 && common_utf8_num > 4 * non_common_utf8_num)
	{ //utf8
		return encoding_convertor_copy_input(in, in_len, out, out_len);
	}

	common_big5_num = get_str_big5_char_number(in, in_len, &non_common_big5_num, &invalid_big5_num);
	if (invalid_big5_num == 0 && common_big5_num > 4 * non_common_big5_num)
	{ //big5
		return big5_to_utf8_s(in, in_len, out, out_len);
	}

	if (common_gbk_num == 0 && common_utf8_num == 0 && common_big5_num == 0) return -1;

	/* 获取发现有效字符数最多的编码 */
	if (common_gbk_num >= common_big5_num)
	{ //gbk or utf8
		if (common_gbk_num >= common_utf8_num)
		{ //gbk
			return gbk_to_utf8_s(in, in_len, out, out_len);
		}
		else
		{ //utf8
			if(invalid_utf8_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(utf8_next_char, utf8_fill_question_mark, in, in_len, out, out_len);
		}
	}
	else
	{ // big5 or utf8
		if (common_utf8_num >= common_big5_num)
		{ //utf8
			if(invalid_utf8_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(utf8_next_char, utf8_fill_question_mark, in, in_len, out, out_len);
		}
		else
		{ //big5
			return big5_to_utf8_s(in, in_len, out, out_len);
		}
	}
}

int32_t to_gbk_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	uint32_t non_common_utf8_num;
	uint32_t common_utf8_num;
	uint32_t invalid_utf8_num;
	uint32_t common_gbk_num;
	uint32_t non_common_gbk_num;
	uint32_t invalid_gbk_num;
	uint32_t common_big5_num;
	uint32_t non_common_big5_num;
	uint32_t invalid_big5_num;

	/* 查找常用字比例超过80%的编码 */
	common_gbk_num = get_str_gbk_char_number(in, in_len, &non_common_gbk_num, &invalid_gbk_num);
	if (invalid_gbk_num == 0 && common_gbk_num > 4 * non_common_gbk_num)
	{ //gbk
		return encoding_convertor_copy_input(in, in_len, out, out_len);
	}

	common_utf8_num = get_str_utf8_char_number(in, in_len, &non_common_utf8_num, &invalid_utf8_num);
	if (invalid_utf8_num == 0 && common_utf8_num > 4 * non_common_utf8_num)
	{ //utf8
		return utf8_to_gbk_s(in, in_len, out, out_len);
	}

	common_big5_num = get_str_big5_char_number(in, in_len, &non_common_big5_num, &invalid_big5_num);
	if (invalid_big5_num == 0 && common_big5_num > 4 * non_common_big5_num)
	{ //big5
		return big5_to_gbk_s(in, in_len, out, out_len);
	}

	if (common_gbk_num == 0 && common_utf8_num == 0 && common_big5_num == 0) return -1;

	/* 获取发现有效字符数最多的编码 */
	if (common_gbk_num >= common_big5_num)
	{ //gbk or utf8
		if (common_gbk_num >= common_utf8_num)
		{ //gbk
			if(invalid_gbk_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(gbk_next_char, gbk_fill_question_mark, in, in_len, out, out_len);
		}
		else
		{ //utf8
			return utf8_to_gbk_s(in, in_len, out, out_len);
		}
	}
	else
	{ // big5 or utf8
		if (common_utf8_num >= common_big5_num)
		{ //utf8
			return utf8_to_gbk_s(in, in_len, out, out_len);
		}
		else
		{ //big5
			return big5_to_gbk_s(in, in_len, out, out_len);
		}
	}
}

int32_t to_big5_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	uint32_t non_common_utf8_num;
	uint32_t common_utf8_num;
	uint32_t invalid_utf8_num;
	uint32_t common_gbk_num;
	uint32_t non_common_gbk_num;
	uint32_t invalid_gbk_num;
	uint32_t common_big5_num;
	uint32_t non_common_big5_num;
	uint32_t invalid_big5_num;

	/* 查找常用字比例超过80%的编码 */
	common_gbk_num = get_str_gbk_char_number(in, in_len, &non_common_gbk_num, &invalid_gbk_num);
	if (invalid_gbk_num == 0 && common_gbk_num > 4 * non_common_gbk_num)
	{ //gbk
		return gbk_to_big5_s(in, in_len, out, out_len);
	}

	common_utf8_num = get_str_utf8_char_number(in, in_len, &non_common_utf8_num, &invalid_utf8_num);
	if (invalid_utf8_num == 0 && common_utf8_num > 4 * non_common_utf8_num)
	{ //utf8
		return utf8_to_big5_s(in, in_len, out, out_len);
	}

	common_big5_num = get_str_big5_char_number(in, in_len, &non_common_big5_num, &invalid_big5_num);
	if (invalid_big5_num == 0 && common_big5_num > 4 * non_common_big5_num)
	{ //big5
		return encoding_convertor_copy_input(in, in_len, out, out_len);
	}

	if (common_gbk_num == 0 && common_utf8_num == 0 && common_big5_num == 0) return -1;

	/* 获取发现有效字符数最多的编码 */
	if (common_gbk_num >= common_big5_num)
	{ //gbk or utf8
		if (common_gbk_num >= common_utf8_num)
		{ //gbk
			return gbk_to_big5_s(in, in_len, out, out_len);
		}
		else
		{ //utf8
			return utf8_to_big5_s(in, in_len, out, out_len);
		}
	}
	else
	{ // big5 or utf8
		if (common_utf8_num >= common_big5_num)
		{ //utf8
			return utf8_to_big5_s(in, in_len, out, out_len);
		}
		else
		{ //big5
			if(invalid_big5_num == 0) return encoding_convertor_copy_input(in, in_len, out, out_len);
			else return fix_encoding(big5_next_char, big5_fill_question_mark, in, in_len, out, out_len);
		}
	}
}

int32_t gbk_to_utf8_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return iconv_wrapper(&gbk_to_utf8_ctx, in, in_len, out, out_len);
}

int32_t big5_to_utf8_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return iconv_wrapper(&big5_to_utf8_ctx, in, in_len, out, out_len);
}

int32_t utf8_to_gbk_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return iconv_wrapper(&utf8_to_gbk_ctx, in, in_len, out, out_len);
}

int32_t big5_to_gbk_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return iconv_wrapper(&big5_to_gbk_ctx, in, in_len, out, out_len);
}

int32_t gbk_to_big5_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return iconv_wrapper(&gbk_to_big5_ctx, in, in_len, out, out_len);
}

int32_t utf8_to_big5_s(const char* in, uint32_t in_len, char* out, uint32_t* out_len)
{
	return iconv_wrapper(&utf8_to_big5_ctx, in, in_len, out, out_len);
}

#if 0
//unit test
#include <stdio.h>
#include <ctype.h>

void detect_encoding(const char* str)
{
	uint32_t len = strlen(str);
	if(is_str_utf8(str, len)) puts("encoding: utf8");
	if(is_str_gbk(str, len)) puts("encoding: gbk");
	if(is_str_big5(str, len)) puts("encoding: big5");
	//puts("encoding: unrecoginized");
}

void print_hex(const char* str)
{
	int i;
	for(i=0; str[i]; ++i)
	{
		printf("%02x", (unsigned char)str[i]);
	}
	putchar('\n');
}

uint8_t hex_value(uint8_t v)
{
	return v>'9' ? toupper(v)-'A' + 10 : v-'0'; 
}

int32_t decode_hex(char* in)
{
	const uint8_t* p = (const uint8_t*)in;
	char* po = in;
	while(*p)
	{
		if(*p=='%')
		{
			*po = hex_value(*++p) << 4;
			*po++ |= hex_value(*++p);
			++p;
		}
		else *po++ = *p++;
	}
	return po - in;
}

int main()
{
	char buf[16*1024];
	char buf1[16*1024];
	char gbk_str[128] = "\xC4\xE3\xBA\xC3\x61\x62\x63"; //你好abc
	char utf8_str[128];
	uint32_t out_len;
	int ret = iconv_module_initialize();
	if(ret != 0)
	{
		perror("init conv ");
		return -1;
	}
	
	detect_encoding(gbk_str);

	out_len = 16*1024;
	ret = to_gbk(gbk_str, strlen(gbk_str), buf, &out_len);
	buf[out_len] = 0;
	printf("gbk :");
	print_hex(buf);

	out_len = 16*1024;
	ret = to_big5(gbk_str, strlen(gbk_str), buf, &out_len);
	buf[out_len] = 0;
	printf("big5 :");
	print_hex(buf);

	out_len = 16*1024;
	ret = to_utf8(gbk_str, strlen(gbk_str), buf, &out_len);
	buf[out_len] = 0;
	printf("utf8 :");
	print_hex(buf);
	
	//thread safe interface test
	out_len = 16*1024;
	ret = to_gbk_s(gbk_str, strlen(gbk_str), buf, &out_len);
	buf[out_len] = 0;
	printf("gbk :");
	print_hex(buf);

	out_len = 16*1024;
	ret = to_big5_s(gbk_str, strlen(gbk_str), buf, &out_len);
	buf[out_len] = 0;
	printf("big5 :");
	print_hex(buf);

	out_len = 16*1024;
	ret = to_utf8_s(gbk_str, strlen(gbk_str), buf, &out_len);
	buf[out_len] = 0;
	printf("utf8 :");
	print_hex(buf);

	while(fgets(buf1, 16*1024, stdin))
	{
		uint32_t n = decode_hex(buf1);
		buf1[n] = 0;
		printf("input: ");
		print_hex(buf1);
		detect_encoding(buf1);
		out_len = 16*1024;
		to_utf8(buf1, n, buf, &out_len);
		buf[out_len] = 0;
		printf("output: ");
		print_hex(buf);
		puts(buf);
	}
}

#endif
