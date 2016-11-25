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

#include <SDL.h>

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

	// NOCOM(#sirver): maybe replace set_zoom through set_view.
	// NOCOM(#sirver): how to 'reverse' a plan?

// Takes t in [0, 1] and returns a value in [0, 1] that defines how much the
// previous values should be used in a path.
using EasingFunction = std::function<float (float t)>;

// Time of a frame in ms. We use a animation resolution of 60 fps, which should
// be fast enough to feel smooth on all framerates.
constexpr float kFrameTimeMs = 1000.f / 60.f;

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

// Returns the view area, i.e. the currently visible rectangle in map pixel
// space for the given 'view'.
Rectf get_view_area(const MapView::View& view, const int width, const int height) {
	return Rectf(view.viewpoint, width * view.zoom, height * view.zoom);
}

float ease_in_cubic(const float t) {
	return std::pow(t, 3.);
}

float ease_out_cubic(const float t) {
	return 1.f - ease_in_cubic(1.f - t);
}

float linear(const float t) {
	return t;
}

float constant(const float /* t */) {
	return 1.;
}

template <typename T>
struct KeyFrame {
	float time_ms;
	T value;
};

template <typename T>
class Interpolator {
public:
	Interpolator(std::vector<KeyFrame<T>> keyframes, std::vector<EasingFunction> easing_functions)
	   : keyframes_(std::move(keyframes)), easing_functions_(std::move(easing_functions)) {
#ifndef NDEBUG
		assert(keyframes_.size() >= 2);
		assert(keyframes_.size() == easing_functions_.size() + 1);
		for (size_t i = 1; i < keyframes_.size(); ++i) {
			assert(keyframes_[i-1].time_ms < keyframes_[i].time_ms);
		}
#endif
	}

	T value(const float time_ms) {
		if (time_ms <= keyframes_.front().time_ms) {
			return keyframes_.front().value;
		}
		if (time_ms >= keyframes_.back().time_ms) {
			return keyframes_.back().value;
		}

		size_t i = 0;
		while (i < keyframes_.size() && keyframes_[i].time_ms < time_ms) {
			++i;
		}
		assert(i > 0);
		const float percent_into_interval =
		   static_cast<float>(time_ms - keyframes_[i - 1].time_ms) /
		   static_cast<float>(keyframes_[i].time_ms - keyframes_[i - 1].time_ms);
		const float t = easing_functions_[i-1](percent_into_interval);
		return keyframes_[i-1].value * (1.0f - t) + keyframes_[i].value * t;
	}

private:
	std::vector<KeyFrame<T>> keyframes_;
	std::vector<EasingFunction> easing_functions_;

	DISALLOW_COPY_AND_ASSIGN(Interpolator);
};

