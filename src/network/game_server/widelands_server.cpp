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
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DL_FATAL 1
#define DL_ERROR 2
#define DL_WARN 3
#define DL_INFO 4
#define DL_DEBUG 5
#define DL_DUMP 6

int send_debug = -1;
int debug_level = 9;

void wllog (int level, const char * fmt, ...)
{
	char buf[2048];
	va_list va;

	if (debug_level < level)
		return;

	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	if (send_debug > 0) {
		//ggz_write_int(send_debug, op_debug_string);
		WLGGZ_writer wr(send_debug);
		std::string sbuf = buf;
		wr.type(op_debug_string);
		wr << sbuf;
		wr.flush();
		//ggz_write_int(send_debug, 0);
	}

	std::cout << "[WLServer]: " << buf << std::endl;
	
}

// Constructor: inherit from ggzgameserver
WidelandsServer::WidelandsServer():
	GGZGameServer(),
	m_wlserver_ip(NULL),
	m_result_gametime(0),
	host_version(),
	host_build(),
	m_reported(false),
	m_map()
{
	wllog(DL_INFO, "");
	wllog(DL_INFO, "launched!");
}

// Destructor
WidelandsServer::~WidelandsServer()
{
	wllog(DL_INFO, "closing!");
	wllog
		(DL_DUMP, "Map %s (%i, %i)", m_map.name().c_str(), m_map.w(), m_map.h());
	wllog(DL_DUMP, "GameTime: %i", m_result_gametime);

	std::string wincond = "unknown (ERROR)";
	switch(m_map.gametype())
	{
		case gametype_endless:
			wincond = "endless";
			break;
		case gametype_defeatall:
			wincond = "defeat all";
			break;
		case gametype_collectors:
			wincond = "collectors";
			break;
		case gametype_tribes_together:
			wincond = "tribes together";
			break;
	}
	wllog(DL_DUMP, "Win Condition: %s", wincond.c_str());
	std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();
	while(it != m_players.end())
	{
		wllog
			(DL_DUMP, "Player (ggz: %i, wl: %i) %s, %s",
			 it->second->ggz_player_number(), it->second->wl_player_number(),
			 it->first.c_str(), it->second->tribe().c_str());

		wllog
			(DL_DUMP, "     Stats: land: %i, buildings: %i, economy-strength: %i",
			 it->second->stats.land, it->second->stats.buildings,
			 it->second->stats.economystrength);

		wllog
			(DL_DUMP, "            points: %i, milbuildingslost: %i, "
			 "civbuildingslost: %i",
			 it->second->stats.points, it->second->stats.milbuildingslost,
			 it->second->stats.civbuildingslost);
		it++;
	}

	wllog(DL_DUMP, "");

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
	std::string statestr = "unkown state";
	switch(state())
	{
		case created:
			statestr = "created";
			break;
		case waiting:
			statestr = "waiting";
			break;
		case playing:
			statestr = "playing";
			break;
		case done:
			statestr = "done";
			break;
		case restored:
			statestr = "restored";
			break;
	}
	wllog(DL_DEBUG, "stateEvent: %s", statestr.c_str());
}

// Player join hook
void WidelandsServer::joinEvent(Client * const client)
{
	wllog(DL_INFO, "joinEvent: %s", client->name.c_str());

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
	wllog
		(DL_DUMP, "Player %i \"%s\" (%s)", client->number,
		 client->name.c_str(), (client->spectator?"spectator":"player"));
}

// Player leave event
void WidelandsServer::leaveEvent(Client * client)
{
	wllog(DL_INFO, "leaveEvent \"%s\"", client->name.c_str());
}

// Spectator join event (ignored)
void WidelandsServer::spectatorJoinEvent(Client * client)
{
	wllog
		(DL_INFO, "spectatorJoinEvent \"%s\"",
		 client->name.c_str());
}

// Spectator leave event (ignored)
void WidelandsServer::spectatorLeaveEvent(Client *)
{
	wllog(DL_INFO, "spectatorLeaveEvent");
}

// Spectator data event (ignored)
void WidelandsServer::spectatorDataEvent(Client *)
{
	wllog(DL_INFO, "spectatorDataEvent");
}

