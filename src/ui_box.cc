/*
 * Copyright (C) 2003 by Widelands Development Team
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

#include "widelands.h"
#include "ui.h"
#include "ui_box.h"


/*
===============
Box::Box

Initialize an empty box
===============
*/
Box::Box(Panel* parent, int x, int y)
	: Panel(parent, x, y, 0, 0)
{
}


/*
===============
Box::resize

Adjust all the children and the box's size.
===============
*/
void Box::resize()
{
	int maxy = 0;
	int x = 0;
	
	for(uint idx = 0; idx < m_panels.size(); idx++)
		{
		m_panels[idx]->set_pos(x, 0);
		
		x += m_panels[idx]->get_w();
		if (m_panels[idx]->get_h() > maxy)
			maxy = m_panels[idx]->get_h();
		}
	
	set_size(x, maxy);
}


/*
===============
Box::add

Add a new panel to be controlled by this box
===============
*/
uint Box::add(Panel* btn)
{
	m_panels.push_back(btn);
	
	resize();
	
	return m_panels.size()-1;
}
