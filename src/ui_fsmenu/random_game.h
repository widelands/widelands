/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_RANDOM_GAME_H
#define WL_UI_FSMENU_RANDOM_GAME_H

#include <memory>

#include "editor/ui_menus/main_menu_random_map.h"
#include "logic/game.h"
#include "logic/single_player_game_settings_provider.h"
#include "ui_basic/icon.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/menu.h"

namespace FsMenu {

class RandomGame : public TwoColumnsFullNavigationMenu {
public:
	explicit RandomGame(MenuCapsule&);
	~RandomGame() override;

	void clicked_ok() override;
	void reactivated() override;
	void layout() override;

private:
	MainMenuNewRandomMapPanel menu_;
	Widelands::Game game_;
	SinglePlayerGameSettingsProvider settings_;
	UI::Icon icon_;
	UI::ProgressWindow* progress_window_;  // not owned
};

}  // namespace FsMenu

#endif  // WL_UI_FSMENU_RANDOM_GAME_H
