/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */
// Thanks to Ulrich Drepper for the md5sum example code.

#ifndef WL_BASE_MD5_H
#define WL_BASE_MD5_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>

/* Structure to save state of computation between the single steps.  */
struct Md5Ctx {
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
struct Md5Checksum {
	uint8_t data[16];

	[[nodiscard]] std::string str() const;

	bool operator==(const Md5Checksum& o) const {
		return memcmp(data, o.data, sizeof(data)) == 0;
	}

	bool operator!=(const Md5Checksum& o) const {
		return !(*this == o);
	}
};

// Note that the implementation of MD5Checksum is basically just
// a wrapper around these functions, which have been taken basically
// verbatim (with some whitespace changes) from the GNU tools; see below.
void* md5_finish_ctx(Md5Ctx*, void* resbuf);
void md5_process_bytes(void const* buffer, uint32_t len, Md5Ctx*);
void md5_process_block(void const* buffer, uint32_t len, Md5Ctx*);

/**
 * This class is responsible for creating a streaming md5 checksum.
 * You simply pass it the data using stream operations, and if you want
 * to read the checksum, first call finish_checksum(), followed by
 * get_checksum().
 *
 * Instances of this class can be copied.
 */
template <typename Base> class MD5Checksum : public Base {
public:
	MD5Checksum() : sum({0}) {
		reset();
	}
	MD5Checksum(const MD5Checksum& other)
	   : Base(), can_handle_data(other.can_handle_data), sum(other.sum), ctx(other.ctx) {
	}

	/// Reset the checksumming machinery to its initial state.
	void reset() {
		can_handle_data = true;
		ctx.A = 0x67452301;
		ctx.B = 0xefcdab89;
		ctx.C = 0x98badcfe;
		ctx.D = 0x10325476;
		ctx.total[0] = ctx.total[1] = 0;
		ctx.buflen = 0;
	}

	/// This function consumes new data. It buffers it and calculates one MD5
	/// block when the buffer is full.
	///
	/// \param newdata data to compute chksum for
	/// \param size size of data
	void data(const void* const newdata, const size_t size) override {
		assert(can_handle_data);
		md5_process_bytes(newdata, size, &ctx);
	}

	/// This function finishes the checksum calculation.
	/// After this, no more data may be written to the checksum.
	void finish_checksum() {
		assert(can_handle_data);
		can_handle_data = false;
		md5_finish_ctx(&ctx, sum.data);
	}

	/// Retrieve the checksum. Note that \ref finish_checksum must be called
	/// before this function.
	///
	/// \return a pointer to an array of 16 bytes containing the checksum.
	[[nodiscard]] const Md5Checksum& get_checksum() const {
		assert(!can_handle_data);
		return sum;
	}

private:
	bool can_handle_data;
	Md5Checksum sum;
	Md5Ctx ctx;
};

class DummyMD5Base {
public:
	virtual ~DummyMD5Base() = default;
	virtual void data(const void*, size_t) = 0;
};
using SimpleMD5Checksum = MD5Checksum<DummyMD5Base>;

#endif  // end of include guard: WL_BASE_MD5_H
