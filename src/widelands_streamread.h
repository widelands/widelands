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

#ifndef WIDELANDS_STREAMREAD_H
#define WIDELANDS_STREAMREAD_H

#include "geometry.h"
#include "streamread.h"

/// A StreamRead that can read Widelands-specific types.
struct WidelandsStreamRead : public StreamRead {

	struct Data_Error {};
	struct Extent_Exceeded : public Data_Error {};
	struct Width_Exceeded : public Extent_Exceeded {
		Width_Exceeded(const Uint16 W, const X_Coordinate X) : w(W), x(X) {}
		Uint16       w;
		X_Coordinate x;
	};
	struct Height_Exceeded : public Extent_Exceeded {
		Height_Exceeded(const Uint16 H, const Y_Coordinate Y) : h(H), y(Y) {}
		Uint16       h;
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
	 * Coords::isNull. Unless the read Coords is null, this will throw an
	 * exception if the width is <= the x coordinate or the height is <= the y
	 * coordinate. Both coordinates are read from the stream before checking and
	 * possibly throwing, so in case such an exception is thrown, it is
	 * guaranteed that the whole coordinate pair has been read.
	 */
	Coords Coords32_allow_null(const Extent extent);

};

inline Coords WidelandsStreamRead::Coords32() {
	const Uint16 x = Unsigned16();
	const Uint16 y = Unsigned16();
	return Coords(x, y);
}

inline Coords WidelandsStreamRead::Coords32(const Extent extent) {
	const Uint16 x = Unsigned16();
	const Uint16 y = Unsigned16();
	if (extent.w <= x) throw Width_Exceeded (extent.w, x);
	if (extent.h <= y) throw Height_Exceeded(extent.h, y);
	return Coords(x, y);
}

inline Coords WidelandsStreamRead::Coords32_allow_null(const Extent extent) {
	const Uint16 x = Unsigned16();
	const Uint16 y = Unsigned16();
	const Coords result(x, y);
	if (not result.isNull()) {
		if (extent.w <= x) throw Width_Exceeded (extent.w, x);
		if (extent.h <= y) throw Height_Exceeded(extent.h, y);
	}
	return result;
}

#endif
