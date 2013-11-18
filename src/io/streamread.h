/*
 * Copyright (C) 2007-2011 by the Widelands Development Team
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

#ifndef STREAMREAD_H
#define STREAMREAD_H

#include <cstring>
#include <string>

#include <boost/noncopyable.hpp>

#include "machdep.h"
#include "wexception.h"


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
class StreamRead : boost::noncopyable {
public:
	explicit StreamRead() {}
	virtual ~StreamRead();

	/**
	 * Read a number of bytes from the stream.
	 *
	 * \return the number of bytes that were actually read. Will return 0 at
	 * end of stream.
	 */
	virtual size_t Data(void * data, size_t bufsize) = 0;

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
	virtual char const * CString() {throw;}

	///  Base of all exceptions that are caused by errors in the data that is
	///  read.
	struct _data_error : public _wexception {
		_data_error(char const * const fmt, ...) PRINTF_FORMAT(2, 3);
	};
#define data_error(...) _data_error(__VA_ARGS__)
};

#endif
