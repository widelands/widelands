/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#include <boost/bind.hpp>
#include <boost/type_traits.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/bind.hpp>

#include "game_main_menu.h"

#include "building_statistics_menu.h"
#include "general_statistics_menu.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "stock_menu.h"
#include "ware_statistics_menu.h"

#include "ui_basic/unique_window.h"


GameMainMenu::GameMainMenu
	(Interactive_Player                         & plr,
	 UI::UniqueWindow::Registry                 & registry,
	 Interactive_Player::Game_Main_Menu_Windows & windows)
:
UI::UniqueWindow(&plr, "main_menu", &registry, 180, 55, _("Main Menu")),
m_player      (plr),
m_windows     (windows),
general_stats
	(this, "general_stats",
	 posx(0, 4), posy(0, 3), buttonw(4), buttonh(1),
	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
	 g_gr->get_picture(PicMod_Game, "pics/menu_general_stats.png"),
	 boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.general_stats)),
	 _("General Statistics")),
ware_stats
	(this, "ware_stats",
	 posx(1, 4), posy(0, 3), buttonw(4), buttonh(1),
	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
	 g_gr->get_picture(PicMod_Game, "pics/menu_ware_stats.png"),
	 boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.ware_stats)),
	 _("Ware Statistics")),
building_stats
	(this, "building_stats",
	 posx(2, 4), posy(0, 3), buttonw(4), buttonh(1),
	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
	 g_gr->get_picture(PicMod_Game, "pics/menu_building_stats.png"),
	 boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.building_stats)),
	 _("Building Statistics")),
stock
	(this, "stock",
	 posx(3, 4), posy(0, 3), buttonw(4), buttonh(1),
	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
	 g_gr->get_picture(PicMod_Game, "pics/menu_stock.png"),
	 boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.stock)),
	 _("Stock"))
{
#define INIT_BTN_HOOKS(registry, btn)                                        \
 assert (not registry.onCreate);                                             \
 assert (not registry.onDelete);                                             \
 registry.onCreate = boost::bind(&UI::Button::set_perm_pressed, &btn, true);  \
 registry.onDelete = boost::bind(&UI::Button::set_perm_pressed, &btn, false); \
 if (registry.window) btn.set_perm_pressed(true);                            \

	INIT_BTN_HOOKS(m_windows.general_stats, general_stats)
	INIT_BTN_HOOKS(m_windows.ware_stats, ware_stats)
	INIT_BTN_HOOKS(m_windows.building_stats, building_stats)
	INIT_BTN_HOOKS(m_windows.stock, stock)

	m_windows.general_stats.constr = boost::lambda::bind
		(boost::lambda::new_ptr<General_Statistics_Menu>(), boost::ref(m_player), boost::lambda::_1);
	m_windows.ware_stats.constr = boost::lambda::bind
		(boost::lambda::new_ptr<Ware_Statistics_Menu>(), boost::ref(m_player), boost::lambda::_1);
	m_windows.building_stats.constr = boost::lambda::bind
		(boost::lambda::new_ptr<Building_Statistics_Menu>(), boost::ref(m_player), boost::lambda::_1);
	m_windows.stock.constr = boost::lambda::bind
		(boost::lambda::new_ptr<Stock_Menu>(), boost::ref(m_player), boost::lambda::_1);

	if (get_usedefaultpos())
		center_to_parent();
}

GameMainMenu::~GameMainMenu() {
	// We need to remove these callbacks because the opened window might
	// live longer than 'this' window, and thus the buttons. The assertions
	// are safeguards in case somewhere else in the code someone would
	// overwrite our hooks.

#define DEINIT_BTN_HOOKS(registry, btn)                                                \
 assert (registry.onCreate == boost::bind(&UI::Button::set_perm_pressed, &btn, true));  \
 assert (registry.onDelete == boost::bind(&UI::Button::set_perm_pressed, &btn, false)); \
 registry.onCreate = 0;                                                                \
 registry.onDelete = 0;                                                                \

	DEINIT_BTN_HOOKS(m_windows.general_stats, general_stats)
	DEINIT_BTN_HOOKS(m_windows.ware_stats, ware_stats)
	DEINIT_BTN_HOOKS(m_windows.building_stats, building_stats)
	DEINIT_BTN_HOOKS(m_windows.stock, stock)
}
