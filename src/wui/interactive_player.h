/*
 * Copyright (C) 2002-2003, 2006-2010 by the Widelands Development Team
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

#ifndef WL_WUI_INTERACTIVE_PLAYER_H
#define WL_WUI_INTERACTIVE_PLAYER_H

#include <vector>

#include <SDL_keyboard.h>

#include "logic/message_id.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_gamebase.h"

namespace UI {
struct MultilineTextarea;
struct Textarea;
}


/**
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 */
class InteractivePlayer : public InteractiveGameBase {
public:
	InteractivePlayer
		(Widelands::Game &,
		 Section         & global_s,
		 Widelands::PlayerNumber,
		 bool              multiplayer);

	~InteractivePlayer();

	void toggle_chat        ();

	bool can_see(Widelands::PlayerNumber) const override;
	bool can_act(Widelands::PlayerNumber) const override;
	Widelands::PlayerNumber player_number() const override;

	void node_action() override;

	bool handle_key(bool down, SDL_Keysym) override;

	Widelands::Player & player() const {
		return game().player(m_player_number);
	}
	Widelands::Player * get_player() const override {
		assert(&game());
		return game().get_player(m_player_number);
	}

	// for savegames
	void set_player_number(uint32_t plrn);

	// For load
	void cleanup_for_load() override;
	void think() override;

	void set_flag_to_connect(Widelands::Coords const location) {
		m_flag_to_connect = location;
	}

	void popup_message(Widelands::MessageId, const Widelands::Message &);
	int32_t calculate_buildcaps(const Widelands::TCoords<Widelands::FCoords> c) override;

private:
	void cmdSwitchPlayer(const std::vector<std::string> & args);

	Widelands::PlayerNumber m_player_number;
	bool                     m_auto_roadbuild_mode;
	Widelands::Coords        m_flag_to_connect;

	UI::Button m_toggle_chat;
	UI::Button m_toggle_options_menu;
	UI::Button m_toggle_statistics_menu;
	UI::Button m_toggle_objectives;
	UI::Button m_toggle_minimap;
	UI::Button m_toggle_message_menu;
	UI::Button m_toggle_help;

	UI::UniqueWindow::Registry m_chat;
	UI::UniqueWindow::Registry m_options;
	UI::UniqueWindow::Registry m_statisticsmenu;
	UI::UniqueWindow::Registry m_objectives;
	UI::UniqueWindow::Registry m_encyclopedia;
	UI::UniqueWindow::Registry m_message_menu;
};


#endif  // end of include guard: WL_WUI_INTERACTIVE_PLAYER_H
