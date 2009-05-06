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

#include "game_main_menu.h"

#include "building_statistics_menu.h"
#include "general_statistics_menu.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "stock_menu.h"
#include "ware_statistics_menu.h"

#include "ui/ui_basic/ui_unique_window.h"


GameMainMenu::GameMainMenu
	(Interactive_Player                         & plr,
	 UI::UniqueWindow::Registry                 & registry,
	 Interactive_Player::Game_Main_Menu_Windows & windows)
:
UI::UniqueWindow(&plr, &registry, 180, 55, _("Main Menu")),
m_player      (plr),
m_windows     (windows),
general_stats
	(this,
	 posx(0, 4), posy(0, 3), buttonw(4), buttonh(1),
	 4,
	 g_gr->get_picture(PicMod_Game, "pics/menu_general_stats.png"),
	 &GameMainMenu::clicked_general_stats, *this,
	 _("General Statistics")),
ware_stats
	(this,
	 posx(1, 4), posy(0, 3), buttonw(4), buttonh(1),
	 4,
	 g_gr->get_picture(PicMod_Game, "pics/menu_ware_stats.png"),
	 &GameMainMenu::clicked_ware_stats, *this,
	 _("Ware Statistics")),
building_stats
	(this,
	 posx(2, 4), posy(0, 3), buttonw(4), buttonh(1),
	 4,
	 g_gr->get_picture(PicMod_Game, "pics/menu_building_stats.png"),
	 &GameMainMenu::clicked_building_stats, *this,
	 _("Building Statistics")),
stock
	(this,
	 posx(3, 4), posy(0, 3), buttonw(4), buttonh(1),
	 4,
	 g_gr->get_picture(PicMod_Game, "pics/menu_stock.png"),
	 &GameMainMenu::clicked_stock, *this,
	 _("Stock"))
{
	if (get_usedefaultpos())
		center_to_parent();
}


void GameMainMenu::clicked_general_stats() {
	if (m_windows.general_stats.window)
		delete m_windows.general_stats.window;
	else
		new General_Statistics_Menu(m_player, m_windows.general_stats);
}
void GameMainMenu::clicked_ware_stats() {
	if (m_windows.ware_stats.window)
		delete m_windows.ware_stats.window;
	else
		new Ware_Statistics_Menu(m_player, m_windows.ware_stats);
}
void GameMainMenu::clicked_building_stats() {
	if (m_windows.building_stats.window)
		delete m_windows.building_stats.window;
	else
		new Building_Statistics_Menu(m_player, m_windows.building_stats);
}
void GameMainMenu::clicked_stock() {
	if (m_windows.stock.window)
		delete m_windows.stock.window;
	else
		new Stock_Menu(m_player, m_windows.stock);
}
