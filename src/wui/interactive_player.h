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

#ifndef INTERACTIVE_PLAYER_H
#define INTERACTIVE_PLAYER_H

#include <vector>

#include <SDL_keyboard.h>

#include "wui/interactive_gamebase.h"
#include "logic/message_id.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

namespace UI {
struct Multiline_Textarea;
struct Textarea;
}


/**
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 */
struct Interactive_Player : public Interactive_GameBase
{
	Interactive_Player
		(Widelands::Game &,
		 Section         & global_s,
		 Widelands::Player_Number,
		 bool              scenario,
		 bool              multiplayer);

	~Interactive_Player();

	void start() override;

	void toggle_chat        ();

	virtual bool can_see(Widelands::Player_Number) const override;
	virtual bool can_act(Widelands::Player_Number) const override;
	virtual Widelands::Player_Number player_number() const override;

	virtual void node_action() override;

	bool handle_key(bool down, SDL_keysym) override;

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
	virtual void cleanup_for_load() override;
	void think() override;
	void postload() override;

	void set_flag_to_connect(Widelands::Coords const location) {
		m_flag_to_connect = location;
	}

	void popup_message(Widelands::Message_Id, const Widelands::Message &);

private:
	void cmdSwitchPlayer(const std::vector<std::string> & args);

	Widelands::Player_Number m_player_number;
	bool                     m_auto_roadbuild_mode;
	Widelands::Coords        m_flag_to_connect;

	UI::Button m_toggle_chat;
	UI::Button m_toggle_options_menu;
	UI::Button m_toggle_statistics_menu;
	UI::Button m_toggle_objectives;
	UI::Button m_toggle_minimap;
	UI::Button m_toggle_buildhelp;
	UI::Button m_toggle_message_menu;
	UI::Button m_toggle_help;

	UI::UniqueWindow::Registry m_chat;
	UI::UniqueWindow::Registry m_options;
	UI::UniqueWindow::Registry m_statisticsmenu;
	UI::UniqueWindow::Registry m_objectives;
	UI::UniqueWindow::Registry m_encyclopedia;
	UI::UniqueWindow::Registry m_message_menu;
};


#endif
