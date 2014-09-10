/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef WL_LOGIC_WIDELANDS_GEOMETRY_IO_H
#define WL_LOGIC_WIDELANDS_GEOMETRY_IO_H

#include "io/fileread.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct DirectionIsNull : public FileRead::DataError {
	DirectionIsNull()
		: DataError("direction is 0 but must be one of {1 (northeast), 2 (east), 3 "
	                 "(southeast), 4 (southwest), 5 (west), 6 (northwest)}") {
	}
};

struct DirectionInvalid : public FileRead::DataError {
	DirectionInvalid(Direction const D)
		: DataError("direction is %u but must be one of {0 (idle), 1 (northeast), 2 "
	                 "(east), 3 (southeast), 4 (southwest), 5 (west), 6 (northwest)}",
	                 D),
	     direction(D) {
	}
	Direction direction;
};
struct ExceededMaxIndex : public FileRead::DataError {
	ExceededMaxIndex(MapIndex const Max, MapIndex const I)
		: DataError("index is %u but max index is only %u", I, Max), max(Max), i(I) {
	}
	MapIndex const max, i;
};
struct ExceededWidth : public FileRead::DataError {
	ExceededWidth(uint16_t const W, const XCoordinate X)
		: DataError("x coordinate is %i but width is only %u", X, W), w(W), x(X) {
	}
	uint16_t const w;
	XCoordinate const x;
};
struct ExceededHeight : public FileRead::DataError {
	ExceededHeight(uint16_t const H, const YCoordinate Y)
		: DataError("y coordinate is %i but height is only %u", Y, H), h(H), y(Y) {
	}
	uint16_t h;
	YCoordinate y;
};

/// Read a Direction from the file. Use this when the result can only be a
/// direction.
///
/// \throws direction_is_null if the direction is 0.
/// \throws direction_invalid if direction is > 6.
Direction ReadDirection8(StreamRead* fr);

/// Read a Direction from the file. Use this when the result can only be a
/// direction or 0 (none).
///
/// \throws direction_invalid if direction is > 6.
Direction ReadDirection8_allow_null(StreamRead* fr);

MapIndex ReadMap_Index32(StreamRead* fr, MapIndex max);

/// Read a Coords from the file. Use this when the result can only be a
/// coordinate pair referring to a node.
///
/// \throws width_exceeded  if extent.w is <= the x coordinate.
/// \throws height_exceeded if extent.h is <= the y coordinate.
/// Both coordinates are read from the file before checking and possibly
/// throwing, so in case such an exception is thrown, it is guaranteed that
/// the whole coordinate pair has been read.
Coords ReadCoords32(StreamRead* stream_read);

Coords ReadCoords32(StreamRead* stream_read, const Extent& extent);

/// Like Coords32 but the result can have the special value indicating
/// invalidity, as defined by Coords::Null.
Coords ReadCoords32_allow_null(StreamRead* fr, const Extent& extent);

Area<Coords, uint16_t> ReadArea48(StreamRead* fr, const Extent& extent);

// Writes 'd' to 'wr'.
void WriteDirection8(StreamWrite* wr, Direction d);

// Writes 'd' into 'wr'.
void WriteDirection8_allow_null(StreamWrite* wr, Direction d);

// Writes 'c' to 'wr'.
void WriteCoords32(StreamWrite* wr, const Coords& c);

// Writes 'area' to 'wr'.
void WriteArea48(StreamWrite* wr, Area<Coords, uint16_t> const area);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_WIDELANDS_GEOMETRY_IO_H
