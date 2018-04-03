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

#ifndef WL_GRAPHIC_MAP_OBJECT_STYLE_INFO_H
#define WL_GRAPHIC_MAP_OBJECT_STYLE_INFO_H

#include <memory>

#include "graphic/color.h"
#include "graphic/font_styles.h"

namespace UI {

struct MapObjectStyleInfo {
	UI::FontStyleInfo building_statistics_font;
	UI::FontStyleInfo census_font;
	UI::FontStyleInfo statistics_font;

	RGBColor construction_color;
	RGBColor neutral_color;
	RGBColor low_color;
	RGBColor medium_color;
	RGBColor high_color;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_MAP_OBJECT_STYLE_INFO_H
