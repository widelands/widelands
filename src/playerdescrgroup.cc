/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "constants.h"
#include "game.h"
#include "player.h"
#include "playerdescrgroup.h"
#include "types.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "tribe.h"
#include "wexception.h"

// hard-coded playercolors
uchar g_playercolors[MAX_PLAYERS][12] = {
	{ // blue
		  2,   2,  74,
		  2,   2, 112,
		  2,   2, 149,
		  2,   2, 198
	},
	{ // red
		119,  19,   0,
		166,  27,   0,
		209,  34,   0,
		255,  41,   0
	},
	{ // yellow
		112, 103,   0,
		164, 150,   0,
		209, 191,   0,
		255, 232,   0
	},
	{ // green
		 26,  99,   1,
		 37, 143,   2,
		 48, 183,   3,
		 59, 223,   3
	},
	{ // black/dark gray
		  0,   0,   0,
		 19,  19,  19,
		 35,  35,  35,
		 57,  57,  57
	},
	{ // orange
		119,  80,   0,
		162, 109,   0,
		209, 141,   0,
		255, 172,   0,
	},
	{ // purple
		 91,   0,  93,
		139,   0, 141,
		176,   0, 179,
		215,   0, 218,
	},
	{ // white
		119, 119, 119,
		166, 166, 166,
		210, 210, 210,
		255, 255, 255
	}
};

void PlayerDescriptionGroup::allow_changes(bool t) {
   m_allow_changes=t;
   m_btnEnablePlayer->set_visible(t);
   m_btnPlayerType->set_enabled(t);
   m_btnPlayerTribe->set_enabled(t);
}

PlayerDescriptionGroup::PlayerDescriptionGroup(UIPanel* parent, int x, int y, Game* game, int plnum)
	: UIPanel(parent, x, y, 450, 20)
{
	m_game = game;
	m_plnum = plnum;
   m_allow_changes=true;
   m_current_tribe=0;

	m_enabled = false;
	set_visible(false);

	// create sub-panels
	m_plr_name=new UITextarea(this, 0, 0, 100, 20, "Player 1", Align_Left);

   m_btnEnablePlayer = new UICheckbox(this, 88, 0);
	m_btnEnablePlayer->set_state(true);
	m_btnEnablePlayer->changedto.set(this, &PlayerDescriptionGroup::enable_player);

	m_btnPlayerType = new UIButton(this, 116, 0, 120, 20, 1);
	m_btnPlayerType->clicked.set(this, &PlayerDescriptionGroup::toggle_playertype);
	if (plnum==1)
		m_playertype = Player::playerLocal;
	else
		m_playertype = Player::playerAI;


   m_btnPlayerTribe = new UIButton(this, 244, 0, 120, 20, 1);
   m_btnPlayerTribe->clicked.set(this, &PlayerDescriptionGroup::toggle_playertribe);

   Tribe_Descr::get_all_tribes(&m_tribes);
   m_btnPlayerTribe->set_title(m_tribes[m_current_tribe].c_str());
}

/** PlayerDescriptionGroup::set_enabled(bool enable)
 *
 + The group is enabled if the map has got a starting position for this player.
 * We need to update the Game class accordingly.
 */
void PlayerDescriptionGroup::set_enabled(bool enable)
{
	if(!m_allow_changes) return;
   if (enable == m_enabled)
		return;

	m_enabled = enable;

	if (!m_enabled)
	{
		if (m_btnEnablePlayer->get_state())
			m_game->remove_player(m_plnum);
		set_visible(false);
	}
	else
	{
		if (m_btnEnablePlayer->get_state())
			m_game->add_player(m_plnum, m_playertype, m_tribes[m_current_tribe].c_str(), g_playercolors[m_plnum-1]);

		const char* string = 0;
		switch(m_playertype) {
		case Player::playerLocal: string = "Human"; break;
		case Player::playerAI: string = "Computer"; break;
		}
		m_btnPlayerType->set_title(string);
		m_btnPlayerType->set_visible(m_btnEnablePlayer->get_state());
		m_btnPlayerTribe->set_visible(m_btnEnablePlayer->get_state());

		set_visible(true);
	}

	changed.call();
}

/** PlayerDescriptionGroup::enable_player(bool on)
 *
 * Update the Game when the checkbox is changed.
 */
void PlayerDescriptionGroup::enable_player(bool on)
{
	if(!m_allow_changes) return;

   if (on) {
		m_game->add_player(m_plnum, m_playertype, m_tribes[m_current_tribe].c_str(), g_playercolors[m_plnum-1]);
	} else {
		m_game->remove_player(m_plnum);
	}

	m_btnPlayerType->set_visible(on);
   m_btnPlayerTribe->set_visible(on);
	changed.call();
}

void PlayerDescriptionGroup::toggle_playertype()
{
	// NOOP: toggling the player type is currently not possible
}

/*
 * toggles the tribe the player will play
 */
void PlayerDescriptionGroup::toggle_playertribe(void) {
   ++m_current_tribe;
   if(m_current_tribe==m_tribes.size()) m_current_tribe=0;
   m_btnPlayerTribe->set_title(m_tribes[m_current_tribe].c_str());
   // set the player
   m_game->remove_player(m_plnum);
   m_game->add_player(m_plnum, m_playertype, m_tribes[m_current_tribe].c_str(), g_playercolors[m_plnum-1]);
}

/*
 * set the current player tribe
 */
void PlayerDescriptionGroup::set_player_tribe(std::string str) {
   uint i=0;
   for(i=0; i<m_tribes.size(); i++) {
      if(m_tribes[i]==str) {
         m_current_tribe=i;
         m_btnPlayerTribe->set_title(m_tribes[m_current_tribe].c_str());
         // set the player
         m_game->remove_player(m_plnum);
         m_game->add_player(m_plnum, m_playertype, m_tribes[m_current_tribe].c_str(), g_playercolors[m_plnum-1]);
         return;
      }
   }
   // never here
   throw wexception("Map defines tribe %s, but it doens't exist!\n", str.c_str());
}

/*
 * Set players name
 */
void PlayerDescriptionGroup::set_player_name(std::string str) {
   m_plr_name->set_text(str.c_str());
}
