/*
 * Copyright (C) 2002-2004, 2006-2012 by the Widelands Development Team
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

#ifndef EDITOR_SET_PORT_SPACE_TOOL_H
#define EDITOR_SET_PORT_SPACE_TOOL_H

#include "editor/tools/editor_tool.h"
#include "logic/widelands.h"

#define FSEL_ESPS_FILENAME "pics/fsel_editor_set_port_space.png"
#define FSEL_EUPS_FILENAME "pics/fsel_editor_unset_port_space.png"

/// Unsets a buildspace for ports.
class Editor_Unset_Port_Space_Tool : public Editor_Tool {
public:
	Editor_Unset_Port_Space_Tool();

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	int32_t handle_undo_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	char const * get_sel_impl() const override {return FSEL_EUPS_FILENAME;}
};


/// Sets a buildspace for ports.
class Editor_Set_Port_Space_Tool : public Editor_Tool {
public:
	Editor_Set_Port_Space_Tool(Editor_Unset_Port_Space_Tool &);

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	int32_t handle_undo_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	char const * get_sel_impl() const override {return FSEL_ESPS_FILENAME;}
};

int32_t Editor_Tool_Set_Port_Space_Callback
	(const Widelands::TCoords<Widelands::FCoords>& c, Widelands::Map& map);

#endif
