/*
 * Copyright (C) 2007-2008, 2010-2011 by the Widelands Development Team
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

#ifndef WIDELANDS_H
#define WIDELANDS_H

#include <cassert>
#include <cstddef>
#include <limits>

#include <stdint.h>

#include "port.h"

namespace Widelands {

//  Type definitions for the game logic.

typedef uint8_t Tribe_Index;

typedef uint16_t Military_Influence;

typedef uint8_t  Player_Number; /// 5 bits used, so 0 .. 31
inline Player_Number Neutral() {return 0;}
#define iterate_player_numbers(p, nr_players) \
   for (Widelands::Player_Number p = 1; p < nr_players + 1; ++p)

/**
 * Every player has a team number. Team number 0 is special,
 * indicating that the player is not part of any team.
 *
 * Players having the same positive team number are allied.
 */
typedef uint8_t TeamNumber;

typedef uint8_t  Terrain_Index;   /// 4 bits used, so 0 .. 15.
typedef uint8_t  Resource_Index;  /// 4 bits used, so 0 .. 15.
typedef uint8_t  Resource_Amount; /// 4 bits used, so 0 .. 15.

typedef uint16_t Vision;

typedef int32_t Time; // FIXME should be unsigned
inline Time Never() {return 0xffffffff;}

typedef uint32_t Duration;
inline Duration Forever() {return 0xffffffff;}

typedef uint32_t Serial; /// Serial number for Map_Object.

/// Index for ware (and worker), building and other game object types.
/// Boxed for type-safety. Has a special null value to indicate invalidity.
/// Has operator bool so that an index can be tested for validity with code
/// like "if (index) ...". Operator bool asserts that the index is not null.
/// The null value is guaranteed to be greater than any valid value. Therefore
/// validity and upper limit can be tested using "if (index < nrItems)".
template <typename T> struct Index_ {
	typedef uint8_t value_t;
	Index_(const Index_ & other = Null()) : i(other.i) {}
	explicit Index_(value_t const I) : i(I) {}
	explicit Index_(size_t  const I)
		: i(static_cast<value_t>(I))
	{
		assert(I < std::numeric_limits<value_t>::max());
	}

	/// For compatibility with old code that use int32_t for building index
	/// and use -1 to indicate invalidity.

	static T First() {return T(static_cast<value_t>(0));}

	/// Returns a special value indicating invalidity.
	static T Null() {return T(std::numeric_limits<value_t>::max());}

	///  Get a value for array subscripting.
	value_t value() const {assert(*this); return i;}

	bool operator== (const Index_& other) const {return i == other.i;}
	bool operator!= (const Index_& other) const {return i != other.i;}
	bool operator<  (const Index_& other) const {return i <  other.i;}
	bool operator<= (const Index_& other) const {return i <=  other.i;}

	T operator++ () {return T(++i);}
	T operator-- () {return T(--i);}

	operator bool() const {return operator!= (Null());}

	/// Implicit conversion to size_t type for array indexing.
	operator size_t() const {return static_cast<size_t>(i);}

	// DO NOT REMOVE THE DECLARATION OF operator int32_t
	// Rationale: If only operator bool() is present, the compiler may
	// choose to use it in an implied cast when a user of this class
	// forgets to use value() in order to obtain a value_t. As long as
	// the declaration of operator int32_t is present, the compile will
	// fail with an ambiguous operator overload error instead of
	// producing erroneous code.
	operator int32_t() const __attribute__((deprecated));

private:
	value_t i;
};

#define DEFINE_INDEX(NAME)                                                    \
   struct NAME : public Index_<NAME> {                                        \
      NAME(const NAME & other = Null()) : Index_<NAME>(other) {}              \
      explicit NAME(value_t const I) : Index_<NAME>(I) {}                     \
      explicit NAME(size_t  const I) : Index_<NAME>(I) {}                     \
      explicit NAME(int32_t const I) __attribute__((deprecated));             \
   };                                                                         \

DEFINE_INDEX(Building_Index)
DEFINE_INDEX(Ware_Index)

typedef uint8_t Direction;

struct Soldier_Strength {
	uint8_t hp, attack, defense, evade;
	bool operator== (const Soldier_Strength & other) const {
		return
			hp      == other.hp      and
			attack  == other.attack  and
			defense == other.defense and
			evade   == other.evade;
	}
	bool operator<  (const Soldier_Strength & other) const {
		return
			hp      <  other.hp or
			(hp      == other.hp and
			 (attack  <  other.attack or
			  (attack  == other.attack and
			   (defense <  other.defense or
			    (defense == other.defense and
			     evade    <  other.evade)))));
	}
};

} // namespace Widelands

#endif
