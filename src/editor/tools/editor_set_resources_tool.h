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

#ifndef WL_EDITOR_TOOLS_EDITOR_SET_RESOURCES_TOOL_H
#define WL_EDITOR_TOOLS_EDITOR_SET_RESOURCES_TOOL_H

#include "logic/mapregion.h"
#include "logic/widelands.h"
#include "editor/tools/editor_tool.h"

///  Decreases the resources of a node by a value.
struct EditorSetResourcesTool : public EditorTool {
	EditorSetResourcesTool()
		: EditorTool(*this, *this), m_cur_res(0), m_set_to(0)
	{}

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

	static void set_res_and_overlay(Widelands::Map& map, const Widelands::World& world, EditorActionArgs& args,
			Widelands::MapRegion<Widelands::Area<Widelands::FCoords> >& mr, int32_t amount);

	EditorActionArgs format_args_impl(EditorInteractive & parent) override;

	char const * get_sel_impl() const override {
		return "pics/fsel_editor_set_resources.png";
	}

	uint8_t get_set_to() const       {return m_set_to;}
	void set_set_to(uint8_t const n) {m_set_to = n;}
	Widelands::DescriptionIndex get_cur_res() const {return m_cur_res;}
	void set_cur_res(Widelands::DescriptionIndex const res)
	{m_cur_res = res;}

private:
	Widelands::DescriptionIndex m_cur_res;
	uint8_t m_set_to;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_EDITOR_SET_RESOURCES_TOOL_H
