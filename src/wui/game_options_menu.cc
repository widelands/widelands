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
#include "wui/game_main_menu_save_game.h"
#include "wui/game_options_sound_menu.h"
#include "wui/unique_window_handler.h"

#define width 200
#define margin 10
#define vspacing 5
#define vgap 3

class GameOptionsMenuExitConfirmBox : public UI::WLMessageBox {
public:
	// TODO(GunChleoc): Arabic: Buttons need more height for Arabic
	GameOptionsMenuExitConfirmBox(UI::Panel& parent, InteractiveGameBase& gb)
		: UI::WLMessageBox(&parent,
								 /** TRANSLATORS: Window label when "Exit game" has been pressed */
								 _("Exit Game Confirmation"),
								 _("Are you sure you wish to exit this game?"),
								 MBoxType::kOkCancel),
		  igb_(gb) {
	}

	void clicked_ok() override {
		igb_.end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}

	void clicked_back() override {
		die();
	}

private:
	InteractiveGameBase& igb_;
};

GameOptionsMenu::GameOptionsMenu
	(InteractiveGameBase                      & gb,
	 UI::UniqueWindow::Registry               & registry,
	 InteractiveGameBase::GameMainMenuWindows & windows)
:
	UI::UniqueWindow(&gb, "options", &registry, 2 * margin + width, 0, _("Options")),
	igb_(gb),
	windows_(windows),
	box_(this, margin, margin, UI::Box::Vertical,
		  width, get_h() - 2 * margin, vspacing),
	sound_
		(&box_, "sound_options",
		 0, 0, width, 0,
		 g_gr->images().get("images/ui_basic/but4.png"),
		 _("Sound Options"),
		/** TRANSLATORS: Button tooltip */
		_("Set sound effect and music options")),
	save_game_
		(&box_, "save_game",
		 0, 0, width, 35,
		 g_gr->images().get("images/ui_basic/but4.png"),
		 g_gr->images().get("images/wui/menus/menu_save_game.png"),
		 /** TRANSLATORS: Button tooltip */
		 _("Save Game")),
	exit_game_
		(&box_, "exit_game",
		 0, 0, width, 35,
		 g_gr->images().get("images/ui_basic/but4.png"),
		 g_gr->images().get("images/wui/menus/menu_exit_game.png"),
		 /** TRANSLATORS: Button tooltip */
		 _("Exit Game"))
{
	box_.add(&sound_, UI::Align::kHCenter);
	box_.add_space(vgap);
	box_.add(&save_game_, UI::Align::kHCenter);
	box_.add(&exit_game_, UI::Align::kHCenter);
	box_.set_size(width, sound_.get_h() + 2 * save_game_.get_h() + vgap + 3 * vspacing);
	set_inner_size(get_inner_w(), box_.get_h() + 2 * margin);

	sound_.sigclicked.connect(boost::bind(&GameOptionsMenu::clicked_sound, boost::ref(*this)));
	save_game_.sigclicked.connect(boost::bind(&GameOptionsMenu::clicked_save_game, boost::ref(*this)));
	exit_game_.sigclicked.connect(boost::bind(&GameOptionsMenu::clicked_exit_game, boost::ref(*this)));

#define INIT_BTN_HOOKS(registry, btn)                                        \
 registry.on_create = std::bind(&UI::Button::set_perm_pressed, &btn, true);  \
 registry.on_delete = std::bind(&UI::Button::set_perm_pressed, &btn, false); \
 if (registry.window) btn.set_perm_pressed(true);                            \

	INIT_BTN_HOOKS(windows_.sound_options, sound_)

	if (get_usedefaultpos())
		center_to_parent();
}


void GameOptionsMenu::clicked_sound() {
	if (windows_.sound_options.window)
		delete windows_.sound_options.window;
	else
		new GameOptionsSoundMenu(igb_, windows_.sound_options);
}

void GameOptionsMenu::clicked_save_game() {
	new GameMainMenuSaveGame(igb_, windows_.savegame);
	die();
}

void GameOptionsMenu::clicked_exit_game() {
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		igb_.end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}
	else {
		new GameOptionsMenuExitConfirmBox(*get_parent(), igb_);
		die();
	}
}
