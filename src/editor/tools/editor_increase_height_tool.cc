/*
 * Copyright (C) 2002-2004, 2006-2008, 2012 by the Widelands Development Team
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

#include "editor/tools/editor_increase_height_tool.h"

#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/mapregion.h"

/// Increases the heights by a value. Chages surrounding nodes if necessary.
int32_t Editor_Increase_Height_Tool::handle_click_impl
	(Widelands::Map & map,
	Widelands::Node_and_Triangle<> center,
	Editor_Interactive & /* parent */,
	Editor_Action_Args & args)
{
	if (args.origHights.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords> > mr
		(map,
		 Widelands::Area<Widelands::FCoords>
		 (map.get_fcoords(center.node), args.sel_radius + MAX_FIELD_HEIGHT / MAX_FIELD_HEIGHT_DIFF + 1));
		do {
			args.origHights.push_back(mr.location().field->get_height());
		} while (mr.advance(map));
	}

	return
	    map.change_height
	    (Widelands::Area<Widelands::FCoords>
	     (map.get_fcoords(center.node), args.sel_radius),
	     args.change_by);
}

int32_t Editor_Increase_Height_Tool::handle_undo_impl
	(Widelands::Map & map,
	Widelands::Node_and_Triangle<> center,
	Editor_Interactive & parent,
	Editor_Action_Args & args)
{
	return m_decrease_tool.handle_undo_impl(map, center, parent, args);
}

Editor_Action_Args Editor_Increase_Height_Tool::format_args_impl(Editor_Interactive & parent)
{
	Editor_Action_Args a(parent);
	a.change_by = m_change_by;
	return a;
}
