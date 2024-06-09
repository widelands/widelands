/*	$OpenBSD: rmd160.h,v 1.17 2012/12/05 23:19:57 deraadt Exp $	*/
/*
 * Copyright (c) 2001 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef  _RMD160_H
#define  _RMD160_H

#include <sys/types.h>

#include <stdint.h>

#define	RMD160_BLOCK_LENGTH		64
#define	RMD160_DIGEST_LENGTH		20
#define	RMD160_DIGEST_STRING_LENGTH	(RMD160_DIGEST_LENGTH * 2 + 1)

/* RMD160 context. */
typedef struct RMD160Context {
	uint32_t state[5];			/* state */
	uint64_t count;				/* number of bits, mod 2^64 */
	uint8_t buffer[RMD160_BLOCK_LENGTH];	/* input buffer */
} RMD160_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void	 RMD160Init(RMD160_CTX *);
void	 RMD160Transform(uint32_t [5], const uint8_t [RMD160_BLOCK_LENGTH]);
void	 RMD160Update(RMD160_CTX *, const uint8_t *, size_t);
void	 RMD160Pad(RMD160_CTX *);
void	 RMD160Final(uint8_t [RMD160_DIGEST_LENGTH], RMD160_CTX *);
char	*RMD160End(RMD160_CTX *, char *);
char	*RMD160File(const char *, char *);
char	*RMD160FileChunk(const char *, char *, off_t, off_t);
char	*RMD160Data(const uint8_t *, size_t, char *);

#ifdef __cplusplus
}
#endif

#endif  /* _RMD160_H */
