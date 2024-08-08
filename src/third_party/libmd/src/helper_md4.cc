/*	$OpenBSD: helper.c,v 1.9 2010/01/08 13:30:21 oga Exp $	*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dkuug.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "third_party/libmd/include/md4.h"

namespace libmd {

#ifndef MIN
# define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/* ARGSUSED */
char *
MD4End(MD4_CTX *ctx, char *buf)
{
	int i;
	uint8_t digest[MD4_DIGEST_LENGTH];
	static const char hex[] = "0123456789abcdef";

	if (buf == NULL && (buf = (char*)malloc(MD4_DIGEST_STRING_LENGTH)) == NULL)
		return (NULL);

	MD4Final(digest, ctx);
	for (i = 0; i < MD4_DIGEST_LENGTH; i++) {
		buf[i + i] = hex[digest[i] >> 4];
		buf[i + i + 1] = hex[digest[i] & 0x0f];
	}
	buf[i + i] = '\0';
	memset(digest, 0, sizeof(digest));
	return (buf);
}

char *
MD4Data(const uint8_t *data, size_t len, char *buf)
{
	MD4_CTX ctx;

	MD4Init(&ctx);
	MD4Update(&ctx, data, len);
	return (MD4End(&ctx, buf));
}

}  // namespace libmd
