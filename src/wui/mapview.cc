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

// Number of keyframes to generate for a plan. The more points, the smoother
// the animation (though we also lineraly interpolate between keyframes), but
// it also means more computational work to plan the animation.
constexpr int kNumKeyFrames = 102;

// The maximum zoom to use in moving animations.
constexpr float kMaxAnimationZoom = 8.f;

// The time used for panning automated map movement only.
constexpr float kShortAnimationMs = 500.f;

// The time used for zooming and panning automated map movement.
constexpr float kLongAnimationMs = 1500.f;

// If the difference between the current zoom and the target zoom in an
// animation plan is smaller than this value, we will do a pan-only movement.
constexpr float kPanOnlyZoomThreshold = 0.25f;

// If the target is less than this many screens at the current zoom level away,
// we will do a pan-only movement.
constexpr float kPanOnlyDistanceThreshold = 2.0f;

// Returns the view area, i.e. the currently visible rectangle in map pixel
// space for the given 'view'.
Rectf get_view_area(const MapView::View& view, const int width, const int height) {
	return Rectf(view.viewpoint, width * view.zoom, height * view.zoom);
}

// Returns the linear interpolation of 'a' and 'b' depending on 't' in [0,
// 1].
template <typename T> T mix(float t, const T& a, const T& b) {
	return a * (1.f - t) + b * t;
}

// https://en.wikipedia.org/wiki/Smoothstep
template <typename T> class SmoothstepInterpolator {
public:
	SmoothstepInterpolator(const T& start, const T& end, float dt)
	   : start_(start), end_(end), dt_(dt) {
	}

	T value(const float time_ms) const {
		const float t = math::clamp(time_ms / dt_, 0.f, 1.f);
		return mix(math::sqr(t) * (3.f - 2.f * t), start_, end_);
	}

private:
	T start_, end_;
	float dt_;

	DISALLOW_COPY_AND_ASSIGN(SmoothstepInterpolator);
};

// In the first half smoothly interpolate from 'start' to 'middle', then in the
// second half interpolate till 'end'.
template <typename T> class DoubleSmoothstepInterpolator {
public:
	DoubleSmoothstepInterpolator(const T& start, const T& middle, const T& end, float dt)
	   : first_(start, middle, dt / 2.f), second_(middle, end, dt / 2.f), dt_(dt) {
	}

	T value(const float time_ms) const {
		const float t = math::clamp(time_ms / dt_, 0.f, 1.f);
		if (t < 0.5f) {
			return first_.value(t * dt_);
		} else {
			return second_.value((t - 0.5f) * dt_);
		}
	}

private:
	const SmoothstepInterpolator<T> first_, second_;
	float dt_;

	DISALLOW_COPY_AND_ASSIGN(DoubleSmoothstepInterpolator);
};

// TODO(sirver): Once c++14 is supported, make this a templated lambda inside
// 'plan_map_transition'. For now it is a particularly ugly stand alone
// function, but it allows us to parametrize over 'zoom_t' withouth a heap
// allocation.
template <typename T>
void do_plan_map_transition(uint32_t start_time,
                            const float duration_ms,
                            const SmoothstepInterpolator<Vector2f>& center_point_t,
                            const T& zoom_t,
                            const int width,
                            const int height,
                            std::deque<MapView::TimestampedView>* plan) {
	for (int i = 1; i < kNumKeyFrames - 2; i++) {
		float dt = (duration_ms / kNumKeyFrames) * i;
		const float zoom = zoom_t.value(dt);
		const Vector2f center_point = center_point_t.value(dt);
		const Vector2f viewpoint = center_point - Vector2f(width * zoom / 2.f, height * zoom / 2.f);
		plan->push_back(MapView::TimestampedView{
		   static_cast<uint32_t>(std::lround(start_time + dt)), MapView::View{viewpoint, zoom}});
	}
}

