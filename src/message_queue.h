/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "message.h"
#include "ui_basic/button.h"
#include "logic/player.h"

namespace Widelands {
	struct Coords;
	struct GameMessageMenu;

	///the Message manager keeps track of messages
	///based of the objective system

struct MessageQueue {
	void registerButton(UI::Callback_Button<GameMessageMenu>*);
	
	static std::vector<Message>& get(Player& p) 
	{
		return get(p.get_player_number());
	}
	static std::vector<Message>& get(int i) 
	{
		static std::vector<std::vector< Message> > myQueue;
		if (i >= myQueue.size()){
			myQueue.resize(i+1);
		}
		return myQueue[i];
	}
	
	static void add(Player& p, Message m)
	{
		add(p.get_player_number(),m);
	}
	static void add(int player_number, Message m)
	{
		get(player_number).push_back(m);
		m_readall(player_number)=false;
	}
	
	static void read(Player& p)
	{
		read(p.get_player_number());
	}
	static void read(int player_number)
	{
		m_readall(player_number)=true;
	}
		
	
private:
	//use char here instead of bool because bool vectors behave odd
	static char& m_readall(int player_number)
	{
		static std::vector<char> m_read;
		if (player_number >= m_read.size()){
			m_read.resize(player_number+1);
		}
		return m_read[player_number];
	}
	
	UI::Callback_Button<GameMessageMenu>* m_button;
};

};

#endif
