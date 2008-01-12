/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__PLAYER_DESCR_GROUP
#define __S__PLAYER_DESCR_GROUP

#include "widelands.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_signal.h"
#include "ui_textarea.h"

#include <vector>

namespace Widelands {struct Game;};

/** class PlayerDescriptionGroup
 *
 * - checkbox to enable/disable player
 * - button to switch between: Human, Remote, AI
 */
struct PlayerDescriptionGroup : public UI::Panel {
	enum changemode_t {
		CHANGE_NOTHING=0,
		CHANGE_ENABLED=1,
		CHANGE_TRIBE=2,
		CHANGE_EVERYTHING=3
	};

	PlayerDescriptionGroup
		(UI::Panel              * parent,
		 int32_t x, int32_t y,
		 Widelands::Game        * game,
		 Widelands::Player_Number plnum,
		 bool                     highlight = false);

	UI::Signal changed;

	void set_enabled(bool enable);
	void allow_changes(changemode_t);
	void enable_player(bool on);
	void set_player_tribe(std::string str);
	void set_player_name(std::string name);
	void set_player_type(int32_t);

private:
	void toggle_playertype();
	void toggle_playertribe();

	Widelands::Game        * m_game;
	Widelands::Player_Number m_plnum;

	bool         m_enabled; //  is this player allowed at all (map-dependent)
	int32_t          m_playertype;
	uint32_t         m_current_tribe;
	changemode_t m_allow_changes;

	std::vector<std::string> m_tribes;

	UI::Textarea                       m_plr_name;
	UI::Checkbox                       m_btnEnablePlayer;
	UI::Button<PlayerDescriptionGroup> m_btnPlayerType;
	UI::Button<PlayerDescriptionGroup> m_btnPlayerTribe;
};


#endif
