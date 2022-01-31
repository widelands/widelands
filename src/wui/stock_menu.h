/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_WUI_STOCK_MENU_H
#define WL_WUI_STOCK_MENU_H

#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "wui/waresdisplay.h"

namespace UI {
struct BuildingStatisticsStyleInfo;
}
class InteractivePlayer;

/*
 * Shows statistics about all stocks currently in the game of
 * one player
 */
struct StockMenu : public UI::UniqueWindow {
	StockMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);

	void think() override;
	void layout() override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kStockMenu;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	InteractivePlayer& player_;
	const UI::BuildingStatisticsStyleInfo& colors_;
	UI::Box main_box_;
	UI::TabPanel tabs_;
	UI::Checkbox solid_icon_backgrounds_;
	StockMenuWaresDisplay* all_wares_;
	StockMenuWaresDisplay* all_workers_;
	StockMenuWaresDisplay* warehouse_wares_;
	StockMenuWaresDisplay* warehouse_workers_;

	void fill_total_waresdisplay(WaresDisplay* waresdisplay, Widelands::WareWorker type);
	void fill_warehouse_waresdisplay(WaresDisplay* waresdisplay, Widelands::WareWorker type);
};

#endif  // end of include guard: WL_WUI_STOCK_MENU_H
