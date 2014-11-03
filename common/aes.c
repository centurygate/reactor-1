/**
# -*- coding:UTF-8 -*-
*/

#include "aes.h"
#include "define.h"
#include "error_code.h"
#include <string.h>

#ifndef _USE_ET_AES

static uint8_t Sbox[16*16]=
{	 /* populate the Sbox matrix */
		/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
		/*0*/  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
		/*1*/  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
		/*2*/  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
		/*3*/  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
		/*4*/  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
		/*5*/  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
		/*6*/  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
		/*7*/  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
		/*8*/  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
		/*9*/  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
		/*a*/  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
		/*b*/  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
		/*c*/  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
		/*d*/  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
		/*e*/  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
		/*f*/  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static uint8_t iSbox[16*16]=
{
	/* populate the iSbox matrix */
		/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
		/*0*/  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
		/*1*/  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
		/*2*/  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
		/*3*/  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
		/*4*/  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
		/*5*/  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
		/*6*/  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
		/*7*/  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
		/*8*/  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
		/*9*/  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
		/*a*/  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
		/*b*/  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
		/*c*/  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
		/*d*/  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
		/*e*/  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
		/*f*/  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static uint8_t Rcon[11*4]=
{
	0x00, 0x00, 0x00, 0x00,  
	0x01, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x00,
	0x10, 0x00, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00,
	0x80, 0x00, 0x00, 0x00,
	0x1b, 0x00, 0x00, 0x00,
	0x36, 0x00, 0x00, 0x00
};

static void SetNbNkNr(ctx_aes* aes, int32_t keyS);
static void AddRoundKey(ctx_aes* aes, int32_t round);
static void SubBytes(ctx_aes* aes);
static void InvSubBytes(ctx_aes* aes);
static void ShiftRows(ctx_aes* aes);
static void InvShiftRows(ctx_aes* aes);
static void MixColumns(ctx_aes* aes);
static void InvMixColumns(ctx_aes* aes);
static uint8_t  gfmultby01(uint8_t b);
static uint8_t gfmultby02(uint8_t b);
static uint8_t gfmultby03(uint8_t b);
static unsigned char gfmultby09(unsigned char b);
static unsigned char gfmultby0b(unsigned char b);
static unsigned char gfmultby0d(unsigned char b);
static unsigned char gfmultby0e(unsigned char b);
static void KeyExpansion(ctx_aes* aes);
static void SubWord(uint8_t *word, uint8_t *result);
static void RotWord(uint8_t *word, uint8_t *result);

void aes_init(ctx_aes* aes, int keySize, uint8_t* keyBytes)
{
	SetNbNkNr(aes, keySize);//初始化
	memcpy(aes->key,keyBytes,keySize);
	KeyExpansion(aes);
	/* expand the seed key into a key schedule and store in w */
}

void aes_cipher(ctx_aes* aes, uint8_t* input, uint8_t* output)  // encipher 16-bit input
{
	// state = input
	int i;
	int round;
	memset(&aes->State[0][0],0,16);
	for (i = 0; i < (4 * aes->Nb); i++)//
	{
        aes->State[i % 4][ i / 4] = input[i];
	}
	AddRoundKey(aes, 0);
	for (round = 1; round <= (aes->Nr - 1); round++)  // main round loop
	{
        SubBytes(aes); 
        ShiftRows(aes);  
        MixColumns(aes); 
        AddRoundKey(aes, round);
	}  // main round loop
	
	SubBytes(aes);
	ShiftRows(aes);
	AddRoundKey(aes, aes->Nr);
	
	// output = state
	for (i = 0; i < (4 * aes->Nb); i++)
	{
        output[i] =  aes->State[i % 4][ i / 4];
	}
	
}  // Cipher()

void aes_invcipher(ctx_aes* aes, uint8_t* input, uint8_t* output)	 // decipher 16-bit input
{
	// state = input
	int i; 
	int round;
	memset(&aes->State[0][0],0,16);
	for (i = 0; i < (4 * aes->Nb); i++)
	{
		aes->State[i % 4][ i / 4] = input[i];
	}
	
	AddRoundKey(aes, aes->Nr);
	
	for (round = aes->Nr-1; round >= 1; round--)  // main round loop
	{
        InvShiftRows(aes);
        InvSubBytes(aes);
        AddRoundKey(aes, round);
        InvMixColumns(aes);
	}  // end main round loop for InvCipher
	
	InvShiftRows(aes);
	InvSubBytes(aes);
	AddRoundKey(aes, 0);
	
	// output = state
	for (i = 0; i < (4 * aes->Nb); i++)
	{
        output[i] =  aes->State[i % 4][ i / 4];
	}
	
}  // InvCipher()

void SetNbNkNr(ctx_aes* aes, int32_t keyS)
{
	aes->Nb = 4;     // block size always = 4 words = 16 bytes = 128 bits for AES
	aes->Nk = 4;
	if (keyS == Bits128)
	{
		aes->Nk = 4;   // key size = 4 words = 16 bytes = 128 bits
		aes->Nr = 10;  // rounds for algorithm = 10
	}
	else if (keyS == Bits192)
	{
		aes->Nk = 6;   // 6 words = 24 bytes = 192 bits
		aes->Nr = 12;
	}
	else if (keyS == Bits256)
	{
		aes->Nk = 8;   // 8 words = 32 bytes = 256 bits
		aes->Nr = 14;
	}
}  // SetNbNkNr()

void AddRoundKey(ctx_aes* aes, int32_t round)
{
	int r,c;
	for (r = 0; r < 4; r++)
	{
        for (c = 0; c < 4; c++)
        {//w:    4*x+y
			aes->State[r][c]=(unsigned char)((int)aes->State[r][c]^(int)aes->w[4*((round*4)+c)+r]);
        }
	}
}  // AddRoundKey()

void SubBytes(ctx_aes* aes)
{
	int r,c;
	for (r = 0; r < 4; r++)
	{
        for (c = 0; c < 4; c++)
        {
			aes->State[r][c] =  Sbox[ 16*(aes->State[r][c] >> 4)+ ( aes->State[r][c] & 0x0f) ];
        }
	}
}  // SubBytes

void InvSubBytes(ctx_aes* aes)
{
	int r,c;
	for (r = 0; r < 4; r++)
	{
        for (c = 0; c < 4; c++)
        {
			aes->State[r][c] =  iSbox[ 16*( aes->State[r][c] >> 4)+( aes->State[r][c] & 0x0f) ];
        }
	}
}  // InvSubBytes

void ShiftRows(ctx_aes* aes)
{
	unsigned char temp[4*4];
	int r,c;
	for (r = 0; r < 4; r++)  // copy State into temp[]
	{
        for (c = 0; c < 4; c++)
        {
			temp[4*r+c] =  aes->State[r][c];
        }
	}
	//??
	for (r = 1; r < 4; r++)  // shift temp into State
	{
        for (c = 0; c < 4; c++)
        {
			aes->State[r][c] = temp[ 4*r+ (c + r) % aes->Nb ];
        }
	}
}  // ShiftRows()

void InvShiftRows(ctx_aes* aes)
{
	unsigned char temp[4*4];
	int r,c;
	for (r = 0; r < 4; r++)  // copy State into temp[]
	{
        for (c = 0; c < 4; c++)
        {
			temp[4*r+c] =  aes->State[r][c];
        }
	}
	for (r = 1; r < 4; r++)  // shift temp into State
	{
        for (c = 0; c < 4; c++)
        {
			aes->State[r][ (c + r) % aes->Nb ] = temp[4*r+c];
        }
	}
}  // InvShiftRows()

void MixColumns(ctx_aes* aes)
{
	unsigned char temp[4*4];
	int r,c;
	for (r = 0; r < 4; r++)  // copy State into temp[]
	{
        for (c = 0; c < 4; c++)
        {
			temp[4*r+c] =  aes->State[r][c];
        }
	}
	
	for (c = 0; c < 4; c++)
	{
		aes->State[0][c] = (unsigned char) ( (int)gfmultby02(temp[0+c]) ^ (int)gfmultby03(temp[4*1+c]) ^
			(int)gfmultby01(temp[4*2+c]) ^ (int)gfmultby01(temp[4*3+c]) );
		aes->State[1][c] = (unsigned char) ( (int)gfmultby01(temp[0+c]) ^ (int)gfmultby02(temp[4*1+c]) ^
			(int)gfmultby03(temp[4*2+c]) ^ (int)gfmultby01(temp[4*3+c]) );
		aes->State[2][c] = (unsigned char) ( (int)gfmultby01(temp[0+c]) ^ (int)gfmultby01(temp[4*1+c]) ^
			(int)gfmultby02(temp[4*2+c]) ^ (int)gfmultby03(temp[4*3+c]) );
		aes->State[3][c] = (unsigned char) ( (int)gfmultby03(temp[0+c]) ^ (int)gfmultby01(temp[4*1+c]) ^
			(int)gfmultby01(temp[4*2+c]) ^ (int)gfmultby02(temp[4*3+c]) );
	}
}  // MixColumns

void InvMixColumns(ctx_aes* aes)
{
	unsigned char temp[4*4];
	int r,c;
	for (r = 0; r < 4; r++)  // copy State into temp[]
	{
        for (c = 0; c < 4; c++)
        {
			temp[4*r+c] =  aes->State[r][c];
        }
	}
	
	for (c = 0; c < 4; c++)
	{
		aes->State[0][c] = (unsigned char) ( (int)gfmultby0e(temp[c]) ^ (int)gfmultby0b(temp[4+c]) ^
			(int)gfmultby0d(temp[4*2+c]) ^ (int)gfmultby09(temp[4*3+c]) );
		aes->State[1][c] = (unsigned char) ( (int)gfmultby09(temp[c]) ^ (int)gfmultby0e(temp[4+c]) ^
			(int)gfmultby0b(temp[4*2+c]) ^ (int)gfmultby0d(temp[4*3+c]) );
		aes->State[2][c] = (unsigned char) ( (int)gfmultby0d(temp[c]) ^ (int)gfmultby09(temp[4+c]) ^
			(int)gfmultby0e(temp[4*2+c]) ^ (int)gfmultby0b(temp[4*3+c]) );
		aes->State[3][c] = (unsigned char) ( (int)gfmultby0b(temp[c]) ^ (int)gfmultby0d(temp[4+c]) ^
			(int)gfmultby09(temp[4*2+c]) ^ (int)gfmultby0e(temp[4*3+c]) );
	}
}  // InvMixColumns

uint8_t  gfmultby01(uint8_t b)
{
	return b;
}

uint8_t gfmultby02(uint8_t b)
{
	if( b < 0x80 )
		return (uint8_t)(int32_t)(b <<1);
	else
		return (uint8_t)( (int32_t)(b << 1) ^ (int32_t)(0x1b) );
}

uint8_t gfmultby03(uint8_t b)
{
	return (uint8_t) ( (int32_t)gfmultby02(b) ^ (int32_t)b );
}

unsigned char gfmultby09(unsigned char b)
{
	return (unsigned char)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)b );
}

