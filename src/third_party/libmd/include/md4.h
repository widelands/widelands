/*	$OpenBSD: md4.h,v 1.16 2012/12/05 23:19:57 deraadt Exp $	*/

/*
 * This code implements the MD4 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 * Todd C. Miller modified the MD5 code to do MD4 based on RFC 1186.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#ifndef _MD4_H_
#define _MD4_H_

#include <sys/types.h>

#include <stdint.h>

#define	MD4_BLOCK_LENGTH		64
#define	MD4_DIGEST_LENGTH		16
#define	MD4_DIGEST_STRING_LENGTH	(MD4_DIGEST_LENGTH * 2 + 1)

typedef struct MD4Context {
	uint32_t state[4];			/* state */
	uint64_t count;				/* number of bits, mod 2^64 */
	uint8_t buffer[MD4_BLOCK_LENGTH];	/* input buffer */
} MD4_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void	 MD4Init(MD4_CTX *);
void	 MD4Update(MD4_CTX *, const uint8_t *, size_t);
void	 MD4Pad(MD4_CTX *);
void	 MD4Final(uint8_t [MD4_DIGEST_LENGTH], MD4_CTX *);
void	 MD4Transform(uint32_t [4], const uint8_t [MD4_BLOCK_LENGTH]);
char	*MD4End(MD4_CTX *, char *);
char	*MD4File(const char *, char *);
char	*MD4FileChunk(const char *, char *, off_t, off_t);
char	*MD4Data(const uint8_t *, size_t, char *);

#ifdef __cplusplus
}
#endif

#endif /* _MD4_H_ */
