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
#include "log.h"
#include "wlggz_exception.h"

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
				player = NULL;
				break;
			case gameinfo_playername:
			{
				wllog(DL_DUMPDATA, "gameinfo_playername");
				CHECKTYPE(l, string)
				if
					(playername.empty() and not
					 l.front().get_string().empty() and player == NULL)
				{
					playername = l.front().get_string();
					wllog
						(DL_DEBUG, "GAMEINFO: add player \"%s\" (wl: %i)",
						 playername.c_str(), playernum);
						 player = g_wls->get_player_by_name(playername, true);
						 player->set_wl_player_number(playernum);
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
				if (player->tribe().empty())
					player->set_tribe(l.front().get_string());
				if (player->tribe().compare(l.front().get_string())) {
					wllog
						(DL_WARN,
						 "GAMEINFO: readinfo tribe: "
						 "clients disagree about tribe: %s, %s",
						 player->tribe().c_str(),
						 l.front().get_string().c_str());
					if (g_wls->get_player_by_name(client->name)->is_host())
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
				if(player)
					player->set_type
						(static_cast<WLGGZPlayerType>(l.front().get_integer()));
				break;
			case gameinfo_version:
			{
				CHECKTYPE(l, string)
				std::string version = l.front().get_string();
				l.pop_front();
				CHECKTYPE(l, string)
				std::string build = l.front().get_string();

				player = g_wls->get_player_by_name(client->name);
				if (not player) {
					player = g_wls->get_player_by_name(client->name, true);
					player->set_ggz_player_number(client->number);
				}

				if (player->is_host())
				{
					m_host_version = version;
					m_host_build = build;
				}
				
				player->set_version(version, build);
				wllog
					(DL_DUMP, "GAMEINFO: Player \"%s\": %s(%s)",
					 client->name.c_str(), version.c_str(), build.c_str());
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

	int num = g_wls->numberofplayers();
	wllog(DL_INFO, "GAMEINFO: number of players %i", num);

	for (int i=0; i < num; i++)
	{
		WidelandsPlayer * plr = g_wls->get_player_by_ggzid(i);
		if (plr)
		{
			wllog
				(DL_INFO,
				 "GAMEINFO: seat %i(%i): %i Name: %s(%i) \"%s\" (%s - %s) %s",
				 i, plr->ggz_player_number(), plr->wl_player_number(),
				 plr->name().c_str(), plr->team(), plr->tribe().c_str(),
				 plr->version().c_str(), plr->build().c_str(),
				 plr->is_host()?"Host":"");
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
				player = g_wls->get_player_by_wlid(playernum);
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
			case gamestat_buildings:
			case gamestat_milbuildingslost:
			case gamestat_civbuildingslost:
			case gamestat_buildingsdefeat:
			case gamestat_milbuildingsconq:
			case gamestat_economystrength:
			case gamestat_militarystrength:
			case gamestat_workers:
			case gamestat_wares:
			case gamestat_productivity:
			case gamestat_casualties:
			case gamestat_kills:
				if (not player)
					wllog(DL_ERROR, "Got a statistic vector but have no player");
				else {
					CHECKTYPE(l, integer)
					int last = l.front().is_integer();
					l.pop_front();
					CHECKTYPE(l, integer);
					int count = l.front().is_integer();
					l.pop_front();
					switch(gameinfo){
						case gamestat_land:
							player->last_stats.land = last;
							break;
						case gamestat_buildings:
							player->last_stats.buildings = last;
							break;
						case gamestat_milbuildingslost:
							player->last_stats.milbuildingslost = last;
							break;
						case gamestat_civbuildingslost:
							player->last_stats.civbuildingslost = last;
							break;
						case gamestat_buildingsdefeat:
							player->last_stats.buildingsdefeat = last;
							break;
						case gamestat_milbuildingsconq:
							player->last_stats.milbuildingsconq = last;
							break;
						case gamestat_economystrength:
							player->last_stats.economystrength = last;
							break;
						case gamestat_militarystrength:
							player->last_stats.militarystrength = last;
							break;
						case gamestat_workers:
							player->last_stats.workers = last;
							break;
						case gamestat_wares:
							player->last_stats.wares = last;
							break;
						case gamestat_productivity:
							player->last_stats.productivity = last;
							break;
						case gamestat_casualties:
							player->last_stats.casualties = last;
							break;
						case gamestat_kills:
							player->last_stats.kills = last;
							break;
						default:
							wllog(DL_ERROR, "unknown statistic vector");
					}

					try {
						read_stat_vector(*player, gameinfo, l, count);
					} catch(_parameterError e) {
						wllog
							(DL_ERROR,
							 "Catched parameter error while "
							 "reading statisitcs vector: %s", e.what());
					}
				}
				break;
			case gamestat_gametime:
				CHECKTYPE(l, integer);
				wllog
					(DL_DEBUG, "game end time by %s(%i): %i",
					 client->name.c_str(), client->number, l.front().get_integer());
				if (m_result_gametime == 0)
					m_result_gametime = l.front().get_integer();
				else
					if (m_result_gametime != l.front().get_integer())
						wllog(DL_WARN, "client report at different gametimes");
				break;
			default:
				wllog
					(DL_WARN, "GAMESTATISTICS: Warning unknown WLGGZGameStats: %i (%i)",
					 gameinfo, l.size());
		}
	}
}

void StatisticsHandler::read_stat_vector
	(WidelandsPlayer& plr, WLGGZGameStats type, WLGGZParameterList& p, int count)
{
	if (count > 1024)
		wllog(DL_WARN, "statistic vector longer than 1024 samples");
	
	int i = 0;
	while(p.size() and i++ < 1024) {
		CHECKTYPE(p, list)
		WLGGZParameterList l = p.front().get_list();
		int sample = p.front().get_list_type();
		p.pop_front();
		if (l.size() < 3)
			throw parameterError();
		CHECKTYPE(l, integer);
		int avg = l.front().get_integer();
		CHECKTYPE(l, integer);
		int min = l.front().get_integer();
		CHECKTYPE(l, integer);
		int max = l.front().get_integer();
		std::cout << "sample "<< sample << ": avg: " << avg << ", min: " <<
			min << ", max: " << max << std::endl;
		if (sample > (plr.stats_avg.size() + 1))
		{
			wllog(DL_ERROR, "got non continous stats");
			return;
		}
		if (sample > count) {
			wllog(DL_ERROR, "got a statistic sample with number > count");
			return;
		}
		switch(type) {
			case gamestat_land:
				plr.stats_avg[sample].land = avg;
				plr.stats_min[sample].land = min;
				plr.stats_max[sample].land = max;
				break;
			case gamestat_buildings:
				plr.stats_avg[sample].buildings = avg;
				plr.stats_min[sample].buildings = min;
				plr.stats_max[sample].buildings = max;
				break;
			case gamestat_milbuildingslost:
				plr.stats_avg[sample].milbuildingslost = avg;
				plr.stats_min[sample].milbuildingslost = min;
				plr.stats_max[sample].milbuildingslost = max;
				break;
			case gamestat_civbuildingslost:
				plr.stats_avg[sample].civbuildingslost = avg;
				plr.stats_min[sample].civbuildingslost = min;
				plr.stats_max[sample].civbuildingslost = max;
				break;
			case gamestat_buildingsdefeat:
				plr.stats_avg[sample].buildingsdefeat = avg;
				plr.stats_min[sample].buildingsdefeat = min;
				plr.stats_max[sample].buildingsdefeat = max;
				break;
			case gamestat_milbuildingsconq:
				plr.stats_avg[sample].milbuildingsconq = avg;
				plr.stats_min[sample].milbuildingsconq = min;
				plr.stats_max[sample].milbuildingsconq = max;
				break;
			case gamestat_economystrength:
				plr.stats_avg[sample].economystrength = avg;
				plr.stats_min[sample].economystrength = min;
				plr.stats_max[sample].economystrength = max;
				break;
			case gamestat_militarystrength:
				plr.stats_avg[sample].militarystrength = avg;
				plr.stats_min[sample].militarystrength = min;
				plr.stats_max[sample].militarystrength = max;
				break;
			case gamestat_workers:
				plr.stats_avg[sample].workers = avg;
				plr.stats_min[sample].workers = min;
				plr.stats_max[sample].workers = max;
				break;
			case gamestat_wares:
				plr.stats_avg[sample].wares = avg;
				plr.stats_min[sample].wares = min;
				plr.stats_max[sample].wares = max;
				break;
			case gamestat_productivity:
				plr.stats_avg[sample].productivity = avg;
				plr.stats_min[sample].productivity = min;
				plr.stats_max[sample].productivity = max;
				break;
			case gamestat_casualties:
				plr.stats_avg[sample].casualties = avg;
				plr.stats_min[sample].casualties = min;
				plr.stats_max[sample].casualties = max;
				break;
			case gamestat_kills:
				plr.stats_avg[sample].kills = avg;
				plr.stats_min[sample].kills = min;
				plr.stats_max[sample].kills = max;
				break;
			default:
				wllog(DL_ERROR, "unknown stat");
		}
	}
	wllog(DL_DUMP, "read %i/%i statistic samples for %i", i, count, type);
}