unsigned char gfmultby0b(unsigned char b)
{
	return (unsigned char)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)gfmultby02(b) ^
		(int)b );
}

unsigned char gfmultby0d(unsigned char b)
{
	return (unsigned char)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)gfmultby02(gfmultby02(b)) ^
		(int)(b) );
}

unsigned char gfmultby0e(unsigned char b)
{
	return (unsigned char)( (int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)gfmultby02(gfmultby02(b)) ^
		(int)gfmultby02(b) );
}

void KeyExpansion(ctx_aes* aes)
{
	int row;
	uint8_t temp[4];
	uint8_t result[4],result2[4];
    memset(aes->w,0,16*15);
	for (row = 0; row < aes->Nk; row++)//Nk=4,6,8
	{
		aes->w[4*row+0] =  aes->key[4*row];
		aes->w[4*row+1] =  aes->key[4*row+1];
		aes->w[4*row+2] =  aes->key[4*row+2];
		aes->w[4*row+3] =  aes->key[4*row+3];
	}
	for (row = aes->Nk; row < aes->Nb * (aes->Nr+1); row++)
	{
        temp[0] =  aes->w[4*(row-1)+0]; 
		temp[1] =  aes->w[4*(row-1)+1];
        temp[2] =  aes->w[4*(row-1)+2]; 
		temp[3] =  aes->w[4*(row-1)+3];
		
        if (row % aes->Nk == 0)  
        {
			RotWord(temp,result);
			SubWord(result,result2);
			memcpy(temp,result2,4);//
			
			temp[0] = (unsigned char)( (int)temp[0] ^ (int) Rcon[4*(row/aes->Nk)+0] );
			temp[1] = (unsigned char)( (int)temp[1] ^ (int) Rcon[4*(row/aes->Nk)+1] );
			temp[2] = (unsigned char)( (int)temp[2] ^ (int) Rcon[4*(row/aes->Nk)+2] );
			temp[3] = (unsigned char)( (int)temp[3] ^ (int) Rcon[4*(row/aes->Nk)+3] );
        }
        else if ( aes->Nk > 6 && (row % aes->Nk == 4) )  
        {
			SubWord(temp,result);
			memcpy(temp,result,4);
        }
        // w[row] = w[row-Nk] xor temp
		aes->w[4*row+0] = (unsigned char)( (int) aes->w[4*(row-aes->Nk)+0] ^ (int)temp[0] );
		aes->w[4*row+1] = (unsigned char)( (int) aes->w[4*(row-aes->Nk)+1] ^ (int)temp[1] );
		aes->w[4*row+2] = (unsigned char)( (int) aes->w[4*(row-aes->Nk)+2] ^ (int)temp[2] );
		aes->w[4*row+3] = (unsigned char)( (int) aes->w[4*(row-aes->Nk)+3] ^ (int)temp[3] );
	}  // for loop
}  // KeyExpansion()