// Calculates a animation plan from 'start_viewpoint' to 'end_viewpoint' - both at 'zoom',
// taking 'duraction_ms'. The animation is assumed to start_viewpoint at the
// time of calling the function
// NOCOM(#sirver): do everything in floats, only convert at the end.
std::vector<MapView::TimestampedView> plan_animation(const Widelands::Map& map,
                                                     const Vector2f& start_viewpoint,
                                                     const Vector2f& end_viewpoint,
                                                     const float start_zoom,
                                                     const int duration_ms,
                                                     const int width,
                                                     const int height) {
	constexpr float kTargetZoom = 8.f;
   // Viewpoint: Accelerate for 25% of time, then move constant, decelerate 25% of time

	// const float v_zoom = (kTargetZoom - start_zoom) / (duration_ms * 0.25);

	   // NOCOM(#sirver): change to always start at 0.
	Interpolator<float> zoom_t(
	   {
	      {0.f, start_zoom},
	      {0.25f * duration_ms, kTargetZoom},
	      {0.75f * duration_ms, kTargetZoom},
	      {1.0f * duration_ms, start_zoom},
	   },
	   {ease_in_cubic, constant, ease_out_cubic});

	const Vector2f start_center =
	   get_view_area(MapView::View{start_viewpoint, start_zoom}, width, height).center();
	const Vector2f end_center =
	   get_view_area(MapView::View{end_viewpoint, start_zoom}, width, height).center();
	const Vector2f center_point_change =
	   MapviewPixelFunctions::calc_pix_difference(map, end_center, start_center);

	Interpolator<Vector2f> center_point_t(
	   {
	      {0, start_center},
			{0.25f * duration_ms, start_center + center_point_change * 0.25f},
			{0.75f * duration_ms, start_center + center_point_change * 0.75f},
	      {1.0f * duration_ms, start_center + center_point_change},
	   },
	   {ease_in_cubic, linear, ease_out_cubic});
	const uint32_t start_time = SDL_GetTicks();
	std::vector<MapView::TimestampedView> plan;

	for (float dt = 0; dt < duration_ms; dt += kFrameTimeMs) {
		float zoom = zoom_t.value(dt);
		// Check if our target field is already visible on screen. If that is the
		// case, we do not zoom out further.
		if (!plan.empty()) {
			const auto& view = get_view_area(plan.back().view, width, height);
			const Vector2f dist = MapviewPixelFunctions::calc_pix_difference(
			   map, view.center(), start_center + center_point_change);
			if (std::abs(dist.x) < view.w / 2.0f && std::abs(dist.y) < view.h / 2.0f) {
				log("#sirver dist.x: %f,dist.w: %f,view.w: %f,view.h: %f\n", dist.x, dist.y, view.w, view.h);
				zoom = std::min(plan.back().view.zoom, zoom);
			}
		}
		// NOCOM(#sirver): what
		zoom = 1.f;
		const Vector2f center_point = center_point_t.value(dt);
		const Vector2f viewpoint = center_point - Vector2f(width * zoom / 2.f, height * zoom / 2.f);
		plan.push_back(MapView::TimestampedView{
		   static_cast<uint32_t>(std::lround(start_time + dt)), MapView::View{viewpoint, zoom}});
	}

	// Correct numeric instabilities. We want to land precisely at 'end_viewpoint'.
	plan.push_back(
	   MapView::TimestampedView{start_time + duration_ms, MapView::View{end_viewpoint, start_zoom}});
	return plan;
}

}  // namespace

MapView::MapView(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, InteractiveBase& player)
   : UI::Panel(parent, x, y, w, h),
     renderer_(new GameRenderer()),
     intbase_(player),
     view_{Vector2f(0.f, 0.f), 1.f},
     dragging_(false) {
}

MapView::~MapView() {
}

Vector2f MapView::get_viewpoint() const {
	return view_.viewpoint;
}

Vector2f MapView::to_panel(const Vector2f& map_pixel) const {
	return MapviewPixelFunctions::map_to_panel(view_.viewpoint, view_.zoom, map_pixel);
}

Vector2f MapView::to_map(const Vector2f& panel_pixel) const {
	return MapviewPixelFunctions::panel_to_map(view_.viewpoint, view_.zoom, panel_pixel);
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
	const Rectf area = view_area();

	const Vector2f view_center = area.center();
	const Vector2f dist = MapviewPixelFunctions::calc_pix_difference(map, view_center, map_pixel);

	// Check if the point is visible on screen.
	if (std::abs(dist.x) > area.w / 2.f || std::abs(dist.y) > area.h / 2.f) {
		return;
	}
	const Vector2f in_panel =
	   to_panel(move_inside(map_pixel, area, MapviewPixelFunctions::get_map_end_screen_x(map),
	                        MapviewPixelFunctions::get_map_end_screen_y(map)));
	set_mouse_pos(round(in_panel));
	track_sel(in_panel);
}

void MapView::draw(RenderTarget& dst) {
	Widelands::EditorGameBase& egbase = intbase().egbase();

	if (!current_plan_.empty()) {
		uint32_t now = SDL_GetTicks();
		size_t i = 0;
		while (i < current_plan_.size() && current_plan_[i].t < now) {
			++i;
		}
		if (i == current_plan_.size()) {
			current_plan_.clear();
		} else {
			set_zoom(current_plan_[i].view.zoom);
			set_viewpoint(current_plan_[i].view.viewpoint, false);
		}
	}

	if (upcast(Widelands::Game, game, &egbase)) {
		// Bail out if the game isn't actually loaded.
		// This fixes a crash with displaying an error dialog during loading.
		if (!game->is_loaded())
			return;
	}

	TextToDraw draw_text = TextToDraw::kNone;
	auto display_flags = intbase().get_display_flags();
	if (display_flags & InteractiveBase::dfShowCensus) {
		draw_text = draw_text | TextToDraw::kCensus;
	}
	if (display_flags & InteractiveBase::dfShowStatistics) {
		draw_text = draw_text | TextToDraw::kStatistics;
	}

	if (upcast(InteractivePlayer const, interactive_player, &intbase())) {
		renderer_->rendermap(
		   egbase, view_.viewpoint, view_.zoom, interactive_player->player(), draw_text, &dst);
	} else {
		renderer_->rendermap(egbase, view_.viewpoint, view_.zoom, static_cast<TextToDraw>(draw_text), &dst);
	}
}

