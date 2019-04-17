/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_STYLES_WARE_INFO_STYLE_H
#define WL_GRAPHIC_STYLES_WARE_INFO_STYLE_H

#include <memory>

#include "graphic/color.h"
#include "graphic/styles/font_style.h"
#include "graphic/image.h"

namespace UI {

enum class WareInfoStyle {
	kNormal,
	kHighlight
};

struct WareInfoStyleInfo {
	explicit WareInfoStyleInfo(UI::FontStyleInfo* init_header_font, UI::FontStyleInfo* init_info_font) :
		header_font_(init_header_font), info_font_(init_info_font) {}

	const UI::FontStyleInfo& header_font() const {
		return *header_font_.get();
	}
	const UI::FontStyleInfo& info_font() const {
		return *info_font_.get();
	}

	const Image* icon_background_image;
	RGBColor icon_frame;
	RGBColor icon_background;
	RGBColor info_background;

private:
	std::unique_ptr<const UI::FontStyleInfo> header_font_;
	std::unique_ptr<const UI::FontStyleInfo> info_font_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_WARE_INFO_STYLE_H
