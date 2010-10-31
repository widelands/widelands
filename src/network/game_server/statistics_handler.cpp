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

#include "widelands_server.h"
#include "statistics_handler.h"
#include "widelands_map.h"
#include "widelands_player.h"
#include "widelands_client.h"
#include "log.h"
#include "wlggz_exception.h"

#include <cassert>

StatisticsHandler::StatisticsHandler():
m_result_gametime(0),
m_map(),
m_host_version(),
m_host_build()
{}

StatisticsHandler::~StatisticsHandler()
{}

#define CHECKTYPE(list, type) if ((list).empty() or not (list).front().is_##type ()) \
	throw _parameterError(__FILE__, __LINE__);

bool StatisticsHandler::report_gameinfo(Client const * client, WLGGZParameterList & p)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;
	WidelandsClient & wlclient = *g_wls->get_client_by_name(client->name);
	assert(&wlclient);

	wllog(DL_DEBUG, "StatisticsHandler::report_gameinfo");

	while(not p.empty())
	{
		CHECKTYPE(p, list)

		int type = p.front().get_list_type();
		WLGGZParameterList l = p.front().get_list();
		p.pop_front();
		switch(type)
		{
			case gameinfo_playerid:
				wllog(DL_DUMPDATA, "gameinfo_playerid");
				CHECKTYPE(l, integer)
				playernum = l.front().get_integer();
				wllog(DL_DUMPDATA, "playerid: %i", playernum);
				playername.erase();
				player = get_player(playernum);
				if (not player) {
					player = new WidelandsPlayer(playernum);
					m_players[playernum] = player;
				}
				break;
			case gameinfo_ownplayerid:
				CHECKTYPE(l, integer)
				{
					wllog
						(DL_DUMP, "Got playerid %i for client %s",
						 l.front().get_integer(),
						 wlclient.name().c_str());
					wlclient.set_wl_number(l.front().get_integer());
				}
				break;
			case gameinfo_playername:
			{
				wllog(DL_DUMPDATA, "gameinfo_playername");
				assert(player);
				CHECKTYPE(l, string)
				if
					(playername.empty() and not
					 l.front().get_string().empty())
				{
					playername = l.front().get_string();
					player->set_name(playername);
					wllog
						(DL_DEBUG, "GAMEINFO: add player \"%s\" (wl: %i)",
						 playername.c_str(), playernum);
				}
				else
					wllog
						(DL_ERROR,
						 "GAMEINFO: error playername \"%s\" %i: %s",
						 playername.c_str(), l.front().get_string().size(),
						 l.front().get_string().c_str());
			}
				break;
			case gameinfo_tribe:
				wllog(DL_DUMPDATA, "gameinfo_tribe");
				CHECKTYPE(l, string)
				assert(player);
				if (player->tribe().empty())
					player->set_tribe(l.front().get_string());
				if (player->tribe().compare(l.front().get_string())) {
					wllog
						(DL_WARN,
						 "GAMEINFO: readinfo tribe: "
						 "clients disagree about tribe: %s, %s",
						 player->tribe().c_str(),
						 l.front().get_string().c_str());
					if (g_wls->is_host(client))
						player->set_tribe(l.front().get_string());
				}		
				break; 
			case gameinfo_gametype:
				wllog(DL_DUMPDATA, "gameinfo_gametype");
				CHECKTYPE(l, integer)
					m_map.set_gametype
						(static_cast<WLGGZGameType>(l.front().get_integer()));
				break;
			case gameinfo_mapname:
				wllog(DL_DUMPDATA, "gameinfo_mapname");
				wllog(DL_DUMPDATA, "list.size(): %i", l.size());
				CHECKTYPE(l, string)
				if(m_map.name().empty())
				{
					std::string mapname = l.front().get_string();
					m_map.set_name(mapname);
				}
				break;
			case gameinfo_mapsize:
			{
				wllog(DL_DUMPDATA, "gameinfo_mapsize");
				int width, height;
				CHECKTYPE(l, integer)
				width = l.front().get_integer();
				l.pop_front();
				CHECKTYPE(l, integer)
				height = l.front().get_integer();
				m_map.set_size(width, height);
				break;
			}
			case gameinfo_playertype:
				CHECKTYPE(l, integer)
				if(&wlclient)
					wlclient.set_type
						(static_cast<WLGGZPlayerType>(l.front().get_integer()));
				break;
			case gameinfo_version:
			{
				CHECKTYPE(l, string)
				std::string version = l.front().get_string();
				l.pop_front();
				CHECKTYPE(l, string)
				std::string build = l.front().get_string();

				if (g_wls->is_host(&wlclient))
				{
					m_host_version = version;
					m_host_build = build;
				}
				
				wlclient.set_version(version, build);
				wllog
					(DL_DUMP, "GAMEINFO: Player \"%s\": %s(%s)",
					 wlclient.name().c_str(), version.c_str(), build.c_str());
				break;
			}
			case gameinfo_teamnumber:
				CHECKTYPE(l, integer)
				if(player)
					player->set_team(l.front().get_integer());
				break;
			default:
				wllog(DL_ERROR, "GAMEINFO: error unknown WLGGZGameInfo: %i", type);
		}
	}

	wllog(DL_INFO, "GAMEINFO: number of players %i", m_players.size());

	for (int i=0; i < highest_playernum(); i++)
	{
		WidelandsPlayer * plr = get_player(i);
		if (plr)
		{
			wllog
				(DL_INFO,
				 "GAMEINFO: player %i(%i): Name: %s(%i) \"%s\"",
				 i, plr->wlid(),
				 plr->name().c_str(), plr->team(), plr->tribe().c_str());
		}
	}
}

