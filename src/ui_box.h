/*
 * Copyright (C) 2003 by the Widelands Development Team
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
// you MUST include ui.h before including this

#ifndef included_ui_box_h
#define included_ui_box_h


/*
class Box
---------
A horizontal bar that holds a number of child panels.
The Panels you add to the Box must be children of the Box.
*/
class Box : public Panel {
public:
	Box(Panel* parent, int x, int y);

	void resize();
	
	int get_nrpanels() const { return m_panels.size(); }
	
	uint add(Panel* btn);
	
private:
	std::vector<Panel*>	m_panels;
};


#endif // included_ui_box_h
