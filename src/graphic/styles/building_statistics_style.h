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

#ifndef WL_GRAPHIC_STYLES_BUILDING_STATISTICS_STYLE_H
#define WL_GRAPHIC_STYLES_BUILDING_STATISTICS_STYLE_H

#include <memory>

#include "graphic/color.h"
#include "graphic/styles/font_style.h"

namespace UI {

struct BuildingStatisticsStyleInfo {
	explicit BuildingStatisticsStyleInfo(UI::FontStyleInfo* init_building_statistics_button_font,
	                                     UI::FontStyleInfo* init_building_statistics_details_font,
	                                     int init_editbox_margin,
	                                     UI::FontStyleInfo* init_census,
	                                     UI::FontStyleInfo* init_statistics,
	                                     const RGBColor& init_construction_color,
	                                     const RGBColor& init_neutral_color,
	                                     const RGBColor& init_low_color,
	                                     const RGBColor& init_medium_color,
	                                     const RGBColor& init_high_color,
	                                     const RGBColor& init_alt_low_color,
	                                     const RGBColor& init_alt_medium_color,
	                                     const RGBColor& init_alt_high_color)
	   : building_statistics_button_font_(init_building_statistics_button_font),
	     building_statistics_details_font_(init_building_statistics_details_font),
	     editbox_margin_(init_editbox_margin),
	     census_font_(init_census),
	     statistics_font_(init_statistics),
	     construction_color_(init_construction_color),
	     neutral_color_(init_neutral_color),
	     low_color_(init_low_color),
	     medium_color_(init_medium_color),
	     high_color_(init_high_color),
	     alternative_low_color_(init_alt_low_color),
	     alternative_medium_color_(init_alt_medium_color),
	     alternative_high_color_(init_alt_high_color) {
	}

	const UI::FontStyleInfo& building_statistics_button_font() const {
		return *building_statistics_button_font_.get();
	}
	const UI::FontStyleInfo& building_statistics_details_font() const {
		return *building_statistics_details_font_.get();
	}
	int editbox_margin() const {
		return editbox_margin_;
	}

	const UI::FontStyleInfo& census_font() const {
		return *census_font_.get();
	}
	const UI::FontStyleInfo& statistics_font() const {
		return *statistics_font_.get();
	}
	const RGBColor& construction_color() const {
		return construction_color_;
	}
	const RGBColor& neutral_color() const {
		return neutral_color_;
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
	const RGBColor& alternative_low_color() const {
		return alternative_low_color_;
	}
	const RGBColor& alternative_medium_color() const {
		return alternative_medium_color_;
	}
	const RGBColor& alternative_high_color() const {
		return alternative_high_color_;
	}

private:
	std::unique_ptr<const UI::FontStyleInfo> building_statistics_button_font_;
	std::unique_ptr<const UI::FontStyleInfo> building_statistics_details_font_;
	int editbox_margin_;
	std::unique_ptr<const UI::FontStyleInfo> census_font_;
	std::unique_ptr<const UI::FontStyleInfo> statistics_font_;
	const RGBColor construction_color_;
	const RGBColor neutral_color_;
	const RGBColor low_color_;
	const RGBColor medium_color_;
	const RGBColor high_color_;
	const RGBColor alternative_low_color_;
	const RGBColor alternative_medium_color_;
	const RGBColor alternative_high_color_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_BUILDING_STATISTICS_STYLE_H
