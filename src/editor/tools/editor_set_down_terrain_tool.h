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

#ifndef __S__EDITOR_SET_DOWN_TERRAIN_TOOL
#define __S__EDITOR_SET_DOWN_TERRAIN_TOOL

#include "editor_tool.h"
#include "set_terrain.h"

/*
=============================
class Editor_Set_Down_Terrain_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Down_Terrain_Tool : public Editor_Tool, public Set_Terrain {
   public:
      Editor_Set_Down_Terrain_Tool() : Editor_Tool(this,this) { }
      virtual ~Editor_Set_Down_Terrain_Tool() { }

      virtual int handle_click_impl(FCoords& fc, Map* m, Editor_Interactive* e) {
         return set_terrain(fc,m,e,false,true);
      }

      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_terrain_down.png"; }
};

#endif // __S__EDITOR_SET_DOWN_TERRAIN_TOOL
