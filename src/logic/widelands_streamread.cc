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

#include "logic/widelands_streamread.h"

#include <cassert>

#include "logic/immovable.h"
#include "logic/tribe.h"
#include "logic/world.h"

// NOCOM(#sirver): rename file.
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

Map_Index ReadMap_Index32(StreamRead* fr, Map_Index const max) {
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

Player_Number ReadPlayer_Number8_allow_null(StreamRead* fr, Player_Number const nr_players) {
	Player_Number const p = fr->Unsigned8();
	if (nr_players < p)
		throw player_nonexistent(nr_players, p);
	return p;
}

Player_Number ReadPlayer_Number8(StreamRead* fr, Player_Number const nr_players) {
	Player_Number const p = ReadPlayer_Number8_allow_null(fr, nr_players);
	if (p == 0)
		throw player_nonexistent(nr_players, 0);
	return p;
}

const Immovable_Descr& ReadImmovable_Type(StreamRead* fr, const Tribe_Descr& tribe) {
	std::string name = fr->CString();
	int32_t const index = tribe.get_immovable_index(name);
	if (index == -1)
		throw tribe_immovable_nonexistent(tribe.name(), name);
	return *tribe.get_immovable_descr(index);
}

const Tribe_Descr& ReadTribe(StreamRead* fr, const Editor_Game_Base& egbase) {
	char const* const name = fr->CString();
	if (Tribe_Descr const* const result = egbase.get_tribe(name))
		return *result;
	else
		throw tribe_nonexistent(name);
}

Tribe_Descr const* ReadTribe_allow_null(StreamRead* fr, const Editor_Game_Base& egbase) {
	char const* const name = fr->CString();
	if (*name)
		if (Tribe_Descr const* const result = egbase.get_tribe(name))
			return result;
		else
			throw tribe_nonexistent(name);
	else
		return nullptr;
}

const Immovable_Descr& ReadImmovable_Type(StreamRead* fr, const World& world) {
	char const* const name = fr->CString();
	int32_t const index = world.get_immovable_index(name);
	if (index == -1)
		throw world_immovable_nonexistent(world.get_name(), name);
	return *world.get_immovable_descr(index);
}

const Immovable_Descr& ReadImmovable_Type(StreamRead* fr, const Editor_Game_Base& egbase) {
	if (Tribe_Descr const* const tribe = ReadTribe_allow_null(fr, egbase))
		return ReadImmovable_Type(fr, *tribe);
	else
		return ReadImmovable_Type(fr, egbase.map().world());
}

const Building_Descr& ReadBuilding_Type(StreamRead* fr, const Tribe_Descr& tribe) {
	char const* const name = fr->CString();
	Building_Index const index = tribe.building_index(name);
	if (index == INVALID_INDEX)
		throw building_nonexistent(tribe.name(), name);
	return *tribe.get_building_descr(index);
}

const Building_Descr& ReadBuilding_Type(StreamRead* fr, const Editor_Game_Base& egbase) {
	return ReadBuilding_Type(fr, ReadTribe(fr, egbase));
}

}
