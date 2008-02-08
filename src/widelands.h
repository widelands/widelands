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

#ifndef __S__WIDELANDS_H
#define __S__WIDELANDS_H

#include <cassert>
#include <limits>

#include <stdint.h>

namespace Widelands {

//  Type definitions for the game logic.

typedef uint8_t Tribe_Index;

typedef uint16_t Military_Influence;

typedef uint8_t  Player_Number; /// 5 bits used, so 0 .. 31
inline Player_Number Neutral() throw () {return 0;}
#define iterate_player_numbers(p, nr_players) \
	for (Widelands::Player_Number p = 1; p <= nr_players; ++p)

typedef uint8_t  Terrain_Index;   /// 4 bits used, so 0 .. 15.
typedef uint8_t  Resource_Index;  /// 4 bits used, so 0 .. 15.
typedef uint8_t  Resource_Amount; /// 4 bits used, so 0 .. 15.

typedef uint16_t Vision;

typedef int32_t Time; // FIXME should be unsigned
inline Time Never() throw () {return 0xffffffff;}

typedef uint32_t Duration;
inline Duration Forever() throw () {return 0xffffffff;}

/// Index for ware (and worker) types. Boxed for type-safety. Has a special
/// null value to indicate invalidity. Has operator bool so that an index can
/// be tested for validity with code like "if (index) ...". Operator bool
/// asserts that the index is not null.
struct Ware_Index {
	typedef uint8_t value_t;
	Ware_Index(Ware_Index const & other = Null()) : i(other.i) {}
	Ware_Index(value_t const I) : i(I) {}

	/// For compatibility with old code that use int32_t for ware/worker index
	/// and use -1 to indicate invalidity.
	Ware_Index(int32_t const I) __attribute__((deprecated))
		:
		i
		(I == -1 ?
		 std::numeric_limits<value_t>::max() : static_cast<value_t>(I))
	{}

	/// Returns a special value indicating invalidity.
	static Ware_Index Null() {
		return Ware_Index(std::numeric_limits<value_t>::max());
	}

	///  Get a value for array subscripting.
	value_t value() const {assert(*this); return i;}

	bool operator==(Ware_Index const other) const {return i == other.i;}
	bool operator!=(Ware_Index const other) const {return i != other.i;}

	operator bool() const throw () {return operator!=(Null());}

	// DO NOT REMOVE THE DECLARATION OF operator int32_t
	// (Note: the function body may eventually be removed)
	// Rationale: If only operator bool() is present, the compiler may
	// choose to use it in an implied cast when a user of this class
	// forgets to use value() in order to obtain a value_t. As long as
	// the declaration of operator int32_t is present, the compile will
	// fail with an ambiguous operator overload error instead of
	// producing erroneous code.
	operator int32_t() const __attribute__((deprecated)) {return *this ? i : -1;}

private:
	value_t i;
};

struct Building_Index {
	typedef uint8_t value_t;
	Building_Index(Building_Index const & other = Null()) : i(other.i) {}
	Building_Index(value_t const I) : i(I) {}

	/// For compatibility with old code that use int32_t for building index
	/// and use -1 to indicate invalidity.
	Building_Index(int32_t const I) __attribute__((deprecated))
		:
		i
		(I == -1 ?
		 std::numeric_limits<value_t>::max() : static_cast<value_t>(I))
	{}

	/// Returns a special value indicating invalidity.
	static Building_Index Null() {
		return Building_Index(std::numeric_limits<value_t>::max());
	}

	///  Get a value for array subscripting.
	value_t value() const {assert(*this); return i;}

	bool operator==(Building_Index const other) const {return i == other.i;}
	bool operator!=(Building_Index const other) const {return i != other.i;}

	operator bool() const throw () {return operator!=(Null());}

	// DO NOT REMOVE THE DECLARATION OF operator int32_t
	// (Note: the function body may eventually be removed)
	// Rationale: If only operator bool() is present, the compiler may
	// choose to use it in an implied cast when a user of this class
	// forgets to use value() in order to obtain a value_t. As long as
	// the declaration of operator int32_t is present, the compile will
	// fail with an ambiguous operator overload error instead of
	// producing erroneous code.
	operator int32_t() const __attribute__((deprecated)) {return *this ? i : -1;}

private:
	value_t i;
};

};

#endif
