/*
 * Copyright (C) 2002-2004 by the Wide Lands Development Team
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

#ifndef MAPRENDERINFO_H
#define MAPRENDERINFO_H

#include <vector>
#include "geometry.h"
#include "types.h"

class Editor_Game_Base;

/*
==============================================================================

MapRenderInfo

==============================================================================
*/

/*
struct MapRenderInfo

This structure contains all the information that is needed by the renderer.

It includes the map itself as well as overlay data (build symbols, road
building symbols, ...)
*/
enum {
	Minimap_Terrain = 1,			// display terrain
	Minimap_PlayerColor = 2,	// color of owner
	Minimap_Flags = 4,			// show flags
	Minimap_Roads = 8,			// show roads
	Minimap_Buildings = 16,		// show buildings
};

struct MapRenderInfo {
	Editor_Game_Base*		egbase;
	
	std::vector<bool>*		visibility; // array of fields, true if the field can be seen
								// can be 0, in which case the whole map is visible
};

#endif
