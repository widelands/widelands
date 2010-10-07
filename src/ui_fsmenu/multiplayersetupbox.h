/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef MULTIPLAYERSETUPBOX_H
#define MULTIPLAYERSETUPBOX_H

#include "constants.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/button.h"

#include <string>

struct GameSettingsProvider;

struct MultiPlayerSetupBoxOptions;
struct MultiPlayerClientGroup;
struct PlayerSettingGroup;

/**
 * struct MultiPlayerSetupBox
 *
 * This struct should give an (as best as possible) intuitive user interface, to
 * set up a multi player game.
 * Depending on the initialization (host/client) and the currently selected game
 * type (normal/savegame/scenario) some parts of the user interface will be
 * disabled and some will be invisible (e.g. the host tab for clients).
 * 
 * The user interface is made up with tabs:
 * - (1) "Clients" tab:
 *      This tab contains n \ref MultiPlayerClientGroup , where n is the maximum
 *      number of clients in the game.
 *      Every newly connected user (including the host) will be placed in the
 *      first unocupied slot - the number is the same as the client number used
 *      on host side.
 * - (2) "Options" tab:
 *      This tab contains UI elements to set up the selected map, savegame or 
 *      scenario.
 *      On the left side m \ref PlayerSettingGroup let the host set up the
 *      players (m is the maximum number of players in the selected map or save)
 *      Information about the selected map and game type is shown on the right
 *      side:
 *        - map name
 *        - game type (normal, savegame, scenario)
 *        - win condition
 * - (3) "Host" tab:
 *      This tab contains the following UI elements:
 *        - A button to load a map
 *        - A button to load a savegame
 *        - A button to start the game
 *        - A button to close the server
 *        - A label + spinbox to set the maximum number of client slots
 *        - A Checkbox with label to close the server temporary to disallow
 *          user connections
 *        - A text area showing, what needs to be done until the game can be
 *          started (e.g. "Client ’Siegfried’ is not yet ready",
 *                        "Player 2 has neither a human player nor an AI") 
 * 
 */
struct MultiPlayerSetupBox : public UI::Panel {
	MultiPlayerSetupBox
		(UI::Panel * parent,
		 int32_t x, int32_t y, int32_t w, int32_t h,
		 GameSettingsProvider * settings,
		 uint32_t usernum,
		 uint32_t butw, uint32_t buth,
		 std::string const & fname = UI_FONT_NAME,
		 uint32_t fsize = UI_FONT_SIZE_SMALL);
	~MultiPlayerSetupBox();

	void refresh();

private:
	void select_map();
	void select_savegame();
	void start_clicked();

	MultiPlayerSetupBoxOptions * d;
	MultiPlayerClientGroup     * c[64];
	PlayerSettingGroup         * p[MAX_PLAYERS];

	UI::Callback_Button<MultiPlayerSetupBox> * m_select_map;
	UI::Callback_Button<MultiPlayerSetupBox> * m_select_save;
	UI::Callback_Button<MultiPlayerSetupBox> * m_ok;

	UI::Tab_Panel                tp;
};


#endif