bool StatisticsHandler::report_game_result
	(Client const * client, WLGGZParameterList & p)
{
	WLGGZGameStats gameinfo;
	int playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;
	WidelandsClient & wlclient = *g_wls->get_client_by_name(client->name);
	std::vector<WidelandsStatSample> *statvec = NULL;

	wllog(DL_DEBUG, "StatisticsHandler::report_game_result");

	while(p.size())
	{
		CHECKTYPE(p, list);
		gameinfo = static_cast<WLGGZGameStats>(p.front().get_list_type());
		WLGGZParameterList l = p.front().get_list();
		p.pop_front();
		//wllog(DL_DEBUG, "gameinfo: %i, l.size: %i", gameinfo, l.size());
		switch(gameinfo)
		{
			case gamestat_playernumber:
			{
				CHECKTYPE(l, integer)
				playernum = l.front().get_integer();
				wllog(DL_DUMP, "got playernumber %i", playernum);
				player = get_player(playernum);
					if(not player)
						wllog
							(DL_ERROR, "GAMESTATISTICS: ERROR: "
							 "got playernumber but could no find the player %i",
							 l.front().get_integer());
				break;
			}
			case gamestat_result:
				CHECKTYPE(l, integer);
				wllog
					(DL_DUMP, "got player result for %i: %i", playernum,
					 l.front().get_integer());
				if(player)
					player->result=l.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got result but have no player");
				break;
			case gamestat_points:
				CHECKTYPE(l, integer);
				wllog
					(DL_DUMP, "got player points for %i: %i", playernum,
					 l.front().get_integer());
				if(player)
					player->points=l.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got points but have no player");
				break;
			case gamestat_land:
				player->last_stats.land =
					read_stat_vector(*player, gameinfo, l, &player->stats.land);
				break;
			case gamestat_buildings:
				player->last_stats.buildings =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.buildings);
				break;
			case gamestat_milbuildingslost:
				player->last_stats.milbuildingslost =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.milbuildingslost);
				break;
			case gamestat_civbuildingslost:
				player->last_stats.civbuildingslost =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.civbuildingslost);
				break;
			case gamestat_buildingsdefeat:
				player->last_stats.buildingsdefeat =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.buildingsdefeat);
				break;
			case gamestat_milbuildingsconq:
				player->last_stats.milbuildingsconq =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.milbuildingsconq);
				break;
			case gamestat_economystrength:
				player->last_stats.economystrength =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.economystrength);
				break;
			case gamestat_militarystrength:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.militarystrength =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.militarystrength);
				break;
			case gamestat_workers:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.workers =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.workers);
				break;
			case gamestat_wares:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.wares =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.wares);
				break;
			case gamestat_productivity:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.productivity =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.productivity);
				break;
			case gamestat_casualties:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.casualties =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.casualties);
				break;
			case gamestat_kills:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.kills =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.kills);
				break;
			case gamestat_customstat:
				if (not player) {
					wllog(DL_ERROR, "Got a statistic vector but have no player");
					break;
				}
				player->last_stats.custom =
					read_stat_vector
						(*player, gameinfo, l, &player->stats.custom);
				break;
			case gamestat_gametime:
				CHECKTYPE(l, integer);
				if (player) {
					wllog
						(DL_DEBUG, "gametime for player %s(%i)",
						 player->name().c_str(), player->wlid());
						if(player->end_time() == 0)
							player->set_end_time(l.front().get_integer());
						else
							if (l.front().get_integer() != player->end_time())
								wllog
									(DL_WARN,
									 "Clients disagree about player (%i) end time. "
									 "Have: %i, %s reported: %i",
									 player->wlid(), player->end_time(),
									 client->name.c_str(), l.front().get_integer());
					break;
				}
				wllog
					(DL_DEBUG, "game end time by %s(%i): %i",
					 client->name.c_str(), client->number, l.front().get_integer());
				if (m_result_gametime == 0)
					m_result_gametime = l.front().get_integer();
				else
					if (m_result_gametime != l.front().get_integer())
						wllog(DL_WARN, "client report at different gametimes");
				break;
			case gamestat_wincondstring:
					CHECKTYPE(l, string);
					if (player) {
						player->wincondstring = l.front().get_string();
						wllog
							(DL_DUMP, "got win condition extradata for \"%s\": %s",
							 player->name().c_str(), l.front().get_string().c_str());
					}
				break;
			default:
				wllog
					(DL_WARN, "GAMESTATISTICS: Warning unknown WLGGZGameStats: %i (%i)",
					 gameinfo, l.size());
		}
	}
	g_wls->check_reports();
}

