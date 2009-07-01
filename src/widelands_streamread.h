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

#include "io/streamread.h"

#include "widelands.h"
#include "widelands_geometry.h"

#include <cassert>

namespace Widelands {

struct Building_Descr;
struct Editor_Game_Base;
struct Immovable_Descr;
struct Tribe_Descr;
struct World;

/// A StreamRead that can read Widelands-specific types. It has no extra data
/// members. Therefore it is binary compatible with StreamRead, so any
/// ::StreamRead can be used as a Widelands::StreamRead to read
/// Widelands-specific types.
struct StreamRead : public ::StreamRead {
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
			(std::string const & Tribename, char const * const Name)
			:
			_data_error
				("tribe %s does not define immovable type \"%s\"",
				 Tribename.c_str(), Name),
			tribename(Tribename), name(Name)
		{}
		std::string const & tribename;
		char        const * const name;
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
			(std::string const & Tribename, char const * const Name)
			:
			_data_error
				("tribe %s does not define building type \"%s\"",
				 Tribename.c_str(), Name),
			tribename(Tribename), name(Name)
		{}
		std::string const & tribename;
		char        const * const name;
	};

	/// Read a Direction from the file. Use this when the result can only be a
	/// direction.
	///
	/// \Throws direction_is_null if the direction is 0.
	/// \Throws direction_invalid if direction is > 6.
	Direction Direction8();

	/// Read a Direction from the file. Use this when the result can only be a
	/// direction or 0 (none).
	///
	/// \Throws direction_is_null if the direction is 0.
	/// \Throws direction_invalid if direction is > 6.
	Direction Direction8_allow_null();

	Map_Index Map_Index32(Map_Index max);

	/// Read a Coords from the file. Use this when the result can only be a
	/// coordinate pair referring to a node.
	///
	/// \Throws width_exceeded  if extent.w is <= the x coordinate.
	/// \Throws height_exceeded if extent.h is <= the y coordinate.
	/// Both coordinates are read from the file before checking and possibly
	/// throwing, so in case such an exception is thrown, it is guaranteed that
	/// the whole coordinate pair has been read.
	Coords Coords32(Extent);

	/// Like Coords32 but the result can have the special value indicating
	/// invalidity, as defined by Coords::Null.
	Coords Coords32_allow_null(Extent);

	Coords Coords32(); /// Unchecked reading.

	Area<Coords, uint16_t> Area48(Extent);

	Player_Number Player_Number8() {return Unsigned8();}

	/// Read a Player_Number from the file. Use this when the result can only be
	/// the number of an existing player.
	///
	/// \Throws player_nonexistent when the player number is 0 or
	/// nr_players < the player number.
	Player_Number Player_Number8              (Player_Number nr_players);

	/// Read a Player_Number from the file. Use this when the result can only be
	/// the number of an existing player or 0 (neutral).
	///
	/// \Throws player_nonexistent when nr_players < the player number.
	Player_Number Player_Number8_allow_null(Player_Number nr_players);

	/// Reads a CString and interprets it as the name of a tribe.
	///
	/// \Returns a pointer to the tribe description.
	///
	/// \Throws Tribe_Nonexistent if the there is no tribe with that name.
	Tribe_Descr     const & Tribe           (Editor_Game_Base const &);

	/// Reads a CString and interprets it as the name of a tribe.
	///
	/// \Returns 0 if the name is empty, otherwise a pointer to the tribe
	/// description.
	///
	/// \Throws Tribe_Nonexistent if the name is not empty and there is no tribe
	/// with that name.
	Tribe_Descr     const * Tribe_allow_null(Editor_Game_Base const &);

	/// Reads a CString and interprets t as the name of an immovable type.
	///
	/// \Returns a reference to the immovable type description.
	///
	/// \Throws Immovable_Nonexistent if there is no imovable type with that
	/// name in the World.
	Immovable_Descr const & Immovable_Type  (World            const &);

	/// Reads a CString and interprets t as the name of an immovable type.
	///
	/// \Returns a reference to the immovable type description.
	///
	/// \Throws Immovable_Nonexistent if there is no imovable type with that
	/// name in the tribe.
	Immovable_Descr const & Immovable_Type  (Tribe_Descr      const &);

	/// Reads a CString and interprets t as the name of an immovable type.
	///
	/// \Returns a reference to the building type description.
	///
	/// \Throws Building_Nonexistent if there is no building type with that
	/// name in the tribe.
	Building_Descr  const & Building_Type   (Tribe_Descr      const &);

	/// Calls Tribe(Editor_Game_Base const &) to read a tribe and then reads a
	/// CString and interprets it as the name of a building type in that tribe.
	///
	/// \Returns a reference to the building type description.
	Building_Descr  const & Building_Type   (Editor_Game_Base const &);

	/// Calls Tribe_allow_null(Editor_Game_Base const &). If it returns a tribe,
	/// Immovable_Type(Tribe_Descr const &) is called with that tribe and the
	/// result is returned. Otherwise Immovable_Type(World const &) is called
	/// and the result is returned.
	Immovable_Descr const & Immovable_Type  (Editor_Game_Base const &);
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

inline Coords StreamRead::Coords32(const Extent extent) {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	if (extent.w <= x)
		throw exceeded_width (extent.w, x);
	if (extent.h <= y)
		throw exceeded_height(extent.h, y);
	return Coords(x, y);
}

inline Coords StreamRead::Coords32_allow_null(const Extent extent) {
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

inline Area<Coords, uint16_t> StreamRead::Area48(Extent const extent) {
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

};

#endif
