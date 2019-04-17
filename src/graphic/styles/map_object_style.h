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

#ifndef WL_GRAPHIC_STYLES_MAP_OBJECT_STYLE_H
#define WL_GRAPHIC_STYLES_MAP_OBJECT_STYLE_H

#include <memory>

#include "graphic/color.h"
#include "graphic/styles/font_style.h"

namespace UI {

struct MapObjectStyleInfo {
	explicit MapObjectStyleInfo(UI::FontStyleInfo* init_building_statistics,
								UI::FontStyleInfo* init_census,
								UI::FontStyleInfo* init_statistics) :
		building_statistics_font_(init_building_statistics),
		census_font_(init_census),
		statistics_font_(init_statistics) {}

	const UI::FontStyleInfo& building_statistics_font() const {
		return *building_statistics_font_.get();
	}
	const UI::FontStyleInfo& census_font() const {
		return *census_font_.get();
	}
	const UI::FontStyleInfo& statistics_font() const {
		return *statistics_font_.get();
	}

	RGBColor construction_color;
	RGBColor neutral_color;
	RGBColor low_color;
	RGBColor medium_color;
	RGBColor high_color;

private:
	std::unique_ptr<const UI::FontStyleInfo> building_statistics_font_;
	std::unique_ptr<const UI::FontStyleInfo> census_font_;
	std::unique_ptr<const UI::FontStyleInfo> statistics_font_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_MAP_OBJECT_STYLE_H
