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

#ifndef WIDELANDS_STREAMREAD_H
#define WIDELANDS_STREAMREAD_H

#include <cassert>

#include "io/streamread.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct Building_Descr;
class Editor_Game_Base;
struct Immovable_Descr;
struct Tribe_Descr;
struct World;

/// A StreamRead that can read Widelands-specific types. It has no extra data
/// members. Therefore it is binary compatible with StreamRead, so any
/// ::StreamRead can be used as a Widelands::StreamRead to read
/// Widelands-specific types.
class StreamRead : public ::StreamRead {
public:
	struct direction_is_null : public _data_error {
		direction_is_null
			()
			:
			_data_error
				("direction is 0 but must be one of {1 (northeast), 2 (east), 3 "
				 "(southeast), 4 (southwest), 5 (west), 6 (northwest)}")
		{}
	};
	struct direction_invalid : public _data_error {
		direction_invalid
			(Direction const D)
			:
			_data_error
				("direction is %u but must be one of {0 (idle), 1 (northeast), 2 "
				 "(east), 3 (southeast), 4 (southwest), 5 (west), 6 (northwest)}",
				 D),
			direction(D)
		{}
		Direction direction;
	};
	struct exceeded_max_index : public _data_error {
		exceeded_max_index
			(Map_Index const Max, Map_Index const I)
			:
			_data_error
				("index is %u but max index is only %u",
				 I, Max),
			max(Max), i(I)
		{}
		Map_Index const max, i;
	};
	struct exceeded_width : public _data_error {
		exceeded_width
			(uint16_t const W, const X_Coordinate X)
			:
			_data_error
				("x coordinate is %i but width is only %u",
				 X, W),
			w(W), x(X)
		{}
		uint16_t     const w;
		X_Coordinate const x;
	};
	struct exceeded_height : public _data_error {
		exceeded_height
			(uint16_t const H, const Y_Coordinate Y)
			:
			_data_error
				("y coordinate is %i but height is only %u",
				 Y, H),
			h(H), y(Y)
		{}
		uint16_t     h;
		Y_Coordinate y;
	};
	struct player_nonexistent          : public _data_error {
		player_nonexistent
			(Player_Number const N, Player_Number const P)
			:
			_data_error
				("player number is %u but there are only %u players",
				 P, N),
			nr_players(N), player_number(P)
		{}
		Player_Number nr_players, player_number;
	};
	struct tribe_nonexistent           : public _data_error {
		tribe_nonexistent
			(char const * const Name)
			:
			_data_error
				("tribe \"%s\" does not exist",
				 Name),
			name(Name) {}
		char const * const name;
	};
	struct tribe_immovable_nonexistent : public _data_error {
		tribe_immovable_nonexistent
			(const std::string & Tribename, const std::string & Name)
			:
			_data_error
				("tribe %s does not define immovable type \"%s\"",
				 Tribename.c_str(), Name.c_str()),
			tribename(Tribename), name(Name)
		{}
		virtual ~tribe_immovable_nonexistent() throw () {}
		std::string tribename;
		std::string name;
	};
	struct world_immovable_nonexistent : public _data_error {
		world_immovable_nonexistent
			(char const * const Worldname, char const * const Name)
			:
			_data_error
				("world %s does not define immovable type \"%s\"",
				 Worldname, Name),
			worldname(Worldname), name(Name)
		{}
		char const * const worldname, * const name;
	};
	struct building_nonexistent : public _data_error {
		building_nonexistent
			(const std::string & Tribename, char const * const Name)
			:
			_data_error
				("tribe %s does not define building type \"%s\"",
				 Tribename.c_str(), Name),
			tribename(Tribename), name(Name)
		{}
		const std::string & tribename;
		char        const * const name;
	};

	/// Read a Direction from the file. Use this when the result can only be a
	/// direction.
	///
	/// \throws direction_is_null if the direction is 0.
	/// \throws direction_invalid if direction is > 6.
	Direction Direction8();

	/// Read a Direction from the file. Use this when the result can only be a
	/// direction or 0 (none).
	///
	/// \throws direction_invalid if direction is > 6.
	Direction Direction8_allow_null();

	Map_Index Map_Index32(Map_Index max);

	/// Read a Coords from the file. Use this when the result can only be a
	/// coordinate pair referring to a node.
	///
	/// \throws width_exceeded  if extent.w is <= the x coordinate.
	/// \throws height_exceeded if extent.h is <= the y coordinate.
	/// Both coordinates are read from the file before checking and possibly
	/// throwing, so in case such an exception is thrown, it is guaranteed that
	/// the whole coordinate pair has been read.
	Coords Coords32(const Extent &);

	/// Like Coords32 but the result can have the special value indicating
	/// invalidity, as defined by Coords::Null.
	Coords Coords32_allow_null(const Extent &);