void WidelandsServer::read_game_information(int fd, Client * client)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;

	ggz_read_int(fd, &gameinfo);
	while(gameinfo)
	{
		std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(fd);

		switch(gameinfo)
		{
			case gameinfo_playerid:
				playernum = parlist.front().get_integer();
				playername.erase();
				player = NULL;
				break;
			case gameinfo_playername:
			{
				parlist.front().get_string();

				if
					(playername.empty() and not
					 parlist.front().get_string().empty() and player == NULL)
				{
					playername = parlist.front().get_string();
					player = m_players[playername];
					if(player == 0)
						m_players[playername] = player =
							new WidelandsPlayer(playername, playernum);
				}
				else
					wllog
						(DL_ERROR,
						 "GAMEINFO: error playername \"%s\" %i: %s",
						 playername.c_str(), parlist.front().get_string().size(),
						 parlist.front().get_string().c_str());
			}
				break;
			case gameinfo_tribe:
				if( player->tribe().empty())
					player->set_tribe(parlist.front().get_string());
				if( player->tribe().compare(parlist.front().get_string()))
					wllog
						(DL_WARN,
						 "GAMEINFO: readinfo tribe: "
						 "clients disagree about tribe: %s, %s",
						 player->tribe().c_str(),
						 parlist.front().get_string().c_str());
				break; 
			case gameinfo_gametype:
				m_map.set_gametype
					(static_cast<WLGGZGameType>(parlist.front().get_integer()));
				break;
			case gameinfo_mapname:
				if(m_map.name().empty())
				{
					std::string mapname = parlist.front().get_string();
					m_map.set_name(mapname);
				}
				break;
			case gameinfo_mapsize:
			{
				int width, height;
				width = parlist.front().get_integer();
				parlist.pop_front();
				height = parlist.front().get_integer();
				m_map.set_size(width, height);
			}
				break;
			case gameinfo_playertype:
				if(player)
					player->set_type
						(static_cast<WLGGZPlayerType>(parlist.front().get_integer()));
				break;
			case gameinfo_version:
			{
				std::string version = parlist.front().get_string();
				parlist.pop_front();
				std::string build = parlist.front().get_string();
				if (client->number == 0)
				{
					host_version = version;
					parlist.pop_front();
					host_build = build;
				}
				if(m_players.find(client->name) != m_players.end())
					m_players[client->name]->set_version(version, build);
				wllog
					(DL_DUMP, "GAMEINFO: Player \"%s\": %s(%s)",
					 client->name.c_str(), version.c_str(), build.c_str());
				break;
			}
			case gameinfo_teamnumber:
				if(player)
					player->set_team(parlist.front().get_integer());
				break;
			default:
				wllog(DL_ERROR, "GAMEINFO: error unknown WLGGZGameInfo!");
		
		}
		ggz_read_int(fd, &gameinfo);
	}

	if (m_players.find(client->name) != m_players.end())
	{
		m_players[client->name]->set_ggz_player_number(client->number);
	}

	int num = playercount(Seat::player);
	wllog(DL_INFO, "GAMEINFO: number of players %i", num);

	for (int i=0; i<num; i++)
	{
		wllog(DL_INFO, "GAMEINFO: seat i: %i");
		if (seat(i))
		{
			if (seat(i)->client)
			{
				wllog
					(DL_INFO, ", s: %i, c: %i, Name: %s", seat(i)->number,
					 seat(i)->client->number, seat(i)->client->name.c_str());
				if(m_players.find(seat(i)->client->name) != m_players.end())
				{
					if(m_players[seat(i)->client->name]->ggz_player_number()==-1)
						m_players[seat(i)->client->name]->
							set_ggz_player_number(seat(i)->client->number);
				}
			} else {
				wllog(DL_INFO, "client* does not exist");
			}
		} else {
			wllog(DL_INFO, "does not exist ...");
		}
	}
}

