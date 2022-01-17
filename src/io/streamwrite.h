/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#ifndef WL_IO_STREAMWRITE_H
#define WL_IO_STREAMWRITE_H

#include <cstring>
#include <string>

#include "base/macros.h"
#include "io/machdep.h"

/**
 * Abstract base class for stream-like data sinks.
 * It is intended for serializing network packets and for writing log-type
 * data to disk.
 *
 * All implementations need to implement \ref data . Some implementations
 * may need to implement \ref flush .
 *
 * Convenience functions are provided for many data types.
 */
class StreamWrite {
public:
	explicit StreamWrite() {
	}
	virtual ~StreamWrite() = default;

	/**
	 * Write a number of bytes to the stream.
	 */
	virtual void data(const void* const write_data, const size_t size) = 0;

	/**
	 * Make sure all data submitted so far is written to disk.
	 *
	 * The default implementation is a no-op. Implementations may want
	 * to override this if they're buffered internally.
	 */
	virtual void flush();

	// TODO(unknown): implement an overloaded method that accepts fmt as std::string
	void print_f(char const*, ...) __attribute__((format(printf, 2, 3)));

	void signed_8(int8_t const x) {
		data(&x, 1);
	}
	void unsigned_8(uint8_t const x) {
		data(&x, 1);
	}
	void signed_16(int16_t const x) {
		int16_t const y = little_16(x);
		data(&y, 2);
	}
	void unsigned_16(uint16_t const x) {
		uint16_t const y = little_16(x);
		data(&y, 2);
	}
	void signed_32(int32_t const x) {
		uint32_t const y = little_32(x);
		data(&y, 4);
	}
	void unsigned_32(uint32_t const x) {
		uint32_t const y = little_32(x);
		data(&y, 4);
	}
	void float_32(const float x) {
		uint32_t y;
		memcpy(&y, &x, 4);
		y = little_32(y);
		data(&y, 4);
	}
	void string(const std::string& str) {
		data(str.c_str(), str.size() + 1);
	}

	//  Write strings with    null terminator.
	void c_string(char const* const x) {
		data(x, strlen(x) + 1);
	}
	void c_string(const std::string& x) {
		data(x.c_str(), x.size() + 1);
	}

	//  Write strings without null terminator.
	void text(char const* const x) {
		data(x, strlen(x));
	}
	void text(const std::string& x) {
		data(x.c_str(), x.size());
	}

private:
	DISALLOW_COPY_AND_ASSIGN(StreamWrite);
};

#endif  // end of include guard: WL_IO_STREAMWRITE_H