// Calculates an animation plan from 'start' to 'end'. Employs heuristics
// to decide what sort of transitions are taken and how long it takes.
std::deque<MapView::TimestampedView> plan_map_transition(const uint32_t start_time,
                                                         const Widelands::Map& map,
                                                         const MapView::View& start,
                                                         const MapView::View& end,
                                                         const int width,
                                                         const int height) {
	const Vector2f start_center = get_view_area(start, width, height).center();
	const Vector2f end_center = get_view_area(end, width, height).center();
	const Vector2f center_point_change =
	   MapviewPixelFunctions::calc_pix_difference(map, end_center, start_center);
	const float zoom_change = std::abs(start.zoom - end.zoom);

	// Heuristic: How many screens is the target point away from the current
	// viewpoint? We use it to decide the zoom out factor and scroll speed.
	float num_screens = std::max(std::abs(center_point_change.x) / (width * start.zoom),
	                             std::abs(center_point_change.y) / (height * start.zoom));

	// Do nothing if we are close.
	if (zoom_change < 0.25f && std::abs(center_point_change.x) < 10 &&
	    std::abs(center_point_change.y) < 10) {
		return {};
	}

	// If the target is more than a copule screens away or we change the zoom we
	// do a sort of jumping animation - zoom out, move and zoom back in.
	// Otherwise we we just linearly interpolate the zoom.
	const bool jumping_animation =
	   num_screens > kPanOnlyDistanceThreshold || zoom_change > kPanOnlyZoomThreshold;
	const float duration_ms = jumping_animation ? kLongAnimationMs : kShortAnimationMs;

	std::deque<MapView::TimestampedView> plan;
	plan.push_back(MapView::TimestampedView{start_time, start});

	const SmoothstepInterpolator<Vector2f> center_point_t(
	   start_center, start_center + center_point_change, duration_ms);

	if (jumping_animation) {
		// We jump higher if the distance is farther - but we never zoom in (i.e.
		// negative jump) or jump higher than 'kMaxAnimationZoom'.
		const float target_zoom = math::clamp(num_screens + start.zoom, end.zoom, kMaxAnimationZoom);
		do_plan_map_transition(
		   start_time, duration_ms, center_point_t,
		   DoubleSmoothstepInterpolator<float>(start.zoom, target_zoom, end.zoom, duration_ms), width,
		   height, &plan);
	} else {
		do_plan_map_transition(start_time, duration_ms, center_point_t,
		                       SmoothstepInterpolator<float>(start.zoom, end.zoom, duration_ms),
		                       width, height, &plan);
	}

	// Correct numeric instabilities. We want to land precisely at 'end'.
	const Vector2f end_viewpoint = (start_center + center_point_change) -
	                               Vector2f(width * end.zoom / 2.f, height * end.zoom / 2.f);
	plan.push_back(
	   MapView::TimestampedView{static_cast<uint32_t>(std::lround(start_time + duration_ms)),
	                            MapView::View{end_viewpoint, end.zoom}});
	return plan;
}

// Plan an animation zoom around 'center' starting at 'start_zoom' and
// ending at 'end_zoom'.
std::deque<MapView::TimestampedView> plan_zoom_transition(const uint32_t start_time,
                                                          const Vector2f& center,
                                                          const float start_zoom,
                                                          const float end_zoom,
                                                          const int width,
                                                          const int height) {
	const SmoothstepInterpolator<float> zoom_t(start_zoom, end_zoom, kShortAnimationMs);
	std::deque<MapView::TimestampedView> plan;
	const auto push = [&](const float dt, const float zoom) {
		plan.push_back(
		   MapView::TimestampedView{static_cast<uint32_t>(std::lround(start_time + dt)),
		                            {center - Vector2f(zoom * width, zoom * height) * 0.5f, zoom}});
	};

	push(0, start_zoom);
	for (int i = 1; i < kNumKeyFrames - 2; i++) {
		float dt = (kShortAnimationMs / kNumKeyFrames) * i;
		push(dt, zoom_t.value(dt));
	}
	push(kShortAnimationMs, end_zoom);
	return plan;
}

// Plan a mouse movement 'start' and ending at 'target'.
std::deque<MapView::TimestampedMouse> plan_mouse_transition(const MapView::TimestampedMouse& start,
                                                            const Vector2i& target) {
	const SmoothstepInterpolator<Vector2f> mouse_t(
	   start.pixel, target.cast<float>(), kShortAnimationMs);
	std::deque<MapView::TimestampedMouse> plan;

	plan.push_back(start);
	for (int i = 1; i < kNumKeyFrames - 2; i++) {
		float dt = (kShortAnimationMs / kNumKeyFrames) * i;
		plan.push_back(MapView::TimestampedMouse{
		   static_cast<uint32_t>(std::lround(start.t + dt)), mouse_t.value(dt)});
	}
	plan.push_back(MapView::TimestampedMouse{
	   static_cast<uint32_t>(std::lround(start.t + kShortAnimationMs)), target.cast<float>()});
	return plan;
}

}  // namespace

MapView::ViewArea::ViewArea(const Rectf& init_rect, const Widelands::Map& map)
   : rect_(init_rect), map_(map) {
}

bool MapView::ViewArea::contains(const Widelands::Coords& c) const {
	return contains_map_pixel(MapviewPixelFunctions::to_map_pixel_with_normalization(map_, c));
}

