/* md5.cc - Functions to compute MD5 message digest of files or memory blocks
 * according to the definition of MD5 in RFC 1321 from April 1992.
 *
 * Thanks to Ulrich Drepper for the md5sum example code
 *
 * Copyright (C) 2002, 2007, 2009 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "md5.h"

#include <cstdio>

#include "compile_diagnostics.h"

using namespace std;

/**
 * Create a hex string out of the MD5 checksum.
 */
string md5_checksum::str() const
{
	string s;

	for (uint32_t i = 0; i < sizeof(data); ++i) {
		char buf[3];
		snprintf(buf, sizeof(buf), "%02x", data[i]);
		s += buf;
	}

	return s;
}


/********************************************************************
 *
 * Down here: private functions originally from Ulrich Drepper
 *
 * From GNU textutils. md5.c
 *******************************************************************/

static const uint8_t fillbuf[64] = {0x80, 0 /*, 0, 0, ... 0 */};

/* Process the remaining bytes in the internal buffer and the usual
   prolog according to the standard and write the result to RESBUF.

   IMPORTANT: On some systems it is required that RESBUF is correctly
   aligned for a 32 bits value.  */
void * md5_finish_ctx (md5_ctx * const ctx, void * const resbuf)
{
	/* Take yet unprocessed bytes into account.  */
	uint32_t bytes = ctx->buflen;
	uint32_t pad;

	/* Now count remaining bytes.  */
	ctx->total[0] += bytes;
	if (ctx->total[0] < bytes)
		++ctx->total[1];

	pad = bytes >= 56 ? 64 + 56 - bytes : 56 - bytes;
	memcpy (&ctx->buffer[bytes], fillbuf, pad);

	//  Put the 64-bit file length in *bits* at the end of the buffer.
GCC_DIAG_OFF("-Wstrict-aliasing");
	*reinterpret_cast<uint32_t *>(&ctx->buffer[bytes + pad])     =
		(ctx->total[0] << 3);
	*reinterpret_cast<uint32_t *>(&ctx->buffer[bytes + pad + 4]) =
		((ctx->total[1] << 3) | (ctx->total[0] >> 29));
GCC_DIAG_ON("-Wstrict-aliasing");

	/* Process last bytes.  */
	md5_process_block (ctx->buffer, bytes + pad + 8, ctx);

	static_cast<uint32_t *>(resbuf)[0] = ctx->A;
	static_cast<uint32_t *>(resbuf)[1] = ctx->B;
	static_cast<uint32_t *>(resbuf)[2] = ctx->C;
	static_cast<uint32_t *>(resbuf)[3] = ctx->D;

	return resbuf;
}

/* Processes some bytes in the internal buffer */
void md5_process_bytes
	(void const * buffer, uint32_t len, struct md5_ctx * const ctx)
{
	/* When we already have some bits in our internal buffer concatenate
		both inputs first.  */
	if (ctx->buflen != 0)
	{
		uint32_t left_over = ctx->buflen;
		uint32_t add = 128 - left_over > len ? len : 128 - left_over;

		memcpy (&ctx->buffer[left_over], buffer, add);
		ctx->buflen += add;

		if (left_over + add > 64)
		{
			md5_process_block (ctx->buffer, (left_over + add) & ~63, ctx);
			/* The regions in the following copy operation cannot overlap.  */
			memcpy
				(ctx->buffer, &ctx->buffer[(left_over + add) & ~63],
				 (left_over + add) & 63);
			ctx->buflen = (left_over + add) & 63;
		}

		buffer = static_cast<const char *>(buffer) + add;
		len -= add;
	}

	/* Process available complete blocks.  */
	if (len > 64)
	{
		md5_process_block (buffer, len & ~63, ctx);
		buffer = static_cast<const char *>(buffer) + (len & ~63);
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

void md5_process_block
	(void const * const buffer, uint32_t const len, md5_ctx * const ctx)
{
	uint32_t correct_words[16];
	uint32_t const *       words  = static_cast<uint32_t const *>(buffer);
	uint32_t         const nwords = len / sizeof (uint32_t);
	uint32_t const * const endp   = words + nwords;
	uint32_t A = ctx->A;
	uint32_t B = ctx->B;
	uint32_t C = ctx->C;
	uint32_t D = ctx->D;

	/* First increment the byte count.  RFC 1321 specifies the possible
		length of the file up to 2^64 bits.  Here we only compute the
		number of bytes.  Do a double word increment.  */
	ctx->total[0] += len;
	if (ctx->total[0] < len)
		++ctx->total[1];

	/* Process all bytes in the buffer with 64 bytes in each round of
		the loop.  */
	while (words < endp) {
		uint32_t * cwp = correct_words;
		uint32_t const A_save = A;
		uint32_t const B_save = B;
		uint32_t const C_save = C;
		uint32_t const D_save = D;

		/* First round: using the given function, the context and a constant
		the next context is computed.  Because the algorithms processing
		unit is a 32-bit word and it is determined to work on words in
		little endian byte order we perhaps have to change the byte order
		before the computation.  To reduce the work for the next steps
		we store the swapped words in the array CORRECT_WORDS.  */

#define OP(a, b, c, d, s, T)                                                  \
   do {                                                                       \
      a += FF (b, c, d) + (*cwp++ = (*words)) + T;                            \
      ++words;                                                                \
      CYCLIC (a, s);                                                          \
      a += b;                                                                 \
   } while (false)                                                            \

	/* It is unfortunate that C does not provide an operator for
		cyclic rotation.  Hope the C compiler is smart enough.  */
#define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

		/* Before we start, one word to the strange constants.
		They are defined in RFC 1321 as

		T[i] = static_cast<int32_t>(4294967296.0 * fabs (sin (i))), i=1..64
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
#define OP(f, a, b, c, d, k, s, T)                                            \
   do {                                                                       \
      a += f (b, c, d) + correct_words[k] + T;                                \
      CYCLIC (a, s);                                                          \
      a += b;                                                                 \
   } while (false)                                                            \

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