void WidelandsServer::read_game_statistics(int fd)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;

	ggz_read_int(fd, &gameinfo);
	while(gameinfo)
	{
		std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(fd);
		
		switch(gameinfo)
		{
			case gamestat_playernumber:
			{
				std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();
				player = NULL;
				if ( not parlist.size() or not parlist.front().is_integer())
				{
					wllog
						(DL_ERROR, "GAMESTATISTICS: ERROR: "
						 "got playernumber but parameter error");
					break;
				}
				while(it != m_players.end())
				{
					if
						(it->second->wl_player_number() ==
						 parlist.front().get_integer())
						player = it->second;
					++it;
				}
				if(not player)
					wllog
						(DL_ERROR, "GAMESTATISTICS: ERROR: "
						 "got playernumber but could no find the player %i",
						parlist.front().get_integer());
				break;
				
			}
			case gamestat_result:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.result=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got result but catched a error");
				break;
			case gamestat_points:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.points=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got points but catched a error");
				break;
			case gamestat_land:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.land=parlist.front().get_integer();
				else
					wllog(DL_ERROR, "GAMESTATISTICS: got land but catched a error");
				break;
			case gamestat_buildings:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.buildings=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got buildings but catched a error");
				break;
			case gamestat_milbuildingslost:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.milbuildingslost=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got milbuildingslost but catched a error");
				break;
			case gamestat_civbuildingslost:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.civbuildingslost=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got civbuildingslost but catched a error");
				break;
			case gamestat_buildingsdefeat:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.buildingsdefeat=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got buildingsdefeat but catched a error");
				break;
			case gamestat_milbuildingsconq:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.milbuildingsconq=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got milbuildingsconq but catched a error");
				break;
			case gamestat_economystrength:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.economystrength=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got economystrength but catched a error");
				break;
			case gamestat_militarystrength:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.militarystrength=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: "
						 "got militarystrength but catched a error");
				break;
			case gamestat_workers:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.workers=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got workers but catched a error");
				break;
			case gamestat_wares:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.wares=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got wares but catched a error");
				break;
			case gamestat_productivity:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.productivity=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR,
						 "GAMESTATISTICS: got productivity but catched a error");
				break;
			case gamestat_casualties:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.casualties=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR,
						 "GAMESTATISTICS: got casulties but catched a error");
				break;
			case gamestat_kills:
				if(player and parlist.size() and parlist.front().is_integer())
					player->stats.kills=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR, "GAMESTATISTICS: got kills but catched a error");
				break;
			case gamestat_gametime:
				if (parlist.size() and parlist.front().is_integer())
					m_result_gametime=parlist.front().get_integer();
				else
					wllog
						(DL_ERROR,
						 "GAMESTATISTICS: got gametime but catched a error");
				break;
			default:
				wllog(DL_WARN,  "GAMESTATISTICS: Warning unknown WLGGZGameStats!");
		}
	ggz_read_int(fd, &gameinfo);
	}

	GGZGameResult results[players()];
	int score[players()], teams[players()];
	for (int p = 0; p < players(); p++) {
		teams[p] = 0;
		results[p] = GGZ_GAME_NONE;
		score[p] = 0;
	}

	if (m_reported)
		return;

	if (m_map.gametype() == gametype_defeatall)
	{
		wllog(DL_INFO, "gametype defeat all");

		std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();
		int number = 0;
		while(it != m_players.end())
		{
			wllog
				(DL_DUMP, "Player (ggz: %i, wl: %i, \"%s\", %s) ",
				 it->second->ggz_player_number(), it->second->wl_player_number(),
				 it->first.c_str(), it->second->tribe().c_str());

			if (it->second->ggz_player_number() >= 0)
			{
				number++;
				if (it->second->ggz_player_number() >= players())
					wllog(DL_ERROR, "ERROR: ggz_player_number() >= players()");
				else
				{
					if (it->second->stats.result == gamestatresult_winner) {
						results[it->second->ggz_player_number()] = GGZ_GAME_WIN;
						score[it->second->ggz_player_number()] = 7;
					} else if (it->second->stats.result == gamestatresult_looser) {
						results[it->second->ggz_player_number()] = GGZ_GAME_LOSS;
						score[it->second->ggz_player_number()] = -3;
					} else 
						results[it->second->ggz_player_number()] = GGZ_GAME_FORFEIT;
				}
			}
			it++;
		}
		if(number < 2)
			wllog(DL_INFO, "Less than two ggz players in game. Do not report");
		if(m_result_gametime < (30 * 60 * 1000))
			wllog(DL_INFO, "Game lasted less than 30 minutes. Do not report");
		// TODO feed teams to reportGame
		if(number > 1 and m_result_gametime > (30 * 60 * 1000))
			reportGame(NULL, results, score);
	} else if (m_map.gametype() == gametype_tribes_together) {
		wllog(DL_WARN, "gametype tribes together. not implemented. WARNING");
	} else if (m_map.gametype() == gametype_collectors) {
		wllog(DL_INFO, "gametype collectors");

		std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();
		int number = 0;
		while(it != m_players.end())
		{
			wllog(DL_DUMP, "Player (ggz: %i, wl: %i, \"%s\", %s) ",
					it->second->ggz_player_number(), it->second->wl_player_number(),
					it->first.c_str(), it->second->tribe().c_str());

			if (it->second->ggz_player_number() >= 0)
			{
				number++;
				if (it->second->ggz_player_number() >= players())
					wllog(DL_ERROR, "ERROR: ggz_player_number() >= players()");
				else
				{
					if (it->second->stats.result == gamestatresult_winner) {
						results[it->second->ggz_player_number()] = GGZ_GAME_WIN;
						score[it->second->ggz_player_number()] = 1;
					} else if (it->second->stats.result == gamestatresult_looser) {
						results[it->second->ggz_player_number()] = GGZ_GAME_LOSS;
						score[it->second->ggz_player_number()] = -1;
					} else 
						results[it->second->ggz_player_number()] = GGZ_GAME_FORFEIT;
				}
			}
			it++;
		}
		if(number < 2)
			wllog(DL_WARN, "Less than two player in game. Do not report");
		if(number > 1)
			reportGame(NULL, results, score);
	} else if (m_map.gametype() == gametype_endless)
	{
		wllog(DL_INFO, "gametype_endless");
	} else
		wllog(DL_ERROR, "Error: Unknow gametype! cannot report game");

	m_reported = true;
}



