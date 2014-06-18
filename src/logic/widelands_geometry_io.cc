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

#include "logic/widelands_geometry_io.h"

#include "io/streamread.h"
#include "io/streamwrite.h"

namespace Widelands {

Direction ReadDirection8(StreamRead* fr) {
	uint8_t const d = fr->Unsigned8();
	if (d == 0)
		throw direction_is_null();
	if (6 < d)
		throw direction_invalid(d);
	return d;
}

Direction ReadDirection8_allow_null(StreamRead* fr) {
	uint8_t const d = fr->Unsigned8();
	if (6 < d)
		throw direction_invalid(d);
	return d;
}

Map_Index ReadMap_Index32(StreamRead* fr, const Map_Index max) {
	uint32_t const i = fr->Unsigned32();
	if (max <= i)
		throw exceeded_max_index(max, i);
	return i;
}

Coords ReadCoords32(StreamRead* stream_read) {
	uint16_t const x = stream_read->Unsigned16();
	uint16_t const y = stream_read->Unsigned16();
	return Coords(x, y);
}

Coords ReadCoords32(StreamRead* stream_read, const Extent& extent) {
	uint16_t const x = stream_read->Unsigned16();
	uint16_t const y = stream_read->Unsigned16();
	if (extent.w <= x)
		throw exceeded_width(extent.w, x);
	if (extent.h <= y)
		throw exceeded_height(extent.h, y);
	return Coords(x, y);
}

Coords ReadCoords32_allow_null(StreamRead* fr, const Extent& extent) {
	uint16_t const x = fr->Unsigned16();
	uint16_t const y = fr->Unsigned16();
	const Coords result(x, y);
	if (result) {
		if (extent.w <= x)
			throw exceeded_width(extent.w, x);
		if (extent.h <= y)
			throw exceeded_height(extent.h, y);
	}
	return result;
}

Area<Coords, uint16_t> ReadArea48(StreamRead* fr, const Extent& extent) {
	Coords const c = ReadCoords32(fr, extent);
	uint16_t const r = fr->Unsigned16();
	return Area<Coords, uint16_t>(c, r);
}

void WriteDirection8(StreamWrite* wr, Direction const d) {
	assert(0 < d);
	assert(d <= 6);
	wr->Data(&d, 1);
}

void WriteDirection8_allow_null(StreamWrite* wr, Direction const d) {
	assert(d <= 6);
	wr->Data(&d, 1);
}

void WriteCoords32(StreamWrite* wr, const Coords& c) {
	assert(static_cast<uint16_t>(c.x) < 0x8000 or c.x == -1);
	assert(static_cast<uint16_t>(c.y) < 0x8000 or c.y == -1);
	{
		uint16_t const x = Little16(c.x);
		wr->Data(&x, 2);
	}
	{
		uint16_t const y = Little16(c.y);
		wr->Data(&y, 2);
	}
}

void WriteArea48(StreamWrite* wr, Area<Coords, uint16_t> const area) {
	WriteCoords32(wr, area);
	wr->Unsigned16(area.radius);
}

}  // namespace Widelands
