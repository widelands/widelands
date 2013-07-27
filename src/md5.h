/*
 * Thanks to Ulrich Drepper for the md5sum example code
 *
 * Copyright (C) 2002, 2007-2008 by the Widelands Development Team
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

#ifndef MD5_H
#define MD5_H

#include <cassert>
#include <cstring>
#include <string>

#include <stdint.h>

/* Structure to save state of computation between the single steps.  */
struct md5_ctx {
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;

	uint32_t total[2];
	uint32_t buflen;
	char buffer[128];
};

/**
 * One MD5 checksum is simply an array of 16 bytes.
 */
struct md5_checksum {
	uint8_t data[16];

	std::string str() const;

	bool operator== (const md5_checksum & o) const {
		return memcmp(data, o.data, sizeof(data)) == 0;
	}

	bool operator!= (const md5_checksum & o) const {return not (*this == o);}
};

// Note that the implementation of MD5Checksum is basically just
// a wrapper around these functions, which have been taken basically
// verbatim (with some whitespace changes) from the GNU tools; see below.
void * md5_finish_ctx (md5_ctx *, void * resbuf);
void md5_process_bytes (void const * buffer, uint32_t len, md5_ctx *);
void md5_process_block (void const * buffer, uint32_t len, md5_ctx *);

/**
 * This class is responsible for creating a streaming md5 checksum.
 * You simply pass it the data using stream operations, and if you want
 * to read the checksum, first call FinishChecksum(), followed by
 * GetChecksum().
 *
 * Instances of this class can be copied.
 */
template <typename Base> class MD5Checksum : public Base {
public:
	MD5Checksum() {Reset();}
	explicit MD5Checksum(const MD5Checksum & other)
		:
		Base(),
		can_handle_data(other.can_handle_data), sum(other.sum), ctx(other.ctx)
	{}

	/// Reset the checksumming machinery to its initial state.
	void Reset() {
		can_handle_data = 1;
		ctx.A = 0x67452301; ctx.B = 0xefcdab89;
		ctx.C = 0x98badcfe; ctx.D = 0x10325476;
		ctx.total[0] = ctx.total[1] = 0;
		ctx.buflen = 0;
	}

	/// This function consumes new data. It buffers it and calculates one MD5
	/// block when the buffer is full.
	///
	/// \param data data to compute chksum for
	/// \param size size of data
	void Data(const void * const newdata, const size_t size) {
		assert(can_handle_data);
		md5_process_bytes(newdata, size, &ctx);
	}

	/// This function finishes the checksum calculation.
	/// After this, no more data may be written to the checksum.
	void FinishChecksum() {
		assert(can_handle_data);
		can_handle_data = 0;
		md5_finish_ctx(&ctx, sum.data);
	}

	/// Retrieve the checksum. Note that \ref FinishChecksum must be called
	/// before this function.
	///
	/// \return a pointer to an array of 16 bytes containing the checksum.
	const md5_checksum & GetChecksum() const {
		assert(!can_handle_data);
		return sum;
	}

private:
	bool can_handle_data;
	md5_checksum sum;
	md5_ctx ctx;
};

class _DummyMD5Base {};
typedef MD5Checksum<_DummyMD5Base> SimpleMD5Checksum;

#endif
