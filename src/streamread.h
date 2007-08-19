/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include <limits>
#include <string>

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
class StreamRead {
public:
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
	virtual bool EndOfFile() = 0;

	void DataComplete(void* const data, const size_t size);

	Sint8 Signed8();
	Uint8 Unsigned8();
	Sint16 Signed16();
	Uint16 Unsigned16();
	Sint32 Signed32();
	Uint32 Unsigned32();
	std::string String();
};

#endif

