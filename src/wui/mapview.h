/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <boost/function.hpp>
#include <boost/signals2.hpp>

#include "logic/widelands_geometry.h"
#include "ui_basic/panel.h"

class GameRenderer;
struct Interactive_Base;

/**
 * Implements a view of a map. It is used to render a valid map on the screen.
 */
struct Map_View : public UI::Panel {
	/**
	 * Callback function type for when the view position changes.
	 *
	 * Parameters are x/y screen coordinates and whether the change should
	 * be considered a "jump" or a smooth scrolling event.
	 */
	typedef boost::function<void (Point, bool)> ChangeViewFn;

	Map_View
		(UI::Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 Interactive_Base &);
	virtual ~Map_View();

	void set_changeview(const ChangeViewFn & fn);

	/**
	 * Called whenever the view position changes, for whatever reason.
	 *
	 * Parameters are x/y position in screen coordinates.
	 */
	boost::signals2::signal<void (int32_t, int32_t)> changeview;

	boost::signals2::signal<void ()> fieldclicked;

	void warp_mouse_to_node(Widelands::Coords);

	void set_viewpoint(Point vp, bool jump);
	void set_rel_viewpoint(Point r, bool jump) {set_viewpoint(m_viewpoint + r, jump);}

	Point get_viewpoint() const {return m_viewpoint;}
	bool is_dragging() const {return m_dragging;}

	// Drawing
	void draw(RenderTarget &) override;

	// Event handling
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mousemove
		(Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;

	void track_sel(Point m);
	void need_complete_redraw() {m_complete_redraw_needed = true;}

protected:
	Interactive_Base & intbase() const {return m_intbase;}

private:
	void stop_dragging();

	std::unique_ptr<GameRenderer> m_renderer;
	Interactive_Base & m_intbase;
	ChangeViewFn m_changeview;
	Point              m_viewpoint;
	bool               m_dragging;
	bool               m_complete_redraw_needed;
};


#endif
