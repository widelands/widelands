/*	$NetBSD: md2.h,v 1.7 2016/07/01 16:42:46 christos Exp $	*/

#ifndef WIDELANDS_LIBMD_MD2_H_
#define WIDELANDS_LIBMD_MD2_H_

#include <sys/types.h>

#include <stdint.h>

namespace libmd {

#define	MD2_DIGEST_LENGTH		16
#define	MD2_DIGEST_STRING_LENGTH	33
#define	MD2_BLOCK_LENGTH		16

/* MD2 context. */
typedef struct MD2Context {
	uint32_t i;
	unsigned char C[16];		/* checksum */
	unsigned char X[48];		/* input buffer */
} MD2_CTX;

void	MD2Init(MD2_CTX *);
void	MD2Update(MD2_CTX *, const unsigned char *, unsigned int);
void	MD2Final(unsigned char[16], MD2_CTX *);
char	*MD2End(MD2_CTX *, char *);
char	*MD2File(const char *, char *);
char	*MD2FileChunk(const char *, char *, off_t, off_t);
char	*MD2Data(const unsigned char *, size_t, char *);

}  // namespace libmd

#endif /* WIDELANDS_LIBMD_MD2_H_ */
