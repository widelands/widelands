/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__SET_TERRAIN
#define __S__SET_TERRAIN

#include "multi_select.h"

class FCoords;
class Field;
class Map;
class Editor_Interactive;

/*
=============================
class Set_Terrain

This is the base class for the terrain set tools.
If offers the functionality to enable/disable terrains
and to set the texture effectivly, but it isn't a tool for 
itself
=============================
*/
class Set_Terrain : public MultiSelect {
   public:
      Set_Terrain() { }
      ~Set_Terrain() { }

   protected:
      int set_terrain(FCoords& fc, Map*, Editor_Interactive*, bool right, bool down);
};

#endif
