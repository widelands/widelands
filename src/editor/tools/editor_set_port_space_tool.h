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

#ifndef WL_EDITOR_TOOLS_EDITOR_SET_PORT_SPACE_TOOL_H
#define WL_EDITOR_TOOLS_EDITOR_SET_PORT_SPACE_TOOL_H

#include "editor/tools/editor_tool.h"
#include "logic/widelands.h"

#define FSEL_ESPS_FILENAME "pics/fsel_editor_set_port_space.png"
#define FSEL_EUPS_FILENAME "pics/fsel_editor_unset_port_space.png"

/// Unsets a buildspace for ports.
class EditorUnsetPortSpaceTool : public EditorTool {
public:
	EditorUnsetPortSpaceTool();

	int32_t handle_click_impl(const Widelands::World& world,
	                          Widelands::NodeAndTriangle<> center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
							  Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::World& world,
	                         Widelands::NodeAndTriangle<> center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
							 Widelands::Map* map) override;

	char const * get_sel_impl() const override {return FSEL_EUPS_FILENAME;}
};


/// Sets a buildspace for ports.
class EditorSetPortSpaceTool : public EditorTool {
public:
	EditorSetPortSpaceTool(EditorUnsetPortSpaceTool &);

	int32_t handle_click_impl(const Widelands::World& world,
	                          Widelands::NodeAndTriangle<> center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
							  Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::World& world,
	                         Widelands::NodeAndTriangle<> center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
							 Widelands::Map* map) override;

	char const * get_sel_impl() const override {return FSEL_ESPS_FILENAME;}
};

int32_t editor_Tool_set_port_space_callback
	(const Widelands::TCoords<Widelands::FCoords>& c, const Widelands::Map& map);

#endif  // end of include guard: WL_EDITOR_TOOLS_EDITOR_SET_PORT_SPACE_TOOL_H
