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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_IO_STREAMREAD_H
#define WL_IO_STREAMREAD_H

#include "base/macros.h"
#include "base/wexception.h"

/**
 * Abstract base class for stream-like data sources.
 * It is intended for deserializing network packets and for reading log-type
 * data from disk.
 *
 * This is not intended for pipes or pipe-like operations, and all reads
 * are "blocking". Once \ref data returns 0, or any number less than the
 * requested number of bytes, the stream is at its end.
 *
 * All implementations need to implement \ref data and \ref end_of_file .
 *
 * Convenience functions are provided for many data types.
 */
class StreamRead {
public:
	explicit StreamRead() {
	}
	virtual ~StreamRead() = default;

	/**
	 * Read a number of bytes from the stream.
	 *
	 * \return the number of bytes that were actually read. Will return 0 at
	 * end of stream.
	 */
	virtual size_t data(void* read_data, size_t bufsize) = 0;

	/**
	 * \return \c true if the end of file / end of stream has been reached.
	 */
	virtual bool end_of_file() const = 0;

	void data_complete(void* data, size_t size);

	int8_t signed_8();
	uint8_t unsigned_8();
	int16_t signed_16();
	uint16_t unsigned_16();
	int32_t signed_32();
	uint32_t unsigned_32();
	float float_32();
	std::string string();
	virtual char const* c_string() {
		throw;
	}

	///  Base of all exceptions that are caused by errors in the data that is
	///  read.
	class DataError : public WException {
	public:
		DataError(char const* const fmt, ...) PRINTF_FORMAT(2, 3);
	};
#define data_error(...) DataError(__VA_ARGS__)

private:
	DISALLOW_COPY_AND_ASSIGN(StreamRead);
};

#endif  // end of include guard: WL_IO_STREAMREAD_H
