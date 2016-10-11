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
     viewpoint_(Point(0, 0)),
	  zoom_(1.0),
     dragging_(false) {
}

MapView::~MapView() {
}

/// Moves the mouse cursor so that it is directly above the given node
void MapView::warp_mouse_to_node(Widelands::Coords const c) {
	const Widelands::Map& map = intbase().egbase().map();
	FloatPoint p;
	MapviewPixelFunctions::get_save_pix(map, c, zoom_, &p);
	p -= viewpoint_.cast<float>();

	//  If the user has scrolled the node outside the viewable area, he most
	//  surely doesn't want to jump there.
	if (p.x < get_w() && p.y < get_h()) {
		if (p.x <= 0)
			warp_mouse_to_node(Widelands::Coords(c.x + map.get_width(), c.y));
		else if (p.y <= 0)
			warp_mouse_to_node(Widelands::Coords(c.x, c.y + map.get_height()));
		else {
			const Point rounded = round(p);
			set_mouse_pos(rounded);
			track_sel(rounded);
		}
	}
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
		renderer_->rendermap(egbase, viewpoint_, zoom_, interactive_player->player(), &dst);
	} else {
		renderer_->rendermap(egbase, viewpoint_, zoom_, &dst);
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
	// NOCOM(#sirver): was this actually correct, changing this to take zoom? Maybe we keep this without zoom?
	MapviewPixelFunctions::normalize_pix(intbase().egbase().map(), zoom_, &vp);
	viewpoint_ = vp;

	// NOCOM(#sirver): why are there 2 callback functions?
	if (changeview_) {
		changeview_(vp, jump);
	}
	changeview(viewpoint_.x, viewpoint_.y);
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

bool MapView::handle_mousewheel(uint32_t which, int32_t /* x */, int32_t y) {
	if (which != 0) {
		return false;
	}

	// NOCOM(#sirver): this should keep the center point in the center, but
	// keeps the top left point centered.
	Point point(viewpoint_.x / zoom_, viewpoint_.y / zoom_);

	constexpr float kPercentPerMouseWheelTick = 0.02f;
	zoom_ *= std::pow(1.f + math::sign(y) * kPercentPerMouseWheelTick, std::abs(y));

	// Somewhat arbitrarily we limit the zoom to a reasonable value.
	// This is for performance and to avoid numeric glitches with
	// more extreme values.
	constexpr float kMaxZoom = 4.f;
	zoom_ = math::clamp(zoom_, 1.f / kMaxZoom, kMaxZoom);

	point.x *= zoom_;
	point.y *= zoom_;

	MapviewPixelFunctions::normalize_pix(intbase().egbase().map(), 1., &point);
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
void MapView::track_sel(Point m) {
	// NOCOM(#sirver): this is also wrong.
	m.x = m.x * zoom_ + viewpoint_.x / zoom_;
	m.y = m.y * zoom_ + viewpoint_.y / zoom_;
	intbase_.set_sel_pos(
	   MapviewPixelFunctions::calc_node_and_triangle(intbase().egbase().map(), m.x, m.y));
}
