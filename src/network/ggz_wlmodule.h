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

#ifndef GGZ_WLMODULE_H
#define GGZ_WLMODULE_H

#include <ggzmod.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "network_ggz.h"
#include "logic/game.h"


/**
 * The third part of ggz. This communicates with the widelands game server
 * module of the ggzd server. It gets an data fd from ggzmod to the game module.
 */
class ggz_wlmodule
{
public:
	static ggz_wlmodule & ref();
	void process();
	bool data_pending() {return false;}

	void set_datafd(int fd) { m_data_fd = fd; }

	char * get_server_ip() { return server_ip_addr; }
	uint32_t get_ext_proto_ver() { return m_server_ver; }

	bool send_game_info
		(std::string mapname, int map_w, int map_h, int win_condition,
		 std::vector<Net_Player_Info> playerinfo);
	bool send_statistics
		(int32_t gametime,
		 const Widelands::Game::General_Stats_vector & resultvec,
		 std::vector<Net_Player_Info> playerinfo);

private:
	ggz_wlmodule();
	int32_t m_data_fd;
	char * server_ip_addr;
	uint32_t m_server_ver;
};

#endif // GGZ_WLMODULE_H