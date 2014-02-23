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

#ifndef EDITOR_SET_RESOURCES_TOOL_H
#define EDITOR_SET_RESOURCES_TOOL_H

#include "editor/tools/editor_tool.h"
#include "logic/world.h"

///  Decreases the resources of a node by a value.
struct Editor_Set_Resources_Tool : public Editor_Tool {
	Editor_Set_Resources_Tool()
		: Editor_Tool(*this, *this), m_cur_res(0), m_set_to(0)
	{}

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	int32_t handle_undo_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	Editor_Action_Args format_args_impl(Editor_Interactive & parent) override;

	char const * get_sel_impl() const override {
		return "pics/fsel_editor_set_resources.png";
	}

	uint8_t get_set_to() const       {return m_set_to;}
	void set_set_to(uint8_t const n) {m_set_to = n;}
	Widelands::Resource_Index get_cur_res() const {return m_cur_res;}
	void set_cur_res(Widelands::Resource_Index const res)
	{m_cur_res = res;}

private:
	Widelands::Resource_Index m_cur_res;
	uint8_t                   m_set_to;
};

#endif