float MapView::get_zoom() const {
	return view_.zoom;
}

void MapView::set_zoom(const float zoom) {
	view_.zoom = zoom;
}

/*
===============
Set the viewpoint to the given pixel coordinates
===============
*/
void MapView::set_viewpoint(const Vector2f& viewpoint, bool jump) {
	view_.viewpoint = viewpoint;
	const Widelands::Map& map = intbase().egbase().map();
	MapviewPixelFunctions::normalize_pix(map, &view_.viewpoint);
	changeview(jump);
}

void MapView::center_view_on_coords(const Widelands::Coords& c) {
	const Widelands::Map& map = intbase().egbase().map();
	assert(0 <= c.x);
	assert(c.x < map.get_width());
	assert(0 <= c.y);
	assert(c.y < map.get_height());

	const Vector2f in_mappixel = MapviewPixelFunctions::to_map_pixel(map.get_fcoords(c));
	center_view_on_map_pixel(in_mappixel);
}

void MapView::center_view_on_map_pixel(const Vector2f& pos) {
	const Rectf area = view_area();
	const Vector2f target_view = pos - Vector2f(area.w / 2.f, area.h / 2.f);
	const Widelands::Map& map = intbase().egbase().map();
	current_plan_ =
	   plan_animation(map, view_.viewpoint, target_view, view_.zoom, 10000, get_w(), get_h());
}

Rectf MapView::view_area() const {
	return get_view_area(view_, get_w(), get_h());
}

void MapView::pan_by(Vector2i delta_pixels) {
	set_viewpoint(get_viewpoint() + delta_pixels.cast<float>() * view_.zoom, false);
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
		track_sel(Vector2f(x, y));

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
		track_sel(Vector2f(x, y));
	return true;
}

bool MapView::handle_mousewheel(uint32_t which, int32_t /* x */, int32_t y) {
	if (which != 0) {
		return false;
	}

	constexpr float kPercentPerMouseWheelTick = 0.02f;
	float zoom = view_.zoom * static_cast<float>(
	                        std::pow(1.f - math::sign(y) * kPercentPerMouseWheelTick, std::abs(y)));
	zoom_around(zoom, last_mouse_pos_.cast<float>());
	return true;
}

void MapView::zoom_around(float new_zoom, const Vector2f& panel_pixel) {
	// Somewhat arbitrarily we limit the zoom to a reasonable value. This is for
	// performance and to avoid numeric glitches with more extreme values.
	constexpr float kMaxZoom = 4.f;
	new_zoom = math::clamp(new_zoom, 1.f / kMaxZoom, kMaxZoom);

	// Zoom around the current mouse position. See
	// http://stackoverflow.com/questions/2916081/zoom-in-on-a-point-using-scale-and-translate
	// for a good explanation of this math.
	const Vector2f offset = -panel_pixel * (new_zoom - view_.zoom);
	view_.zoom = new_zoom;
	set_viewpoint(view_.viewpoint + offset, false);
}

/*
===============
MapView::track_sel(int32_t mx, int32_t my)

Move the sel to the given mouse position.
Does not honour sel freeze.
===============
*/
void MapView::track_sel(const Vector2f& p) {
	Vector2f p_in_map = to_map(p);
	intbase_.set_sel_pos(MapviewPixelFunctions::calc_node_and_triangle(
	   intbase().egbase().map(), p_in_map.x, p_in_map.y));
}

bool MapView::handle_key(bool down, SDL_Keysym code) {
	if (!down) {
		return false;
	}
	if (!(code.mod & KMOD_CTRL)) {
		return false;
	}

	constexpr float kPercentPerKeyPress = 0.10f;
	switch (code.sym) {
	case SDLK_PLUS:
		zoom_around(view_.zoom - kPercentPerKeyPress, Vector2f(get_w() / 2.f, get_h() / 2.f));
		return true;
	case SDLK_MINUS:
		zoom_around(view_.zoom + kPercentPerKeyPress, Vector2f(get_w() / 2.f, get_h() / 2.f));
		return true;
	case SDLK_0:
		zoom_around(1.f, Vector2f(get_w() / 2.f, get_h() / 2.f));
		return true;
	default:
		return false;
	}
	NEVER_HERE();
}
