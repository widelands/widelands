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

#include "ggz_wlmodule.h"
#include "game_server/protocol.h"
#include "game_server/protocol_helpers.h"
#include "game_server/protocol_helper_read_list.h"
#include "log.h"
#include "ggz_ggzmod.h"
#include "ggz_ggzcore.h"
#include "warning.h"

ggz_wlmodule::ggz_wlmodule():
	m_data_fd     (0),
	server_ip_addr(0),
	m_server_ver  (0)
{}

void ggz_wlmodule::process()
{
	int32_t op;
	char * ipstring;
	char * greeter;
	int32_t greeterversion;
	char ipaddress[17];

	if (m_data_fd < 0 or m_data_fd > FD_SETSIZE)
		return;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(m_data_fd, &fdset);

	{
		int32_t const ret = select(m_data_fd + 1, &fdset, 0, 0, &timeout);
		if (ret <= 0)
			return;
		log
			("GGZWLMODULE/process ## select() returns: %i for fd %i\n",
			 ret, m_data_fd);
	}

	{
		int32_t const ret = ggz_read_int(m_data_fd, &op);
		if (ret < 0) {
			#warning TODO Handle read error from ggzmod datafd
			log("GGZWLMODULE/process ## read error. Exit ggz\n");
			NetGGZ::ref().deinit();
			//	use_ggz = false;
			return;
		}
		log("GGZWLMODULE/process ## received opcode: %i (%i)\n", op, ret);
		
	}

	switch (op) {
	case op_greeting:
		ggz_read_string_alloc(m_data_fd, &greeter);
		ggz_read_int(m_data_fd, &greeterversion);
		log
			("GGZWLMODULE/process ## server is: '%s' '%i'\n",
			 greeter, greeterversion);
		ggz_free(greeter);
		ggz_write_int(m_data_fd, op_request_protocol_ext);
		break;
	case op_request_ip:
		// This it not used (anymore?). Return 255.255.255.255 to the server
		log("GGZWLMODULE/process ## ip request!\n");
		snprintf(ipaddress, sizeof(ipaddress), "%i.%i.%i.%i", 255, 255, 255, 255);
		ggz_write_int(m_data_fd, op_reply_ip);
		ggz_write_string(m_data_fd, ipaddress);
		break;
	case op_broadcast_ip:
		ggz_read_string_alloc(m_data_fd, &ipstring);
		log("GGZWLMODULE/process ## got ip broadcast: '%s'\n", ipstring);
		server_ip_addr = ggz_strdup(ipstring);
		ggz_free(ipstring);
		break;
	case op_unreachable:
		NetGGZ::ref().deinit();
		throw warning
			(_("Connection problem"), "%s",
			 _
			  ("Your Server was not reachable from the Internet.\n"
			   "Please try to solve the problem - Reading the notes\n"
			   "at http://wl.widelands.org/wiki/InternetGaming can\n"
			   "be advantageous."));
	case op_request_protocol_ext:
#warning TODO
		break;
	case op_reply_protocol_ext:
		log("GGZWLMODULE/process ## got extended protocol version: ");
		{
			std::list<WLGGZParameter> parlist = 
			wlggz_read_parameter_list(m_data_fd);
			if (parlist.size() < 2) {
				log
					("\nGGZWLMODULE ## invalid data: %i parameters\n",
					 parlist.size());
			} else {
				m_server_ver = parlist.front().get_integer() << 16;
				parlist.pop_front();
				m_server_ver += parlist.front().get_integer();
				parlist.pop_front();
				log("%X\n", m_server_ver);
				if (parlist.size())
					log
						("GGZWLMODULE/process ## %i parameters left\n",
						 parlist.size());
			}
			ggz_write_int(m_data_fd, op_set_debug);
			ggz_write_int(m_data_fd, 0);
		}
		break;
	default: 
		if (m_server_ver > 0)
			switch(op) {
			case op_debug_string:
			{
				std::list<WLGGZParameter> parlist =
					wlggz_read_parameter_list(m_data_fd);
				if(parlist.size())
					log("GGZ server ## %s\n", parlist.front().get_string().c_str());
			}
				break;
			default:
				log("GGZWLMODULE ## opcode unknown - extended protocol\n");
				wlggz_read_parameter_list(m_data_fd);
			}
		else
			log("GGZWLMODULE ## opcode unknown! - old protocol path\n");
	}
}

