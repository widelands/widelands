/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "wui/mapview.h"

#include "base/macros.h"
#include "base/math.h"
#include "base/transform.h"
#include "graphic/game_renderer.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map.h"
#include "logic/player.h"
#include "wlapplication.h"
#include "wui/interactive_base.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelfunctions.h"

MapView::MapView(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, InteractiveBase& player)
   : UI::Panel(parent, x, y, w, h),
     renderer_(new GameRenderer()),
     intbase_(player),
     panel_to_mappixel_(Transform2f::identity()),
     dragging_(false) {
}

MapView::~MapView() {
}

Point MapView::get_viewpoint() const {
	return round(panel_to_mappixel_.translation());
}

/// Moves the mouse cursor so that it is directly above the given node
void MapView::warp_mouse_to_node(Widelands::Coords const c) {
	log("#sirver c.x: %d,c.y: %d\n", c.x, c.y);
	const Widelands::Map& map = intbase().egbase().map();
	FloatPoint in_mappixel = MapviewPixelFunctions::to_map_pixel_with_normalization(map, c);
	log("#sirver in_mappixel.x: %f,in_mappixel.y: %f\n", in_mappixel.x, in_mappixel.y);

	auto foo = panel_to_mappixel_.inverse();
	log("#sirver panel_to_mappixel: %f %f (%f)\n", foo.translation().x,
	    foo.translation().y, foo.zoom());
	const Point in_panel = round(panel_to_mappixel_.inverse().apply(in_mappixel));
	log("#sirver in_panel.x: %d,in_panel.y: %d\n", in_panel.x, in_panel.y);

	//  If the user has scrolled the node outside the viewable area, he most
	//  surely doesn't want to jump there.
	if (in_panel.x < 0 || in_panel.y < 0 || in_panel.x >= get_w() || in_panel.y >= get_h()) {
		return;
	}
	set_mouse_pos(in_panel);
	track_sel(in_panel);
}

void MapView::draw(RenderTarget& dst) {
	Widelands::EditorGameBase& egbase = intbase().egbase();

	if (upcast(Widelands::Game, game, &egbase)) {
		// Bail out if the game isn't actually loaded.
		// This fixes a crash with displaying an error dialog during loading.
		if (!game->is_loaded())
			return;
	}

	if (upcast(InteractivePlayer const, interactive_player, &intbase())) {
		renderer_->rendermap(egbase, panel_to_mappixel_, interactive_player->player(), &dst);
	} else {
		renderer_->rendermap(egbase, panel_to_mappixel_, &dst);
	}
}

void MapView::set_changeview(const MapView::ChangeViewFn& fn) {
	changeview_ = fn;
}

/*
===============
Set the viewpoint to the given pixel coordinates
===============
*/
void MapView::set_viewpoint(Point vp, bool jump) {
	const Widelands::Map& map = intbase().egbase().map();
	MapviewPixelFunctions::normalize_pix(map, &vp);

	panel_to_mappixel_ =
	   Transform2f::from_translation(vp.cast<float>() - panel_to_mappixel_.translation())
	      .chain(panel_to_mappixel_);

	// Normalize the translation to be positive. This guarantees that transform
	// into pixel space are also always positive.
	// NOCOM(#sirver): is this required? the jumping to coord is not properly
	// working when zoomed out, but I cannot fathom yet why.
	const uint32_t map_end_screen_x = MapviewPixelFunctions::get_map_end_screen_x(map);
	const uint32_t map_end_screen_y = MapviewPixelFunctions::get_map_end_screen_y(map);
	while (panel_to_mappixel_.translation().x < 0.f) {
		panel_to_mappixel_ =
		   Transform2f::from_translation(FloatPoint(map_end_screen_x, 0.f)).chain(panel_to_mappixel_);
	}
	while (panel_to_mappixel_.translation().y < 0.f) {
		panel_to_mappixel_ =
		   Transform2f::from_translation(FloatPoint(0.f, map_end_screen_y)).chain(panel_to_mappixel_);
	}

	// NOCOM(#sirver): why are there 2 callback functions?
	if (changeview_) {
		changeview_(vp, jump);
	}
	changeview(get_view_area());
}