	Coords Coords32(); /// Unchecked reading.

	Area<Coords, uint16_t> Area48(const Extent &);

	Player_Number Player_Number8() {return Unsigned8();}

	/// Read a Player_Number from the file. Use this when the result can only be
	/// the number of an existing player.
	///
	/// \throws player_nonexistent when the player number is 0 or
	/// nr_players < the player number.
	Player_Number Player_Number8              (Player_Number nr_players);

	/// Read a Player_Number from the file. Use this when the result can only be
	/// the number of an existing player or 0 (neutral).
	///
	/// \throws player_nonexistent when nr_players < the player number.
	Player_Number Player_Number8_allow_null(Player_Number nr_players);

	/// Reads a CString and interprets it as the name of a tribe.
	///
	/// \returns a pointer to the tribe description.
	///
	/// \throws Tribe_Nonexistent if the there is no tribe with that name.
	const Tribe_Descr     & Tribe           (const Editor_Game_Base &);

	/// Reads a CString and interprets it as the name of a tribe.
	///
	/// \returns 0 if the name is empty, otherwise a pointer to the tribe
	/// description.
	///
	/// \throws Tribe_Nonexistent if the name is not empty and there is no tribe
	/// with that name.
	Tribe_Descr     const * Tribe_allow_null(const Editor_Game_Base &);

	/// Reads a CString and interprets t as the name of an immovable type.
	///
	/// \returns a reference to the immovable type description.
	///
	/// \throws Immovable_Nonexistent if there is no imovable type with that
	/// name in the World.
	const Immovable_Descr & Immovable_Type  (const World            &);

	/// Reads a CString and interprets t as the name of an immovable type.
	///
	/// \returns a reference to the immovable type description.
	///
	/// \throws Immovable_Nonexistent if there is no imovable type with that
	/// name in the tribe.
	const Immovable_Descr & Immovable_Type  (const Tribe_Descr      &);

	/// Reads a CString and interprets t as the name of an immovable type.
	///
	/// \returns a reference to the building type description.
	///
	/// \throws Building_Nonexistent if there is no building type with that
	/// name in the tribe.
	const Building_Descr  & Building_Type   (const Tribe_Descr      &);

	/// Calls Tribe(const Editor_Game_Base &) to read a tribe and then reads a
	/// CString and interprets it as the name of a building type in that tribe.
	///
	/// \returns a reference to the building type description.
	const Building_Descr  & Building_Type   (const Editor_Game_Base &);

	/// Calls Tribe_allow_null(const Editor_Game_Base &). If it returns a tribe,
	/// Immovable_Type(const Tribe_Descr &) is called with that tribe and the
	/// result is returned. Otherwise Immovable_Type(const World &) is called
	/// and the result is returned.
	const Immovable_Descr & Immovable_Type  (const Editor_Game_Base &);
};

inline Direction StreamRead::Direction8() {
	uint8_t const d = Unsigned8();
	if (d == 0)
		throw direction_is_null();
	if (6 < d)
		throw direction_invalid(d);
	return d;
}

inline Direction StreamRead::Direction8_allow_null() {
	uint8_t const d = Unsigned8();
	if (6 < d)
		throw direction_invalid(d);
	return d;
}

inline Map_Index StreamRead::Map_Index32(Map_Index const max) {
	uint32_t const i = Unsigned32();
	if (max <= i)
		throw exceeded_max_index(max, i);
	return i;
}

inline Coords StreamRead::Coords32(const Extent & extent) {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	if (extent.w <= x)
		throw exceeded_width (extent.w, x);
	if (extent.h <= y)
		throw exceeded_height(extent.h, y);
	return Coords(x, y);
}

inline Coords StreamRead::Coords32_allow_null(const Extent & extent) {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	const Coords result(x, y);
	if (result) {
		if (extent.w <= x)
			throw exceeded_width (extent.w, x);
		if (extent.h <= y)
			throw exceeded_height(extent.h, y);
	}
	return result;
}

inline Coords StreamRead::Coords32() {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	return Coords(x, y);
}

inline Area<Coords, uint16_t> StreamRead::Area48(const Extent & extent) {
	Coords   const c =   Coords32(extent);
	uint16_t const r = Unsigned16();
	return Area<Coords, uint16_t>(c, r);
}

inline Player_Number StreamRead::Player_Number8_allow_null
	(Player_Number const nr_players)
{
	Player_Number const p = Player_Number8();
	if (nr_players < p)
		throw player_nonexistent(nr_players, p);
	return p;
}

inline Player_Number StreamRead::Player_Number8(Player_Number const nr_players)
{
	Player_Number const p = Player_Number8_allow_null(nr_players);
	if (p == 0)
		throw player_nonexistent(nr_players, 0);
	return p;
}

}

#endif
