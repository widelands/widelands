/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_WINDOW_H
#define WL_UI_BASIC_WINDOW_H

#include <memory>

#include "graphic/note_graphic_resolution_changed.h"
#include "graphic/styles/window_style.h"
#include "ui_basic/button.h"

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
	/// Height the top border must have
	static constexpr int16_t kTopBorderThickness = 20;
	/// Height the bottom border must have
	static constexpr int16_t kBottomBorderThickness = 20;
	/// Width the vertical border graphics must have
	static constexpr int16_t kVerticalBorderThickness = 20;

	/// Do not use richtext for 'title'.
	/// Text conventions: Title Case for the 'title'
	Window(Panel* parent,
	       WindowStyle style,
	       const std::string& name,
	       int32_t x,
	       int32_t y,
	       uint32_t w,
	       uint32_t h,
	       const std::string& title);
	~Window() override;

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
	virtual void restore();
	virtual void minimize();

	[[nodiscard]] ZOrder get_z() const override {
		if (is_pinned()) {
			return std::max(Panel::ZOrder::kPinned, NamedPanel::get_z());
		}
		return NamedPanel::get_z();
	}

	bool is_pinned() const {
		return pinned_;
	}
	void set_pinned(bool p) {
		pinned_ = p;
		update_toolbar_buttons();
	}
	Button* get_button_pin() {
		return button_pin_;
	}

	/** Whether the user ever moved the window. */
	[[nodiscard]] bool moved_by_user() const {
		return moved_by_user_;
	}

	// Drawing and event handlers
	void draw(RenderTarget&) override;
	void draw_border(RenderTarget&) override;

	void think() override;

	bool handle_mousepress(uint8_t btn, int32_t mx, int32_t my) override;
	bool handle_mouserelease(uint8_t btn, int32_t mx, int32_t my) override;
	bool
	handle_mousemove(uint8_t state, int32_t mx, int32_t my, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
	bool handle_tooltip() override;
	bool handle_key(bool down, SDL_Keysym code) override;

	enum class WindowLayoutID {
		kNone,
		kFsMenuDefault,
		kFsMenuOptions,
		kFsMenuKeyboardOptions,
		kFsMenuAbout
	};
	virtual WindowLayoutID window_layout_id() const {
		return WindowLayoutID::kNone;
	}

protected:
	void die() override;
	void layout() override;
	void update_desired_size() override;

	virtual void clicked_button_close();

	bool is_focus_toplevel() const override {
		return true;
	}

	void do_not_layout_on_resolution_change() {
		graphic_resolution_changed_subscriber_.reset();
	}

	const WindowStyle window_style_;
	const WindowStyleInfo& window_style_info() const;
	const FontStyleInfo& title_style() const;

	virtual void on_resolution_changed_note(const GraphicResolutionChanged& note);

private:
	bool is_minimal_{false};
	uint32_t oldh_;  // if it is minimized, this is the old height
	bool dragging_{false};
	bool moved_by_user_{false};
	int32_t drag_start_win_x_{0};
	int32_t drag_start_win_y_{0};
	int32_t drag_start_mouse_x_{0};
	int32_t drag_start_mouse_y_{0};
	bool pinned_{false};

	std::string title_;

	Panel* center_panel_{nullptr};
	Panel* fastclick_panel_{nullptr};

	Button* button_close_;
	Button* button_pin_;
	Button* button_minimize_;
	void update_toolbar_buttons();

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_WINDOW_H
