/*
 * Copyright © 2018 Guillem Jover <guillem@hadrons.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

static int
hexchar2bin(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	assert(!"invalid hexadecimal input");
}

static void
hex2bin(uint8_t *bin, const char *str, size_t bin_len)
{
	size_t i;

	for (i = 0; i < bin_len; i++)
		bin[i] = hexchar2bin(str[i * 2]) << 4 |
			 hexchar2bin(str[i * 2 + 1]);
}

static int
test_eq(const char *ref, const char *got)
{
	if (strcmp(ref, got) == 0)
		return 1;

	fprintf(stderr, "ref <%s> != got <%s>\n", ref, got);
	return 0;
}

#define DEF_TEST_DIGEST(name, type) \
static void \
test_##name(const char *hash_str_ref, const char *data) \
{ \
	uint8_t hash_bin_ref[name##_DIGEST_LENGTH]; \
	uint8_t hash_bin_got[name##_DIGEST_LENGTH]; \
	char hash_str_got[name##_DIGEST_STRING_LENGTH]; \
	type##_CTX ctx; \
\
	hex2bin(hash_bin_ref, hash_str_ref, name##_DIGEST_LENGTH); \
\
	name##Data((const uint8_t *)data, strlen(data), hash_str_got); \
	assert(test_eq(hash_str_ref, hash_str_got)); \
\
	name##Init(&ctx); \
	name##Update(&ctx, (const uint8_t *)data, strlen(data)); \
	name##End(&ctx, hash_str_got); \
	assert(test_eq(hash_str_ref, hash_str_got)); \
\
	name##Init(&ctx); \
	name##Update(&ctx, (const uint8_t *)data, strlen(data)); \
	name##Final(hash_bin_got, &ctx); \
	assert(memcmp(hash_bin_ref, hash_bin_got, sizeof(hash_bin_ref)) == 0); \
}
