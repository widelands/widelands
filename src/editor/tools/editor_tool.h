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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef EDITOR_TOOL_H
#define EDITOR_TOOL_H

#define MAX_TOOL_AREA 9

#include <boost/noncopyable.hpp>

#include "editor/tools/editor_action_args.h"
#include "logic/widelands_geometry.h"

struct Editor_Interactive;
namespace Widelands {class Map;}

/**
 * An editor tool is a tool that can be selected in the editor. Examples are:
 * modify height, place bob, place critter, place building. A Tool only makes
 * one function (like delete_building, place building, modify building are 3
 * tools).
 */
class Editor_Tool : boost::noncopyable {
public:
	Editor_Tool(Editor_Tool & second, Editor_Tool & third, bool uda = true) :
		m_second(second), m_third(third), undoable(uda)
	{}
	virtual ~Editor_Tool() {}

	enum Tool_Index {First, Second, Third};
	int32_t handle_click
		(const Tool_Index i,
		Widelands::Map & map, Widelands::Node_and_Triangle<> const center,
		Editor_Interactive & parent, Editor_Action_Args & args)
	{
		return
		    (i == First ? *this : i == Second ? m_second : m_third)
		    .handle_click_impl(map, center, parent, args);
	}

	int32_t handle_undo
		(const Tool_Index i,
		Widelands::Map & map, Widelands::Node_and_Triangle<> const center,
		Editor_Interactive & parent, Editor_Action_Args & args)
	{
		return
		    (i == First ? *this : i == Second ? m_second : m_third)
		    .handle_undo_impl(map, center, parent, args);
	}

	const char * get_sel(const Tool_Index i) {
		return
		    (i == First ? *this : i == Second ? m_second : m_third)
		    .get_sel_impl();
	}

	Editor_Action_Args format_args(const Tool_Index i, Editor_Interactive & parent) {
		return
		    (i == First ? *this : i == Second ? m_second : m_third)
		    .format_args_impl(parent);
	}

	bool is_unduable() {return undoable;}
	virtual bool has_size_one() const {return false;}
	virtual Editor_Action_Args format_args_impl(Editor_Interactive & parent) {
		return Editor_Action_Args(parent);
	}
	virtual int32_t handle_click_impl
	(Widelands::Map &, Widelands::Node_and_Triangle<>, Editor_Interactive &, Editor_Action_Args &)
	= 0;
	virtual int32_t handle_undo_impl
	(Widelands::Map &, Widelands::Node_and_Triangle<>, Editor_Interactive &, Editor_Action_Args &)
	{return 0;} // non unduable tools don't need to implement this.
	virtual const char * get_sel_impl() const = 0;
	virtual bool operates_on_triangles() const {return false;};

protected:
	Editor_Tool & m_second, & m_third;
	bool undoable;
};

#endif