bool ggz_wlmodule::send_game_info
	(std::string mapname, int map_w, int map_h,
	 int win_condition, std::vector<Net_Player_Info> playerinfo)
{
	log("GGZWLMODULE NetGGZ::send_game_info()\n");
	if (get_ext_proto_ver() == 0)
	{
		log("GGZWLMODULE not supported by server\n");
		return false;
	}

	if (NetGGZ::ref().core().is_in_table()) {
		WLGGZ_writer w = WLGGZ_writer(m_data_fd, op_game_information);

		w.open_list(gameinfo_mapname);
		w << mapname;
		w.close_list();

		w.open_list(gameinfo_mapsize);
		w << map_w << map_h << static_cast<int>(playerinfo.size());
		w.close_list();

		w.open_list(gameinfo_gametype);
		w << win_condition;
		w.close_list();

		w.open_list(gameinfo_version);
		w << build_id() << build_type();
		w.close_list();

		
		log("GGZWLMODULE ## Iterate Players\n");
		std::vector<Net_Player_Info>::iterator pit = playerinfo.begin();
		while (pit != playerinfo.end())
		{
			w.open_list(gameinfo_playerid);
			w << pit->playernum;
			w.close_list();
			
			w.open_list(gameinfo_playername);
			w << pit->name;
			w.close_list();
			
			w.open_list(gameinfo_tribe);
			w << pit->tribe;
			w.close_list();
			
			w.open_list(gameinfo_playertype);
			w << static_cast<int>(pit->type);
			w.close_list();
			
			w.open_list(gameinfo_teamnumber);
			w << pit->team;
			w.close_list();
			pit++;
		}
		log("GGZWLMODULE ## Player Iterate finished\n");
		w.flush();
	} else
		log("GGZWLMODULE ERROR: GGZ not used!\n");
	log("GGZWLMODULE NetGGZ::send_game_info(): ende\n");
	return true;
}

// how may statistic samples do we have in five minutes
#define sample_count (5 * 60 * 1000 / STATISTICS_SAMPLE_TIME)

void send_stat(WLGGZ_writer & wr, std::vector<uint32_t> stat)
{
	uint32_t c = 0;
	uint32_t cur = 0;
	uint32_t min, max;
	double avg;
	for (; c < stat.size(); c++)
	{
		if(cur == 0) {
			min = max = stat.at(c);
			avg = static_cast<double>(stat.at(c));
		}
		if (stat.at(c) > max)
			max = stat.at(c);
		if (stat.at(c) < min)
			min = stat.at(c);
		avg = (avg + stat.at(c) / 2.0);
		if (cur == sample_count or c == (stat.size() -1 ))
		{
			wr.open_list(c);
			wr << static_cast<int>(avg);
			wr << static_cast<int>(min);
			wr << static_cast<int>(max);
			wr.close_list();
			cur = 0;
		}
	}
}

bool ggz_wlmodule::send_statistics
	(int32_t gametime,
	 const Widelands::Game::General_Stats_vector & resultvec,
	 std::vector<Net_Player_Info> playerinfo)
{
	if (get_ext_proto_ver() == 0)
	{
		log("GGZWLMODULE ##  not supported by server\n");
		return false;
	}

	if (NetGGZ::ref().core().is_in_table()) {
		log
			("GGZWLMODULE ## NetGGZ::send_game_statistics: "
			 "send statistics to metaserver now!\n");
		WLGGZ_writer w = WLGGZ_writer(m_data_fd, op_game_statistics);

		w.open_list(gamestat_gametime);
		w << gametime;
		w.close_list();

		log
			("GGZWLMODULE ## resultvec size: %d, playerinfo size: %d\n",
			 resultvec.size(), playerinfo.size());
 
		for (unsigned int i = 0; i < playerinfo.size(); i++) {
			w.open_list(gamestat_playernumber);
			w << static_cast<int>(i);
			w.close_list();

			w.open_list(gamestat_result);
			w << playerinfo.at(i).result;
			w.close_list();

			w.open_list(gamestat_points);
			w << playerinfo.at(i).points;
			w.close_list();

			w.open_list(gamestat_land);
			send_stat(w, resultvec.at(i).land_size);
			w.close_list();

			w.open_list(gamestat_buildings);
			send_stat(w, resultvec.at(i).nr_buildings);

			w.open_list(gamestat_militarystrength);
			send_stat(w, resultvec.at(i).miltary_strength);
			w.close_list();

			w.open_list(gamestat_casualties);
			send_stat(w, resultvec.at(i).nr_casualties);
			w.close_list();

			w.open_list(gamestat_land);
			send_stat(w, resultvec[i].nr_civil_blds_defeated);
			w.close_list();

			w.open_list(gamestat_civbuildingslost);
			send_stat(w, resultvec.at(i).nr_civil_blds_lost);
			w.close_list();

			w.open_list(gamestat_kills);
			send_stat(w, resultvec.at(i).nr_kills);
			w.close_list();

			w.open_list(gamestat_buildingsdefeat);
			send_stat(w, resultvec[i].nr_msites_defeated);
			w.close_list();

			w.open_list(gamestat_milbuildingslost);
			send_stat(w, resultvec.at(i).nr_msites_lost);
			w.close_list();

			w.open_list(gamestat_wares);
			send_stat(w, resultvec.at(i).nr_wares);
			w.close_list();

			w.open_list(gamestat_workers);
			send_stat(w, resultvec.at(i).nr_workers);
			w.close_list();

			w.open_list(gamestat_productivity);
			send_stat(w, resultvec.at(i).productivity);
			w.close_list();
		}

		/*
		- gamestat_playernumber
		gamestat_result
		gamestat_points
		- gamestat_land
		- gamestat_buildings
		- gamestat_milbuildingslost
		- gamestat_civbuildingslost
		gamestat_buildingsdefeat
		gamestat_milbuildingsconq
		gamestat_economystrength
		gamestat_militarystrength
		- gamestat_workers
		- gamestat_wares
		- gamestat_productivity
		- gamestat_casualties
		- gamestat_kills */
		w.flush();
		} else
			log("GGZWLMODULE ## ERROR: not in table!\n");
		return true;
}
