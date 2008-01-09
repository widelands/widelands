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

#include "geometry.h"
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
	virtual bool EndOfFile() = 0;

	void DataComplete(void* const data, const size_t size);

	int8_t Signed8();
	uint8_t Unsigned8();
	int16_t Signed16();
	uint16_t Unsigned16();
	int32_t Signed32();
	uint32_t Unsigned32();
	std::string String();

	struct Data_Error {};
	struct Extent_Exceeded : public Data_Error {};
	struct Width_Exceeded : public Extent_Exceeded {
		Width_Exceeded (uint16_t const W, X_Coordinate const X) : w(W), x(X) {}
		uint16_t     w;
		X_Coordinate x;
	};
	struct Height_Exceeded : public Extent_Exceeded {
		Height_Exceeded(uint16_t const H, Y_Coordinate const Y) : h(H), y(Y) {}
		uint16_t     h;
		Y_Coordinate y;
	};

	/**
	 * Read a Coords from the stream. Use this when the result can only be a node
	 * coordinate. Will throw an exception if the width is <= the x coordinate or
	 * the height is <= the y coordinate. Both coordinates are read from the
	 * stream before checking and possibly throwing, so in case such an exception
	 * is thrown, it is guaranteed that the whole coordinate pair has been read.
	 */
	Coords Coords32(const Extent extent);

	Coords Coords32();

	/**
	 * Read Coords from the stream. Use this when the result can only be a node
	 * coordinate or the special value indicating invalidity, as defined by
	 * Coords::Null. Unless the read Coords is null, this will throw an exception
	 * if the width is <= the x coordinate or the height is <= the y coordinate.
	 * Both coordinates are read from the stream before checking and possibly
	 * throwing, so in case such an exception is thrown, it is guaranteed that
	 * the whole coordinate pair has been read.
	 */
	Coords Coords32_allow_null(const Extent extent);

private:
	StreamRead & operator=(StreamRead const &);
	explicit StreamRead   (StreamRead const &);
};


inline Coords StreamRead::Coords32() {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	return Coords(x, y);
}

inline Coords StreamRead::Coords32(const Extent extent) {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	if (extent.w <= x) throw Width_Exceeded (extent.w, x);
	if (extent.h <= y) throw Height_Exceeded(extent.h, y);
	return Coords(x, y);
}

inline Coords StreamRead::Coords32_allow_null(const Extent extent) {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	const Coords result(x, y);
	if (result) {
		if (extent.w <= x) throw Width_Exceeded (extent.w, x);
		if (extent.h <= y) throw Height_Exceeded(extent.h, y);
	}
	return result;
}

#endif

