// Widelands server for GGZ
// Copyright (C) 2004 Josef Spillner <josef@ggzgamingzone.org>
// Copyright (C) 2009 The Widelands Development Team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef WIDELANDS_SERVER_H
#define WIDELANDS_SERVER_H

// GGZ includes
#include "ggzgameserver.h"

// Definitions
#define ARRAY_WIDTH 20
#define ARRAY_HEIGHT 10

// WidelandsServer server object
class WidelandsServer : public GGZGameServer
{
	public:
		WidelandsServer();
		~WidelandsServer();
		void stateEvent();
		void joinEvent(Client *client);
		void leaveEvent(Client *client);
		void spectatorJoinEvent(Client *client);
		void spectatorLeaveEvent(Client *client);
		void spectatorDataEvent(Client *client);
		void dataEvent(Client *client);
		void errorEvent();

	private:
		void game_start();
		void game_stop();
		void game_end();

		char * m_ip;
};

#endif

