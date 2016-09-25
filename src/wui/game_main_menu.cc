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

// NOCOM we have this multiple times - can we refactor?
#define INIT_BTN_HOOKS(registry, btn)                                                              \
	assert(!registry.on_create);                                                                    \
	assert(!registry.on_delete);                                                                    \
	registry.on_create = std::bind(&UI::Button::set_style, &btn, UI::Button::Style::kPermpressed);  \
	registry.on_delete = std::bind(&UI::Button::set_style, &btn, UI::Button::Style::kRaised);       \
	if (registry.window)                                                                            \
		btn.set_style(UI::Button::Style::kPermpressed);

	INIT_BTN_HOOKS(windows_.general_stats, general_stats)
	INIT_BTN_HOOKS(windows_.ware_stats, ware_stats)
	INIT_BTN_HOOKS(windows_.building_stats, building_stats)
	INIT_BTN_HOOKS(windows_.stock, stock)

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
// We need to remove these callbacks because the opened window might
// live longer than 'this' window, and thus the buttons. The assertions
// are safeguards in case somewhere else in the code someone would
// overwrite our hooks.

#define DEINIT_BTN_HOOKS(registry, btn)                                                            \
	registry.on_create = 0;                                                                         \
	registry.on_delete = 0;

	DEINIT_BTN_HOOKS(windows_.general_stats, general_stats)
	DEINIT_BTN_HOOKS(windows_.ware_stats, ware_stats)
	DEINIT_BTN_HOOKS(windows_.building_stats, building_stats)
	DEINIT_BTN_HOOKS(windows_.stock, stock)
}
