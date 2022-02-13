/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_MAPVIEW_H
#define WL_WUI_MAPVIEW_H

#include "base/rect.h"
#include "base/vector.h"
#include "graphic/gl/fields_to_draw.h"
#include "logic/map.h"
#include "logic/widelands_geometry.h"
#include "ui_basic/panel.h"

/**
 * Implements a view of a map. It is used to render a valid map on the screen.
 */
class MapView : public UI::Panel {
public:
	// A rectangle on a Torus (i.e. a Widelands Map).
	class ViewArea {
	public:
		// View in map pixels that is spanned by this.
		const Rectf& rect() const {
			return rect_;
		}

		// Returns true if 'coords' is contained inside this view. Containing
		// is defined as such that the shortest distance between the center of
		// 'rect()' is smaller than (rect().w / 2, rect().h / 2).
		bool contains(const Widelands::Coords& coords) const;

		// Returns a map pixel 'p' such that rect().x <= p.x <= rect().x + rect().w similar
		// for y. This requires that 'contains' would return true for 'coords', otherwise this will
		// be an infinite loop.
		Vector2f find_pixel_for_coordinates(const Widelands::Coords& coords) const;

	private:
		friend class MapView;

		ViewArea(const Rectf& rect, const Widelands::Map& map);

		// Returns true if 'map_pixel' is inside this view area.
		bool contains_map_pixel(const Vector2f& map_pixel) const;

		const Rectf rect_;
		const Widelands::Map& map_;
	};

	struct View {
		View(Vector2f init_viewpoint, float init_zoom) : viewpoint(init_viewpoint), zoom(init_zoom) {
		}
		View() : View(Vector2f::zero(), 1.0f) {
		}

		bool zoom_near(float other_zoom) const;

		bool view_near(const View& other) const;

		bool view_roughly_near(const View& other) const;

		// Mappixel of top-left pixel of this MapView.
		Vector2f viewpoint;

		// Current zoom value.
		float zoom;
	};

	// Time in milliseconds since the game was launched. Animations always
	// happen in real-time, not in gametime. Therefore they are also not
	// affected by pause.
	struct TimestampedView {
		uint32_t t;
		View view;
	};

	struct TimestampedMouse {
		TimestampedMouse(uint32_t init_t, Vector2f init_pixel) : t(init_t), pixel(init_pixel) {
		}
		TimestampedMouse() : t(0), pixel(Vector2f::zero()) {
		}
		uint32_t t;
		Vector2f pixel = Vector2f::zero();
	};

	MapView(UI::Panel* const parent,
	        const Widelands::Map& map,
	        const int32_t x,
	        const int32_t y,
	        const uint32_t w,
	        const uint32_t h);
	~MapView() override = default;

	// Called whenever the view changed, also during automatic animations.
	Notifications::Signal<> changeview;

	// Called whenever the view changed by a call to scroll_to_field or scroll_to_map_pixel, or by
	// starting to drag the view.
	// Note: This signal is called *before* the view actually starts to move.
	Notifications::Signal<> jump;

	// Called when the user clicked on a field.
	Notifications::Signal<const Widelands::NodeAndTriangle<>&> field_clicked;

	// Called when the field under the mouse cursor has changed.
	Notifications::Signal<const Widelands::NodeAndTriangle<>&> track_selection;

	// Defines if an animation should be immediate (one-frame) or nicely
	// animated for the user to follow.
	enum class Transition { Smooth, Jump };

	// Set the view to 'view'.
	void set_view(const View& view, const Transition& transition);

	// Moves the view so that 'coords' is centered.
	void scroll_to_field(const Widelands::Coords& coords, const Transition& transition);

	// Moves the view so that 'pos' is centered. The 'pos' is in map pixel
	// coordinates.
	void scroll_to_map_pixel(const Vector2f& pos, const Transition& transition);

	// Moves the mouse cursor so that it is directly above the given field. Does
	// nothing if the field is not currently visible on screen.
	void mouse_to_field(const Widelands::Coords& coords, const Transition& transition);

	// Moves the mouse to the 'pixel' in the current panel. With 'transition' ==
	// Jump, this behaves exactly like 'set_mouse_pos'.
	void mouse_to_pixel(const Vector2i& pixel, const Transition& transition);

	// Move the view by 'delta_pixels'.
	void pan_by(Vector2i delta_pixels, const Transition& transition);

	// The current view area visible in the MapView in map pixel coordinates.
	// The returned value always has 'x' > 0 and 'y' > 0.
	ViewArea view_area() const;

	// The current view.
	const View& view() const;

	// Set the zoom to the 'new_zoom'. This keeps the map_pixel that is
	// displayed at 'panel_pixel' unchanging, i.e. the center of the zoom.
	void zoom_around(float new_zoom, const Vector2f& panel_pixel, const Transition& transition);

	// Reset the zoom to 1.0f
	void reset_zoom();
	// Zoom in a bit
	void increase_zoom();
	// Zoom out a bit
	void decrease_zoom();

	// True if the user is currently dragging the map.
	bool is_dragging() const;

	// True if a 'Transition::Smooth' animation is playing.
	bool is_animating() const;

	// Scrolls the map and returns true if it did.
	bool scroll_map();

	// Schedules drawing of the terrain of this MapView. The returned value can
	// be used to override contents of 'fields_to_draw' for player knowledge and
	// visibility, and to correctly draw map objects, overlays and text.
	FieldsToDraw* draw_terrain(const Widelands::EditorGameBase& egbase,
	                           const Widelands::Player*,
	                           const Workareas& workarea,
	                           bool grid,
	                           RenderTarget* dst);

	// Not overriden from UI::Panel, instead we expect to be passed the data through.
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
	bool handle_key(bool down, SDL_Keysym code) override;
	void think() override;

private:
	void stop_dragging();

	// Returns the target view of the last entry in 'view_plans_' or (now,
	// 'view_') if we are not animating.
	TimestampedView animation_target_view() const;

	// Returns the target mouse position 'mouse_plans_' or (now,
	// current mouse) if we are not animating.
	TimestampedMouse animation_target_mouse() const;

	// Turns 'm' into the corresponding NodeAndTrinangle and calls 'track_selection'.
	Widelands::NodeAndTriangle<> track_sel(const Vector2i& p);

	Vector2f to_panel(const Vector2f& map_pixel) const;
	Vector2f to_map(const Vector2i& panel_pixel) const;

	const bool animate_map_panning_;
	const Widelands::Map& map_;

	// This is owned and handled by us, but handed to the RenderQueue, so we
	// basically promise that this stays valid for one frame.
	FieldsToDraw fields_to_draw_;

	View view_;
	Vector2i last_mouse_pos_;
	bool dragging_;

	bool edge_scrolling_;
	bool invert_movement_;
	int8_t is_scrolling_x_, is_scrolling_y_;

	// The queue of plans to execute as animations.
	std::deque<std::deque<TimestampedView>> view_plans_;
	std::deque<std::deque<TimestampedMouse>> mouse_plans_;
};

#endif  // end of include guard: WL_WUI_MAPVIEW_H
