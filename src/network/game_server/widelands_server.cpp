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

// Header file
#include "widelands_server.h"

// Widelands includes
#include "protocol.h"
#include "protocol_helpers.h"

// GGZ includes
#include <ggz.h>

// System includes
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Constructor: inherit from ggzgameserver
WidelandsServer::WidelandsServer()
: GGZGameServer()
{
	std::cout << "WidelandsServer: launched!" << std::endl;
	m_wlserver_ip = NULL;
}

// Destructor
WidelandsServer::~WidelandsServer()
{
	std::cout << "WidelandsServer: closing!" << std::endl;
	std::cout << "WidelandsServer: Map \"" << m_map.name() << "\" (" << m_map.w() <<
		", " << m_map.h() << ")" << std::endl;
	std::cout << "WidelandsServer: GameTime: " << m_result_gametime << std::endl;
	
	std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();
	while(it != m_players.end())
	{
		std::cout << "WidelandsServer: Player (ggz: " << it->second->ggz_player_number() << 
			", wl: " <<  it->second->wl_player_number() << " , \"" << it->first << 
			"\") " << it->second->tribe() << std::endl;
			
		std::cout << "WidelandsServer:      Stats: land: " << it->second->stats.land << ", buildings: " <<
			it->second->stats.buildings << ", economy-strength: " << it->second->stats.economystrength << std::endl;
		std::cout << "WidelandsServer:             points: " << it->second->stats.points << ", milbuildingslost: " <<
			it->second->stats.milbuildingslost << ", civbuildingslost: " << it->second->stats.civbuildingslost << std::endl;
		it++;
	}
	
	if(m_wlserver_ip)
		ggz_free(m_wlserver_ip);
}
/*
result;
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
		int kills;*/

// State change hook
void WidelandsServer::stateEvent()
{
	std::cout << "WidelandsServer: stateEvent: ";
	switch(state())
	{
		case created:
			std::cout << "created" << std::endl;
			break;
		case waiting:
			std::cout << "waiting" << std::endl;
			break;
		case playing:
			std::cout << "playing" << std::endl;
			break;
		case done:
			std::cout << "done" << std::endl;
			break;
		case restored:
			std::cout << "restored" << std::endl;
			break;
		default:
			std::cout << "unkown state" << std::endl;
	}
}

// Player join hook
void WidelandsServer::joinEvent(Client * const client)
{
	std::cout << "WidelandsServer: joinEvent" << std::endl;

	// Send greeter
	int const channel = fd(client->number);
	ggz_write_int(channel, op_greeting);
	ggz_write_string(channel, "widelands server");
	ggz_write_int(channel, WIDELANDS_PROTOCOL);

	if (client->number == 0) {
		ggz_write_int(channel, op_request_ip);
		changeState(GGZGameServer::playing);
	} else {
		ggz_write_int(channel, op_broadcast_ip);
		ggz_write_string(channel, m_wlserver_ip);
	}
	std::cout << "WidelandsServer: Player " << client->number << " \"" << client->name;
	std::cout << "\"" << (client->spectator?" (is spectator)":"") <<  std::endl;
}

// Player leave event
void WidelandsServer::leaveEvent(Client * client)
{
	std::cout << "WidelandsServer: leaveEvent \"" << client->name << "\"" << std::endl;
}

// Spectator join event (ignored)
void WidelandsServer::spectatorJoinEvent(Client *)
{
	std::cout << "WidelandsServer: spectatorJoinEvent" << std::endl;
}

// Spectator leave event (ignored)
void WidelandsServer::spectatorLeaveEvent(Client *)
{
	std::cout << "WidelandsServer: spectatorLeaveEvent" << std::endl;
}

// Spectator data event (ignored)
void WidelandsServer::spectatorDataEvent(Client *)
{
	std::cout << "WidelandsServer: spectatorDataEvent" << std::endl;
}

