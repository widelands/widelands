/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "logic/widelands_geometry_io.h"

#include <cassert>

#include "io/streamread.h"
#include "io/streamwrite.h"

namespace Widelands {

Direction read_direction_8(StreamRead* fr) {
	uint8_t const d = fr->unsigned_8();
	if (d == 0) {
		throw DirectionIsNull();
	}
	if (6 < d) {
		throw DirectionInvalid(d);
	}
	return d;
}

Direction read_direction_8_allow_null(StreamRead* fr) {
	uint8_t const d = fr->unsigned_8();
	if (6 < d) {
		throw DirectionInvalid(d);
	}
	return d;
}

Coords read_coords_32(StreamRead* stream_read) {
	uint16_t const x = stream_read->unsigned_16();
	uint16_t const y = stream_read->unsigned_16();
	return Coords(x, y);
}

Coords read_coords_32(StreamRead* stream_read, const Extent& extent) {
	uint16_t const x = stream_read->unsigned_16();
	uint16_t const y = stream_read->unsigned_16();
	if (extent.w <= x) {
		throw ExceededWidth(extent.w, x);
	}
	if (extent.h <= y) {
		throw ExceededHeight(extent.h, y);
	}
	return Coords(x, y);
}

Coords read_coords_32_allow_null(StreamRead* fr, const Extent& extent) {
	uint16_t const x = fr->unsigned_16();
	uint16_t const y = fr->unsigned_16();
	const Coords result(x, y);
	if (result) {
		if (extent.w <= x) {
			throw ExceededWidth(extent.w, x);
		}
		if (extent.h <= y) {
			throw ExceededHeight(extent.h, y);
		}
	}
	return result;
}

void write_direction_8(StreamWrite* wr, Direction const d) {
	assert(0 < d);
	assert(d <= 6);
	wr->data(&d, 1);
}

void write_direction_8_allow_null(StreamWrite* wr, Direction const d) {
	assert(d <= 6);
	wr->data(&d, 1);
}

void write_coords_32(StreamWrite* wr, const Coords& c) {
	assert(static_cast<uint16_t>(c.x) < 0x8000 || c.x == -1);
	assert(static_cast<uint16_t>(c.y) < 0x8000 || c.y == -1);
	wr->unsigned_16(c.x);
	wr->unsigned_16(c.y);
}

}  // namespace Widelands
