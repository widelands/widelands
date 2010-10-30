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

struct WidelandsStatSample {
	uint32_t min, max, avg;
};

struct WidelandsPlayerStatVecs
{
		std::vector<WidelandsStatSample> land;
		std::vector<WidelandsStatSample> buildings;
		std::vector<WidelandsStatSample> milbuildingslost;
		std::vector<WidelandsStatSample> civbuildingslost;
		std::vector<WidelandsStatSample> buildingsdefeat;
		std::vector<WidelandsStatSample> milbuildingsconq;
		std::vector<WidelandsStatSample> economystrength;
		std::vector<WidelandsStatSample> militarystrength;
		std::vector<WidelandsStatSample> workers;
		std::vector<WidelandsStatSample> wares;
		std::vector<WidelandsStatSample> productivity;
		std::vector<WidelandsStatSample> casualties;
		std::vector<WidelandsStatSample> kills;
		std::vector<WidelandsStatSample> custom;
};

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
		uint32_t custom;
};

#define SUPPORT_B16_PROTOCOL(p) (p != 0 and p->support_build16_proto())

class WidelandsPlayer
{
	public:
		WidelandsPlayer(int id):
			result(0),
			points(0),
			bonus(0),
			m_name(),
			m_wlid(id),
			m_tribe(),
			m_team(0)
			{}

		std::string tribe() { return m_tribe; }

		unsigned int team() { return m_team; }

		void set_tribe(std::string tribe) { m_tribe=tribe; }

		void set_team(unsigned int t)
			{ m_team = t; }
			
		void set_name(std::string name)
			{ m_name = name; }

		std::string name() { return m_name; }
		int wlid() { return m_wlid; }

		WidelandsPlayerStats last_stats;
		WidelandsPlayerStatVecs stats;

		void set_end_time(int d) { m_end_time = d; }
		int end_time() { return m_end_time; }

		uint32_t result;
		uint32_t points;
		uint32_t bonus;

		int max_wares, max_workers, max_military, max_buildings;

	private:
		WidelandsPlayer();
		std::string m_name;
		int m_wlid;
		std::string m_tribe;

		unsigned int m_team;

		int m_end_time;
};

#endif //__WIDELANDS_PLAYER_H__
