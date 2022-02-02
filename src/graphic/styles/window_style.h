/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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
	                const Image* img_bg,
	                const std::string& b_pin,
	                const std::string& b_unpin,
	                const std::string& b_minimize,
	                const std::string& b_unminimize,
	                const std::string& b_close)
	   : window_border_focused_(col_f),
	     window_border_unfocused_(col_uf),
	     border_top_(img_t),
	     border_bottom_(img_b),
	     border_left_(img_l),
	     border_right_(img_r),
	     background_(img_bg),
	     button_close_(b_close),
	     button_pin_(b_pin),
	     button_unpin_(b_unpin),
	     button_minimize_(b_minimize),
	     button_unminimize_(b_unminimize) {
	}
	WindowStyleInfo(const WindowStyleInfo&) = default;

	const RGBAColor& window_border_focused() const {
		return window_border_focused_;
	}
	const RGBAColor& window_border_unfocused() const {
		return window_border_unfocused_;
	}

	const Image* background() const {
		return background_;
	}
	const Image* border_top() const {
		return border_top_;
	}
	const Image* border_right() const {
		return border_right_;
	}
	const Image* border_left() const {
		return border_left_;
	}
	const Image* border_bottom() const {
		return border_bottom_;
	}

	const std::string& button_close() const {
		return button_close_;
	}
	const std::string& button_pin() const {
		return button_pin_;
	}
	const std::string& button_unpin() const {
		return button_unpin_;
	}
	const std::string& button_unminimize() const {
		return button_unminimize_;
	}
	const std::string& button_minimize() const {
		return button_minimize_;
	}

private:
	const RGBAColor window_border_focused_, window_border_unfocused_;
	const Image* border_top_;
	const Image* border_bottom_;
	const Image* border_left_;
	const Image* border_right_;
	const Image* background_;
	const std::string button_close_;
	const std::string button_pin_;
	const std::string button_unpin_;
	const std::string button_minimize_;
	const std::string button_unminimize_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_WINDOW_STYLE_H
