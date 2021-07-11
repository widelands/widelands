/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "logic/widelands.h"
#include "ui_basic/unique_window.h"
#include "wui/plot_area.h"

class FileRead;
class InteractiveBase;
class InteractivePlayer;
struct StatisticWaresDisplay;

namespace UI {
struct Box;
struct TabPanel;
}  // namespace UI

struct WareStatisticsMenu : public UI::UniqueWindow {
public:
	WareStatisticsMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);
	void set_time(int32_t);

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kWareStats;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

protected:
	void layout() override;

private:
	InteractivePlayer& iplayer_;
	UI::Box* main_box_;
	UI::TabPanel* tab_panel_;
	StatisticWaresDisplay* display_;
	WuiPlotAreaSlider* slider_;

	WuiPlotArea* plot_production_;
	WuiPlotArea* plot_consumption_;
	WuiPlotArea* plot_stock_;
	DifferentialPlotArea* plot_economy_;
	std::map<Widelands::DescriptionIndex, uint8_t> color_map_;  // Maps ware index to colors
	std::vector<bool> active_colors_;
	std::list<Widelands::DescriptionIndex> active_indices_;

	void cb_changed_to(Widelands::DescriptionIndex, bool);
};

#endif  // end of include guard: WL_WUI_WARE_STATISTICS_MENU_H
