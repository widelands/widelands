/*
 * Copyright © 2011 Guillem Jover <guillem@hadrons.org>
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

#ifndef LIBMD_SHA512_H
#define LIBMD_SHA512_H

/* For compatibility with FreeBSD and DragonFlyBSD. */

#include <sha2.h>

typedef SHA2_CTX SHA512_CTX;
typedef SHA2_CTX SHA384_CTX;

#define SHA384_Init		SHA384Init
#define SHA384_Update		SHA384Update
#define SHA384_Final		SHA384Final
#define SHA384_End		SHA384End
#define SHA384_Data		SHA384Data
#define SHA384_File		SHA384File
#define SHA384_FileChunk	SHA384FileChunk

#define SHA512_Init		SHA512Init
#define SHA512_Update		SHA512Update
#define SHA512_Final		SHA512Final
#define SHA512_End		SHA512End
#define SHA512_Data		SHA512Data
#define SHA512_File		SHA512File
#define SHA512_FileChunk	SHA512FileChunk

#endif
