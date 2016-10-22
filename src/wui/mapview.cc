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
#include "graphic/game_renderer.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/map.h"
#include "logic/map_objects/draw_text.h"
#include "logic/player.h"
#include "wlapplication.h"
#include "wui/interactive_base.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelfunctions.h"

namespace {

// Returns the shortest distance between two points on a wrapping line of
// length width.
float shortest_distance_on_torus(float x1, float x2, const float width) {
	if (x1 > x2) {
		std::swap(x1, x2);
	}
	while (x2 - x1 > width / 2.f) {
		x1 += width;
	}
	return x2 - x1;
}

// Given 'p' on a torus of dimension ('h', 'h') and 'r' that contains this
// point, change 'p' so that r.x < p.x < r.x + r.w and similar for y.
// Containing is defined as such that the shortest distance between the center
// of 'r' is smaller than (r.w / 2, r.h / 2). If 'p' is NOT contained in 'r'
// this method will loop forever.
Vector2f move_inside(Vector2f p, const Rectf& r, float w, float h) {
	while (p.x < r.x && r.x < r.x + r.w) {
		p.x += w;
	}
	while (p.x > r.x && r.x > r.x + r.w) {
		p.x -= w;
	}
	while (p.y < r.y && r.y < r.y + r.y) {
		p.y += h;
	}
	while (p.y > r.y && r.y > r.y + r.y) {
		p.y -= h;
	}
	return p;
}

}  // namespace

MapView::MapView(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, InteractiveBase& player)
   : UI::Panel(parent, x, y, w, h),
     renderer_(new GameRenderer()),
     intbase_(player),
     viewpoint_(0.f, 0.f),
     zoom_(1.f),
     dragging_(false) {
}

MapView::~MapView() {
}

Vector2i MapView::get_viewpoint() const {
	return round(viewpoint_);
}

Vector2f MapView::to_panel(const Vector2f& map_pixel) const {
	return MapviewPixelFunctions::map_to_panel(viewpoint_, zoom_, map_pixel);
}

Vector2f MapView::to_map(const Vector2f& panel_pixel) const {
	return MapviewPixelFunctions::panel_to_map(viewpoint_, zoom_, panel_pixel);
}

/// Moves the mouse cursor so that it is directly above the given node
void MapView::warp_mouse_to_node(Widelands::Coords const c) {
	// This problem is surprisingly hard: We want to figure out if the
	// 'minimap_pixel' is currently visible on screen and if so, what pixel it
	// has. Since Wideland's map is a torus, the current 'view_area' could span
	// the origin. Without loss of generality we only discuss x - y follows
	// accordingly.
	// Depending on the interpretation, the area spanning the origin means:
	// 1) either view_area.x + view_area.w < view_area.x - which would be surprising to
	//    the rest of Widelands.
	// 2) map_pixel.x > get_map_end_screen_x(map).
	//
	// We are dealing with the problem in two steps: first we figure out if
	// 'map_pixel' is visible on screen. To do this, we calculate the shortest
	// distance to 'view_area.center()' on a torus. If the distance is less than
	// 'view_area.w / 2', the point is visible.
	// If that is the case, we move the point by adding or substracting
	// 'get_map_end_screen_x()' such that the point is contained inside of
	// 'view_area'. If we now convert to panel pixels, we are guaranteed that
	// the pixel we get back is inside the panel.

	const Widelands::Map& map = intbase().egbase().map();
	const Vector2f map_pixel = MapviewPixelFunctions::to_map_pixel_with_normalization(map, c);
	const Rectf view_area = get_view_area();

	const Vector2f view_center = view_area.center();
	const int w = MapviewPixelFunctions::get_map_end_screen_x(map);
	const int h = MapviewPixelFunctions::get_map_end_screen_y(map);
	const float dist_x = shortest_distance_on_torus(view_center.x, map_pixel.x, w);
	const float dist_y = shortest_distance_on_torus(view_center.y, map_pixel.y, h);

	// Check if the point is visible on screen.
	if (dist_x > view_area.w / 2.f || dist_y > view_area.h / 2.f) {
		return;
	}
	const Vector2i in_panel = round(to_panel(move_inside(map_pixel, view_area, w, h)));
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

	int draw_text = DrawText::kNone;
	auto display_flags = intbase().get_display_flags();
	if (display_flags & InteractiveBase::dfShowCensus) {
		draw_text |= DrawText::kCensus;
	}
	if (display_flags & InteractiveBase::dfShowStatistics) {
		draw_text |= DrawText::kStatistics;
	}

	if (upcast(InteractivePlayer const, interactive_player, &intbase())) {
		renderer_->rendermap(egbase, viewpoint_, zoom_, interactive_player->player(),
		                     static_cast<DrawText>(draw_text), &dst);
	} else {
		renderer_->rendermap(egbase, viewpoint_, zoom_, static_cast<DrawText>(draw_text), &dst);
	}
}

