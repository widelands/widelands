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

#ifndef WL_UI_BASIC_WINDOW_H
#define WL_UI_BASIC_WINDOW_H

#include "ui_basic/panel.h"

namespace UI {
/**
 * Windows are cached by default.
 *
 * The graphics (see pic__*) are used in the following manner: (Example)
 *
 * top:
 *  <--20leftmostpixel_of_top-->
 *     <60Pixels as often as possible to reach window with from top>
 *  <20rightmost pixel of top>
 * site:
 *  ^
 *  20 topmost pixels of l_border                       <--- > same for r_border
 *  as often as needed: 60 pixels of l_border           <--- > same for r_border
 *  20 bottom pixels of l_border                        <--- > same for r_border
 * bottom:
 *  <--20leftmostpixel_of_bot-->
 *     <60Pixels as often as possible to reach window with from bot>
 *  <20rightmost pixel of bot>
 *
 * So: the l_border and the r_border pics MUST have a height of 100, while the
 *     width must be 20 and the top and bot pics MUST have a width of 100, while
 *     the height must be 20
 *
 * A click with the middle mouse button (or STRG+LClick) minimizes a window.
 * Minimize means, that the window is only the caption bar, nothing inside.
 * Another click on this bar resizes the window again
 */
class Window : public NamedPanel {
public:
	/// Do not use richtext for 'title'.
	Window(Panel* parent,
	       const std::string& name,
	       int32_t x,
	       int32_t y,
	       uint32_t w,
	       uint32_t h,
	       const std::string& title);

	/// This will set the window title. Do not use richtext for 'text'.
	void set_title(const std::string& text);
	const std::string& get_title() const {
		return title_;
	}

	void set_center_panel(Panel* panel);
	void move_out_of_the_way();
	void move_inside_parent() override;
	void center_to_parent();
	void warp_mouse_to_fastclick_panel();
	void set_fastclick_panel(Panel* p) {
		fastclick_panel_ = p;
	}

	bool is_minimal() const {
		return is_minimal_;
	}
	void restore();
	void minimize();
	bool is_snap_target() const override {
		return true;
	}

	// Drawing and event handlers
	void draw(RenderTarget&) override;
	void draw_border(RenderTarget&) override;

	void think() override;

	bool handle_mousepress(uint8_t btn, int32_t mx, int32_t my) override;
	bool handle_mouserelease(uint8_t btn, int32_t mx, int32_t my) override;
	bool
	handle_mousemove(uint8_t state, int32_t mx, int32_t my, int32_t xdiff, int32_t ydiff) override;
	bool handle_tooltip() override;

protected:
	void die() override;
	void layout() override;
	void update_desired_size() override;

private:
	bool is_minimal_;
	uint32_t oldh_;  // if it is, this is the old height
	bool dragging_, docked_left_, docked_right_, docked_bottom_;
	int32_t drag_start_win_x_, drag_start_win_y_;
	int32_t drag_start_mouse_x_, drag_start_mouse_y_;

	std::string title_;

	const Image* pic_lborder_;
	const Image* pic_rborder_;
	const Image* pic_top_;
	const Image* pic_bottom_;
	const Image* pic_background_;

	Panel* center_panel_;
	Panel* fastclick_panel_;
};
}

#endif  // end of include guard: WL_UI_BASIC_WINDOW_H
