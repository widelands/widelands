/* md5.cc - Functions to compute MD5 message digest of files or memory blocks
 * according to the definition of MD5 in RFC 1321 from April 1992.
 *
 * Thanks to Ulrich Drepper for the md5sum example code
 *
 * Copyright (C) 2002, 2007 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <cassert>

#include "md5.h"


// Note that the implementation of MD5Checksum is basically just
// a wrapper around these functions, which have been taken basically
// verbatim (with some whitespace changes) from the GNU tools; see below.
static void * md5_finish_ctx (md5_ctx* ctx, void* resbuf);
static void md5_process_bytes (const void* buffer, ulong len, struct md5_ctx* ctx);
static void md5_process_block (const void* buffer, ulong len, md5_ctx* ctx);


/**
 * Create a hex string out of the MD5 checksum.
 */
std::string md5_checksum::str() const
{
	std::string s;

	for(uint i = 0; i < sizeof(data); ++i) {
		char buf[3];
		snprintf(buf, sizeof(buf), "%02x", data[i]);
		s += buf;
	}

	return s;
}


/**
 * Default Constructor
 */
MD5Checksum::MD5Checksum()
{
	Reset();
}


/**
 * Reset the checksumming machinery to its initial state.
 */
void MD5Checksum::Reset()
{
	can_handle_data = 1;

	ctx.A = 0x67452301;
	ctx.B = 0xefcdab89;
	ctx.C = 0x98badcfe;
	ctx.D = 0x10325476;

	ctx.total[0] = ctx.total[1] = 0;
	ctx.buflen = 0;
}


/**
 * This function consumes new data. It buffers it and calculates
 * one MD5 block when the buffer is full.
 *
 * \param data data to compute chksum for
 * \param length len of data
 */
void MD5Checksum::Data(const void * const data, const size_t length)
{
	assert(can_handle_data);

	md5_process_bytes(data, length, &ctx);
}


/**
 * This function finishes the checksum calculation.
 * After this, no more data may be written to the checksum.
 */
void MD5Checksum::FinishChecksum()
{
	assert(can_handle_data);

	can_handle_data = 0;

	md5_finish_ctx(&ctx, sum.data);
}


/**
 * Retrieve the checksum. Note that \ref FinishChecksum must be called
 * before this function.
 *
 * \return a pointer to an array of 16 bytes containing the checksum.
 */
const md5_checksum& MD5Checksum::GetChecksum() const
{
	 assert(!can_handle_data);

	 return sum;
}


/********************************************************************
 *
 * Down here: private functions originally from Ulrich Drepper
 *
 * From GNU textutils. md5.c
 *******************************************************************/

static const unsigned char fillbuf[64] = { 0x80, 0 /* , 0, 0, ...  */ };

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
static void * md5_finish_ctx (md5_ctx* ctx, void* resbuf)
{
	/* Take yet unprocessed bytes into account.  */
	ulong bytes = ctx->buflen;
	ulong pad;

	/* Now count remaining bytes.  */
	ctx->total[0] += bytes;
	if (ctx->total[0] < bytes)
		++ctx->total[1];

	pad = bytes >= 56 ? 64 + 56 - bytes : 56 - bytes;
	memcpy (&ctx->buffer[bytes], fillbuf, pad);

	/* Put the 64-bit file length in *bits* at the end of the buffer.  */
	*(ulong *) &ctx->buffer[bytes + pad] = (ctx->total[0] << 3);
	*(ulong *) &ctx->buffer[bytes + pad + 4] =
			((ctx->total[1] << 3) | (ctx->total[0] >> 29));

	/* Process last bytes.  */
	md5_process_block (ctx->buffer, bytes + pad + 8, ctx);


	((ulong *) resbuf)[0] = (ctx->A);
	((ulong *) resbuf)[1] = (ctx->B);
	((ulong *) resbuf)[2] = (ctx->C);
	((ulong *) resbuf)[3] = (ctx->D);


	return resbuf;
}

