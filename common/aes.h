/**
# -*- coding:UTF-8 -*-
*/

#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif

#define Bits128				16  /*密钥长度*/
#define Bits192				24
#define Bits256				32
#define ENCRYPT_BLOCK_SIZE	16

typedef struct  
{
	int32_t Nb;         /* block size in 32-bit words.  Always 4 for AES.  (128 bits). */
	int32_t Nk;         /* key size in 32-bit words.  4, 6, 8.  (128, 192, 256 bits). */
	int32_t Nr;         /* number of rounds. 10, 12, 14. */
	uint8_t	   State[4][4];/* State matrix */
	uint8_t	   key[32];    /* the seed key. size will be 4 *keySize from ctor. */
	uint8_t    w[16*15];   /* key schedule array. (Nb*(Nr+1))*4 */
}ctx_aes;

void aes_init(ctx_aes* aes, int keySize, uint8_t* keyBytes);
void aes_cipher(ctx_aes* aes, uint8_t* input, uint8_t* output);  /* encipher 16-bit input */
void aes_invcipher(ctx_aes* aes, uint8_t* input, uint8_t* output);

int32_t aes_encrypt(char* in_buffer ,uint32_t in_len, char* out_buffer, uint32_t* out_len, uint8_t key[16]);
int32_t aes_decrypt(char* in_buffer ,uint32_t in_len, char* out_buffer, uint32_t* out_len, uint8_t key[16]);

#ifdef __cplusplus
}
#endif

#endif