// Game data event
void WidelandsServer::dataEvent(Client * const client)
{
	int opcode;
	int ret;
	struct sockaddr* addr;
	socklen_t addrsize;
	WidelandsPlayer * player;
	try {
		player = m_players.at(client->name);
	} catch (std::out_of_range) {
		player = 0;
	}

	wllog(DL_DEBUG, "data event from %s", client->name.c_str());

	// Read data
	int const channel = fd(client->number);

	ggz_read_int(channel, &opcode);

	switch (opcode) {
	case op_reply_ip:
		char * ip;

		//  Do not use IP provided by client. Instead, determine peer IP address.
		ggz_read_string_alloc(channel, &ip);
		wllog(DL_DEBUG, "reply_ip: %s", ip);
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
			wllog(DL_ERROR, "GAME: unreachable -> done!");
			ggz_write_int(channel, op_unreachable);
			changeState(GGZGameServer::done);
			break;
		}

		m_wlserver_ip = ggz_strdup(ip);
		ggz_free(ip);
		{ 	// test for connectablity ???
			// This code only tests if the string m_wlserver_ip is usable.
			addrinfo * ai = 0;
			if (getaddrinfo(m_wlserver_ip, "7396", 0, &ai)) {
				wllog(DL_ERROR, "GAME: unreachable -> done!");
				ggz_write_int(channel, op_unreachable);
				changeState(GGZGameServer::done);
				break;
			}
			freeaddrinfo(ai);
		}
		wllog(DL_DUMP, "GAME: reachable -> waiting!");
		changeState(GGZGameServer::waiting);
		break;
	case op_state_playing:
		wllog(DL_DEBUG, "GAME: playing!");
		changeState(GGZGameServer::playing);
		break;
	case op_state_done:
		wllog(DL_DUMP, "GAME: done!");
		/* ToDo: Do not switch to state done directly. This exit the widelands
		 * server imediately. Switch to waiting first and wait for statistics.
		 * This should have a timeout
		 */
		changeState(GGZGameServer::done);
		break;
	case op_request_protocol_ext:
		if (not player)
			m_players[client->name] = player =
				new WidelandsPlayer(client->name, client->number);
		player->set_build16_proto(true);
		{
			WLGGZ_writer wr(client->fd);
			wr.type(op_reply_protocol_ext);
			wr << WIDELANDS_PROTOCOL_EXT_MAJOR << WIDELANDS_PROTOCOL_EXT_MINOR;
		}
		wllog(DL_DEBUG, "got ext proto request from %s", client->name.c_str());
		ggz_write_int(client->fd, op_request_protocol_ext);
		break;
	default:
		if (WLGGZ_OLD_OPCODE(opcode) or not SUPPORT_B16_PROTOCOL(player))
		{
			wllog
				(DL_ERROR,
				 "Data error. Unhandled opcode (%i)! not a handled old opcode",
				 opcode);
			return;
		}
	}
	if(WLGGZ_OLD_OPCODE(opcode))
		return;

	switch(opcode) {
	case op_game_statistics:
		wllog(DL_DEBUG, "GAME: read stats!");
		read_game_statistics(channel);
		break;
	case op_game_information:
		wllog(DL_DUMP, "GAME: read game info!");
		read_game_information(channel, client);
		break;
	case op_set_debug:
		wlggz_read_parameter_list(channel);
		if (client->number == 0)
		{
			wllog
				(DL_DEBUG, "debug request from host: "
				 "enable sending of debug messages");
			send_debug = client->fd;
			}
		break;
	default:
		//  Discard
		wllog(DL_ERROR, "Data error. Unhandled opcode (%i)!", opcode);
		wlggz_read_parameter_list(channel);
	break;
	}
}

// Error handling event
void WidelandsServer::errorEvent()
{
	wllog(DL_ERROR, "errorEvent");
}

