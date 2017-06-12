/*
 * Copyright (C) 2017 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_PANEL_STYLES_H
#define WL_GRAPHIC_PANEL_STYLES_H

#include "graphic/color.h"
#include "graphic/image.h"

namespace UI {

// Buttons
enum class ButtonStyle {
	kFsMenuMenu,
	kFsMenuPrimary,
	kFsMenuSecondary,
	kWuiMenu,
	kWuiPrimary,
	kWuiSecondary,
	kWuiBuildingStats
};
enum class SliderStyle { kFsMenu, kWuiLight, kWuiDark };

// Backgrounds
enum class PanelStyle { kFsMenu, kWui };
enum class TabPanelStyle { kFsMenu, kWuiLight, kWuiDark };

struct PanelStyleInfo {
	PanelStyleInfo(const Image* init_image, const RGBAColor& init_color)
	   : image(init_image), color(init_color) {
	}
	PanelStyleInfo() : PanelStyleInfo(nullptr, RGBAColor(0, 0, 0, 0)) {
	}

	const Image* image;
	const RGBAColor color;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_PANEL_STYLES_H
