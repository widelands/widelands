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

#ifndef WL_UI_FSMENU_PLAYERSETUPBOX_H
#define WL_UI_FSMENU_PLAYERSETUPBOX_H

#include <memory>

#include "logic/game_settings.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/textarea.h"
#include "wui/suggested_teams_ui.h"

namespace Widelands {
class Map;
}

class PlayerSetupBox : public UI::Box {

public:
	PlayerSetupBox(UI::Panel* const parent,
	                     GameSettingsProvider* const settings,
	                     uint32_t standard_element_height,
	                     uint32_t padding);

	void reset_teams(const Widelands::Map& map);

protected:
	virtual void update() = 0;
	virtual void reset() = 0;
	virtual void update_player_group(size_t index) = 0;

	void select_teams();
	void check_teams();
	void update_team(PlayerSlot pos);

	uint32_t standard_height_;

	GameSettingsProvider* const settings_;

	UI::Box scrollable_playerbox_;
	UI::Textarea title_;
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;

	SuggestedTeamsDropdown suggested_teams_dropdown_;
	const Widelands::SuggestedTeamLineup* selected_lineup_;
	bool suggested_team_selection_in_progress_;
};
#endif  // WL_UI_FSMENU_PLAYERSETUPBOX_H