uint32_t StatisticsHandler::read_stat_vector
	(WidelandsPlayer& plr, WLGGZGameStats type, WLGGZParameterList& p,
	 std::vector<WidelandsStatSample> *statvec)
{
	CHECKTYPE(p, integer)
	int last = p.front().get_integer();
	p.pop_front();
	CHECKTYPE(p, integer);
	int count = p.front().get_integer();
	p.pop_front();
	wllog
		(DL_DUMP, "got statistic vector %i. Size: %i, last: %i",
		 type, count, last);
	try {
		if (count > 1024)
			wllog(DL_WARN, "statistic vector longer than 1024 samples");
		
		int i = 0, lastsample = 0;
		while(p.size() and i++ < 1024) {
			CHECKTYPE(p, list)
			WLGGZParameterList l = p.front().get_list();
			int sample = p.front().get_list_type();
			p.pop_front();
			if (l.size() < 3)
				throw parameterError();
			CHECKTYPE(l, integer);
			int avg = l.front().get_integer();
			l.pop_front();
			CHECKTYPE(l, integer);
			int min = l.front().get_integer();
			l.pop_front();
			CHECKTYPE(l, integer);
			int max = l.front().get_integer();
			l.pop_front();
			if (sample > lastsample++)
			{
				wllog(DL_ERROR, "got non continous stats");
				return last;
			}

			if (sample >= count) {
				wllog(DL_ERROR, "got a statistic sample with number > count");
				return last;
			}

			if (statvec) {
				if (count > statvec->size()) {
					statvec->resize((count>1024)?1024:count);
				}
				statvec->at(sample).min = min;
				statvec->at(sample).max = max;
				statvec->at(sample).avg = avg;
			}
		}
		wllog(DL_DUMP, "read %i/%i statistic samples for %i", i, count, type);
		return last;
	} catch(_parameterError e) {
		wllog
			(DL_ERROR,
			 "Catched parameter error while "
			 "reading statisitcs vector: %s", e.what());
		return 0;
	} catch (std::exception e) {
		wllog
			(DL_FATAL,
			 "oops catched std::exception while "
			 "reading stat vector: %s", e.what());
		return 0;
	}
}

