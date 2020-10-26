/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_LAUNCH_SPG_H
#define WL_UI_FSMENU_LAUNCH_SPG_H

#include <memory>

#include "logic/game_settings.h"
#include "ui_fsmenu/launch_game.h"
#include "ui_fsmenu/singleplayersetupbox.h"

namespace Widelands {
class Game;
}

class FullscreenMenuLaunchSPG : public FullscreenMenuLaunchGame {
public:
	FullscreenMenuLaunchSPG(GameSettingsProvider*,
	                        Widelands::EditorGameBase& egbase,
	                        bool preconfigured,
	                        GameController* = nullptr);
	~FullscreenMenuLaunchSPG() override;

	void start() override;

protected:
	void clicked_ok() override;
	void clicked_back() override;
	bool clicked_select_map() override;

private:
	void win_condition_selected() override;
	void layout() override;

	SinglePlayerSetupBox player_setup;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void update();
	void enforce_player_names_and_tribes(Widelands::Map& map);
	const bool preconfigured_;
	Widelands::EditorGameBase& egbase_;  // Not owned
};

#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG_H
