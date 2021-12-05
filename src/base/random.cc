/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#include "base/random.h"

#include <chrono>

#include "base/wexception.h"
#include "io/streamread.h"
#include "io/streamwrite.h"

RNG::RNG() : state0(0), state1(0) {
}

void RNG::seed(uint32_t s) {
	state0 ^= state1;
	state1 = SIMPLE_RAND(state1) + s;
}

uint32_t RNG::rand() {
	//  apply a bijective mixing operation
	state0 = SIMPLE_RAND(state0);
	state1 = SIMPLE_RAND(state1);

	state0 += state1;
	state1 += state0;

	return state0 ^ state1;  // the next random number
}

// this is a nonlinear sbox used for mixing the state
// note that it consists of 4 parallel permutations

const uint32_t rng_sbox[256] = {
   0x2FC5C487, 0x721E317E, 0x46337246, 0xD3A14DD5, 0x336D4A5F, 0x076C9EAE, 0x4843031E, 0x68C3C945,
   0x0AB927B9, 0x5479614C, 0x11917750, 0xA52E5A97, 0x3CAC674B, 0x6FFB2A42, 0x5373E52E, 0x4CE7B88A,
   0x267A150A, 0xEF1D1F59, 0x30A346CE, 0x2E0F2926, 0xD6545436, 0xED373033, 0x411340D1, 0x4AA98AE9,
   0xAAD5DFB8, 0x009C9C14, 0x9AB25228, 0xBB7F44BC, 0x4D236F5E, 0xCBA4060E, 0x44831C76, 0xD1F0B601,
   0x5FBB3BEF, 0xCD89B243, 0x785D1100, 0x34885C25, 0x25691365, 0x3F753FC5, 0xBE3108C9, 0xF56E163F,
   0x99BC9694, 0xC116A656, 0xDF09F0A4, 0x70273885, 0x0E1FE1F3, 0xA0A2CEB0, 0x4E48FA69, 0xFC366389,
   0x47E00B3A, 0x8170AE71, 0x9CBE3E4A, 0x5D9AEE1D, 0x012D9330, 0xFB0381E1, 0xB041E6BE, 0x79022016,
   0xD5978B37, 0x710CB547, 0x825C4723, 0x3D0E3C8F, 0x209E7FB2, 0x1A958F4D, 0x9ED62564, 0xB2AE3339,
   0xD48A64EA, 0x365579F7, 0xBA10EA7A, 0xB52B5586, 0x0F3D6B17, 0x0864690B, 0x7BBFA995, 0xDA1A7B9D,
   0x8CDBD9C3, 0x56D9CDFF, 0x6DA5E34E, 0x3E5798B5, 0xC884001F, 0xE8FD9981, 0xD908BFBA, 0xAEA61002,
   0x77CFD67D, 0x9112B3AA, 0x7AB38E22, 0xBCE4282D, 0x69C16DD8, 0xFEBA4B29, 0x0698F6E5, 0x90C64378,
   0x49742F74, 0xC73BE7DE, 0x89226609, 0x50ABEB99, 0x03BDC883, 0x985F2251, 0x385BAF18, 0x6A7D53F0,
   0x61445BF2, 0xE1D2EC8D, 0x098035FE, 0xAB58DAD2, 0x88D304F4, 0x74813658, 0x62C84CD9, 0x5A61A788,
   0x5B0B9BFC, 0xAFD43957, 0x5940B944, 0xF1E14FA6, 0x97323A13, 0x8B38217B, 0x529DFF35, 0xFFF19D6C,
   0x398D578E, 0x37A86E10, 0x7FF419C4, 0xC949F2CA, 0x278B59FB, 0x183F3703, 0xB7D0C055, 0x2A2FD16E,
   0x14B51A31, 0xF8B4F86B, 0xF94D60EB, 0x05A078C0, 0xEC4A75C7, 0x40EA7093, 0x1920CAA9, 0xC326F45C,
   0x3ACABD96, 0xF6E9C149, 0xCC35CCE0, 0x9B182D11, 0x4B2465B6, 0x66D7A17F, 0x51A75FE3, 0xBD5E90DB,
   0xFA920F2B, 0xFD1762DA, 0x943C889C, 0x6C66AA7C, 0x24DEB09B, 0x10C7AD8B, 0xB84C7D6F, 0x1B7841C2,
   0x0BF8944F, 0x16E58CCF, 0x1EFAC23B, 0xA950C6E7, 0x87EDA412, 0xE615E953, 0xF0392CC1, 0x608F7E67,
   0x225AA363, 0x156FFDD6, 0x13B0D8F5, 0xCA3056AC, 0x1F7C233C, 0xD863C58C, 0x8571FCB7, 0x92767406,
   0x8D14E438, 0x4FF71D08, 0xF2876AED, 0x55AF9FA1, 0xE259D791, 0xB12AA82A, 0x93D884A7, 0x6B040C5D,
   0xE485349E, 0x2894BA68, 0xD0FFFEAD, 0x96ADB7CC, 0x7DC2FB5B, 0xB6CE0E79, 0xDB25D2DD, 0xA3B1871B,
   0x84DFA55A, 0x0D1C1821, 0xF760ABE2, 0x9D6AD061, 0xE74E0D84, 0xCFDD50F6, 0x0CAA1EA8, 0x427EDDD0,
   0x3BC0899A, 0x5C670948, 0xA1D15E52, 0x584F0741, 0x2BDADCEC, 0x35C9BC1A, 0x9F964E07, 0x31FC971C,
   0x80C4DECD, 0xCECC26EE, 0xB9287C0F, 0xE3E832C8, 0xE51BF775, 0xA22173C6, 0xACE64298, 0xC5E2BEDF,
   0x23620AD4, 0x1747D5E8, 0xEA6812DC, 0x5EEB80D7, 0x8FF3DBB1, 0x1D7BA0FD, 0xBF462E72, 0x7E6571B4,
   0x734BF166, 0x2DCDBB54, 0xA6455882, 0x95113D80, 0x7C86E040, 0x640149A0, 0x020ACFE6, 0xA7DC0592,
   0xEB05A215, 0x836B86CB, 0xD7EF7AD3, 0xDC00D390, 0xDE56E2A3, 0xA83483BB, 0xE08C17A2, 0xB4E3F534,
   0xA4F25D0D, 0x29F568F1, 0x6553012F, 0x12429505, 0xE9776C70, 0xF452AC77, 0x1C8EEDB3, 0x21B8489F,
   0x63729A27, 0xD29FC32C, 0xADB685BD, 0xF3072B6A, 0x76F9F904, 0x863EB40C, 0x6E9091F9, 0x75ECF3F8,
   0x3219513E, 0x2CFE02A5, 0x8A999232, 0x430D4573, 0x67B78D6D, 0x45821BAB, 0xC2062462, 0xB3F67619,
   0xC4CBEF3D, 0xDD3A14AF, 0xC69B82FA, 0xC029B1BF, 0xEE51E8E4, 0x0493CB60, 0x572CC720, 0x8EEED424};

