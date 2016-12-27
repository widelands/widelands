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

// Number of keyframes to generate for a plan. The more points, the smoother
// the animation (though we also lineraly interpolate between keyframes) and
// the more work.
constexpr int kNumKeyFrames = 102;

// The maximum zoom to use in moving animations.
constexpr float kMaxAnimationZoom = 8.f;

// The time used for paning only automated map movement.
constexpr float kPanOnlyAnimationTimeMs = 500.f;

// The time used for zooming and paning automated map movement.
constexpr float kPanAndZoomAnimationTimeMs = 1500.f;

// If the difference between the current zoom and the target zoom in an
// animation plan is smaller than this value, we will do a pan-only movement.
constexpr float kPanOnlyZoomThreshold = 0.25f;

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

constexpr float pow2(float t) {
	return t * t;
}

constexpr float pow3(float t) {
	return t * t * t;
}

template <typename T>
T mix(float t, const T& a, const T& b) {
	return a * (1.f - t) + b * t;
}

// https://en.wikipedia.org/wiki/Smoothstep
template <typename T>
class SmoothstepInterpolator {
public:
	SmoothstepInterpolator(const T& start, const T& end, float dt)
	   : start_(start), end_(end), dt_(dt) {
	}

	T value(const float time_ms) {
		const float t = math::clamp(time_ms / dt_, 0.f, 1.f);
		return mix(pow2(t) * (3.f - 2.f * t), start_, end_);
		// NOCOM(#sirver): Smootherstep - maybe accelerations too slowly, but definitvely smoother.
		// return mix(pow3(t) * (t * (t * 6.f - 15.f) + 10.f), start_, end_);
	}

private:
	T start_, end_;
	float dt_;

	DISALLOW_COPY_AND_ASSIGN(SmoothstepInterpolator);
};

template <typename T, typename P>
class SymmetricInterpolator {
public:
	SymmetricInterpolator(const T& start, const T& end, float dt)
	   : inner_(start, end, dt / 2.f), dt_(dt) {
	}

	T value(const float time_ms) {
		const float t = math::clamp(time_ms / dt_, 0.f, 1.f);
		if (t < 0.5f) {
			return inner_.value(t * dt_);
		} else {
			return inner_.value((1.f - t) * dt_);
		}
	}

private:
	P inner_;
	float dt_;

	DISALLOW_COPY_AND_ASSIGN(SymmetricInterpolator);
};