FloatRect MapView::get_view_area() const {
	const FloatPoint min = panel_to_mappixel_.apply(FloatPoint());
	const FloatPoint max = panel_to_mappixel_.apply(FloatPoint(get_w(), get_h()));
	return FloatRect(min.x, min.y, max.x - min.x, max.y - min.y);
}

void MapView::set_rel_viewpoint(Point vp, bool jump) {
	set_viewpoint(get_viewpoint() + vp, jump);
}

void MapView::stop_dragging() {
	WLApplication::get()->set_mouse_lock(false);
	grab_mouse(false);
	dragging_ = false;
}

/**
 * Mousepressess and -releases on the map:
 * Right-press:   enable  dragging
 * Right-release: disable dragging
 * Left-press:    field action window
 */
bool MapView::handle_mousepress(uint8_t const btn, int32_t const x, int32_t const y) {
	if (btn == SDL_BUTTON_LEFT) {
		stop_dragging();
		track_sel(Point(x, y));

		fieldclicked();
	} else if (btn == SDL_BUTTON_RIGHT) {
		dragging_ = true;
		grab_mouse(true);
		WLApplication::get()->set_mouse_lock(true);
	}
	return true;
}

bool MapView::handle_mouserelease(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_RIGHT && dragging_)
		stop_dragging();
	return true;
}

bool MapView::handle_mousemove(
   uint8_t const state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) {
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;

	if (dragging_) {
		if (state & SDL_BUTTON(SDL_BUTTON_RIGHT))
			set_rel_viewpoint(Point(xdiff, ydiff), false);
		else
			stop_dragging();
	}

	if (!intbase().get_sel_freeze())
		track_sel(Point(x, y));
	return true;
}

// NOCOM(#sirver): change handle_mousewheel to also pass through the mouse position
bool MapView::handle_mousewheel(uint32_t which, int32_t /* x */, int32_t y) {
	if (which != 0) {
		return false;
	}

	const Transform2f translation =
	   Transform2f::from_translation(FloatPoint(last_mouse_pos_.x, last_mouse_pos_.y));
	Transform2f mappixel_to_panel = panel_to_mappixel_.inverse();
	const float old_zoom = mappixel_to_panel.zoom();
	constexpr float kPercentPerMouseWheelTick = 0.02f;
	float zoom = old_zoom * static_cast<float>(std::pow(
	                           1.f + math::sign(y) * kPercentPerMouseWheelTick, std::abs(y)));

	// Somewhat arbitrarily we limit the zoom to a reasonable value. This is for
	// performance and to avoid numeric glitches with more extreme values.
	constexpr float kMaxZoom = 4.f;
	zoom = math::clamp(zoom, 1.f / kMaxZoom, kMaxZoom);
	mappixel_to_panel = translation.chain(Transform2f::from_zoom(zoom / old_zoom))
	                                   .chain(translation.inverse())
	                                   .chain(mappixel_to_panel);
	panel_to_mappixel_ = mappixel_to_panel.inverse();

	auto point = round(panel_to_mappixel_.translation());
	MapviewPixelFunctions::normalize_pix(intbase().egbase().map(), &point);
	set_viewpoint(point, false);
	return true;
}

/*
===============
MapView::track_sel(int32_t mx, int32_t my)

Move the sel to the given mouse position.
Does not honour sel freeze.
===============
*/
void MapView::track_sel(Point p) {
	FloatPoint p_in_map = panel_to_mappixel_.apply(p.cast<float>());
	intbase_.set_sel_pos(MapviewPixelFunctions::calc_node_and_triangle(
	   intbase().egbase().map(), p_in_map.x, p_in_map.y));
}