void SubWord(uint8_t *word,uint8_t *result)
{
	result[0] =  Sbox[ 16*(word[0] >> 4)+ (word[0] & 0x0f) ];
	result[1] =  Sbox[ 16*(word[1] >> 4)+ (word[1] & 0x0f) ];
	result[2] =  Sbox[ 16*(word[2] >> 4)+ (word[2] & 0x0f) ];
	result[3] =  Sbox[ 16*(word[3] >> 4)+ (word[3] & 0x0f) ];
}

void RotWord(uint8_t *word, uint8_t *result)
{
	result[0] = word[1];
	result[1] = word[2];
	result[2] = word[3];
	result[3] = word[0];
}

#endif

int32_t aes_encrypt(char* in_buffer ,uint32_t in_len, char* out_buffer, uint32_t* out_len, uint8_t key[16])
{
    ctx_aes aes;
    int offset;
    uint32_t need_len;
    uint32_t remain_len;
    uint8_t buf[ENCRYPT_BLOCK_SIZE];

    if(in_buffer==NULL || out_buffer==NULL || out_len==NULL || key==NULL)
        return ERR_INVALID_PARAMETER;

    need_len = (in_len&~(ENCRYPT_BLOCK_SIZE-1)) + ENCRYPT_BLOCK_SIZE;

    if(*out_len < need_len) return ERR_NOT_ENOUGH_BUFFER;

    aes_init(&aes, 16, key);

    *out_len = need_len;
    need_len -= ENCRYPT_BLOCK_SIZE;
    for(offset = 0 ; offset != need_len; offset+=ENCRYPT_BLOCK_SIZE)
    {
        aes_cipher(&aes, (uint8_t*)in_buffer+offset, (uint8_t*)out_buffer+offset);
    }
    remain_len = in_len&(ENCRYPT_BLOCK_SIZE-1);
    memcpy(buf, in_buffer+offset, remain_len);
    memset(buf+remain_len, ENCRYPT_BLOCK_SIZE-remain_len, ENCRYPT_BLOCK_SIZE-remain_len);
    aes_cipher(&aes, buf, (uint8_t*)out_buffer+offset);

    return 0;
}

int32_t aes_decrypt(char* in_buffer ,uint32_t in_len, char* out_buffer, uint32_t* out_len, uint8_t key[16])
{
    ctx_aes aes;
    int offset;

    if(in_buffer==NULL || out_buffer==NULL || out_len==NULL || key==NULL)
        return ERR_INVALID_PARAMETER;

    if(in_len%ENCRYPT_BLOCK_SIZE != 0 ) return ERR_INVALID_PARAMETER;

    if(*out_len < in_len) return ERR_NOT_ENOUGH_BUFFER;

    aes_init(&aes, 16, key);

    for(offset = 0 ; in_len != offset; offset+=ENCRYPT_BLOCK_SIZE)
    {
        aes_invcipher(&aes, (uint8_t*)in_buffer+offset, (uint8_t*)out_buffer+offset);
    }

    if(out_buffer[in_len-1] > ENCRYPT_BLOCK_SIZE)
        return ERR_INVALID_PARAMETER;

    *out_len = in_len - out_buffer[in_len-1];

    return 0;
}
