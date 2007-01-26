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

#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_set_height_tool.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"

/// Sets the height to a value, Changes surrounding nodes if necessary.
int Editor_Set_Height_Tool::handle_click_impl
(Map & map, const Node_and_Triangle center, Editor_Interactive & parent)
{
	const uint radius = parent.get_sel_radius();
	return radius ?
		map.set_height(Area(center.node, radius), m_set_to)
		:
		map.set_height
		(FCoords(center.node, map.get_field(center.node)), m_set_to);
}
