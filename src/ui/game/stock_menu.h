/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_UI_GAME_STOCK_MENU_H
#define WL_UI_GAME_STOCK_MENU_H

#include "ui/basic/box.h"
#include "ui/basic/checkbox.h"
#include "ui/basic/tabpanel.h"
#include "ui/basic/unique_window.h"
#include "ui/game/waresdisplay.h"

namespace UI {
struct BuildingStatisticsStyleInfo;
}  // namespace UI
class InteractivePlayer;

/*
 * Shows statistics about all stocks currently in the game of
 * one player
 */
struct StockMenu : public UI::UniqueWindow {
	struct Registry : public UI::UniqueWindow::Registry {
		size_t active_tab{2};
		bool solid_icon_backgrounds{false};
	};

	StockMenu(InteractivePlayer&, Registry&);

	void layout() override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kStockMenu;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	const UI::BuildingStatisticsStyleInfo& colors_;
	UI::Box main_box_;
	UI::TabPanel tabs_;
	UI::Checkbox solid_icon_backgrounds_;
	StockMenuWaresDisplay* all_wares_;
	StockMenuWaresDisplay* all_workers_;
	StockMenuWaresDisplay* warehouse_wares_;
	StockMenuWaresDisplay* warehouse_workers_;
};

#endif  // end of include guard: WL_UI_GAME_STOCK_MENU_H
