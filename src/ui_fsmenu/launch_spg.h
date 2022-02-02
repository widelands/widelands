/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_UI_FSMENU_LAUNCH_SPG_H
#define WL_UI_FSMENU_LAUNCH_SPG_H

#include <memory>

#include "logic/game_settings.h"
#include "ui_fsmenu/launch_game.h"
#include "ui_fsmenu/singleplayersetupbox.h"

struct MapData;
namespace Widelands {
class Game;
}

namespace FsMenu {

class LaunchSPG : public LaunchGame {
public:
	LaunchSPG(MenuCapsule&,
	          GameSettingsProvider&,
	          Widelands::Game&,
	          const MapData* /* nullptr for preconfigured games */,
	          bool scenario);
	~LaunchSPG() override = default;

protected:
	void clicked_ok() override;
	void clicked_select_map() override {
		NEVER_HERE();  // not available in singleplayer
	}
	void clicked_select_savegame() override {
		NEVER_HERE();  // not available in singleplayer
	}

private:
	void win_condition_selected() override;
	void layout() override;

	SinglePlayerSetupBox player_setup_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	void update();
	void enforce_player_names_and_tribes(const Widelands::Map& map);
	const bool preconfigured_;
	Widelands::Game& game_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_LAUNCH_SPG_H