/* Processes some bytes in the internal buffer */
static void md5_process_bytes (const void* buffer, ulong len, struct md5_ctx* ctx)
{
	/* When we already have some bits in our internal buffer concatenate
		both inputs first.  */
	if (ctx->buflen != 0)
	{
		ulong left_over = ctx->buflen;
		ulong add = 128 - left_over > len ? len : 128 - left_over;

		memcpy (&ctx->buffer[left_over], buffer, add);
		ctx->buflen += add;

		if (left_over + add > 64)
		{
			md5_process_block (ctx->buffer, (left_over + add) & ~63, ctx);
			/* The regions in the following copy operation cannot overlap.  */
			memcpy (ctx->buffer, &ctx->buffer[(left_over + add) & ~63],
				(left_over + add) & 63);
			ctx->buflen = (left_over + add) & 63;
		}

		buffer = (const char *) buffer + add;
		len -= add;
	}

	/* Process available complete blocks.  */
	if (len > 64)
	{
		md5_process_block (buffer, len & ~63, ctx);
		buffer = (const char *) buffer + (len & ~63);
		len &= 63;
	}

	/* Move remaining bytes in internal buffer.  */
	if (len > 0)
	{
		memcpy (ctx->buffer, buffer, len);
		ctx->buflen = len;
	}
}


/* These are the four functions used in the four steps of the MD5 algorithm
   and defined in the RFC 1321.  The first function is a little bit optimized
   (as found in Colin Plumbs public domain implementation).  */
/* #define FF(b, c, d) ((b & c) | (~b & d)) */
#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

/* Process LEN bytes of BUFFER, accumulating context into CTX.
   It is assumed that LEN % 64 == 0.  */

