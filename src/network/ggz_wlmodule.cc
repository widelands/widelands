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
#include "log.h"
#include "ggz_ggzmod.h"
#include "ggz_ggzcore.h"
#include "warning.h"

ggz_wlmodule       * ggzwlmodule = 0;

ggz_wlmodule & ggz_wlmodule::ref() {
	if (!ggzwlmodule)
		ggzwlmodule = new ggz_wlmodule();
	return *ggzwlmodule;
}

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
		log("GGZWLMODULE/process ## received opcode: %i (%i)\n", op, ret);
		if (ret < 0) {
			ggz_ggzmod::ref().disconnect();
			#warning TODO Handle read error from ggzmod datafd
			//				use_ggz = false;
			return;
		}
	}

	switch (op) {
	case op_greeting:
		ggz_read_string_alloc(m_data_fd, &greeter);
		ggz_read_int(m_data_fd, &greeterversion);
		log
			("GGZMOD/WLDATA/process ## server is: '%s' '%i'\n",
			 greeter, greeterversion);
		ggz_free(greeter);
		ggz_write_int(m_data_fd, op_request_protocol_ext);
		break;
	case op_request_ip:
		// This it not used (anymore?). Return 255.255.255.255 to the server
		log("GGZMOD/WLDATA/process ## ip request!\n");
		snprintf(ipaddress, sizeof(ipaddress), "%i.%i.%i.%i", 255, 255, 255, 255);
		ggz_write_int(m_data_fd, op_reply_ip);
		ggz_write_string(m_data_fd, ipaddress);
		break;
	case op_broadcast_ip:
		ggz_read_string_alloc(m_data_fd, &ipstring);
		log("GGZMOD/WLDATA/process ## got ip broadcast: '%s'\n", ipstring);
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
		log("GGZMOD/WLDATA/process ## got extended protocol version: ");
		{
			std::list<WLGGZParameter> parlist = 
			wlggz_read_parameter_list(m_data_fd);
			if (parlist.size() < 2) {
				log("invalid data: %i parameters\n", parlist.size());
			} else {
				m_server_ver = parlist.front().get_integer() << 16;
				parlist.pop_front();
				m_server_ver += parlist.front().get_integer();
				parlist.pop_front();
				log("%X\n", m_server_ver);
				if (parlist.size())
					log
						("GGZMOD/WLDATA/process ## %i parameters left\n",
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
					log("server module ## %s\n", parlist.front().get_string().c_str());
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
	if (ggz_wlmodule::ref().get_ext_proto_ver() == 0)
	{
		log("GGZWLMODULE not supported by server\n");
		return false;
	}
	if (ggz_ggzcore::ref().is_in_table()) {
		if (ggz_write_int(m_data_fd, op_game_information) < 0)
			log("GGZWLMODULE ERROR: Game information could not be send!\n");

		WLGGZ_writer w = WLGGZ_writer(m_data_fd);

		w.type(gameinfo_mapname);
		w << mapname;

		w.type(gameinfo_mapsize);
		w << map_w << map_h;
			
		w.type(gameinfo_gametype);
		w << win_condition;
			
		w.type(gameinfo_version);
		w << build_id() << build_type();
			
		log("Iterate Players\n");
		std::vector<Net_Player_Info>::iterator pit = playerinfo.begin();
		while (pit != playerinfo.end())
		{
			w.type(gameinfo_playerid);
			w << pit->playernum;
			w.type(gameinfo_playername);
			w << pit->name;
			w.type(gameinfo_tribe);
			w << pit->tribe;
			w.type(gameinfo_playertype);
			w << static_cast<int>(pit->type);
			w.type(gameinfo_teamnumber);
			w << pit->team;
			pit++;
		}
		log("Player Iterate finished\n");
		w.flush();
		ggz_write_int(m_data_fd, 0); // why this???
	} else
		log("GGZWLMODULE ERROR: GGZ not used!\n");
	log("GGZWLMODULE NetGGZ::send_game_info(): ende\n");
	return true;
}

bool ggz_wlmodule::send_statistics
	(int32_t gametime,
	 const Widelands::Game::General_Stats_vector & resultvec,
	 std::vector<Net_Player_Info> playerinfo)
{
	if (get_ext_proto_ver() == 0)
	{
		log("GGZMOD not supported by server\n");
		return false;
	}

	if (ggz_ggzcore::ref().is_in_table()) {
		log("NetGGZ::send_game_statistics: send statistics to metaserver now!\n");
		ggz_write_int(m_data_fd, op_game_statistics);

		WLGGZ_writer w = WLGGZ_writer(m_data_fd);

		w.type(gamestat_gametime);
		w << gametime;

		log
			("resultvec size: %d, playerinfo size: %d\n",
			 resultvec.size(), playerinfo.size());

		for (unsigned int i = 0; i < playerinfo.size(); i++) {
			w.type(gamestat_playernumber);
			w << static_cast<int>(i);

			w.type(gamestat_result);
			w << playerinfo.at(i).result;

			w.type(gamestat_points);
			w << playerinfo.at(i).points;

			w.type(gamestat_land);
			if (resultvec.at(i).land_size.size())
				w << static_cast<int>(resultvec.at(i).land_size.back());
			else
				w << 0;

			w.type(gamestat_buildings);
			if (resultvec.at(i).nr_buildings.size())
				w << static_cast<int>(resultvec.at(i).nr_buildings.back());
			else
				w << 0;

			w.type(gamestat_militarystrength);
			if (resultvec.at(i).miltary_strength.size())
				w << static_cast<int>(resultvec.at(i).miltary_strength.back());
			else
				w << 0;

			w.type(gamestat_casualties);
			if (resultvec.at(i).nr_casualties.size())
				w << static_cast<int>(resultvec.at(i).nr_casualties.back());
			else
				w << 0;

			w.type(gamestat_land);
			w << static_cast<int>(resultvec[i].nr_civil_blds_defeated.back());

			w.type(gamestat_civbuildingslost);
			if (resultvec.at(i).nr_civil_blds_lost.size())
				w << static_cast<int>(resultvec.at(i).nr_civil_blds_lost.back());
			else
				w << 0;

			w.type(gamestat_kills);
			if (resultvec.at(i).nr_kills.size())
				w << static_cast<int>(resultvec.at(i).nr_kills.back());
			else
				w << 0;

			w.type(gamestat_buildingsdefeat);
			w << static_cast<int>(resultvec[i].nr_msites_defeated.back());

			w.type(gamestat_milbuildingslost);
			if (resultvec.at(i).nr_msites_lost.size())
				w << static_cast<int>(resultvec.at(i).nr_msites_lost.back());
			else
				w << 0;

			w.type(gamestat_wares);
			if (resultvec.at(i).nr_wares.size())
				w << static_cast<int>(resultvec.at(i).nr_wares.back());
			else
				w << (0);

			w.type(gamestat_workers);
			if (resultvec.at(i).nr_workers.size())
				w << static_cast<int>(resultvec.at(i).nr_workers.back());
			else
				w << (0);

			w.type(gamestat_productivity);
			if (resultvec.at(i).productivity.size())
				w << static_cast<int>(resultvec.at(i).productivity.back());
			else
				w << (0);
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
		ggz_write_int(m_data_fd, 0);
		} else
			log("GGZMOD ERROR: not in table!\n");
		return true;
}
