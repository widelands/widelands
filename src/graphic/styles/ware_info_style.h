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

#ifndef WL_GRAPHIC_STYLES_WARE_INFO_STYLE_H
#define WL_GRAPHIC_STYLES_WARE_INFO_STYLE_H

#include <memory>

#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/styles/font_style.h"

namespace UI {

enum class WareInfoStyle { kNormal, kHighlight };

struct WareInfoStyleInfo {
	explicit WareInfoStyleInfo(UI::FontStyleInfo* init_header_font,
	                           UI::FontStyleInfo* init_info_font,
	                           const Image* init_icon_background_image,
	                           const RGBColor& init_icon_frame,
	                           const RGBColor& init_icon_background,
	                           const RGBColor& init_info_background)
	   : header_font_(init_header_font),
	     info_font_(init_info_font),
	     icon_background_image_(init_icon_background_image),
	     icon_frame_(init_icon_frame),
	     icon_background_(init_icon_background),
	     info_background_(init_info_background) {
	}

	const UI::FontStyleInfo& header_font() const {
		return *header_font_.get();
	}
	const UI::FontStyleInfo& info_font() const {
		return *info_font_.get();
	}
	const Image* icon_background_image() const {
		return icon_background_image_;
	}
	const RGBColor& icon_frame() const {
		return icon_frame_;
	}
	const RGBColor& icon_background() const {
		return icon_background_;
	}
	const RGBColor& info_background() const {
		return info_background_;
	}

private:
	std::unique_ptr<const UI::FontStyleInfo> header_font_;
	std::unique_ptr<const UI::FontStyleInfo> info_font_;
	const Image* icon_background_image_;
	const RGBColor icon_frame_;
	const RGBColor icon_background_;
	const RGBColor info_background_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_WARE_INFO_STYLE_H
