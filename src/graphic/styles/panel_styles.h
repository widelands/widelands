/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_STYLES_PANEL_STYLES_H
#define WL_GRAPHIC_STYLES_PANEL_STYLES_H

#include "graphic/color.h"
#include "graphic/image.h"

namespace UI {

// Backgrounds
enum class PanelStyle { kFsMenu, kWui };
enum class TabPanelStyle { kFsMenu, kWuiLight, kWuiDark };

struct PanelStyleInfo {
	PanelStyleInfo(const Image* init_image, const RGBAColor& init_color, int init_margin)
	   : margin_(init_margin), image_(init_image), color_(init_color) {
	}
	PanelStyleInfo(const PanelStyleInfo& other)
	   : margin_(other.margin()), image_(other.image()), color_(other.color()) {
	}

	const RGBAColor& color() const {
		return color_;
	}
	const Image* image() const {
		return image_;
	}

	int margin() const {
		return margin_;
	}

private:
	int margin_;
	const Image* image_;
	RGBAColor color_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_PANEL_STYLES_H
