/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

// define current version
#define VERSION "build-6"

#define TEXTURE_W 64        // texture have a fixed size and are squares
#define TEXTURE_H TEXTURE_W // TEXTURE_H is just defined for easier understandement of the code

/*
================================================================================

      COMPILE TIME CONFIGURATION

================================================================================
*/

// the actual game logic doesn't know about frames (it works with millisecond-precise
// timing)
// FRAME_LENGTH is just the default animation speed
#define FRAME_LENGTH 250

// Maxi numbers of players in a game
#define MAX_PLAYERS		8

#endif
