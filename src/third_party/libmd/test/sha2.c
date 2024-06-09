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

#include <config.h>

#include <sha2.h>

#include "test.h"

DEF_TEST_DIGEST(SHA224, SHA2)
DEF_TEST_DIGEST(SHA256, SHA2)
DEF_TEST_DIGEST(SHA384, SHA2)
DEF_TEST_DIGEST(SHA512, SHA2)
DEF_TEST_DIGEST(SHA512_256, SHA2)

int
main(int argc, char *argv[])
{
	test_SHA224("d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f",
	            "");
	test_SHA224("23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7",
	            "abc");
	test_SHA224("a7470858e79c282bc2f6adfd831b132672dfd1224c1e78cbf5bcd057",
	            "12345");

	test_SHA256("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
	            "");
	test_SHA256("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
	            "abc");
	test_SHA256("5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5",
	            "12345");

	test_SHA384("38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b",
	            "");
	test_SHA384("cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7",
	            "abc");
	test_SHA384("0fa76955abfa9dafd83facca8343a92aa09497f98101086611b0bfa95dbc0dcc661d62e9568a5a032ba81960f3e55d4a",
	            "12345");

	test_SHA512("cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e",
	            "");
	test_SHA512("ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f",
	            "abc");
	test_SHA512("3627909a29c31381a071ec27f7c9ca97726182aed29a7ddd2e54353322cfb30abb9e3a6df2ac2c20fe23436311d678564d0c8d305930575f60e2d3d048184d79",
	            "12345");

	test_SHA512_256("c672b8d1ef56ed28ab87c3622c5114069bdd3ad7b8f9737498d0c01ecef0967a",
	                "");
	test_SHA512_256("53048e2681941ef99b2e29b76b4c7dabe4c2d0c634fc6d46e0e2f13107e7af23",
	                "abc");
	test_SHA512_256("ee039e3bed452ceb91427fcef9f0e01b6af73272c8a103e5bb853c9e3170edef",
	                "12345");

	return 0;
}
