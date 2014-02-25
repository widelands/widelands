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

#ifndef EDITOR_INFO_TOOL_H
#define EDITOR_INFO_TOOL_H

#include "editor/tools/editor_tool.h"

/// A simple tool to show information about the clicked node.
struct Editor_Info_Tool : public Editor_Tool {
	Editor_Info_Tool() : Editor_Tool(*this, *this, false) {}

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	char const * get_sel_impl() const override {return "pics/fsel_editor_info.png";}
};

#endif
