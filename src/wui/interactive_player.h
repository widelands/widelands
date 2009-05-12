/*
 * Copyright (C) 2002-2003, 2006-2009 by the Widelands Development Team
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

#ifndef INTERACTIVE_PLAYER_H
#define INTERACTIVE_PLAYER_H

#include "debugconsole.h"
#include "interactive_gamebase.h"

#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

#include <SDL_keyboard.h>

#include <vector>

namespace UI {
struct Multiline_Textarea;
struct Textarea;
};


/**
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 */
struct Interactive_Player :
	public Interactive_GameBase, public DebugConsole::Handler
{
	Interactive_Player
		(Widelands::Game &,
		 Section         & global_s,
		 Widelands::Player_Number,
		 bool              scenario,
		 bool              multiplayer);

	void start();

	void toggle_statistics_menu();
	void toggle_chat        ();
	void toggle_options_menu();
	void toggle_objectives();
	void toggle_buildhelp ();
	void toggle_resources ();
	void toggle_help      ();
	void toggle_message_menu();

	virtual void node_action();

	bool handle_key(bool down, SDL_keysym);

	Widelands::Player_Number get_player_number() const {return m_player_number;}
	Widelands::Player & player() const throw () {
		return game().player(m_player_number);
	}
	Widelands::Player * get_player() const throw () {
		assert(&game());
		return game().get_player(m_player_number);
	}

	// for savegames
	void set_player_number(uint32_t plrn);

	// For load
	virtual void cleanup_for_load();
	void think();
	void postload();

	void set_flag_to_connect(Widelands::Coords const location) {
		m_flag_to_connect = location;
	}

private:
	void cmdSwitchPlayer(std::vector<std::string> const & args);

	Widelands::Player_Number m_player_number;
	bool                     m_auto_roadbuild_mode;
	Widelands::Coords        m_flag_to_connect;

	UI::Callback_Button<Interactive_Player> m_toggle_chat;
	UI::Callback_Button<Interactive_Player> m_toggle_options_menu;
	UI::Callback_Button<Interactive_Player> m_toggle_statistics_menu;
	UI::Callback_Button<Interactive_Player> m_toggle_objectives;
	UI::Callback_Button<Interactive_Player> m_toggle_minimap;
	UI::Callback_Button<Interactive_Player> m_toggle_buildhelp;
	UI::Callback_Button<Interactive_Player> m_toggle_message_menu;
	//  UI::Callback_Button<Interactive_Player> m_toggle_resources;
	UI::Callback_Button<Interactive_Player> m_toggle_help;

	UI::UniqueWindow::Registry m_chat;
	UI::UniqueWindow::Registry m_options;
	UI::UniqueWindow::Registry m_statisticsmenu;
	UI::UniqueWindow::Registry m_objectives;
	UI::UniqueWindow::Registry m_fieldaction;
	UI::UniqueWindow::Registry m_encyclopedia;
	UI::UniqueWindow::Registry m_debugconsole;
	UI::UniqueWindow::Registry m_message_menu;
};


#endif
