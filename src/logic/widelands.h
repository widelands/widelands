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

#ifndef WL_LOGIC_WIDELANDS_H
#define WL_LOGIC_WIDELANDS_H

#include <cassert>
#include <cstddef>
#include <limits>

#include <stdint.h>

#include "base/macros.h"

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

typedef uint8_t  Terrain_Index;
typedef uint8_t  Resource_Index;  /// 4 bits used, so 0 .. 15.
typedef uint8_t  Resource_Amount; /// 4 bits used, so 0 .. 15.

typedef uint16_t Vision;

typedef int32_t Time; // TODO(unknown): should be unsigned
inline Time Never() {return 0xffffffff;}

typedef uint32_t Duration;
inline Duration Forever() {return 0xffffffff;}

typedef uint32_t Serial; /// Serial number for Map_Object.

constexpr uint8_t INVALID_INDEX = std::numeric_limits<uint8_t>::max();
typedef uint8_t Ware_Index;
typedef uint8_t Building_Index;
typedef uint8_t Direction;

struct Soldier_Strength {
	uint8_t hp, attack, defense, evade;
	bool operator== (const Soldier_Strength & other) const {
		return
			hp      == other.hp      &&
			attack  == other.attack  &&
			defense == other.defense &&
			evade   == other.evade;
	}
	bool operator<  (const Soldier_Strength & other) const {
		return
			hp      <  other.hp ||
			(hp      == other.hp &&
			 (attack  <  other.attack ||
			  (attack  == other.attack &&
			   (defense <  other.defense ||
			    (defense == other.defense &&
			     evade    <  other.evade)))));
	}
};

} // namespace Widelands

#endif  // end of include guard: WL_LOGIC_WIDELANDS_H
