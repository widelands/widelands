/*
 * Copyright (C) 2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__INET_LOBBY_H
#define __S__INET_LOBBY_H

#include "fullscreen_menu_base.h"

#include <string>
#include <vector>

namespace UI {
struct Edit_Box;
struct Multiline_Textarea;
template <typename T> struct Listselect;
};
class Game_Server_Connection;

/*
 * This class represents the 'lobby'. this is where the user can join games
 * and chat.
 */
struct Fullscreen_Menu_InetLobby: public Fullscreen_Menu_Base {
	Fullscreen_Menu_InetLobby(Game_Server_Connection*);
	~Fullscreen_Menu_InetLobby();

	void think();

	void server_message(std::string str);
	void user_entered(std::string name, std::string room, bool enters);
	void user_info(std::string name, std::string name, std::string room);
	void room_info(std::vector< std::string > users);
	void chat_message(std::string user, std::string room, bool is_action);
	void critical_error(std::string str);
	void disconnect();

private:
	Game_Server_Connection * m_gsc;
	UI::Edit_Box           * m_chatbox;
	UI::Multiline_Textarea * m_chatarea;
	UI::Listselect<void *> * m_userlist;
	bool                     m_disconnect_expected;

private:
	void changed();
	void clicked_back();
};

#endif