void MapView::set_changeview(const MapView::ChangeViewFn& fn) {
	changeview_ = fn;
}

float MapView::get_zoom() const {
	return zoom_;
}

void MapView::set_zoom(const float zoom) {
	zoom_ = zoom;
}

/*
===============
Set the viewpoint to the given pixel coordinates
===============
*/
void MapView::set_viewpoint(Vector2i vp, bool jump) {
	const Widelands::Map& map = intbase().egbase().map();
	MapviewPixelFunctions::normalize_pix(map, &vp);

	viewpoint_ = vp.cast<float>();

	// NOCOM(#sirver): why are there 2 callback functions?
	if (changeview_) {
		changeview_(vp, jump);
	}
	changeview(get_view_area());
}

void MapView::center_view_on_coords(const Widelands::Coords& c) {
	const Widelands::Map& map = intbase().egbase().map();
	assert(0 <= c.x);
	assert(c.x < map.get_width());
	assert(0 <= c.y);
	assert(c.y < map.get_height());

	const Vector2i in_mappixel = round(MapviewPixelFunctions::to_map_pixel(map.get_fcoords(c)));
	center_view_on_map_pixel(in_mappixel);
}

void MapView::center_view_on_map_pixel(const Vector2i& pos) {
	const Rectf view_area = get_view_area();
	set_viewpoint(pos - Vector2i(view_area.w / 2.f, view_area.h / 2.f), true);
}

Rectf MapView::get_view_area() const {
	return Rectf(viewpoint_, get_w() * zoom_, get_h() * zoom_);
}

void MapView::pan_by(Vector2i delta_pixels) {
	set_viewpoint(get_viewpoint() + delta_pixels * zoom_, false);
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
		track_sel(Vector2i(x, y));

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
		if (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			pan_by(Vector2i(xdiff, ydiff));
		} else {
			stop_dragging();
		}
	}

	if (!intbase().get_sel_freeze())
		track_sel(Vector2i(x, y));
	return true;
}

bool MapView::handle_mousewheel(uint32_t which, int32_t /* x */, int32_t y) {
	if (which != 0) {
		return false;
	}

	constexpr float kPercentPerMouseWheelTick = 0.02f;
	float zoom = zoom_ * static_cast<float>(
	                        std::pow(1.f - math::sign(y) * kPercentPerMouseWheelTick, std::abs(y)));

	// Somewhat arbitrarily we limit the zoom to a reasonable value. This is for
	// performance and to avoid numeric glitches with more extreme values.
	constexpr float kMaxZoom = 4.f;
	zoom = math::clamp(zoom, 1.f / kMaxZoom, kMaxZoom);

	// Zoom around the current mouse position. See
	// http://stackoverflow.com/questions/2916081/zoom-in-on-a-point-using-scale-and-translate
	// for a good explanation of this math.
	const Vector2f offset = -last_mouse_pos_.cast<float>() * (zoom - zoom_);

	zoom_ = zoom;
	set_viewpoint(round(viewpoint_ + offset), false);
	return true;
}

/*
===============
MapView::track_sel(int32_t mx, int32_t my)

Move the sel to the given mouse position.
Does not honour sel freeze.
===============
*/
void MapView::track_sel(Vector2i p) {
	Vector2f p_in_map = to_map(p.cast<float>());
	intbase_.set_sel_pos(MapviewPixelFunctions::calc_node_and_triangle(
	   intbase().egbase().map(), p_in_map.x, p_in_map.y));
}
