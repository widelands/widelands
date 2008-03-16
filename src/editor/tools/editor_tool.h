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

#ifndef EDITOR_TOOL_H
#define EDITOR_TOOL_H

#define MAX_TOOL_AREA 9

#include "widelands_geometry.h"

struct Editor_Interactive;
namespace Widelands {struct Map;};

/**
 * An editor tool is a tool that can be selected in the editor. Examples are:
 * modify height, place bob, place critter, place building. A Tool only makes
 * one function (like delete_building, place building, modify building are 3
 * tools).
 */
struct Editor_Tool {
	Editor_Tool(Editor_Tool & second, Editor_Tool & third) :
	m_second(second), m_third(third)
	{}
	virtual ~Editor_Tool() {}

	enum Tool_Index {First, Second, Third};
	int32_t handle_click
		(const Tool_Index i,
		 Widelands::Map & map, Widelands::Node_and_Triangle<> const center,
		 Editor_Interactive & parent)
	{
		return
			(i == First ? *this : i == Second ? m_second : m_third)
			.handle_click_impl(map, center, parent);
	}
	const char * get_sel(const Tool_Index i) {
		return
			(i == First ? *this : i == Second ? m_second : m_third)
			.get_sel_impl();
	}

	virtual int32_t handle_click_impl
		(Widelands::Map &, Widelands::Node_and_Triangle<>, Editor_Interactive &)
		= 0;
	virtual const char * get_sel_impl() const throw () = 0;
	virtual bool operates_on_triangles() const {return false;};

protected:
	Editor_Tool & m_second, & m_third;
};

#endif