void StatisticsHandler::evaluate()
{
	//std::map<int, team> m_teams;
	winning_team = -1;
	//std::list<int> looser_teams;
	plr_max_wares = -1;
	plr_max_army = -1;
	plr_max_workers = -1;
	plr_max_buildings = -1;
	defeated_players = 0;
	defeated_ais = 0;
	hours_without_fights = 0;

	if (not have_stats())
		return;

	std::map<int,WidelandsPlayer*>::iterator it = m_players.begin();
	while(it != m_players.end())
	{
		WidelandsPlayer & player = *it->second;
		
		player.max_buildings = -1;
		player.max_workers = -1;
		player.max_military = -1;
		player.max_wares = -1;
		std::vector<WidelandsStatSample>::iterator it;
		it = player.stats.wares.begin();
		while (it != player.stats.wares.end()) {
			if (it->max > player.max_wares)
				player.max_wares = it->max;
			it++;
		}

		it = player.stats.workers.begin();
		while (it != player.stats.workers.end()) {
			if (it->max > player.max_workers)
				player.max_wares = it->max;
			it++;
		}

		it = player.stats.militarystrength.begin();
		while (it != player.stats.militarystrength.end()) {
			if (it->max > player.max_military)
				player.max_wares = it->max;
			it++;
		}

		it = player.stats.buildings.begin();
		while (it != player.stats.buildings.end()) {
			if (it->max > player.max_buildings)
				player.max_wares = it->max;
			it++;
		}

		if (plr_max_wares < player.max_wares) {
			plr_max_wares = player.max_wares;
		}
		if (plr_max_workers < player.max_workers) {
			plr_max_workers = player.max_workers;
		}
		if (plr_max_army >= player.max_military) {
			plr_max_army = player.max_military;
		}
		if (plr_max_buildings >= player.max_buildings) {
			plr_max_buildings = player.max_buildings;
		}

/*
		if (player.type() == playertype_human) {
			m_teams[player.team()].players++;
			m_teams[player.team()].members.push_back(&player);
			if (player.result == gamestatresult_looser)
				defeated_players++;

		} else if 
			(player.type() == playertype_ai_aggressive or
			 player.type() == playertype_ai_defensive or
			 player.type() == playertype_ai_normal)
		{
			m_teams[player.team()].ais++;
			m_teams[player.team()].members.push_back(&player);
			if (player.result == gamestatresult_looser)
				defeated_ais++;
		} else
		{
			wllog
				(DL_WARN, "Unknown Player type. %s (%i)",
				 player.name().c_str(), player.wlid());
		}
*/
		it++;
	}

	// If game run longer than three hours we have to check for time hours
	// without military activity
	if (m_result_gametime > (3 * 60 * 60 * 1000)) {
		int last_change = 23;
		int last_val[m_players.size()];
		// go through all statistic samples. There are 12 samples per hour.
		// Skip the first 2 hours (24 samples). Gametime is given in miliseconds
		// one sampe every 5 minutes.
		for (int i = 23; i <= (m_result_gametime / (1000 * 60 * 5)); i++) {
			bool change = false;
			int pc = 0;
			std::map<int, WidelandsPlayer*>::iterator it = m_players.begin();
			while(it != m_players.end()) {
				WidelandsPlayer & player = *it->second;
				if (i < player.stats.casualties.size()) {
					if (player.stats.casualties.at(i).max != last_val[pc]) {
						last_change = i;
						last_val[pc] = player.stats.casualties.at(i).max;
					}
				}
				pc++;
				it++;
			}
			if ((i - last_change) > 11) {
				hours_without_fights;
				last_change = i;
			}
		}
	}
}

void StatisticsHandler::dump_player(int num)
{
	WidelandsPlayer & p = *get_player(num);
	if (not &p) {
		wllog(DL_WARN, "dump_player(%i): do not have player", num);
		return;
	}
	wllog(DL_DUMP, "Player (%i): \"%s\"", p.wlid(), p.name().c_str());
	if (p.result == gamestatresult_winner)
		wllog(DL_DUMP, "    won, points: %i (time: %i)", p.points, p.end_time());
	else if (p.result == gamestatresult_looser)
		wllog(DL_DUMP, "    lost, points: %i (time: %i)", p.points, p.end_time());
	else if (p.result == gamestatresult_leave)
		wllog(DL_DUMP, "    left the game, points: %i (time: %i)", p.points, p.end_time());
	else 
		wllog(DL_DUMP, "    unknown result, points: %i (time: %i)", p.points, p.end_time());

	wllog
		(DL_DUMP, "     Stats: land: %i, buildings: %i, economy-strength: %i",
		 p.last_stats.land, p.last_stats.buildings,
		 p.last_stats.economystrength);

	wllog
		(DL_DUMP, "            points: %i, milbuildingslost: %i, "
		 "civbuildingslost: %i",
		 p.points, p.last_stats.milbuildingslost,
		 p.last_stats.civbuildingslost);
}

