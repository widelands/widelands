/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include <stdint.h>

namespace Widelands {

//  Type definitions for the game logic.

typedef uint8_t Tribe_Index;

typedef uint32_t Military_Influence; //  FIXME should be uint16_t

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

};

#endif