void WidelandsServer::read_game_information(int fd)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;

	//std::cout << "WidelandsServer: GAMEINFO: start reading" << std::endl;

	ggz_read_int(fd, &gameinfo);
	//std::cout << "WidelandsServer: GAMEINFO: read_int ("<< gameinfo <<") gameinfo_type\n";
	while(gameinfo)
	{
		std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(fd);
		
		switch(gameinfo)
		{
			case gameinfo_playerid:
				//std::cout << "WidelandsServer: GAMEINFO: gameinfo_playerid\n";
				playernum = parlist.front().get_integer();
				playername="";
				player = NULL;
				break;
			case gameinfo_playername:
			{
				//std::cout << "WidelandsServer: GAMEINFO: gameinfo_playername\n";
				parlist.front().get_string();
				
				if(playername=="" and not parlist.front().get_string().empty() and player == NULL)
				{
					playername = parlist.front().get_string();
					player = m_players[playername];
					if(player == 0)
						m_players[playername] = player = new WidelandsPlayer(playername, playernum);
				}
				else
					std::cout << "WidelandsServer: GAMEINFO: error playername \"" << playername <<
						"\" " << parlist.front().get_string().size() << ": " << parlist.front().get_string() << std::endl;
			}
				break;
			case gameinfo_tribe:
				//std::cout << "WidelandsServer: GAMEINFO: gameinfo_tribe\n";
				if( player->tribe().empty())
					player->set_tribe(parlist.front().get_string());
				if( player->tribe().compare(parlist.front().get_string()))
					std::cout << "WidelandsServer: GAMEINFO: readinfo tribe: clients disagree about tribe: " <<
						player->tribe() << ", " << parlist.front().get_string() << std::endl;
				break; 
			case gameinfo_gametype:
				//std::cout << "WidelandsServer: GAMEINFO: gameinfo_gametype\n";
				m_map.set_gametype(static_cast<WLGGZGameType>(parlist.front().get_integer()));
				break;
			case gameinfo_mapname:
				//std::cout << "WidelandsServer: GAMEINFO: gameinfo_mapname" << std::endl;
				if(m_map.name().empty())
				{
					std::string mapname = parlist.front().get_string();
					m_map.set_name(mapname);
					//std::cout << "WidelandsServer: GAMEINFO: map_name \"" << mapname << "\"" << std::endl;
				}
				break;
			case gameinfo_mapsize:
			{
				//std::cout << "WidelandsServer: GAMEINFO: gameinfo_mapsize" << std::endl;
				int width, height;
				width = parlist.front().get_integer();
				parlist.pop_front();
				height = parlist.front().get_integer();
				m_map.set_size(width, height);
				//std::cout << "WidelandsServer: GAMEINFO: map_size (" << width << ", " << height << ")" << std::endl;
			}
				break;
			case gameinfo_playertype:
				if(player)
					player->set_type(static_cast<WLGGZPlayerType>(parlist.front().get_integer()));
				break;
			default:
				std::cout << "WidelandsServer: GAMEINFO: error unknown WLGGZGameInfo!" << std::endl;
		
		}
	ggz_read_int(fd, &gameinfo);
	//std::cout << "WidelandsServer: GAMEINFO: read_int ("<< gameinfo <<") gameinfo_type\n";
	}
	//std::cout << "WidelandsServer: GAMEINFO: finished" << std::endl;
}

void WidelandsServer::read_game_statistics(int fd)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;

	//std::cout << "WidelandsServer: GAMESTATISTICS: start reading" << std::endl;

	ggz_read_int(fd, &gameinfo);
	//std::cout << "WidelandsServer: GAMESTATISTICS: read_int ("<< gameinfo <<") stats_type\n";
	while(gameinfo)
	{
		std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(fd);
		
		switch(gameinfo)
		{
			case gamestat_playernumber:
			{
				std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();
				player=NULL;
				while(it != m_players.end())
				{
					if(it->second->wl_player_number()==parlist.front().get_integer())
						player=it->second;
					it++;
				}
				if(not player)
					std::cout << "GGZ:: GAMESTATISTICS: ERROR: got playernumber but could no find the player " <<
					parlist.front().get_integer() << std::endl;
				break;
			}
			case gamestat_result:
				if(player)
					player->stats.result=parlist.front().get_integer();
				break;
			case gamestat_points:
				if(player)
					player->stats.points=parlist.front().get_integer();
				break;
			case gamestat_land:
				if(player)
					player->stats.land=parlist.front().get_integer();
				break;
			case gamestat_buildings:
				if(player)
					player->stats.buildings=parlist.front().get_integer();
				break;
			case gamestat_milbuildingslost:
				if(player)
					player->stats.milbuildingslost=parlist.front().get_integer();
				break;
			case gamestat_civbuildingslost:
				if(player)
					player->stats.civbuildingslost=parlist.front().get_integer();
				break;
			case gamestat_buildingsdefeat:
				if(player)
					player->stats.buildingsdefeat=parlist.front().get_integer();
				break;
			case gamestat_milbuildingsconq:
				if(player)
					player->stats.milbuildingsconq=parlist.front().get_integer();
				break;
			case gamestat_economystrength:
				if(player)
					player->stats.economystrength=parlist.front().get_integer();
				break;
			case gamestat_militarystrength:
				if(player)
					player->stats.militarystrength=parlist.front().get_integer();
				break;
			case gamestat_workers:
				if(player)
					player->stats.workers=parlist.front().get_integer();
				break;
			case gamestat_wares:
				if(player)
					player->stats.wares=parlist.front().get_integer();
				break;
			case gamestat_productivity:
				if(player)
					player->stats.productivity=parlist.front().get_integer();
				break;
			case gamestat_casualties:
				if(player)
					player->stats.casualties=parlist.front().get_integer();
				break;
			case gamestat_kills:
				if(player)
					player->stats.kills=parlist.front().get_integer();
				break;
			case gamestat_gametime:
				m_result_gametime=parlist.front().get_integer();
				break;
			default:
				std::cout << "WidelandsServer: GAMESTATISTICS: error unknown WLGGZGameStats!" << std::endl;
		}
	ggz_read_int(fd, &gameinfo);
	//std::cout << "WidelandsServer: GAMESTATISTICS: read_int ("<< gameinfo <<") stats_type\n";
	}
	//std::cout << "WidelandsServer: GAMESTATISTICS: finished" << std::endl;
}


