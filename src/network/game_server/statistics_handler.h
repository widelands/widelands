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

#ifndef __STATISITCS_HANDLER_H__
#define __STATISITCS_HANDLER_H__

#include "protocol_helpers.h"

class Client;
class WidelandsMap;
class WidelandsPlayer;

class StatisticsHandler {
	public:
		StatisticsHandler();
		~StatisticsHandler();
		bool report_gameinfo (Client const * client, WLGGZParameterList & p);
		bool report_game_result (Client const * client, WLGGZParameterList & p);

		bool have_stats() { return m_result_gametime != 0; }
		WidelandsMap & map() {return m_map; }
		int game_end_time() { return m_result_gametime; }
	private:
		void read_stat_vector
			(WidelandsPlayer & plr, WLGGZGameStats type,
			 WLGGZParameterList & p, int count);
		WidelandsMap m_map;
		std::string m_host_version, m_host_build;
		int m_result_gametime;
};

#endif //__STATISITCS_HANDLER_H__