Vector2f MapView::ViewArea::move_inside(const Widelands::Coords& c) const {
	// We want to figure out to which pixel 'c' maps inside our rect_. Since
	// Wideland's map is a torus, the current 'rect_' could span the origin.
	// Without loss of generality we only discuss x - y follows accordingly.
	// Depending on the interpretation, the area spanning the origin means:
	// 1) either view_area.x + view_area.w < view_area.x - which would be
	// surprising to the rest of Widelands.
	// 2) map_pixel.x > get_map_end_screen_x(map).
	// We move the point by adding or substracting 'get_map_end_screen_x()' such
	// that the point is contained inside of 'rect_'. If we now convert to
	// panel pixels, we are guaranteed that the pixel we get back is inside the
	// screen bounds.
	Vector2f p = MapviewPixelFunctions::to_map_pixel_with_normalization(map_, c);
	assert(contains_map_pixel(p));

	const float w = MapviewPixelFunctions::get_map_end_screen_x(map_);
	const float h = MapviewPixelFunctions::get_map_end_screen_y(map_);
	while (p.x < rect_.x && rect_.x < rect_.x + rect_.w) {
		p.x += w;
	}
	while (p.x > rect_.x && rect_.x > rect_.x + rect_.w) {
		p.x -= w;
	}
	while (p.y < rect_.y && rect_.y < rect_.y + rect_.y) {
		p.y += h;
	}
	while (p.y > rect_.y && rect_.y > rect_.y + rect_.y) {
		p.y -= h;
	}
	return p;
}

bool MapView::ViewArea::contains_map_pixel(const Vector2f& map_pixel) const {
	// We figure out if 'map_pixel' is visible on screen. To do this, we
	// calculate the shortest distance to 'view_area.center()' on a torus. If
	// the distance is less than 'view_area.w / 2', the point is visible.
	const Vector2f view_center = rect_.center();
	const Vector2f dist = MapviewPixelFunctions::calc_pix_difference(map_, view_center, map_pixel);

	// Check if the point is visible on screen.
	return std::abs(dist.x) <= (rect_.w / 2.f) && std::abs(dist.y) <= (rect_.h / 2.f);
}

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

Vector2f MapView::to_panel(const Vector2f& map_pixel) const {
	return MapviewPixelFunctions::map_to_panel(view_.viewpoint, view_.zoom, map_pixel);
}

Vector2f MapView::to_map(const Vector2i& panel_pixel) const {
	return MapviewPixelFunctions::panel_to_map(
	   view_.viewpoint, view_.zoom, panel_pixel.cast<float>());
}

void MapView::mouse_to_field(const Widelands::Coords& c, const Transition& transition) {
	const ViewArea area = view_area();
	if (!area.contains(c)) {
		return;
	}
	mouse_to_pixel(round(to_panel(area.move_inside(c))), transition);
}

void MapView::mouse_to_pixel(const Vector2i& pixel, const Transition& transition) {
	switch (transition) {
	case Transition::Jump:
		track_sel(pixel);
		set_mouse_pos(pixel);
		return;

	case Transition::Smooth: {
		const TimestampedMouse current = animation_target_mouse();
		const auto plan = plan_mouse_transition(current, pixel);
		if (!plan.empty()) {
			mouse_plans_.push_back(plan);
		}
		return;
	}
	}
	NEVER_HERE();
}

void MapView::draw(RenderTarget& dst) {
	Widelands::EditorGameBase& egbase = intbase().egbase();

	uint32_t now = SDL_GetTicks();
	while (!view_plans_.empty()) {
		auto& plan = view_plans_.front();
		while (plan.size() > 1 && plan[1].t < now) {
			plan.pop_front();
		}
		if (plan.size() == 1) {
			view_plans_.pop_front();
			continue;
		}

		// Linearly interpolate between the next and the last.
		const float t = (now - plan[0].t) / static_cast<float>(plan[1].t - plan[0].t);
		const View new_view = {
		   mix(t, plan[0].view.viewpoint, plan[1].view.viewpoint),
		   mix(t, plan[0].view.zoom, plan[1].view.zoom),
		};
		set_view(new_view, Transition::Jump);
		break;
	}

	while (!mouse_plans_.empty()) {
		auto& plan = mouse_plans_.front();
		while (plan.size() > 1 && plan[1].t < now) {
			plan.pop_front();
		}
		if (plan.size() == 1) {
			mouse_plans_.pop_front();
			continue;
		}

		// Linearly interpolate between the next and the last.
		const float t = (now - plan[0].t) / static_cast<float>(plan[1].t - plan[0].t);
		mouse_to_pixel(round(mix(t, plan[0].pixel, plan[1].pixel)), Transition::Jump);
		break;
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
		renderer_->rendermap(
		   egbase, view_.viewpoint, view_.zoom, static_cast<TextToDraw>(draw_text), &dst);
	}
}

