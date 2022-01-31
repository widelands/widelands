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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_STYLES_STATISTICS_PLOT_STYLE_H
#define WL_GRAPHIC_STYLES_STATISTICS_PLOT_STYLE_H

#include <memory>

#include "graphic/color.h"
#include "graphic/styles/font_style.h"

namespace UI {

struct StatisticsPlotStyleInfo {
	explicit StatisticsPlotStyleInfo(UI::FontStyleInfo* init_x_tick_font,
	                                 UI::FontStyleInfo* init_y_min_value_font,
	                                 UI::FontStyleInfo* init_y_max_value_font,
	                                 const RGBColor& init_axis_line_color,
	                                 const RGBColor& init_zero_line_color)
	   : x_tick_font_(init_x_tick_font),
	     y_min_value_font_(init_y_min_value_font),
	     y_max_value_font_(init_y_max_value_font),
	     axis_line_color_(init_axis_line_color),
	     zero_line_color_(init_zero_line_color) {
	}

	const UI::FontStyleInfo& x_tick_font() const {
		return *x_tick_font_.get();
	}
	const UI::FontStyleInfo& y_min_value_font() const {
		return *y_min_value_font_.get();
	}
	const UI::FontStyleInfo& y_max_value_font() const {
		return *y_max_value_font_.get();
	}
	const RGBColor& axis_line_color() const {
		return axis_line_color_;
	}
	const RGBColor& zero_line_color() const {
		return zero_line_color_;
	}

private:
	std::unique_ptr<const UI::FontStyleInfo> x_tick_font_;
	std::unique_ptr<const UI::FontStyleInfo> y_min_value_font_;
	std::unique_ptr<const UI::FontStyleInfo> y_max_value_font_;
	const RGBColor axis_line_color_;
	const RGBColor zero_line_color_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_STATISTICS_PLOT_STYLE_H
