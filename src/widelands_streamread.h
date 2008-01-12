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

#ifndef WIDELANDS_STREAMREAD_H
#define WIDELANDS_STREAMREAD_H

#include "streamread.h"

#include "widelands.h"
#include "widelands_geometry.h"

#include <cassert>

namespace Widelands {

/// A StreamRead that can read Widelands-specific types. It has no extra data
/// members. Therefore it is binary compatible with StreamRead, so any
/// ::StreamRead can be used as a Widelands::StreamRead to read
/// Widelands-specific types.
struct StreamRead : public ::StreamRead {
	struct Extent_Exceeded : public Data_Error {};
	struct Width_Exceeded : public Extent_Exceeded {
		Width_Exceeded (uint16_t const W, const X_Coordinate X) : w(W), x(X) {}
		std::string message() const {
			std::string result;
			result += "x coordinate is ";
			result += x;
			result += " but width is only ";
			result += w;
			return result;
		}
		uint16_t     w;
		X_Coordinate x;
	};
	struct Height_Exceeded : public Extent_Exceeded {
		Height_Exceeded(uint16_t const H, const Y_Coordinate Y) : h(H), y(Y) {}
		std::string message() const {
			std::string result;
			result += "y coordinate is ";
			result += y;
			result += " but height is only ";
			result += h;
			return result;
		}
		uint16_t     h;
		Y_Coordinate y;
	};

	/// Read a Coords from the file. Use this when the result can only be a
	/// coordinate pair referring to a node.
	///
	/// \throws Width_Exceeded  if extent.w is <= the x coordinate.
	/// \throws Height_Exceeded if extent.h is <= the y coordinate.
	/// Both coordinates are read from the file before checking and possibly
	/// throwing, so in case such an exception is thrown, it is guaranteed that
	/// the whole coordinate pair has been read.
	Coords Coords32(const Extent extent);

	/// Like Coords32 but the result can have the special value indicating
	/// invalidity, as defined by Coords::Null.
	Coords Coords32_allow_null(const Extent extent);

	Coords Coords32(); /// Unchecked reading.

	Player_Number Player_Number8() {return Unsigned8();}
};

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

inline Coords StreamRead::Coords32() {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	return Coords(x, y);
}

};

#endif