void MapView::set_view(const View& target_view, const Transition& transition) {
	const Widelands::Map& map = intbase().egbase().map();
	switch (transition) {
	case Transition::Jump: {
		view_ = target_view;
		MapviewPixelFunctions::normalize_pix(map, &view_.viewpoint);
		changeview();
		return;
	}

	case Transition::Smooth: {
		const TimestampedView current = animation_target_view();
		const auto plan =
		   plan_map_transition(current.t, map, current.view, target_view, get_w(), get_h());
		if (!plan.empty()) {
			view_plans_.push_back(plan);
		}
		return;
	}
	}
}

void MapView::scroll_to_field(const Widelands::Coords& c, const Transition& transition) {
	const Widelands::Map& map = intbase().egbase().map();
	assert(0 <= c.x);
	assert(c.x < map.get_width());
	assert(0 <= c.y);
	assert(c.y < map.get_height());

	const Vector2f in_mappixel = MapviewPixelFunctions::to_map_pixel(map.get_fcoords(c));
	scroll_to_map_pixel(in_mappixel, transition);
}

void MapView::scroll_to_map_pixel(const Vector2f& pos, const Transition& transition) {
	const Rectf area = view_area().rect();
	const Vector2f target_view = pos - Vector2f(area.w / 2.f, area.h / 2.f);
	set_view(View{target_view, view_.zoom}, transition);
}

MapView::ViewArea MapView::view_area() const {
	return ViewArea(get_view_area(view_, get_w(), get_h()), intbase().egbase().map());
}

const MapView::View& MapView::view() const {
	return view_;
}

void MapView::pan_by(Vector2i delta_pixels) {
	if (is_animating()) {
		return;
	}
	set_view(
	   {view_.viewpoint + delta_pixels.cast<float>() * view_.zoom, view_.zoom}, Transition::Jump);
}

void MapView::stop_dragging() {
	WLApplication::get()->set_mouse_lock(false);
	grab_mouse(false);
	dragging_ = false;
}

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

	if (is_animating()) {
		return true;
	}
	constexpr float kPercentPerMouseWheelTick = 0.02f;
	float zoom = view_.zoom * static_cast<float>(std::pow(
	                             1.f - math::sign(y) * kPercentPerMouseWheelTick, std::abs(y)));
	zoom_around(zoom, last_mouse_pos_.cast<float>(), Transition::Jump);
	return true;
}

void MapView::zoom_around(float new_zoom,
                          const Vector2f& panel_pixel,
                          const Transition& transition) {
	// Somewhat arbitrarily we limit the zoom to a reasonable value. This is for
	// performance and to avoid numeric glitches with more extreme values.
	constexpr float kMaxZoom = 4.f;
	new_zoom = math::clamp(new_zoom, 1.f / kMaxZoom, kMaxZoom);

	const TimestampedView current = animation_target_view();
	switch (transition) {
	case Transition::Jump: {
		// Zoom around the current mouse position. See
		// http://stackoverflow.com/questions/2916081/zoom-in-on-a-point-using-scale-and-translate
		// for a good explanation of this math.
		set_view({current.view.viewpoint - panel_pixel * (new_zoom - current.view.zoom), new_zoom},
		         Transition::Jump);
		return;
	}

	case Transition::Smooth: {
		const int w = get_w();
		const int h = get_h();
		const auto plan = plan_zoom_transition(
		   current.t, get_view_area(current.view, w, h).center(), current.view.zoom, new_zoom, w, h);
		if (!plan.empty()) {
			view_plans_.push_back(plan);
		}
		return;
	}
	}
	NEVER_HERE();
}

bool MapView::is_dragging() const {
	return dragging_;
}

bool MapView::is_animating() const {
	return !view_plans_.empty() || !mouse_plans_.empty();
}

void MapView::track_sel(const Vector2i& p) {
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
		zoom_around(animation_target_view().view.zoom - kPercentPerKeyPress,
		            Vector2f(get_w() / 2.f, get_h() / 2.f), Transition::Smooth);
		return true;
	case SDLK_MINUS:
		zoom_around(animation_target_view().view.zoom + kPercentPerKeyPress,
		            Vector2f(get_w() / 2.f, get_h() / 2.f), Transition::Smooth);
		return true;
	case SDLK_0:
		zoom_around(1.f, Vector2f(get_w() / 2.f, get_h() / 2.f), Transition::Smooth);
		return true;
	default:
		return false;
	}
	NEVER_HERE();
}

MapView::TimestampedView MapView::animation_target_view() const {
	if (view_plans_.empty()) {
		return TimestampedView{SDL_GetTicks(), view_};
	}
	return view_plans_.back().back();
}

MapView::TimestampedMouse MapView::animation_target_mouse() const {
	if (mouse_plans_.empty()) {
		return TimestampedMouse{SDL_GetTicks(), get_mouse_position().cast<float>()};
	}
	return mouse_plans_.back().back();
}
