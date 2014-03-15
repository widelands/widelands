/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#include "wui/game_options_menu.h"

#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/type_traits.hpp>

#include "graphic/graphic.h"
#include "i18n.h"
#include "sound/sound_handler.h"
#include "ui_fsmenu/fileview.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_sound_menu.h"

GameOptionsMenu::GameOptionsMenu
	(Interactive_GameBase                         & gb,
	 UI::UniqueWindow::Registry                   & registry,
	 Interactive_GameBase::Game_Main_Menu_Windows & windows)
:
	UI::UniqueWindow
		(&gb, "options", &registry,
		 145,
		 vmargin()
		 + 4 * (20 + vspacing()) + 2 * vgap() +
		 35 + vspacing() + 35 +
		 vmargin(),
		 _("Options")),
	m_gb(gb),
	m_windows(windows),
	readme
		(this, "readme",
		 posx(0, 1),
		 vmargin() + 0 * (20 + vspacing()) + 0 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("README")),
	license
		(this, "license",
		 posx(0, 1),
		 vmargin() + 1 * (20 + vspacing()) + 0 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("License")),
	authors
		(this, "authors",
		 posx(0, 1),
		 vmargin() + 2 * (20 + vspacing()) + 0 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("Authors")),
	sound
		(this, "sound_options",
		 posx(0, 1),
		 vmargin() + 3 * (20 + vspacing()) + 1 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("Sound Options")),
	save_game
		(this, "save_game",
		 posx(0, 1),
		 vmargin() + 4 * (20 + vspacing()) + 2 * vgap(),
		 buttonw(1), 35,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/menu_save_game.png"),
		 _("Save Game")),
	exit_game
		(this, "exit_game",
		 posx(0, 1),
		 vmargin() + 4 * (20 + vspacing()) + 2 * vgap() +
		 35 + vspacing(),
		 buttonw(1), 35,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/menu_exit_game.png"),
		 _("Exit Game"))
{
	readme.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.readme)));
	license.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.license)));
	authors.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_windows.authors)));
	sound.sigclicked.connect(boost::bind(&GameOptionsMenu::clicked_sound, boost::ref(*this)));
	save_game.sigclicked.connect(boost::bind(&GameOptionsMenu::clicked_save_game, boost::ref(*this)));
	exit_game.sigclicked.connect(boost::bind(&GameOptionsMenu::clicked_exit_game, boost::ref(*this)));


	m_windows.readme.constr = boost::bind
		(&fileview_window, boost::ref(m_gb),
		 boost::ref(m_windows.readme),
		 "txts/README.lua");
	m_windows.license.constr = boost::bind
		(&fileview_window, boost::ref(m_gb),
		 boost::ref(m_windows.license),
		 "txts/license");
	m_windows.authors.constr = boost::bind
		(&fileview_window, boost::ref(m_gb),
		 boost::ref(m_windows.authors),
		 "txts/developers");

#define INIT_BTN_HOOKS(registry, btn)                                        \
 registry.onCreate = boost::bind(&UI::Button::set_perm_pressed, &btn, true);  \
 registry.onDelete = boost::bind(&UI::Button::set_perm_pressed, &btn, false); \
 if (registry.window) btn.set_perm_pressed(true);                            \

	INIT_BTN_HOOKS(m_windows.readme, readme)
	INIT_BTN_HOOKS(m_windows.license, license)
	INIT_BTN_HOOKS(m_windows.authors, authors)
	INIT_BTN_HOOKS(m_windows.sound_options, sound)

	set_inner_size
		(hmargin() + hmargin() +
		 std::max<int32_t>(get_inner_w(), readme.get_w()),
		 get_inner_h());
	if (get_usedefaultpos())
		center_to_parent();
}

void GameOptionsMenu::clicked_sound() {
	if (m_windows.sound_options.window)
		delete m_windows.sound_options.window;
	else
		new GameOptionsSoundMenu(m_gb, m_windows.sound_options);
}

void GameOptionsMenu::clicked_save_game() {
	new Game_Main_Menu_Save_Game(m_gb, m_windows.savegame);
	die();
}

void GameOptionsMenu::clicked_exit_game() {m_gb.end_modal(0);}
