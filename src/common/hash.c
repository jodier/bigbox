/* Author : Jerome ODIER
 * Email : odier@hypnos-solutions.com
 *
 * Version : 1.0 (2016-![VALUE YEAR])
 *
 *
 * This file is part of BIGBOX.
 *
 *
 * Hypnos-Solutions RESTRICTED
 * ___________________________
 *
 *  Hypnos-Solutions
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Hypnos-Solutions and its suppliers, if any.
 * The intellectual and technical concepts contained herein are
 * proprietary to Hypnos-Solutions and its suppliers and are
 * protected by trade secret or copyright law. Dissemination of
 * this information or reproduction of this material is strictly
 * forbidden unless prior written permission is obtained from
 * Hypnos-Solutions.
 */

/*-------------------------------------------------------------------------*/

#include "internal.h"

/*-------------------------------------------------------------------------*/

#define cROUNDS 2
#define dROUNDS 4

/*-------------------------------------------------------------------------*/

static __inline__ uint64_t rotl(uint64_t x, uint64_t b) /* will be optimized by the compiler */
{
	return (x << b) | (x >> (64 - b));
}

/*-------------------------------------------------------------------------*/

#define SIP_ROUND()                                                         \
	do {                                                                \
		v0 += v1;                                                   \
		v1 = rotl(v1, 13);                                          \
		v1 ^= v0;                                                   \
		v0 = rotl(v0, 32);                                          \
		v2 += v3;                                                   \
		v3 = rotl(v3, 16);                                          \
		v3 ^= v2;                                                   \
		v0 += v3;                                                   \
		v3 = rotl(v3, 21);                                          \
		v3 ^= v0;                                                   \
		v2 += v1;                                                   \
		v1 = rotl(v1, 17);                                          \
		v1 ^= v2;                                                   \
		v2 = rotl(v2, 32);                                          \
	} while(0)

/*-------------------------------------------------------------------------*/

uint64_t bigbox_hash(BUFF_t buff, size_t size, uint64_t seed1, uint64_t seed2)
{
	int i;

	/*-----------------------------------------------------------------*/

	uint64_t v0 = 0x736f6d6570736575ULL;
	uint64_t v1 = 0x646f72616e646f6dULL;
	uint64_t v2 = 0x6c7967656e657261ULL;
	uint64_t v3 = 0x7465646279746573ULL;

	v3 ^= seed2;
	v2 ^= seed1;
	v1 ^= seed2;
	v0 ^= seed1;

	/*-----------------------------------------------------------------*/

	const uint64_t size_div_8 = size / 8UL;
	const uint64_t size_mod_8 = size % 8UL;

	register /***/ uint8_t *p = ((uint8_t *) buff) + 0 * size_div_8;
	register const uint8_t *q = ((uint8_t *) buff) + 8 * size_div_8;

	/*-----------------------------------------------------------------*/

	register uint64_t m;

	for(m = *p; p != q; m = *(p += 8))
	{
		v3 ^= m;
		for(i = 0; i < cROUNDS; ++i) SIP_ROUND();
		v0 ^= m;
	}

	/*-----------------------------------------------------------------*/

	uint64_t b = ((uint64_t) size) << (64 - 8);

	switch(size_mod_8)
	{
		case 7:
			b |= ((uint64_t) p[6]) << 48;
		case 6:
			b |= ((uint64_t) p[5]) << 40;
		case 5:
			b |= ((uint64_t) p[4]) << 32;
		case 4:
			b |= ((uint64_t) p[3]) << 24;
		case 3:
			b |= ((uint64_t) p[2]) << 16;
		case 2:
			b |= ((uint64_t) p[1]) << 8;
		case 1:
			b |= ((uint64_t) p[0]) << 0;
		case 0:
			break;
	}

	/*-----------------------------------------------------------------*/

	v3 ^= b;
	for(i = 0; i < cROUNDS; i++) SIP_ROUND();
	v0 ^= b;

	/*-----------------------------------------------------------------*/

	v2 ^= 0xEE;
	for(i = 0; i < dROUNDS; i++) SIP_ROUND();
	v1 ^= 0x00;

	/*-----------------------------------------------------------------*/

	return v0 ^ v1 ^ v2 ^ v3;

	/*-----------------------------------------------------------------*/
}

/*-------------------------------------------------------------------------*/
