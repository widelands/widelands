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

#include "geometry.h"
#include "ui_panel.h"
#include "ui_signal.h"

class Interactive_Base;

/**
 * Implements a view of a map. It is used to render a valid map on the screen.
 */
class Map_View : public UIPanel {
public:
	Map_View(UIPanel *parent, int x, int y, uint w, uint h, Interactive_Base *);

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
	bool handle_mouseclick(const Uint8 btn, const bool down, int x, int y);
	void handle_mousemove(int x, int y, int xdiff, int ydiff);

	void track_fsel(int mx, int my);
   inline void need_complete_redraw( void ) { m_complete_redraw_needed = true; }

private:
   Interactive_Base*	m_intbase;
	Point					m_viewpoint;
	bool					m_dragging;
   bool              m_complete_redraw_needed;
};


#endif /* __S__MAPVIEW_H */
