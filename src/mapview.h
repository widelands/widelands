/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#ifndef __S__MAPVIEW_H
#define __S__MAPVIEW_H

#include "graphic.h"
#include "ui.h"

class Interactive_Base;
class Game;
class Map;

/* class Map_View
 *
 * this implements a view of a map. it's used
 * to render a valid map on the screen
 *
 * Depends: class Map
 * 			g_gr
 */
class Map_View : public Panel {
public:
	Map_View(Panel *parent, int x, int y, uint w, uint h, Interactive_Base *);
	~Map_View();

	UISignal2<int,int> warpview; // x/y in screen coordinates
	UISignal fieldclicked;

	void warp_mouse_to_field(Coords c);

	// Function to set the viewpoint
	void set_viewpoint(Point vp);
	void set_rel_viewpoint(Point r) { set_viewpoint(m_viewpoint + r); }

	Point get_viewpoint() const { return m_viewpoint; }
	bool is_dragging() const { return m_dragging; }

	// Drawing
	void draw(RenderTarget* dst);

	// Event handling
	bool handle_mouseclick(uint btn, bool down, int x, int y);
	void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);

	void track_fsel(int mx, int my);

private:
   Interactive_Base*	m_intbase;
	Point					m_viewpoint;
	bool					m_dragging;
};


#endif /* __S__MAPVIEW_H */
