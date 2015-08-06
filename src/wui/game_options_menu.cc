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

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "sound/sound_handler.h"
#include "ui_fsmenu/fileview.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_sound_menu.h"
#include "wui/unique_window_handler.h"

class GameOptionsMenuExitConfirmBox : public UI::WLMessageBox {
public:
	GameOptionsMenuExitConfirmBox(UI::Panel& parent, InteractiveGameBase& gb)
		: UI::WLMessageBox(&parent,
								 /** TRANSLATORS: Window label when "Exit game" has been pressed */
								 _("Exit Game Confirmation"),
								 _("Are you sure you wish to exit this game?"),
								 MBoxType::kOkCancel),
		  m_gb(gb) {
	}

	void clicked_ok() override {
		m_gb.end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}

	void clicked_back() override {
		die();
	}

private:
	InteractiveGameBase& m_gb;
};

GameOptionsMenu::GameOptionsMenu
	(InteractiveGameBase                      & gb,
	 UI::UniqueWindow::Registry               & registry,
	 InteractiveGameBase::GameMainMenuWindows & windows)
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
		 _("README"),
		/** TRANSLATORS: Button tooltip */
		_("Show general information about Widelands and keyboard shortcuts")),
	license
		(this, "license",
		 posx(0, 1),
		 vmargin() + 1 * (20 + vspacing()) + 0 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("License"),
		/** TRANSLATORS: Button tooltip */
		_("Show the distribution licence document")),
	authors
		(this, "authors",
		 posx(0, 1),
		 vmargin() + 2 * (20 + vspacing()) + 0 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("Authors"),
		/** TRANSLATORS: Button tooltip */
		_("Show information about the Widelands Development Team")),
	sound
		(this, "sound_options",
		 posx(0, 1),
		 vmargin() + 3 * (20 + vspacing()) + 1 * vgap(),
		 buttonw(1), 20,
		 g_gr->images().get("pics/but4.png"),
		 _("Sound Options"),
		/** TRANSLATORS: Button tooltip */
		_("Set sound effect and music options")),
	save_game
		(this, "save_game",
		 posx(0, 1),
		 vmargin() + 4 * (20 + vspacing()) + 2 * vgap(),
		 buttonw(1), 35,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/menu_save_game.png"),
		 /** TRANSLATORS: Button tooltip */
		 _("Save Game")),
	exit_game
		(this, "exit_game",
		 posx(0, 1),
		 vmargin() + 4 * (20 + vspacing()) + 2 * vgap() +
		 35 + vspacing(),
		 buttonw(1), 35,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/menu_exit_game.png"),
		 /** TRANSLATORS: Button tooltip */
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


	m_windows.readme.open_window = boost::bind
		(&fileview_window, boost::ref(m_gb),
		 boost::ref(m_windows.readme),
		 "txts/README.lua");
	m_windows.license.open_window = boost::bind
		(&fileview_window, boost::ref(m_gb),
		 boost::ref(m_windows.license),
		 "txts/LICENSE.lua");
	m_windows.authors.open_window = boost::bind
		(&fileview_window, boost::ref(m_gb),
		 boost::ref(m_windows.license),
		 "txts/AUTHORS.lua");

#define INIT_BTN_HOOKS(registry, btn)                                        \
 registry.on_create = std::bind(&UI::Button::set_perm_pressed, &btn, true);  \
 registry.on_delete = std::bind(&UI::Button::set_perm_pressed, &btn, false); \
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
	new GameMainMenuSaveGame(m_gb, m_windows.savegame);
	die();
}

void GameOptionsMenu::clicked_exit_game() {
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		m_gb.end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}
	else {
		new GameOptionsMenuExitConfirmBox(*get_parent(), m_gb);
		die();
	}
}
