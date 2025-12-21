/*
 * Copyright (C) 2020-2025 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_STYLES_WINDOW_STYLE_H
#define WL_GRAPHIC_STYLES_WINDOW_STYLE_H

#include "graphic/color.h"
#include "graphic/image.h"

namespace UI {

enum class WindowStyle { kFsMenu, kWui };

struct WindowStyleInfo {
	WindowStyleInfo(const RGBAColor& col_f,
	                const RGBAColor& col_uf,
	                const Image* img_t,
	                const Image* img_b,
	                const Image* img_r,
	                const Image* img_l,
	                const Image* img_c_bl,
	                const Image* img_c_br,
	                const Image* img_c_tl,
	                const Image* img_c_tr,
	                const Image* img_c_ml,
	                const Image* img_c_mr,
	                const Image* img_bg,
	                const std::string& b_pin,
	                const std::string& b_unpin,
	                const std::string& b_minimize,
	                const std::string& b_unminimize,
	                const std::string& b_close,
	                int b_spacing)
	   : window_border_focused_(col_f),
	     window_border_unfocused_(col_uf),
	     border_top_(img_t),
	     border_bottom_(img_b),
	     border_left_(img_l),
	     border_right_(img_r),
	     corner_bottom_left_(img_c_bl),
	     corner_bottom_right_(img_c_br),
	     corner_top_left_(img_c_tl),
	     corner_top_right_(img_c_tr),
	     corner_minimal_left_(img_c_ml),
	     corner_minimal_right_(img_c_mr),
	     background_(img_bg),
	     button_close_(b_close),
	     button_pin_(b_pin),
	     button_unpin_(b_unpin),
	     button_minimize_(b_minimize),
	     button_unminimize_(b_unminimize),
	     button_spacing_(b_spacing) {
		// TODO(tothxa): allow different width
		button_size_ = top_border_thickness() - 2 * button_spacing_;
		buttons_y_ = -top_border_thickness() + button_spacing_;
		button_close_x_ = right_border_thickness() - button_size_ - button_spacing_;
		button_pin_x_ = -left_border_thickness() + button_spacing_;
		button_minimize_x_ = button_pin_x_ + button_size_ + button_spacing_;
	}

	WindowStyleInfo(const WindowStyleInfo&) = default;

	[[nodiscard]] const RGBAColor& window_border_focused() const {
		return window_border_focused_;
	}
	[[nodiscard]] const RGBAColor& window_border_unfocused() const {
		return window_border_unfocused_;
	}

	[[nodiscard]] const Image* background() const {
		return background_;
	}
	[[nodiscard]] const Image* border_top() const {
		return border_top_;
	}
	[[nodiscard]] const Image* border_right() const {
		return border_right_;
	}
	[[nodiscard]] const Image* border_left() const {
		return border_left_;
	}
	[[nodiscard]] const Image* border_bottom() const {
		return border_bottom_;
	}

	[[nodiscard]] const Image* corner_bottom_left() const {
		return corner_bottom_left_;
	}
	[[nodiscard]] const Image* corner_bottom_right() const {
		return corner_bottom_right_;
	}
	[[nodiscard]] const Image* corner_top_left() const {
		return corner_top_left_;
	}
	[[nodiscard]] const Image* corner_top_right() const {
		return corner_top_right_;
	}
	[[nodiscard]] const Image* corner_minimal_left() const {
		return corner_minimal_left_;
	}
	[[nodiscard]] const Image* corner_minimal_right() const {
		return corner_minimal_right_;
	}

	[[nodiscard]] const std::string& button_close() const {
		return button_close_;
	}
	[[nodiscard]] const std::string& button_pin() const {
		return button_pin_;
	}
	[[nodiscard]] const std::string& button_unpin() const {
		return button_unpin_;
	}
	[[nodiscard]] const std::string& button_unminimize() const {
		return button_unminimize_;
	}
	[[nodiscard]] const std::string& button_minimize() const {
		return button_minimize_;
	}

	[[nodiscard]] int top_border_thickness() const {
		return border_top_->height();
	}
	[[nodiscard]] int bottom_border_thickness() const {
		return border_bottom_->height();
	}
	[[nodiscard]] int left_border_thickness() const {
		return border_left_->width();
	}
	[[nodiscard]] int right_border_thickness() const {
		return border_right_->width();
	}

	[[nodiscard]] int button_spacing() const {
		return button_spacing_;
	}
	// TODO(Nordfriese): Allow buttons with arbitrary widths
	[[nodiscard]] int button_size() const {
		return button_size_;
	}

	[[nodiscard]] int buttons_y() const {
		return buttons_y_;
	}
	[[nodiscard]] int button_close_x() const {
		return button_close_x_;
	}
	[[nodiscard]] int button_pin_x() const {
		return button_pin_x_;
	}
	[[nodiscard]] int button_minimize_x() const {
		return button_minimize_x_;
	}

private:
	const RGBAColor window_border_focused_, window_border_unfocused_;
	const Image* border_top_;
	const Image* border_bottom_;
	const Image* border_left_;
	const Image* border_right_;
	const Image* corner_bottom_left_;
	const Image* corner_bottom_right_;
	const Image* corner_top_left_;
	const Image* corner_top_right_;
	const Image* corner_minimal_left_;
	const Image* corner_minimal_right_;
	const Image* background_;
	const std::string button_close_;
	const std::string button_pin_;
	const std::string button_unpin_;
	const std::string button_minimize_;
	const std::string button_unminimize_;
	const int button_spacing_;
	int button_size_;
	int buttons_y_;
	int button_close_x_;
	int button_pin_x_;
	int button_minimize_x_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_WINDOW_STYLE_H
