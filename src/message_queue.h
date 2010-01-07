/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <cassert>
#include <string>
#include <vector>

#include "logic/game.h"
#include "graphic/graphic.h"
#include "message.h"
#include "ui_basic/button.h"
#include "logic/player.h"
#include "sound/sound_handler.h"

#define NO_NEW_MESSAGES "pics/menu_toggle_oldmessage_menu.png"
#define NEW_MESSAGES "pics/menu_toggle_newmessage_menu.png"
#define MAX_QUEUE_SIZE 255

// Definitions of message strings:
#define MSG_WAREHOUSE     "warehouse"
#define MSG_UNDER_ATTACK  "under_attack"
#define MSG_SITE_LOST     "site_lost"
#define MSG_SITE_DEFEATED "site_defeated"
#define MSG_MINE          "mine"


namespace Widelands {
	struct Coords;
	struct GameMessageMenu;

	///the Message manager keeps track of messages
	///based of the objective system


struct MessageQueue {
	void registerButton(UI::Callback_Button<GameMessageMenu>*);

	static std::vector<Message> & get (const Player & p)
	{
		return get(p.player_number());
	}
	static std::vector<Message> & get (Player_Number const player_number)
	{
		static std::vector<std::vector< Message> > myQueue;
		if (player_number >= myQueue.size())
		{
			myQueue.resize(player_number + 1);
		}
		//log("messagequeue for player %i at address %i\n",player_number,
		//&myQueue[player_number]);
		return myQueue[player_number];
	}

	static void add (Player const & p, Message const m) {
		add(p.player_number(), m);
	}
	static void add (Player_Number const player_number, Message const m) {
		std::vector<Message> & myQueue = get(player_number);
		myQueue.push_back(m);
		if (myQueue.size() >= MAX_QUEUE_SIZE) {
			std::vector<Message>::iterator b = myQueue.begin();
			while (b != myQueue.end())
				if (b->get_is_visible())
					b = myQueue.erase(b);
				else
					++b;
			//no empty messages to delete so remove the first one
			if (myQueue.size() >= MAX_QUEUE_SIZE)
				myQueue.erase(myQueue.begin());
		}

		m_readall(player_number) = false;
		if (player_number == m_player_number(0)) {
			// (If we are not starting up the game) play a sound
			if (m.time() > 0) {
				g_sound_handler.play_fx("message", 200, PRIO_ALWAYS_PLAY);
				// And be even louder, if we are under attack!
				if (m.sender() == MSG_UNDER_ATTACK)
					g_sound_handler.play_fx("under_attack",  128, PRIO_ALWAYS_PLAY);
				if (m.sender() == MSG_SITE_LOST)
					g_sound_handler.play_fx("site_lost",     128, PRIO_ALWAYS_PLAY);
				if (m.sender() == MSG_SITE_DEFEATED)
					g_sound_handler.play_fx("site_defeated", 128, PRIO_ALWAYS_PLAY);
			}
			if (m_button(0))
				m_button(0)->set_pic(g_gr->get_picture(PicMod_Game, NEW_MESSAGES));

		}
	}


	/**
	 * Adds message m to the message queue of player player_number, if
	 * there has not been a message from the same sender
	 * in the last timeout milliseconds in a radius radius around
	 * the coordinates of m.
	 *
	 * \note This code has been moved here from militarysite.cc
	 */
	static void addWithTimeout
		(Game        &       game,
		 Player_Number const player_number,
		 uint32_t      const timeout,
		 uint32_t      const radius,
		 Message       const m)
	{
		Map const & map  = game.map();

		Coords const coords = m.get_coords();
		std::vector<Message> & msgQueue = MessageQueue::get(player_number);
		for
			(struct {
			 	std::vector<Message>::const_iterator       current;
			 	std::vector<Message>::const_iterator const end;
			 } i = {msgQueue.begin(), msgQueue.end()};;
			 ++i.current)
			if (i.current == i.end) {
				MessageQueue::add (player_number, m);
				break;
			} else if
				(i.current->sender() == m.sender() and
				 map.calc_distance(i.current->get_coords(), coords) <= radius and
				 game.get_gametime() - i.current->time() < timeout)
				break;
	}

	static void read_all (const Player & p)
	{
		read_all(p.player_number());
	}
	static void read_all (Player_Number const player_number)
	{
		m_readall(player_number) = true;
		if (player_number == m_player_number(0) and m_button(0))
			m_button(0)->set_pic(g_gr->get_picture(PicMod_Game, NO_NEW_MESSAGES));
	}

	static UI::Callback_Button<Interactive_Player> * m_button
		(UI::Callback_Button<Interactive_Player> * const b)
	{
		static UI::Callback_Button<Interactive_Player> * m_but = 0;
		if (b)
			m_but = b;
		return m_but;
	}

	static void set_player_number(const Player_Number & player_number) {
		m_player_number(player_number);
	}

private:
	//use char here instead of bool because bool vectors behave odd
	static char & m_readall(Player_Number const player_number)
	{
		static std::vector<char> m_read;
		if (player_number >= m_read.size()) {
			m_read.resize(player_number + 1);
		}
		return m_read[player_number];
	}

	static Player_Number & m_player_number(Player_Number const & player_number) {
		static Player_Number plrnum = 0;
		if (player_number != 0)
			plrnum = player_number;
		return plrnum;
	}


};

}

#endif
