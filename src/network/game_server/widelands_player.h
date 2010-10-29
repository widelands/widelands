/*
* Copyright (C) 2010 The Widelands Development Team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __WIDELANDS_PLAYER_H__
#define __WIDELANDS_PLAYER_H__

#include <string>
#include <stdint.h>
#include <vector>
#include "protocol.h"
#include "widelands_server.h"

struct WidelandsPlayerStats
{
		uint32_t land;
		uint32_t buildings;
		uint32_t milbuildingslost;
		uint32_t civbuildingslost;
		uint32_t buildingsdefeat;
		uint32_t milbuildingsconq;
		uint32_t economystrength;
		uint32_t militarystrength;
		uint32_t workers;
		uint32_t wares;
		uint32_t productivity;
		uint32_t casualties;
		uint32_t kills;
};

#define SUPPORT_B16_PROTOCOL(p) (p != 0 and p->support_build16_proto())

class WidelandsPlayer
{
	public:
		WidelandsPlayer(std::string playername):
			m_name(playername),
			m_tribe(),
			m_wl_player_number(-1),
			m_ggz_player_number(-1),
			m_ggz_spectator_number(-1),
			m_type(playertype_null),
			m_team(0),
			m_build16_protocol(false)
			{}

		int wl_player_number() { return m_wl_player_number; }
		int ggz_player_number() { 
			return m_ggz_player_number; }
		std::string tribe() { return m_tribe; }
		WLGGZPlayerType type() { return m_type; }
		std::string version() { return m_version; }
		std::string build() { return m_build; }
		unsigned int team() { return m_team; }
		bool support_build16_proto() { return m_build16_protocol; }
		
		void set_ggz_player_number(int num) 
			{ m_ggz_player_number = num; }
		void set_wl_player_number(int num)
			{ m_wl_player_number = num; }
		void set_ggz_spectator_number(int num)
			{ m_ggz_spectator_number = num; }
		void set_tribe(std::string tribe) { m_tribe=tribe; }
		void set_type(WLGGZPlayerType type) { m_type=type; }
		void set_version(std::string v, std::string b)
			{ m_build = b; m_version = v; }
		void set_team(unsigned int t)
			{ m_team = t; }
		void set_build16_proto(bool b) { m_build16_protocol = b; }
		
		bool is_spectator() { return m_ggz_spectator_number >= 0; }
		
		std::string name() { return m_name; }
		/*
		bool is_player()
		{
			return m_ggz_player_number >= 0 and
		}
		*/
		/*
		bool is_bot() 
		*/
		bool is_player_or_bot() { return m_ggz_player_number >= 0; }

		bool is_host() {
			if
				(m_host_username.empty() and m_name.length() > 0 and
				 (m_ggz_player_number == 0 or m_ggz_spectator_number == 0))
			{
				m_host_username = m_name;
				return true;
			}
			return
				not m_host_username.empty() and m_host_username == m_name;
		}

		WidelandsPlayerStats last_stats;
		std::vector<WidelandsPlayerStats> stats_min, stats_max, stats_avg;

		void set_end_time(int d) { m_end_time = d; }
		int end_time() { return m_end_time; }

		uint32_t result;
		uint32_t points;

		/// client communication error counter. This are recoverable error like
		/// invalid opcodes.
		int soft_error_count;

		/// client communication error counter. This are hard errors like not
		/// being able to read from socket.
		int hard_error_count;

		/// set to true if soft_error_count was to high or if we had an error
		/// while reading parameter list
		bool desync;

		/// set to true if communication to client is broken.
		bool connection_failed;
		/// set to true if this client send a game report.
		bool reported_game;
		/// set to true if this clint has send game information.
	private:
		WidelandsPlayer();
		std::string m_name;
		std::string m_tribe;
		int m_ggz_player_number;
		int m_wl_player_number;
		WLGGZPlayerType m_type;
		std::string m_build, m_version;
		unsigned int m_team;
		/// This client supports the new (after build16) protocol
		bool m_build16_protocol;
		int m_ggz_spectator_number;

		static std::string m_host_username;
		int m_end_time;
};

#endif //__WIDELANDS_PLAYER_H__
