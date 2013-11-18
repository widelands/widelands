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

#ifndef STREAMWRITE_H
#define STREAMWRITE_H

#include <cassert>
#include <cstring>
#include <limits>
#include <string>

#include <boost/noncopyable.hpp>

#include "machdep.h"
#include "port.h"

/**
 * Abstract base class for stream-like data sinks.
 * It is intended for serializing network packets and for writing log-type
 * data to disk.
 *
 * All implementations need to implement \ref Data . Some implementations
 * may need to implement \ref Flush .
 *
 * Convenience functions are provided for many data types.
 */
class StreamWrite : boost::noncopyable {
public:
	explicit StreamWrite() {}
	virtual ~StreamWrite();

	/**
	 * Write a number of bytes to the stream.
	 */
	virtual void Data(const void * const data, const size_t size) = 0;

	/**
	 * Make sure all data submitted so far is written to disk.
	 *
	 * The default implementation is a no-op. Implementations may want
	 * to override this if they're buffered internally.
	 */
	virtual void Flush();

	//TODO: implement an overloaded method that accepts fmt as std::string
	void Printf(char const *, ...) __attribute__((format(printf, 2, 3)));

	void   Signed8  (int8_t const x) {Data(&x, 1);}
	void Unsigned8 (uint8_t const x) {Data(&x, 1);}
	void   Signed16(int16_t const x) {
		int16_t const y = Little16(x);
		Data(&y, 2);
	}
	void Unsigned16(uint16_t const x) {
		uint16_t const y = Little16(x);
		Data(&y, 2);
	}
	void   Signed32(int32_t const x) {
		uint32_t const y = Little32(x);
		Data(&y, 4);
	}
	void Unsigned32(uint32_t const x) {
		uint32_t const y = Little32(x);
		Data(&y, 4);
	}
	void String(const std::string & str) {
		Data(str.c_str(), str.size() + 1);
	}

	//  Write strings with    null terminator.
	void CString(char        const * const x) {Data(x,         strlen(x) + 1);}
	void CString(const std::string &       x) {Data(x.c_str(), x.size()  + 1);}

	//  Write strings without null terminator.
	void Text   (char        const * const x) {Data(x,         strlen(x));}
	void Text   (const std::string &       x) {Data(x.c_str(), x.size());}
};

#endif
