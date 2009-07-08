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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef EDITOR_SET_RESOURCES_TOOL_H
#define EDITOR_SET_RESOURCES_TOOL_H

#include "editor_tool.h"
#include "world.h"

///  Decreases the resources of a node by a value.
struct Editor_Set_Resources_Tool : public Editor_Tool {
	Editor_Set_Resources_Tool()
		: Editor_Tool(*this, *this), m_cur_res(0), m_set_to(0)
	{}

	int32_t handle_click_impl
		(Widelands::Map &, Widelands::Node_and_Triangle<>, Editor_Interactive &);
	char const * get_sel_impl() const {
		return "pics/fsel_editor_set_resources.png";
	}

	uint8_t get_set_to() const throw () {return m_set_to;}
	void set_set_to(uint8_t const n) throw () {m_set_to = n;}
	Widelands::Resource_Index get_cur_res() const throw () {return m_cur_res;}
	void set_cur_res(Widelands::Resource_Index const res) throw () {
		m_cur_res = res;
	}

private:
	Widelands::Resource_Index m_cur_res;
	uint8_t               m_set_to;
};

#endif
