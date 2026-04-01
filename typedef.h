
#include "stdint.h"
#include "user.h"

#ifndef _typedef_H
#define _typedef_H

typedef unsigned char         UINT8;
typedef unsigned int          UINT16;
typedef unsigned long         UINT32;

//typedef unsigned char         uint8_t;
//typedef unsigned int          uint16_t;
//typedef unsigned long         uint32_t;

typedef unsigned char         u8;
typedef unsigned int          u16;
typedef unsigned long         u32;


#define   i8	int8_t
#define   i16 int16_t
#define   i32 int32_t

typedef union
{
	struct
	{
		uint8_t bit0:1;
		uint8_t bit1:1;
		uint8_t bit2:1;
		uint8_t bit3:1;
		uint8_t bit4:1;
		uint8_t bit5:1;
		uint8_t bit6:1;
		uint8_t bit7:1;
	}bitn;
	uint8_t byte;
} byte_bit_t;

typedef union
{
	struct
	{
		uint8_t bit0:1;
		uint8_t bit1:1;
		uint8_t bit2:1;
		uint8_t bit3:1;
		uint8_t bit4:1;
		uint8_t bit5:1;
		uint8_t bit6:1;
		uint8_t bit7:1;
		uint8_t bit8:1;
		uint8_t bit9:1;
		uint8_t bit10:1;
		uint8_t bit11:1;
		uint8_t bit12:1;
		uint8_t bit13:1;
		uint8_t bit14:1;
		uint8_t bit15:1;
	}bitn;
	uint16_t uint16;
} uint16_bit_t;

typedef union
{
	struct
	{
		uint8_t bit0:1;
		uint8_t bit1:1;
		uint8_t bit2:1;
		uint8_t bit3:1;
		uint8_t bit4:1;
		uint8_t bit5:1;
		uint8_t bit6:1;
		uint8_t bit7:1;
		uint8_t bit8:1;
		uint8_t bit9:1;
		uint8_t bit10:1;
		uint8_t bit11:1;
		uint8_t bit12:1;
		uint8_t bit13:1;
		uint8_t bit14:1;
		uint8_t bit15:1;
		uint8_t bit16:1;
		uint8_t bit17:1;
		uint8_t bit18:1;
		uint8_t bit19:1;
		uint8_t bit20:1;
		uint8_t bit21:1;
		uint8_t bit22:1;
		uint8_t bit23:1;
		uint8_t bit24:1;
		uint8_t bit25:1;
		uint8_t bit26:1;
		uint8_t bit27:1;
		uint8_t bit28:1;
		uint8_t bit29:1;
		uint8_t bit30:1;
		uint8_t bit31:1;
	}bitn;
	uint32_t word;
} word_bit_t;



typedef union
{
	u8	data8[2];
	u16 data16;
} union_16and8_t;




typedef void (* func_void_t)();	//ÃÂ¸ÃÃ²ÃÃÂ·ÂµÂ»ÃÃÂµÂµÃÂºÂ¯ÃÃ½ÃÂ¸ÃÃ«

//32bitÃÃ½Â¾ÃÃÃ ÃÃÂµÃÃÃ Â»Â¥ÃÂªÂ»Â»
typedef union
{
	float		f32;		//floatÃÃÃÃ½Â¾Ã
	uint32_t 	uint32;		//uint32_tÃÃÃÃ½Â¾Ã
	uint16_t	uint16[2];	//uint16_tÃÃÃÃ½Â¾Ã
	uint8_t		uint8[4];	//uint8_tÃÃÃÃ½Â¾Ã
	int32_t		int32;		//int32_tÃÃÃÃ½Â¾Ã
	int16_t		int16[2];	//int16_tÃÃÃÃ½Â¾Ã
	int8_t		int8[4];	//int8_tÃÃÃÃ½Â¾Ã
}type32_t;


#define Bit(n)					(1 << (n))			//bitn
#define BitSet(data, bits)		((data) |= (bits))		//Ã¤Â½ÂÃ§Â½Â®1
#define BitClr(data, bits)		((data) &= ~(bits))	//Ã¤Â½ÂÃ¦Â¸ÂÃ©Â?
#define BitXor(data, bits)		((data) ^= (bits))		//Ã¤Â½ÂÃ¥ÂÂÃ¥Â?
#define BitCheck(data, bits)	((data) & (bits))		//Ã¤Â½ÂÃ¥ÂÂ¤Ã¦Â?


#endif
