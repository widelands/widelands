/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef STREAMREAD_H
#define STREAMREAD_H

#include "machdep.h"
#include "wexception.h"

#include <string>
#include <cstring>

/**
 * Abstract base class for stream-like data sources.
 * It is intended for deserializing network packets and for reading log-type
 * data from disk.
 *
 * This is not intended for pipes or pipe-like operations, and all reads
 * are "blocking". Once \ref Data returns 0, or any number less than the
 * requested number of bytes, the stream is at its end.
 *
 * All implementations need to implement \ref Data and \ref EndOfFile .
 *
 * Convenience functions are provided for many data types.
 */
struct StreamRead {
	explicit StreamRead() {}
	virtual ~StreamRead();

	/**
	 * Read a number of bytes from the stream.
	 *
	 * \return the number of bytes that were actually read. Will return 0 at
	 * end of stream.
	 */
	virtual size_t Data(void* const data, const size_t bufsize) = 0;

	/**
	 * \return \c true if the end of file / end of stream has been reached.
	 */
	virtual bool EndOfFile() const = 0;

	void DataComplete(void * data, size_t size);

	int8_t Signed8();
	uint8_t Unsigned8();
	int16_t Signed16();
	uint16_t Unsigned16();
	int32_t Signed32();
	uint32_t Unsigned32();
	std::string String();
	__attribute__((noreturn)) virtual char const * CString() {throw;}

	///  Base of all exceptions that are caused by errors in the data that is
	///  read.
	struct _data_error : public _wexception {
		_data_error(char const * const fmt, ...) throw () PRINTF_FORMAT(2, 3);
	};
#define data_error(...) _data_error(__VA_ARGS__)

private:
	StreamRead & operator=(StreamRead const &);
	explicit StreamRead   (StreamRead const &);
};

#endif

