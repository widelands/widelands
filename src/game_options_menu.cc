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

#include "game_options_menu.h"

#include "building_statistics_menu.h"
#include "fullscreen_menu_fileview.h"
#include "game_options_menu.h"
#include "game_options_sound_menu.h"
#include "game_main_menu_save_game.h"
#include "general_statistics_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "sound/sound_handler.h"
#include "stock_menu.h"
#include "ware_statistics_menu.h"


GameOptionsMenu::GameOptionsMenu
	(Interactive_Player                         & plr,
	 UI::UniqueWindow::Registry                 & registry,
	 Interactive_Player::Game_Main_Menu_Windows & windows)
:
UI::UniqueWindow
	(&plr, &registry,
	 102,
	 vmargin()
	 + 4 * (20 + vspacing()) + 2 * vgap() +
	 35 + vspacing() + 35 +
	 vmargin(),
	 _("Options")),
m_player(plr),
m_windows(windows),
readme
	(this,
	 posx(0, 1),
	 vmargin() + 0 * (20 + vspacing()) + 0 * vgap(),
	 buttonw(1), 20,
	 4,
	 &GameOptionsMenu::clicked_readme, this,
	 _("README")),
license
	(this,
	 posx(0, 1),
	 vmargin() + 1 * (20 + vspacing()) + 0 * vgap(),
	 buttonw(1), 20,
	 4,
	 &GameOptionsMenu::clicked_license, this,
	 _("License")),
authors
	(this,
	 posx(0, 1),
	 vmargin() + 2 * (20 + vspacing()) + 0 * vgap(),
	 buttonw(1), 20,
	 4,
	 &GameOptionsMenu::clicked_authors, this,
	 _("Authors")),
sound
	(this,
	 posx(0, 1),
	 vmargin() + 3 * (20 + vspacing()) + 1 * vgap(),
	 buttonw(1), 20,
	 4,
	 &GameOptionsMenu::clicked_sound, this,
	 _("Sound options")),
save_game
	(this,
	 posx(0, 1),
	 vmargin() + 4 * (20 + vspacing()) + 2 * vgap(),
	 buttonw(1), 35,
	 4,
	 g_gr->get_picture(PicMod_Game, "pics/menu_save_game.png"),
	 &GameOptionsMenu::clicked_save_game, this,
	 _("Save game")),
exit_game
	(this,
	 posx(0, 1),
	 vmargin() + 4 * (20 + vspacing()) + 2 * vgap() +
	 35 + vspacing(),
	 buttonw(1), 35,
	 4,
	 g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.png"),
	 &GameOptionsMenu::clicked_exit_game, this,
	 _("Exit game"))
{
	set_inner_size
		(hmargin() + std::max(get_inner_w(), readme.get_w()) + hmargin(),
		 get_inner_h());
	if (get_usedefaultpos())
		center_to_parent();
}


void GameOptionsMenu::clicked_readme()
{fileview_window(&m_player, &m_windows.readme,  "txts/README");}

void GameOptionsMenu::clicked_license()
{fileview_window(&m_player, &m_windows.license, "txts/COPYING");}

void GameOptionsMenu::clicked_authors()
{fileview_window(&m_player, &m_windows.authors, "txts/developers");}

void GameOptionsMenu::clicked_sound() {
	if (m_windows.sound_options.window) delete m_windows.sound_options.window;
	else new GameOptionsSoundMenu(m_player, m_windows.sound_options);
}

void GameOptionsMenu::clicked_save_game() {
	new Game_Main_Menu_Save_Game(&m_player, &m_windows.savegame);
	die();
}

void GameOptionsMenu::clicked_exit_game() {m_player.end_modal(0);}
