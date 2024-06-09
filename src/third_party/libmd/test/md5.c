/*
 * Copyright © 2016 Guillem Jover <guillem@hadrons.org>
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

#define LIBMD_MD5_ALADDIN 1

#include <config.h>

#include <assert.h>
#include <md5.h>
#include <string.h>

#include "test.h"

DEF_TEST_DIGEST(MD5, MD5)

static void
test_MD5_aladdin(const char *hash_str_ref, const char *data)
{
	uint8_t hash_bin_ref[MD5_DIGEST_LENGTH];
	uint8_t hash_bin_got[MD5_DIGEST_LENGTH];
	md5_state_t pms;

	hex2bin(hash_bin_ref, hash_str_ref, MD5_DIGEST_LENGTH);

	md5_init(&pms);
	md5_append(&pms, (const uint8_t *)data, strlen(data));
	md5_finish(&pms, hash_bin_got);
	assert(memcmp(hash_bin_ref, hash_bin_got, MD5_DIGEST_LENGTH) == 0);
}

static void
test_MD5_all(const char *hash_str_ref, const char *data)
{
	test_MD5(hash_str_ref, data);
	test_MD5_aladdin(hash_str_ref, data);
}

int
main(int argc, char *argv[])
{
	test_MD5_all("d41d8cd98f00b204e9800998ecf8427e", "");
	test_MD5_all("900150983cd24fb0d6963f7d28e17f72", "abc");
	test_MD5_all("827ccb0eea8a706c4c34a16891f84e7b", "12345");

	return 0;
}
