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

#ifndef __S__EDITOR_PLACE_IMMOVABLE_TOOL
#define __S__EDITOR_PLACE_IMMOVABLE_TOOL

#include "editor_tool.h"
#include "multi_select.h"
#include "editor_delete_immovable_tool.h"

/*
=============================
class Editor_Place_Immovable_Tool

this places immovables on the map
=============================
*/
class Editor_Place_Immovable_Tool : public Editor_Tool, public MultiSelect {
   public:
      Editor_Place_Immovable_Tool(Editor_Delete_Immovable_Tool* tool) : Editor_Tool(tool, tool) {
      }
      ~Editor_Place_Immovable_Tool() { }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_place_immovable.png"; }
};

#endif
