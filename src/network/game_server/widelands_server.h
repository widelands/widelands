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

#include <string>
#include <vector>
#include <map>

// Definitions
#define ARRAY_WIDTH 20
#define ARRAY_HEIGHT 10
#include "protocol.h"

class WidelandsPlayerStats
{
	public:
		int result;
		int points;
		int land;
		int buildings;
		int milbuildingslost;
		int civbuildingslost;
		int buildingsdefeat;
		int milbuildingsconq;
		int economystrength;
		int militarystrength;
		int workers;
		int wares;
		int productivity;
		int casualties;
		int kills;
};

class WidelandsPlayer
{
	public:
		WidelandsPlayer(std::string playername, int wl_num):
			m_name(playername),
			m_tribe(),
			m_wl_player_number(wl_num),
			m_ggz_player_number(-1),
			m_type(playertype_null)
			{}

		int wl_player_number() { return m_wl_player_number; }
		int ggz_player_number() { return m_ggz_player_number; }
		std::string tribe() { return m_tribe; }
		WLGGZPlayerType type() { return m_type; }
		std::string version() { return m_version; }
		std::string build() { return m_build; }
		
		void set_ggz_player_number(int num) 
			{ m_ggz_player_number=num; }
		void set_tribe(std::string tribe) { m_tribe=tribe; }
		void set_type(WLGGZPlayerType type) { m_type=type; }
		void set_version(std::string v, std::string b)
			{ m_build = b; m_version = v; }

		WidelandsPlayerStats stats;

	private:
		std::string m_name;
		std::string m_tribe;
		int m_ggz_player_number;
		int m_wl_player_number;
		WLGGZPlayerType m_type;
		std::string m_build, m_version;
};

class WidelandsMap 
{
	public:
		WidelandsMap():
			m_name(""),
			m_w(0),
			m_h(0)
			{}
		unsigned int w() {return m_w; } 
		unsigned int h() {return m_w; }
		std::string name() {return m_name; }
		WLGGZGameType gametype() { return m_gametype; }
		
		void set_name(std::string name) { m_name = name; }
		void set_size(int w, int h) { m_w = w; m_h = h; }
		void set_gametype(WLGGZGameType type) { m_gametype=type; }

	private:
		std::string m_name;
		unsigned int m_w, m_h;
		WLGGZGameType m_gametype;
};

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
		void read_game_information(int fd, Client * client);
		void read_game_statistics(int fd);

		WidelandsMap m_map;

		std::map<std::string,WidelandsPlayer *> m_players;

		char * m_wlserver_ip;

		int m_result_gametime;
		std::string host_version, host_build;
		bool m_reported;
};

#endif

