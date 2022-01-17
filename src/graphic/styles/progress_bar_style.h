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

#ifndef WL_GRAPHIC_STYLES_PROGRESS_BAR_STYLE_H
#define WL_GRAPHIC_STYLES_PROGRESS_BAR_STYLE_H

#include <memory>

#include "graphic/color.h"
#include "graphic/styles/font_style.h"

namespace UI {

struct ProgressbarStyleInfo {
	explicit ProgressbarStyleInfo(UI::FontStyleInfo* init_font,
	                              const RGBColor& init_low_color,
	                              const RGBColor& init_medium_color,
	                              const RGBColor& init_high_color)
	   : font_(init_font),
	     low_color_(init_low_color),
	     medium_color_(init_medium_color),
	     high_color_(init_high_color) {
	}
	explicit ProgressbarStyleInfo(const ProgressbarStyleInfo& other)
	   : font_(new UI::FontStyleInfo(other.font())),
	     low_color_(other.low_color()),
	     medium_color_(other.medium_color()),
	     high_color_(other.high_color()) {
	}

	const UI::FontStyleInfo& font() const {
		return *font_.get();
	}
	const RGBColor& low_color() const {
		return low_color_;
	}
	const RGBColor& medium_color() const {
		return medium_color_;
	}
	const RGBColor& high_color() const {
		return high_color_;
	}

private:
	std::unique_ptr<const UI::FontStyleInfo> font_;
	const RGBColor low_color_;
	const RGBColor medium_color_;
	const RGBColor high_color_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_PROGRESS_BAR_STYLE_H
