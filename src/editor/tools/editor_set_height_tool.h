/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EDITOR_SET_HEIGHT_TOOL_H
#define __S__EDITOR_SET_HEIGHT_TOOL_H

#include "editor_tool.h"

///  Decreases the height of a node by a value.
struct Editor_Set_Height_Tool : public Editor_Tool {
	Editor_Set_Height_Tool() : Editor_Tool(*this, *this), m_set_to(10) {}

	int handle_click_impl(Map &, const Node_and_Triangle, Editor_Interactive &);
	const char * get_sel_impl() const throw ()
	{return "pics/fsel_editor_set_height.png";}

	Uint8 get_set_to() const throw () {return m_set_to;}
	void set_set_to(const Uint8 n) throw () {m_set_to = n;}

private:
	Uint8 m_set_to;
};

#endif
