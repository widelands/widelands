/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_WUI_STOCK_MENU_H
#define WL_WUI_STOCK_MENU_H

#include "ui_basic/unique_window.h"
#include "wui/waresdisplay.h"

class InteractivePlayer;

/*
 * Shows statistics about all stocks currently in the game of
 * one player
 */
struct StockMenu : public UI::UniqueWindow {
	StockMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);

	void think() override;

private:
	InteractivePlayer& player_;
	WaresDisplay* all_wares_;
	WaresDisplay* all_workers_;
	WaresDisplay* warehouse_wares_;
	WaresDisplay* warehouse_workers_;

	void fill_total_waresdisplay(WaresDisplay* waresdisplay, Widelands::WareWorker type);
	void fill_warehouse_waresdisplay(WaresDisplay* waresdisplay, Widelands::WareWorker type);
};

#endif  // end of include guard: WL_WUI_STOCK_MENU_H
