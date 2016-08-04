/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_WUI_WARE_STATISTICS_MENU_H
#define WL_WUI_WARE_STATISTICS_MENU_H

#include <vector>

#include "logic/widelands.h"
#include "ui_basic/slider.h"
#include "ui_basic/unique_window.h"
#include "wui/plot_area.h"

struct DifferentialPlotArea;
class InteractivePlayer;
struct WuiPlotArea;

struct WareStatisticsMenu : public UI::UniqueWindow {
public:
	WareStatisticsMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);
	void set_time(int32_t);

private:
	InteractivePlayer* parent_;
	WuiPlotArea* plot_production_;
	WuiPlotArea* plot_consumption_;
	WuiPlotArea* plot_stock_;
	DifferentialPlotArea* plot_economy_;
	std::vector<uint8_t> color_map_;  // Maps ware index to colors
	std::vector<bool> active_colors_;

	void clicked_help();
	void cb_changed_to(Widelands::DescriptionIndex, bool);
};

/**
 * A discrete slider with plot time steps preconfigured, automatic signal
 * setup and the set_time callback function from WareStatisticsMenu.
 *
 */
struct WuiPlotGenericAreaSlider : public UI::DiscreteSlider {
	WuiPlotGenericAreaSlider(Panel* const parent,
	                         WuiPlotArea& plot_area,
	                         WareStatisticsMenu* signal_listener,
	                         const int32_t x,
	                         const int32_t y,
	                         const int w,
	                         const int h,
	                         const Image* background_picture_id,
	                         const std::string& tooltip_text = std::string(),
	                         const uint32_t cursor_size = 20,
	                         const bool enabled = true)
	   : DiscreteSlider(parent,
	                    x,
	                    y,
	                    w,
	                    h,
	                    plot_area.get_labels(),
	                    plot_area.get_time_id(),
	                    background_picture_id,
	                    tooltip_text,
	                    cursor_size,
	                    enabled) {
		changedto.connect(boost::bind(&WareStatisticsMenu::set_time, signal_listener, _1));
	}
};

#endif  // end of include guard: WL_WUI_WARE_STATISTICS_MENU_H
