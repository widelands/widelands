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

#ifndef __S__EDITOR_NOISE_HEIGHT_TOOL_H
#define __S__EDITOR_NOISE_HEIGHT_TOOL_H

#include "editor_set_height_tool.h"

//  Set the height of a node to a random value within a defined interval.
struct Editor_Noise_Height_Tool : public Editor_Tool {
	Editor_Noise_Height_Tool
		(Editor_Set_Height_Tool & the_set_tool,
		 Uint8 lower = 10, Uint8 upper = 14)
		:
		Editor_Tool(the_set_tool, the_set_tool),
		m_set_tool(the_set_tool),
		m_lower_value(lower), m_upper_value(upper)
	{}

	int handle_click_impl(Map &, const Node_and_Triangle, Editor_Interactive &);
	const char * get_sel_impl() const throw ()
	{return "pics/fsel_editor_noise_height.png";}

	void get_values(Uint8 & lower, Uint8 & upper) const throw ()
	{lower = m_lower_value; upper = m_upper_value;}
	void set_values(const Uint8 lower, const Uint8 upper) throw ()
	{m_lower_value = lower; m_upper_value = upper;}

	Editor_Set_Height_Tool & set_tool() const throw () {return m_set_tool;}

private:
	Editor_Set_Height_Tool & m_set_tool;
	Uint8 m_lower_value, m_upper_value;
};

#endif