static void md5_process_block (const void* buffer, ulong len, md5_ctx* ctx)
{
	ulong correct_words[16];
	const ulong *words = (ulong*) buffer;
	ulong nwords = len / sizeof (ulong);
	const ulong *endp = words + nwords;
	ulong A = ctx->A;
	ulong B = ctx->B;
	ulong C = ctx->C;
	ulong D = ctx->D;

	/* First increment the byte count.  RFC 1321 specifies the possible
		length of the file up to 2^64 bits.  Here we only compute the
		number of bytes.  Do a double word increment.  */
	ctx->total[0] += len;
	if (ctx->total[0] < len)
		++ctx->total[1];

	/* Process all bytes in the buffer with 64 bytes in each round of
		the loop.  */
	while (words < endp)
	{
		ulong *cwp = correct_words;
		ulong A_save = A;
		ulong B_save = B;
		ulong C_save = C;
		ulong D_save = D;

		/* First round: using the given function, the context and a constant
		the next context is computed.  Because the algorithms processing
		unit is a 32-bit word and it is determined to work on words in
		little endian byte order we perhaps have to change the byte order
		before the computation.  To reduce the work for the next steps
		we store the swapped words in the array CORRECT_WORDS.  */

#define OP(a, b, c, d, s, T)						\
	do								\
	{								\
		a += FF (b, c, d) + (*cwp++ = (*words)) + T;		\
		++words;							\
		CYCLIC (a, s);						\
		a += b;							\
	}								\
	while (0)

		/* It is unfortunate that C does not provide an operator for
		cyclic rotation.  Hope the C compiler is smart enough.  */
#define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

		/* Before we start, one word to the strange constants.
		They are defined in RFC 1321 as

		T[i] = static_cast<int>(4294967296.0 * fabs (sin (i))), i=1..64
		*/

		/* Round 1.  */
		OP (A, B, C, D,  7, 0xd76aa478);
		OP (D, A, B, C, 12, 0xe8c7b756);
		OP (C, D, A, B, 17, 0x242070db);
		OP (B, C, D, A, 22, 0xc1bdceee);
		OP (A, B, C, D,  7, 0xf57c0faf);
		OP (D, A, B, C, 12, 0x4787c62a);
		OP (C, D, A, B, 17, 0xa8304613);
		OP (B, C, D, A, 22, 0xfd469501);
		OP (A, B, C, D,  7, 0x698098d8);
		OP (D, A, B, C, 12, 0x8b44f7af);
		OP (C, D, A, B, 17, 0xffff5bb1);
		OP (B, C, D, A, 22, 0x895cd7be);
		OP (A, B, C, D,  7, 0x6b901122);
		OP (D, A, B, C, 12, 0xfd987193);
		OP (C, D, A, B, 17, 0xa679438e);
		OP (B, C, D, A, 22, 0x49b40821);

		/* For the second to fourth round we have the possibly swapped words
		in CORRECT_WORDS.  Redefine the macro to take an additional first
		argument specifying the function to use.  */
#undef OP
#define OP(f, a, b, c, d, k, s, T)					\
	do 								\
	{								\
		a += f (b, c, d) + correct_words[k] + T;			\
		CYCLIC (a, s);						\
		a += b;							\
	}								\
	while (0)

		/* Round 2.  */
		OP (FG, A, B, C, D,  1,  5, 0xf61e2562);
		OP (FG, D, A, B, C,  6,  9, 0xc040b340);
		OP (FG, C, D, A, B, 11, 14, 0x265e5a51);
		OP (FG, B, C, D, A,  0, 20, 0xe9b6c7aa);
		OP (FG, A, B, C, D,  5,  5, 0xd62f105d);
		OP (FG, D, A, B, C, 10,  9, 0x02441453);
		OP (FG, C, D, A, B, 15, 14, 0xd8a1e681);
		OP (FG, B, C, D, A,  4, 20, 0xe7d3fbc8);
		OP (FG, A, B, C, D,  9,  5, 0x21e1cde6);
		OP (FG, D, A, B, C, 14,  9, 0xc33707d6);
		OP (FG, C, D, A, B,  3, 14, 0xf4d50d87);
		OP (FG, B, C, D, A,  8, 20, 0x455a14ed);
		OP (FG, A, B, C, D, 13,  5, 0xa9e3e905);
		OP (FG, D, A, B, C,  2,  9, 0xfcefa3f8);
		OP (FG, C, D, A, B,  7, 14, 0x676f02d9);
		OP (FG, B, C, D, A, 12, 20, 0x8d2a4c8a);

		/* Round 3.  */
		OP (FH, A, B, C, D,  5,  4, 0xfffa3942);
		OP (FH, D, A, B, C,  8, 11, 0x8771f681);
		OP (FH, C, D, A, B, 11, 16, 0x6d9d6122);
		OP (FH, B, C, D, A, 14, 23, 0xfde5380c);
		OP (FH, A, B, C, D,  1,  4, 0xa4beea44);
		OP (FH, D, A, B, C,  4, 11, 0x4bdecfa9);
		OP (FH, C, D, A, B,  7, 16, 0xf6bb4b60);
		OP (FH, B, C, D, A, 10, 23, 0xbebfbc70);
		OP (FH, A, B, C, D, 13,  4, 0x289b7ec6);
		OP (FH, D, A, B, C,  0, 11, 0xeaa127fa);
		OP (FH, C, D, A, B,  3, 16, 0xd4ef3085);
		OP (FH, B, C, D, A,  6, 23, 0x04881d05);
		OP (FH, A, B, C, D,  9,  4, 0xd9d4d039);
		OP (FH, D, A, B, C, 12, 11, 0xe6db99e5);
		OP (FH, C, D, A, B, 15, 16, 0x1fa27cf8);
		OP (FH, B, C, D, A,  2, 23, 0xc4ac5665);

		/* Round 4.  */
		OP (FI, A, B, C, D,  0,  6, 0xf4292244);
		OP (FI, D, A, B, C,  7, 10, 0x432aff97);
		OP (FI, C, D, A, B, 14, 15, 0xab9423a7);
		OP (FI, B, C, D, A,  5, 21, 0xfc93a039);
		OP (FI, A, B, C, D, 12,  6, 0x655b59c3);
		OP (FI, D, A, B, C,  3, 10, 0x8f0ccc92);
		OP (FI, C, D, A, B, 10, 15, 0xffeff47d);
		OP (FI, B, C, D, A,  1, 21, 0x85845dd1);
		OP (FI, A, B, C, D,  8,  6, 0x6fa87e4f);
		OP (FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
		OP (FI, C, D, A, B,  6, 15, 0xa3014314);
		OP (FI, B, C, D, A, 13, 21, 0x4e0811a1);
		OP (FI, A, B, C, D,  4,  6, 0xf7537e82);
		OP (FI, D, A, B, C, 11, 10, 0xbd3af235);
		OP (FI, C, D, A, B,  2, 15, 0x2ad7d2bb);
		OP (FI, B, C, D, A,  9, 21, 0xeb86d391);

		/* Add the starting values of the context.  */
		A += A_save;
		B += B_save;
		C += C_save;
		D += D_save;
	}

	/* Put checksum in context given as argument.  */
	ctx->A = A;
	ctx->B = B;
	ctx->C = C;
	ctx->D = D;
}
