/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EDITOR_SET_TERRAIN_TOOL
#define __S__EDITOR_SET_TERRAIN_TOOL

#include "editor_tool.h"
#include "multi_select.h"

struct Editor_Set_Terrain_Tool : public Editor_Tool, public MultiSelect {
	Editor_Set_Terrain_Tool() : Editor_Tool(*this, *this) {}

	int32_t handle_click_impl
		(Widelands::Map &, Widelands::Node_and_Triangle<>, Editor_Interactive &);
	const char * get_sel_impl() const throw () {return "pics/fsel.png";}
	bool operates_on_triangles() const throw () {return true;};
};

#endif // __S__EDITOR_SET_TERRAIN_TOOL
