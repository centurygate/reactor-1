/**
# -*- coding:UTF-8 -*-
*/

#ifndef _MD5_H_
#define _MD5_H_

#ifdef __cplusplus
extern "C" 
{
#endif

//小数据量 md5 算法, < 8KB, 超过会崩溃
void md5_hash_data(const char* data, int data_len, char* val);

#ifdef __cplusplus
}
#endif

#endif
