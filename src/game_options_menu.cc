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

#include "game_options_menu.h"

#include "building_statistics_menu.h"
#include "fullscreen_menu_fileview.h"
#include "game_options_menu.h"
#include "game_main_menu_save_game.h"
#include "game_main_menu_load_game.h"
#include "general_statistics_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "sound_handler.h"
#include "stock_menu.h"
#include "ware_statistics_menu.h"


GameOptionsMenu::GameOptionsMenu
(Interactive_Player & plr,
 UI::UniqueWindow::Registry & registry,
 Interactive_Player::Game_Main_Menu_Windows & windows)
:
UI::UniqueWindow
(&plr, &registry,
 102,
 vmargin()
 + 3 * (20 + vspacing()) + 2 * (STATEBOX_HEIGHT + vspacing()) +
 35 + vspacing() + 35 +
 vmargin(),
 _("Options")),
m_player(plr),
m_windows(windows),

readme
(this,
 posx(0, 1),
 vmargin() + 0 * (20 + vspacing()) + 0 * (STATEBOX_HEIGHT + vspacing()),
 buttonw(1), 20,
 4,
 &GameOptionsMenu::clicked_readme, this,
 _("README")),

license
(this,
 posx(0, 1),
 vmargin() + 1 * (20 + vspacing()) + 0 * (STATEBOX_HEIGHT + vspacing()),
 buttonw(1), 20,
 4,
 &GameOptionsMenu::clicked_license, this,
 _("License")),

authors
(this,
 posx(0, 1),
 vmargin() + 2 * (20 + vspacing()) + 0 * (STATEBOX_HEIGHT + vspacing()),
 buttonw(1), 20,
 4,
 &GameOptionsMenu::clicked_authors, this,
 _("Authors")),

ingame_music
(this, hmargin(),
 vmargin() + 3 * (20 + vspacing()) + 0 * (STATEBOX_HEIGHT + vspacing())),
ingame_music_label
(this, hmargin () + STATEBOX_WIDTH + hspacing(),
 vmargin() + 3 * (20 + vspacing()) + 0 * (STATEBOX_HEIGHT + vspacing()),
 _("Ingame Music")),
ingame_sound
(this, hmargin(),
 vmargin() + 3 * (20 + vspacing()) + 1 * (STATEBOX_HEIGHT + vspacing())),
ingame_sound_label
(this, hmargin () + STATEBOX_WIDTH + hspacing(),
 vmargin() + 3 * (20 + vspacing()) + 1 * (STATEBOX_HEIGHT + vspacing()),
 _("Sound FX")),

save_game
(this,
 posx(0, 2),
 vmargin() + 3 * (20 + vspacing()) + 2 * (STATEBOX_HEIGHT + vspacing()),
 buttonw(2), 35,
 4,
 g_gr->get_picture(PicMod_Game, "pics/menu_save_game.png"),
 &GameOptionsMenu::clicked_save_game, this,
 _("Save game")),

load_game
(this,
 posx(1, 2),
 vmargin() + 3 * (20 + vspacing()) + 2 * (STATEBOX_HEIGHT + vspacing()),
 buttonw(2), 35,
 4,
 g_gr->get_picture(PicMod_Game, "pics/menu_load_game.png"),
 &GameOptionsMenu::clicked_load_game, this,
 _("Load game")),

exit_game
(this,
 posx(0, 1),
 vmargin() + 3 * (20 + vspacing()) + 2 * (STATEBOX_HEIGHT + vspacing()) +
 35 + vspacing(),
 buttonw(1), 35,
 4,
 g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.png"),
 &GameOptionsMenu::clicked_exit_game, this,
 _("Exit game"))

{
	ingame_music.changedto.set(this, &GameOptionsMenu::changed_ingame_music);
	ingame_sound.changedto.set(this, &GameOptionsMenu::changed_ingame_sound);

	ingame_music.set_state(not g_sound_handler.get_disable_music());
	ingame_sound.set_state(not g_sound_handler.get_disable_fx());

	if (g_sound_handler.m_lock_audio_disabling) {
		ingame_music.set_enabled(false);
		ingame_sound.set_enabled(false);
   }

	set_inner_size
		(hmargin()
		 + STATEBOX_WIDTH + hspacing() +
		 std::max
		 (get_inner_w(),
		  std::max(ingame_music_label.get_w(), ingame_sound_label.get_w()))
		 +
		 hmargin(),
		 get_inner_h());
	if (get_usedefaultpos())
		center_to_parent();
}


void GameOptionsMenu::clicked_readme()
{fileview_window(&m_player, &m_windows.readme,  "txts/README");}

void GameOptionsMenu::clicked_license()
{fileview_window(&m_player, &m_windows.licence, "txts/COPYING");}

void GameOptionsMenu::clicked_authors()
{fileview_window(&m_player, &m_windows.authors, "txts/developers");}

void GameOptionsMenu::clicked_save_game() {
	new Game_Main_Menu_Save_Game(&m_player, &m_windows.savegame);
	die();
}

void GameOptionsMenu::clicked_load_game() {
	new Game_Main_Menu_Load_Game(&m_player, &m_windows.loadgame);
	die();
}

void GameOptionsMenu::clicked_exit_game() {m_player.end_modal(0);}


/*
 * One of the checkboxes have been toggled
 */
void GameOptionsMenu::changed_ingame_music(bool t) {
   g_sound_handler.set_disable_music( !t );
}

void GameOptionsMenu::changed_ingame_sound(bool t) {
   g_sound_handler.set_disable_fx( !t );
}
