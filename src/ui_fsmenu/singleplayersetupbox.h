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

#ifndef WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
#define WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H

#include <memory>
#include <ui_basic/textarea.h>
#include <vector>

#include "logic/game_settings.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_fsmenu/singleplayerdropdown.h"
#include "wui/suggested_teams_ui.h"

namespace Widelands {
class Map;
}

// horizontal group
class SinglePlayerActivePlayerGroup : public UI::Box {
public:
	SinglePlayerActivePlayerGroup(UI::Panel* const parent,
	                              int32_t const w,
	                              int32_t const h,
	                              PlayerSlot id,
	                              GameSettingsProvider* const settings);

	void update();
	void force_new_dimensions(float scale, uint32_t standard_element_height);

private:
	PlayerSlot id_;
	GameSettingsProvider* const settings_;
	UI::Button player_;
	SinglePlayerPlayerTypeDropdown player_type_;
	SinglePlayerTribeDropdown tribe_;
	SinglePlayerStartTypeDropdown start_type;
	SinglePlayerTeamDropdown teams_;
};

class SinglePlayerSetupBox : public UI::Box {

public:
	SinglePlayerSetupBox(UI::Panel* const parent,
	                     GameSettingsProvider* const settings,
	                     uint32_t standard_element_height,
	                     uint32_t padding);

	void force_new_dimensions(float scale, uint32_t standard_element_height);
	void reset_teams(const Widelands::Map& map);

private:
	void update();
	void reset();

	void select_teams();
	void check_teams();
	void update_team(PlayerSlot pos);

	GameSettingsProvider* const settings_;
	uint32_t standard_height;
	UI::Box scrollable_playerbox;
	UI::Textarea title_;
	UI::Textarea title2_;
	std::vector<SinglePlayerActivePlayerGroup*> active_player_groups;  // not owned
	std::unique_ptr<Notifications::Subscriber<NoteGameSettings>> subscriber_;


	SuggestedTeamsDropdown suggested_teams_dropdown_;
	const Widelands::SuggestedTeamLineup* selected_lineup_;
};
#endif  // WL_UI_FSMENU_SINGLEPLAYERSETUPBOX_H