#define RNG_SAVE_MAGIC 0xf0057763

void RNG::read_state(StreamRead& sr) {
	uint32_t const magic = sr.unsigned_32();
	if (magic != RNG_SAVE_MAGIC) {
		throw wexception(
		   "Different RNG version (magic = %08x, expected %08x)", magic, RNG_SAVE_MAGIC);
	}

	state0 = sr.unsigned_32();
	state1 = sr.unsigned_32();
}

void RNG::write_state(StreamWrite& sw) {
	sw.unsigned_32(RNG_SAVE_MAGIC);
	sw.unsigned_32(state0);
	sw.unsigned_32(state1);
}

static RNG static_rng_(
   std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now())
      .time_since_epoch()
      .count());
uint32_t RNG::static_rand() {
	return static_rng_.rand();
}

std::string generate_random_uuid() {
	uint32_t values[4];
	RNG temp_rng;
	int64_t seed = clock();
	for (uint32_t& val_ref : values) {
		seed += std::chrono::time_point_cast<std::chrono::nanoseconds>(
		           std::chrono::high_resolution_clock::now())
		           .time_since_epoch()
		           .count();
		temp_rng.seed(seed % 0xfedcba98);
		val_ref = temp_rng.rand();
	}

	char buffer[16 * 4 + 4 + 1];
	snprintf(buffer, sizeof(buffer), "%04x%04x-%04x-%04x-%04x-%04x%04x%04x", values[0] & 0xffff,
	         values[1] & 0xffff, values[2] & 0xffff, values[3] & 0xffff,
	         (values[0] & 0xffff0000) >> 16, (values[1] & 0xffff0000) >> 16,
	         (values[2] & 0xffff0000) >> 16, (values[3] & 0xffff0000) >> 16);
	return buffer;
}
