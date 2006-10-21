/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "building_statistics_menu.h"
#include "game_chat_menu.h"
#include "game_main_menu.h"
#include "game_main_menu_save_game.h"
#include "game_main_menu_load_game.h"
#include "game_objectives_menu.h"
#include "game_options_menu.h"
#include "general_statistics_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "stock_menu.h"
#include "ui_unique_window.h"
#include "ware_statistics_menu.h"

GameMainMenu::GameMainMenu
(Interactive_Player & plr,
 UIUniqueWindowRegistry & registry,
 Interactive_Player::Game_Main_Menu_Windows & windows)
:
UIUniqueWindow(&plr, &registry, 180, 160, _("Main Menu")),
m_player      (plr),
m_windows     (windows),
general_stats (this, posx(0, 4), posy(0, 3), buttonw(4), buttonh(3), 4),
ware_stats    (this, posx(1, 4), posy(0, 3), buttonw(4), buttonh(3), 4),
building_stats(this, posx(2, 4), posy(0, 3), buttonw(4), buttonh(3), 4),
stock         (this, posx(3, 4), posy(0, 3), buttonw(4), buttonh(3), 4),
objectives    (this, posx(0, 2), posy(1, 3), buttonw(2), buttonh(3), 4),
chat          (this, posx(1, 2), posy(1, 3), buttonw(2), buttonh(3), 4),
options_menu  (this, posx(0, 1), posy(2, 3), buttonw(1), buttonh(3), 4)
{
	general_stats .set_pic
		(g_gr->get_picture(PicMod_Game, "pics/menu_general_stats.png"));
	ware_stats    .set_pic
		(g_gr->get_picture(PicMod_Game, "pics/menu_ware_stats.png"));
	building_stats.set_pic
		(g_gr->get_picture(PicMod_Game, "pics/menu_building_stats.png"));
	stock.set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_stock.png"));
	objectives    .set_pic
		(g_gr->get_picture(PicMod_Game, "pics/menu_objectives.png"));
	chat.set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_chat.png"));
	options_menu  .set_pic
		(g_gr->get_picture(PicMod_Game, "pics/menu_options_menu.png"));

	general_stats .clicked.set(this, &GameMainMenu::clicked_general_stats);
	ware_stats    .clicked.set(this, &GameMainMenu::clicked_ware_stats);
	building_stats.clicked.set(this, &GameMainMenu::clicked_building_stats);
	stock         .clicked.set(this, &GameMainMenu::clicked_stock);
	objectives    .clicked.set(this, &GameMainMenu::clicked_objectives);
	chat          .clicked.set(this, &GameMainMenu::clicked_chat);
	options_menu  .clicked.set(this, &GameMainMenu::clicked_options_menu);

	general_stats .set_tooltip(_("General statistics") .c_str());
	ware_stats    .set_tooltip(_("Ware statistics")    .c_str());
	building_stats.set_tooltip(_("Building statistics").c_str());
	stock         .set_tooltip(_("Stock")              .c_str());
	objectives    .set_tooltip(_("Objectives")         .c_str());
	chat          .set_tooltip(_("Chat")               .c_str());
	options_menu  .set_tooltip(_("Options")            .c_str());

	if (get_usedefaultpos())
		center_to_parent();
}


void GameMainMenu::clicked_general_stats() {
	if (m_windows.general_stats.window) delete m_windows.general_stats.window;
	else new General_Statistics_Menu(m_player, m_windows.general_stats);
}
void GameMainMenu::clicked_ware_stats() {
	if (m_windows.ware_stats.window) delete m_windows.ware_stats.window;
	else new Ware_Statistics_Menu(m_player, m_windows.ware_stats);
}
void GameMainMenu::clicked_building_stats() {
	if (m_windows.building_stats.window) delete m_windows.building_stats.window;
	else new Building_Statistics_Menu(m_player, m_windows.building_stats);
}
void GameMainMenu::clicked_stock() {
	if (m_windows.stock.window) delete m_windows.stock.window;
	else new Stock_Menu(m_player, m_windows.stock);
}
void GameMainMenu::clicked_objectives() {
	if (m_windows.objectives.window) delete m_windows.objectives.window;
	else new GameObjectivesMenu
		(m_player, m_windows.objectives, *m_player.get_game());
}
void GameMainMenu::clicked_chat() {
	if (m_windows.chat.window) delete m_windows.chat.window;
	else new GameChatMenu
			(m_player, m_windows.chat, m_player.get_game()->get_netgame());
}
void GameMainMenu::clicked_options_menu() {
	if (m_windows.options.window) delete m_windows.options.window;
	else new GameOptionsMenu(m_player, m_windows.options, m_windows);
}
