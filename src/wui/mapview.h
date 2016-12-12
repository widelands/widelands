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

#include "base/rect.h"
#include "base/vector.h"
#include "logic/widelands_geometry.h"
#include "ui_basic/panel.h"

class GameRenderer;
class InteractiveBase;

/**
 * Implements a view of a map. It is used to render a valid map on the screen.
 */
struct MapView : public UI::Panel {
	MapView(UI::Panel* const parent,
	        const int32_t x,
	        const int32_t y,
	        const uint32_t w,
	        const uint32_t h,
	        InteractiveBase&);
	virtual ~MapView();

	/**
	 * Called when the view changed.  'jump' defines if the change should be
	 * considered a "jump" or a smooth scrolling event.
	 */
	boost::signals2::signal<void(bool jump)> changeview;

	boost::signals2::signal<void()> fieldclicked;

	void warp_mouse_to_node(Widelands::Coords);

	void set_viewpoint(const Vector2f& vp, bool jump);
	void center_view_on_coords(const Widelands::Coords& coords);
	void center_view_on_map_pixel(const Vector2f& pos);

	Vector2f get_viewpoint() const;
	Rectf get_view_area() const;
	float get_zoom() const;

	// Set the zoom to the new value without changing view_point. For the user
	// the view will perceivably jump.
	void set_zoom(float zoom);

	// Set the zoom to the 'new_zoom'. This keeps the map_pixel that is
	// displayed at 'panel_pixel' unchanging, i.e. the center of the zoom.
	void zoom_around(float new_zoom, const Vector2f& panel_pixel);

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
	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_Keysym code) override;

	void track_sel(const Vector2f& m);

protected:
	InteractiveBase& intbase() const {
		return intbase_;
	}

	// Move the view by 'delta_pixels'.
	void pan_by(Vector2i delta_pixels);

private:
	void stop_dragging();

	Vector2f to_panel(const Vector2f& map_pixel) const;
	Vector2f to_map(const Vector2f& panel_pixel) const;

	std::unique_ptr<GameRenderer> renderer_;
	InteractiveBase& intbase_;
	Vector2f viewpoint_;
	float zoom_;
	Vector2i last_mouse_pos_;
	bool dragging_;
};

#endif  // end of include guard: WL_WUI_MAPVIEW_H
