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

#include "protocol.h"
#include "widelands_player.h"
#include "widelands_map.h"

// GGZ includes. class GGZGameServer
#include "ggzgameserver.h"

#include <string>
#include <vector>
#include <map>

class StatisticsHandler;
class ProtocolHandler;

/// WidelandsServer server object. This class does the interaction with ggzd
class WidelandsServer : public GGZGameServer
{
	public:
		WidelandsServer();
		~WidelandsServer();
		const char * get_host_ip() { return m_wlserver_ip; }

		/// @{
		/// change the game state
		void set_state_playing() { changeState(GGZGameServer::playing); }
		void set_state_done();
		/// @}

		/// Game ended. report the results if possible
		void game_done();

		/// get player structure by name. If id is given create if not existing
		WidelandsPlayer * get_player_by_name(std::string name, bool create = false);
		WidelandsPlayer * get_player_by_wlid(int);
		WidelandsPlayer * get_player_by_ggzid(int);
		
		GGZGameServer::State game_state() { return state(); }
		
		int numberofplayers() 
			{ return playercount(Seat::player) + playercount(Seat::bot); }
		
		
		StatisticsHandler & stat_handler();
		ProtocolHandler & proto_handler();

		void check_reports();
		bool is_playing() { return state() == GGZGameServer::waiting; }

		std::map<std::string,WidelandsPlayer *> m_players;

	private:
		// @{
		/// These event functions are invoked by ggzd
		void stateEvent();
		void joinEvent(Client *client);
		void leaveEvent(Client *client);
		void spectatorJoinEvent(Client *client);
		void spectatorLeaveEvent(Client *client);
		void spectatorDataEvent(Client *client);
		void dataEvent(Client *client);
		void seatEvent(Seat * seat);
		void spectatorEvent(Spectator * spectator);
		void errorEvent();
		// @}

		/// The ip address of the widelands client hosting the game
		char * m_wlserver_ip;
		/// Results of the game have been reported to ggzd
		bool m_reported;
};

extern WidelandsServer * g_wls;

#endif //WIDELANDS_SERVER_H
