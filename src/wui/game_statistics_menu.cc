/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "wui/game_statistics_menu.h"

#include <boost/bind.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "ui_basic/unique_window.h"
#include "wui/building_statistics_menu.h"
#include "wui/general_statistics_menu.h"
#include "wui/ship_statistics_menu.h"
#include "wui/interactive_player.h"
#include "wui/stock_menu.h"
#include "wui/ware_statistics_menu.h"

GameStatisticsMenu::GameStatisticsMenu(InteractivePlayer& plr,
                                       UI::UniqueWindow::Registry& registry,
                                       InteractivePlayer::GameMainMenuWindows& windows)
   : UI::UniqueWindow(&plr, "main_menu", &registry, 0, 0, _("Statistics Menu")),
     player_(plr),
     windows_(windows),
     box_(this, 0, 0, UI::Box::Horizontal, 0, 0, 5) {
	add_button("wui/menus/menu_general_stats", "general_stats", _("General Statistics"),
	           &windows_.general_stats);
	add_button(
	   "wui/menus/menu_ware_stats", "ware_stats", _("Ware Statistics"), &windows_.ware_stats);
	add_button("wui/menus/menu_building_stats", "building_stats", _("Building Statistics"),
	           &windows_.building_stats);
	add_button("wui/menus/menu_stock", "stock", _("Stock"), &windows_.stock);
	add_button("wui/ship/ship_scout_ne", "ship_stats", _("Ship Statistics"),
				  &windows_.ship_stats);
	box_.set_pos(Vector2i(10, 10));
	box_.set_size((34 + 5) * 5, 34);
	set_inner_size(box_.get_w() + 20, box_.get_h() + 20);

	windows_.general_stats.open_window = [this] {
		new GeneralStatisticsMenu(player_, windows_.general_stats);
	};
	windows_.ware_stats.open_window = [this] {
		new WareStatisticsMenu(player_, windows_.ware_stats);
	};
	windows_.building_stats.open_window = [this] {
		new BuildingStatisticsMenu(player_, windows_.building_stats);
	};
	windows_.ship_stats.open_window = [this] {
		new ShipStatisticsMenu(player_, windows_.ship_stats);
	};
	// The stock window is defined in InteractivePlayer because of the keyboard shortcut.

	if (get_usedefaultpos())
		center_to_parent();
}

UI::Button* GameStatisticsMenu::add_button(const std::string& image_basename,
                                           const std::string& name,
                                           const std::string& tooltip_text,
                                           UI::UniqueWindow::Registry* window) {
	UI::Button* button =
	   new UI::Button(&box_, name, 0, 0, 34U, 34U, g_gr->images().get("images/ui_basic/but4.png"),
	                  g_gr->images().get("images/" + image_basename + ".png"), tooltip_text);
	box_.add(button, UI::Align::kLeft);
	if (window) {
		if (!window->on_create) {
			window->assign_toggle_button(button);
			registries_.push_back(*window);
		}
		button->sigclicked.connect(
		   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(*window)));
	}
	return button;
}

GameStatisticsMenu::~GameStatisticsMenu() {
	for (auto& registry : registries_) {
		registry.unassign_toggle_button();
	}
}
