/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_EDITOR_INCREASE_RESOURCES_TOOL_H
#define WL_EDITOR_TOOLS_EDITOR_INCREASE_RESOURCES_TOOL_H

#include "editor/tools/editor_decrease_resources_tool.h"
#include "editor/tools/editor_set_resources_tool.h"
#include "logic/widelands_geometry.h"

/// Increases the resources of a node by a value.
struct EditorIncreaseResourcesTool : public EditorTool {
	EditorIncreaseResourcesTool(EditorDecreaseResourcesTool& the_decrease_tool,
											 EditorSetResourcesTool& the_set_to_tool)
	   : EditorTool(the_decrease_tool, the_set_to_tool),
	     m_decrease_tool(the_decrease_tool),
	     m_set_tool(the_set_to_tool),
	     m_change_by(1),
	     m_cur_res(0) {
	}

	int32_t handle_click_impl(Widelands::Map& map,
	                          const Widelands::World& world,
	                          Widelands::NodeAndTriangle<> center,
	                          EditorInteractive& parent,
	                          EditorActionArgs& args) override;

	int32_t handle_undo_impl(Widelands::Map& map,
	                         const Widelands::World& world,
	                         Widelands::NodeAndTriangle<> center,
	                         EditorInteractive& parent,
	                         EditorActionArgs& args) override;

	EditorActionArgs format_args_impl(EditorInteractive & parent) override;

	char const * get_sel_impl() const override {
		return "pics/fsel_editor_increase_resources.png";
	}

	int32_t get_change_by() const        {return m_change_by;}
	void set_change_by(const int32_t n)  {m_change_by = n;}
	Widelands::DescriptionIndex get_cur_res() const {return m_cur_res;}
	void set_cur_res(Widelands::DescriptionIndex const res) {
		m_cur_res = res;
	}

	EditorDecreaseResourcesTool & decrease_tool() const {
		return m_decrease_tool;
	}
	EditorSetResourcesTool    &   set_tool() const {return m_set_tool;}

private:
	EditorDecreaseResourcesTool & m_decrease_tool;
	EditorSetResourcesTool& m_set_tool;
	int32_t m_change_by;
	Widelands::DescriptionIndex m_cur_res;
};

int32_t editor_change_resource_tool_callback
	(const Widelands::TCoords<Widelands::FCoords>& c, Widelands::Map& map,
	 const Widelands::World& world, int32_t const curres);

#endif  // end of include guard: WL_EDITOR_TOOLS_EDITOR_INCREASE_RESOURCES_TOOL_H
