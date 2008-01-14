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
	struct Max_Index_Exceeded : public Data_Error {
		Max_Index_Exceeded(Map_Index const Max, Map_Index const I)
			: max(Max), i(I)
		{}
		std::string message() const {
			std::string result;
			result  = "index is ";
			result += i;
			result += " but max index is only ";
			result += max;
			return result;
		}
		Map_Index const max, i;
	};
	struct Extent_Exceeded : public Data_Error {};
	struct Width_Exceeded : public Extent_Exceeded {
		Width_Exceeded (uint16_t const W, const X_Coordinate X) : w(W), x(X) {}
		std::string message() const {
			std::string result;
			result  = "x coordinate is ";
			result += x;
			result += " but width is only ";
			result += w;
			return result;
		}
		uint16_t     const w;
		X_Coordinate const x;
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
	struct Tribe_Nonexistent           : public Data_Error {
		Tribe_Nonexistent(char const * const Name) : name(Name) {}
		std::string message() const {
			std::string result;
			result  = "tribe \"";
			result += name;
			result += "\" does not exist";
			return result;
		}
		char const * const name;
	};
	struct Immovable_Error             : public Data_Error {};
	struct Immovable_Nonexistent : public Immovable_Error {
		Immovable_Nonexistent(char const * const Name) : name(Name) {}
		char const * const name;
	};
	struct Tribe_Immovable_Nonexistent : public Immovable_Nonexistent {
		Tribe_Immovable_Nonexistent
			(std::string const & Tribename, char const * const Name)
			: Immovable_Nonexistent(Name), tribename(Tribename)
		{}
		std::string message() const {
			std::string result;
			result  = "immovable \"";
			result += name;
			result += "\" does not exist in tribe ";
			result += tribename;
			return result;
		}
		std::string const & tribename;
	};
	struct World_Immovable_Nonexistent : public Immovable_Nonexistent {
		World_Immovable_Nonexistent(char const * const Name)
			: Immovable_Nonexistent(Name)
		{}
		std::string message() const {
			std::string result;
			result  = "immovable \"";
			result += name;
			result += "\" does not exist in world";
			return result;
		}
	};
	struct Building_Nonexistent : public Data_Error {
		Building_Nonexistent
			(std::string const & Tribename, char const * const Name)
			: tribename(Tribename), name(Name)
		{}
		std::string message() const {
			std::string result;
			result  = "building \"";
			result += name;
			result += "\" does not exist in tribe ";
			result += tribename;
			return result;
		}
		std::string const &  tribename;
		char        const * const name;
	};

	Map_Index Map_Index32(Map_Index max);

	/// Read a Coords from the file. Use this when the result can only be a
	/// coordinate pair referring to a node.
	///
	/// \Throws Width_Exceeded  if extent.w is <= the x coordinate.
	/// \Throws Height_Exceeded if extent.h is <= the y coordinate.
	/// Both coordinates are read from the file before checking and possibly
	/// throwing, so in case such an exception is thrown, it is guaranteed that
	/// the whole coordinate pair has been read.
	Coords Coords32(const Extent extent);

	/// Like Coords32 but the result can have the special value indicating
	/// invalidity, as defined by Coords::Null.
	Coords Coords32_allow_null(const Extent extent);

	Coords Coords32(); /// Unchecked reading.

	Player_Number Player_Number8() {return Unsigned8();}

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

inline Map_Index StreamRead::Map_Index32(Map_Index const max) {
	uint32_t const i = Unsigned32();
	if (max <= i) throw Max_Index_Exceeded(max, i);
	return i;
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

inline Coords StreamRead::Coords32() {
	uint16_t const x = Unsigned16();
	uint16_t const y = Unsigned16();
	return Coords(x, y);
}

};

#endif
