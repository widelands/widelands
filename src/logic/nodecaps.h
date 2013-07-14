/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef NODECAPS_H
#define NODECAPS_H

namespace Widelands {

enum NodeCaps {
	CAPS_NONE = 0,
	/** can we build normal buildings? (use BUILDCAPS_SIZEMASK for binary
	 * masking)*/
	BUILDCAPS_SMALL = 1,
	BUILDCAPS_MEDIUM = 2,
	BUILDCAPS_BIG = 3,
	BUILDCAPS_SIZEMASK = 3,

	/** can we build a flag on this field?*/
	BUILDCAPS_FLAG = 4,

	/** can we build a mine on this field (completely independent from build
	 * size!)*/
	BUILDCAPS_MINE = 8,

	/** (only if BUILDCAPS_BIG): can we build a port on this field? This gets set
	 * for BUILDCAPS_BIG fields that have a Map::is_port_space() as well as a
	 * swimmable second-order neighbour
	 */
	BUILDCAPS_PORT = 16,

	/** can we build any building on this field?*/
	BUILDCAPS_BUILDINGMASK = BUILDCAPS_SIZEMASK|BUILDCAPS_MINE|BUILDCAPS_PORT,

	/// Can Map_Objects walk or swim here? Also used for
	/// Map_Object_Descr::movecaps. If MOVECAPS_WALK, any walking being can walk
	/// to this field.
	MOVECAPS_WALK = 32,

	/// If MOVECAPS_SWIM, any swimming being (including ships) can go there.
	/// Additionally, swimming beings can temporarily visit nodes that are
	/// walkable but not swimmable if those nodes are at the start or end of
	/// their path. Without this clause, ports would be kind of impossible ;)
	/// This clause stops ducks from "swimwalking" along the coast.
	MOVECAPS_SWIM = 64,
};

}

#endif
