/*
 * Copyright (C) 2002-2003, 2006-2008 by the Widelands Development Team
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

#ifndef INTERACTIVE_SPECTATOR_H
#define INTERACTIVE_SPECTATOR_H

#include <SDL_keyboard.h>

#include "wui/interactive_gamebase.h"
#include "ui_basic/button.h"

namespace Widelands {class Game;}

/**
 * This class shows a game for somebody who is only a spectator.
 *
 * It is used for replays and network games.
 *
 * This class provides the UI, runs the game logic, etc.
 */
struct Interactive_Spectator : public Interactive_GameBase {
	Interactive_Spectator
		(Widelands::Game &, Section & global_s, bool multiplayer = false);

	~Interactive_Spectator();

	void start() override;

	Widelands::Player * get_player() const override;

	bool handle_key(bool down, SDL_keysym) override;

private:
	void toggle_chat();
	void toggle_options_menu();
	void toggle_statistics();
	void exit_btn();
	void save_btn();
	virtual bool can_see(Widelands::Player_Number) const override;
	virtual bool can_act(Widelands::Player_Number) const override;
	virtual Widelands::Player_Number player_number() const override;
	virtual void node_action() override;

private:
	UI::Button m_toggle_chat;
	UI::Button m_exit;
	UI::Button m_save;
	UI::Button m_toggle_options_menu;
	UI::Button m_toggle_statistics;
	UI::Button m_toggle_minimap;


	UI::UniqueWindow::Registry m_chat;
	UI::UniqueWindow::Registry m_options;
};


#endif
