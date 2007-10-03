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

#ifndef __S__EDITOR_DELETE_IMMOVABLE_TOOL
#define __S__EDITOR_DELETE_IMMOVABLE_TOOL

#include "editor_tool.h"

/// Deletes immovables from the map.
struct Editor_Delete_Immovable_Tool : public Editor_Tool {
	Editor_Delete_Immovable_Tool() : Editor_Tool(*this, *this) {}

	int32_t handle_click_impl(Map &, const Node_and_Triangle<>, Editor_Interactive &);
	const char * get_sel_impl() const throw ()
	{return "pics/fsel_editor_delete.png";}
};

#endif