// Calculates a animation plan from 'start' to 'end_viewpoint' - both at 'zoom',
// taking 'duraction_ms'. The animation is assumed to start at the
// time of calling the function
// NOCOM(#sirver): do everything in floats, only convert at the end.
std::vector<MapView::TimestampedView> plan_animation(const Widelands::Map& map,
                                                     const Vector2f& start,
                                                     const Vector2f& end,
                                                     const float start_zoom,
                                                     const int width,
                                                     const int height) {
	const Vector2f start_center =
	   get_view_area(MapView::View{start, start_zoom}, width, height).center();
	const Vector2f end_center =
	   get_view_area(MapView::View{end, start_zoom}, width, height).center();
	const Vector2f center_point_change =
	   MapviewPixelFunctions::calc_pix_difference(map, end_center, start_center);

	// Heuristic: How many screens is the target point away from the current
	// viewpoint? We use it to decide the zoom out factor and scroll speed.
	float num_screens = std::max(std::abs(center_point_change.x) / (width * start_zoom),
	                             std::abs(center_point_change.y) / (height * start_zoom));

	// If the target is 4 screens away, we zoom out to x4. If we would not zoom
	// out, we do not interpolate the zoom at all. This avoids rounding errors.
	const float target_zoom = math::clamp(num_screens, start_zoom, kMaxAnimationZoom);
	const float delta_zoom = target_zoom - start_zoom;
	const bool pan_and_zoom_animation = delta_zoom > kPanOnlyZoomThreshold;
	const float duration_ms =
	   pan_and_zoom_animation ? kPanAndZoomAnimationTimeMs : kPanOnlyAnimationTimeMs;

	SymmetricInterpolator<float, SmoothstepInterpolator<float>> zoom_t(
	   start_zoom, target_zoom, static_cast<float>(duration_ms));

	SmoothstepInterpolator<Vector2f> center_point_t(
	   start_center, start_center + center_point_change, duration_ms);
	const uint32_t start_time = SDL_GetTicks();
	std::vector<MapView::TimestampedView> plan;
	plan.push_back(MapView::TimestampedView{start_time, MapView::View{start, start_zoom}});
	for (int i = 1; i < kNumKeyFrames - 2; i++) {
		float dt = (duration_ms / kNumKeyFrames) * i;
		const float zoom = pan_and_zoom_animation ? zoom_t.value(dt) : start_zoom;
		const Vector2f center_point = center_point_t.value(dt);
		const Vector2f viewpoint = center_point - Vector2f(width * zoom / 2.f, height * zoom / 2.f);
		plan.push_back(MapView::TimestampedView{
		   static_cast<uint32_t>(std::lround(start_time + dt)), MapView::View{viewpoint, zoom}});
	}
	// Correct numeric instabilities. We want to land precisely at 'end'.
	const Vector2f end_viewpoint = (start_center + center_point_change) -
	                               Vector2f(width * start_zoom / 2.f, height * start_zoom / 2.f);
	plan.push_back(
	   MapView::TimestampedView{static_cast<uint32_t>(std::lround(start_time + duration_ms)),
	                            MapView::View{end_viewpoint, start_zoom}});
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
		} else if (i > 0) {
			// Linearly interpolate between the next and the last.
			float t = (now - current_plan_[i - 1].t) /
			          static_cast<float>(current_plan_[i].t - current_plan_[i - 1].t);
			const float zoom =
			   mix(t, current_plan_[i - 1].view.zoom, current_plan_[i].view.zoom);
			set_zoom(zoom);
			const Vector2f viewpoint = mix(
			   t, current_plan_[i - 1].view.viewpoint, current_plan_[i].view.viewpoint);
			set_viewpoint(viewpoint, Transition::Jump);
			// log("#sirver %d,%.4f,%.4f,%.4f\n", now, viewpoint.x, viewpoint.y, zoom);
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
void MapView::set_viewpoint(const Vector2f& target_view, const Transition& transition) {
	switch (transition) {
		case Transition::Smooth: {
			const Widelands::Map& map = intbase().egbase().map();
			current_plan_ =
				plan_animation(map, view_.viewpoint, target_view, view_.zoom, get_w(), get_h());
			return;
	   }

	   case Transition::Jump:
			view_.viewpoint = target_view;
			const Widelands::Map& map = intbase().egbase().map();
			MapviewPixelFunctions::normalize_pix(map, &view_.viewpoint);
			changeview(false /* jump */ ); // NOCOM(#sirver): I truly hate this function :/
			return;
	}
	NEVER_HERE();
}

void MapView::center_on_coords(const Widelands::Coords& c, const Transition& transition) {
	const Widelands::Map& map = intbase().egbase().map();
	assert(0 <= c.x);
	assert(c.x < map.get_width());
	assert(0 <= c.y);
	assert(c.y < map.get_height());

	const Vector2f in_mappixel = MapviewPixelFunctions::to_map_pixel(map.get_fcoords(c));
	center_on_map_pixel(in_mappixel, transition);
}

void MapView::center_on_map_pixel(const Vector2f& pos, const Transition& transition) {
	const Rectf area = view_area();
	const Vector2f target_view = pos - Vector2f(area.w / 2.f, area.h / 2.f);
	set_viewpoint(target_view, transition);
}

Rectf MapView::view_area() const {
	return get_view_area(view_, get_w(), get_h());
}

const MapView::View& MapView::view() const {
	return view_;
}


void MapView::pan_by(Vector2i delta_pixels) {
	set_viewpoint(get_viewpoint() + delta_pixels.cast<float>() * view_.zoom, Transition::Jump);
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
	set_viewpoint(view_.viewpoint + offset, Transition::Jump);
}

bool MapView::is_dragging() const {
	return dragging_;
}

bool MapView::is_animating() const {
	return !current_plan_.empty();
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
