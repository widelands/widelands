/*	$OpenBSD: md5.h,v 1.17 2012/12/05 23:19:57 deraadt Exp $	*/

/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 */

#ifndef _MD5_H_
#define _MD5_H_

#include <sys/types.h>

#include <stdint.h>

#define	MD5_BLOCK_LENGTH		64
#define	MD5_DIGEST_LENGTH		16
#define	MD5_DIGEST_STRING_LENGTH	(MD5_DIGEST_LENGTH * 2 + 1)

typedef struct MD5Context {
	uint32_t state[4];			/* state */
	uint64_t count;				/* number of bits, mod 2^64 */
	uint8_t buffer[MD5_BLOCK_LENGTH];	/* input buffer */
} MD5_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void	 MD5Init(MD5_CTX *);
void	 MD5Update(MD5_CTX *, const uint8_t *, size_t);
void	 MD5Pad(MD5_CTX *);
void	 MD5Final(uint8_t [MD5_DIGEST_LENGTH], MD5_CTX *);
void	 MD5Transform(uint32_t [4], const uint8_t [MD5_BLOCK_LENGTH]);
char	*MD5End(MD5_CTX *, char *);
char	*MD5File(const char *, char *);
char	*MD5FileChunk(const char *, char *, off_t, off_t);
char	*MD5Data(const uint8_t *, size_t, char *);

#ifdef __cplusplus
}
#endif

/* Avoid polluting the namespace. Even though this makes this usage
 * implementation-specific, defining it unconditionally should not be
 * a problem, and better than possibly breaking unexpecting code. */
#ifdef LIBMD_MD5_ALADDIN

/*
 * Interface compatibility with Aladdin Enterprises independent
 * implementation from RFC 1321.
 */

typedef uint8_t md5_byte_t;
typedef uint32_t md5_word_t;
typedef MD5_CTX md5_state_t;

#define md5_init(pms) MD5Init(pms)
#define md5_append(pms, data, nbytes) MD5Update(pms, data, nbytes)
#define md5_finish(pms, digest) MD5Final(digest, pms)

#endif /* LIBMD_MD5_ALADDIN */

#endif /* _MD5_H_ */
