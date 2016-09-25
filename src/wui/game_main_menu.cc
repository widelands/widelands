/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/game_main_menu.h"

#include <boost/bind.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "ui_basic/unique_window.h"
#include "wui/building_statistics_menu.h"
#include "wui/general_statistics_menu.h"
#include "wui/interactive_player.h"
#include "wui/stock_menu.h"
#include "wui/ware_statistics_menu.h"

GameMainMenu::GameMainMenu(InteractivePlayer& plr,
                           UI::UniqueWindow::Registry& registry,
                           InteractivePlayer::GameMainMenuWindows& windows)
   : UI::UniqueWindow(&plr, "main_menu", &registry, 180, 55, _("Statistics Menu")),
     player_(plr),
     windows_(windows),
     general_stats(this,
                   "general_stats",
                   posx(0, 4),
                   posy(0, 3),
                   buttonw(4),
                   buttonh(1),
                   g_gr->images().get("images/ui_basic/but4.png"),
                   g_gr->images().get("images/wui/menus/menu_general_stats.png"),
                   _("General Statistics")),
     ware_stats(this,
                "ware_stats",
                posx(1, 4),
                posy(0, 3),
                buttonw(4),
                buttonh(1),
                g_gr->images().get("images/ui_basic/but4.png"),
                g_gr->images().get("images/wui/menus/menu_ware_stats.png"),
                _("Ware Statistics")),
     building_stats(this,
                    "building_stats",
                    posx(2, 4),
                    posy(0, 3),
                    buttonw(4),
                    buttonh(1),
                    g_gr->images().get("images/ui_basic/but4.png"),
                    g_gr->images().get("images/wui/menus/menu_building_stats.png"),
                    _("Building Statistics")),
     stock(this,
           "stock",
           posx(3, 4),
           posy(0, 3),
           buttonw(4),
           buttonh(1),
           g_gr->images().get("images/ui_basic/but4.png"),
           g_gr->images().get("images/wui/menus/menu_stock.png"),
           _("Stock")) {
	general_stats.sigclicked.connect(
	   boost::bind(&GeneralStatisticsMenu::Registry::toggle, boost::ref(windows_.general_stats)));
	ware_stats.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(windows_.ware_stats)));
	building_stats.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(windows_.building_stats)));
	stock.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(windows_.stock)));

	windows_.general_stats.assign_toggle_button(&general_stats);
	windows_.ware_stats.assign_toggle_button(&ware_stats);
	windows_.building_stats.assign_toggle_button(&building_stats);
	windows_.stock.assign_toggle_button(&stock);

	windows_.general_stats.open_window = [this] {
		new GeneralStatisticsMenu(player_, windows_.general_stats);
	};
	windows_.ware_stats.open_window = [this] {
		new WareStatisticsMenu(player_, windows_.ware_stats);
	};
	windows_.building_stats.open_window = [this] {
		new BuildingStatisticsMenu(player_, windows_.building_stats);
	};

	if (get_usedefaultpos())
		center_to_parent();
}

GameMainMenu::~GameMainMenu() {
	windows_.general_stats.unassign_toggle_button();
	windows_.ware_stats.unassign_toggle_button();
	windows_.building_stats.unassign_toggle_button();
	windows_.stock.unassign_toggle_button();
}