// Game data event
void WidelandsServer::dataEvent(Client * const client)
{
	int opcode;
	int ret;
	struct sockaddr* addr;
	socklen_t addrsize;

	std::cout << "WidelandsServer: dataEvent" << std::endl;

	// Read data
	int const channel = fd(client->number);

	ggz_read_int(channel, &opcode);
	//std::cout << "WidelandsServer: dataEvent: read opcode ("<< opcode <<")"<< std::endl;

	int teams[2] = {0, 1};
	GGZGameResult gr[2] = {GGZ_GAME_WIN, GGZ_GAME_LOSS};
	int scores[2] = {100, 10};

	switch (opcode) {
	case op_reply_ip:
		char * ip;

		//  Do not use IP provided by client. Instead, determine peer IP address.
		ggz_read_string_alloc(channel, &ip);
		std::cout << "WidelandsServer: reply_ip: " << ip << std::endl;
		//  m_wlserver_ip = ggz_strdup(ip);
		ggz_free(ip);

		addrsize = 256;
		addr = static_cast<struct sockaddr *>(malloc(addrsize));
		ret = getpeername(channel, addr, &addrsize);

		//  FIXME: IPv4 compatibility?
		if (addr->sa_family == AF_INET6) {
			ip = static_cast<char *>(ggz_malloc(INET6_ADDRSTRLEN));
			inet_ntop
				(AF_INET6,
				 static_cast<void *>
				 	(&(reinterpret_cast<struct sockaddr_in6 *>(addr))->sin6_addr),
				 ip,
				 INET6_ADDRSTRLEN);
		} else if(addr->sa_family == AF_INET) {
			ip = static_cast<char *>(ggz_malloc(INET_ADDRSTRLEN));
			inet_ntop
				(AF_INET,
				 static_cast<void *>
				 	(&(reinterpret_cast<struct sockaddr_in *>(addr))->sin_addr),
				 ip,
				 INET_ADDRSTRLEN);
		} else {
			ip = NULL;
			std::cout << "WidelandsServer: GAME: unreachable -> done!" << std::endl;
			ggz_write_int(channel, op_unreachable);
			changeState(GGZGameServer::done);
			break;
		}

		//std::cout << "WidelandsServer: broadcast IP: " << ip << std::endl;
		m_wlserver_ip = ggz_strdup(ip);
		ggz_free(ip);
		{ 	// test for connectablity ???
			// This code only tests if the string m_wlserver_ip is usable.
			addrinfo * ai = 0;
			if (getaddrinfo(m_wlserver_ip, "7396", 0, &ai)) {
				std::cout << "WidelandsServer: GAME: unreachable -> done!" << std::endl;
				ggz_write_int(channel, op_unreachable);
				changeState(GGZGameServer::done);
				break;
			}
			freeaddrinfo(ai);
		}
		std::cout << "WidelandsServer: GAME: reachable -> waiting!" << std::endl;
		changeState(GGZGameServer::waiting);
		break;
	case op_state_playing:
		std::cout << "WidelandsServer: GAME: playing!" << std::endl;
		changeState(GGZGameServer::playing);
		break;
	case op_state_done:
		reportGame(teams, gr, scores);
		std::cout << "WidelandsServer: GAME: done!" << std::endl;
		/* ToDo: Do not switch to state done directly. This exit the widelands server
		 * imediately. Switch to waiting first and wait for statistics. This should have
		 * a timeout */
		changeState(GGZGameServer::done);
		break;
	case op_game_statistics:
		{
			std::cout << "WidelandsServer: GAME: read stats!" << std::endl;
			read_game_statistics(channel);
			
			/*
			int teams[players()];
			GGZGameResult results[players()];
			for (int p = 0; p < players(); p++) {
				//teams[p] = seat(p)->team;
				results[p] = GGZ_GAME_LOSS;
			}
			results[0] =  GGZ_GAME_WIN;
			//reportGame(int *teams, GGZGameResult *results, int *scores);
			reportGame(NULL, results, NULL);
			*/
		}
		break;
	case op_game_information:
	{
		std::cout << "WidelandsServer: GAME: read game info!" << std::endl;
		read_game_information(channel);
		WidelandsPlayer * player = m_players[client->name];
		if(!&player)
			std::cout << "WidelandsServer: GAMEINFO: read gameinfo but player does not exist: " << client->name << std::endl;
		else
			player->set_ggz_player_number(client->number);
		break;
	}
	default:
		//  Discard
		std::cerr << "WidelandsServer: Data error. Unhandled opcode(" << opcode << ")!" << std::endl;
		break;
	}

}

// Error handling event
void WidelandsServer::errorEvent()
{
	std::cout << "WidelandsServer: errorEvent" << std::endl;
}

/*
void WidelandsServer::game_start()
{
	std::cout << "WidelandsServer: game_start()" << std::endl;
}

void WidelandsServer::game_stop()
{
	std::cout << "WidelandsServer: game_stop()" << std::endl;
}

void WidelandsServer::game_end()
{
	std::cout << "WidelandsServer: game_end()" << std::endl;
}
*/
