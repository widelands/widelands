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

#ifndef EDITOR_NOISE_HEIGHT_TOOL_H
#define EDITOR_NOISE_HEIGHT_TOOL_H

#include "editor/tools/editor_set_height_tool.h"

/// Set the height of a node to a random value within a defined interval.
struct Editor_Noise_Height_Tool : public Editor_Tool {
	Editor_Noise_Height_Tool
	(Editor_Set_Height_Tool & the_set_tool,
	 const interval<Widelands::Field::Height> the_interval =
	     interval<Widelands::Field::Height>(10, 14))
		:
		Editor_Tool(the_set_tool, the_set_tool),
		m_set_tool(the_set_tool),
		m_interval(the_interval)
	{}

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	int32_t handle_undo_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	Editor_Action_Args format_args_impl(Editor_Interactive & parent) override;

	char const * get_sel_impl() const override {
		return "pics/fsel_editor_noise_height.png";
	}

	interval<Widelands::Field::Height> get_interval() const {
		return m_interval;
	}
	void set_interval(interval<Widelands::Field::Height> const i) {
		m_interval = i;
	}

	Editor_Set_Height_Tool & set_tool() const {return m_set_tool;}

private:
	Editor_Set_Height_Tool & m_set_tool;
	interval<Widelands::Field::Height> m_interval;
};

#endif
