/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_WUI_MAPVIEW_H
#define WL_WUI_MAPVIEW_H

#include <memory>

#include <boost/function.hpp>
#include <boost/signals2.hpp>

#include "base/transform.h"
#include "logic/widelands_geometry.h"
#include "ui_basic/panel.h"

class GameRenderer;
class InteractiveBase;

/**
 * Implements a view of a map. It is used to render a valid map on the screen.
 */
struct MapView : public UI::Panel {
	/**
	 * Callback function type for when the view position changes.
	 *
	 * Parameters are x/y screen coordinates and whether the change should
	 * be considered a "jump" or a smooth scrolling event.
	 */
	using ChangeViewFn = boost::function<void(Point, bool)>;

	MapView(UI::Panel* const parent,
	        const int32_t x,
	        const int32_t y,
	        const uint32_t w,
	        const uint32_t h,
	        InteractiveBase&);
	virtual ~MapView();

	void set_changeview(const ChangeViewFn& fn);

	/**
	 * Called whenever the view position changes, for whatever reason.
	 *
	 * Parameters are x/y position in screen coordinates.
	 */
	boost::signals2::signal<void(int32_t, int32_t)> changeview;

	boost::signals2::signal<void()> fieldclicked;

	void warp_mouse_to_node(Widelands::Coords);

	void set_viewpoint(Point vp, bool jump);
	void set_rel_viewpoint(Point r, bool jump);

	Point get_viewpoint() const;
	bool is_dragging() const {
		return dragging_;
	}

	// Drawing
	void draw(RenderTarget&) override;

	// Event handling
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool
	handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;

	void track_sel(Point m);

protected:
	InteractiveBase& intbase() const {
		return intbase_;
	}

private:
	void stop_dragging();

	std::unique_ptr<GameRenderer> renderer_;
	InteractiveBase& intbase_;
	ChangeViewFn changeview_;
	Transform2f panel_to_mappixel_;
	Point last_mouse_pos_;
	bool dragging_;
};

#endif  // end of include guard: WL_WUI_MAPVIEW_H
