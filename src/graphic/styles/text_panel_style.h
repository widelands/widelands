/*
 * Copyright (C) 2018-2022 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_GRAPHIC_STYLES_TEXT_PANEL_STYLE_H
#define WL_GRAPHIC_STYLES_TEXT_PANEL_STYLE_H

#include <memory>

#include "graphic/styles/font_style.h"
#include "graphic/styles/panel_styles.h"

namespace UI {

enum class SliderStyle { kFsMenu, kWuiLight, kWuiDark };

struct TextPanelStyleInfo {
	explicit TextPanelStyleInfo(const UI::FontStyleInfo* init_font,
	                            const UI::PanelStyleInfo* init_background)
	   : background_(init_background), font_(init_font) {
	}
	explicit TextPanelStyleInfo(const TextPanelStyleInfo& other)
	   : background_(new UI::PanelStyleInfo(other.background())),
	     font_(new UI::FontStyleInfo(other.font())) {
	}

	const UI::FontStyleInfo& font() const {
		return *font_.get();
	}
	void set_font(const UI::FontStyleInfo& new_font) {
		font_.reset(new UI::FontStyleInfo(new_font));
	}

	const UI::PanelStyleInfo& background() const {
		return *background_.get();
	}

private:
	std::unique_ptr<const UI::PanelStyleInfo> background_;
	std::unique_ptr<const UI::FontStyleInfo> font_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_TEXT_PANEL_STYLE_H
