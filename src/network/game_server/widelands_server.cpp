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
#include "log.h"

// Widelands includes

#include "protocol_handler.h"
#include "statistics_handler.h"
#include "wlggz_exception.h"

// GGZ includes
#include <ggz.h>

// System includes
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

ProtocolHandler wlproto;
StatisticsHandler wlstat;

// Constructor: inherit from ggzgameserver
WidelandsServer::WidelandsServer():
	GGZGameServer(),
	m_wlserver_ip(NULL),
	m_reported(false)
{
	wllog(DL_INFO, "");
	wllog(DL_INFO, "launched!");
}

// Destructor
WidelandsServer::~WidelandsServer()
{
	wllog(DL_INFO, "closing!");
	wllog
		(DL_DUMP, "Map %s (%i, %i)", wlproto.m_map.name().c_str(),
		 wlproto.m_map.w(), wlproto.m_map.h());
	wllog(DL_DUMP, "GameTime: %i", wlproto.m_result_gametime);

	std::string wincond = "unknown (ERROR)";
	switch(wlproto.m_map.gametype())
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
			wincond = "tribes together (deprecated)";
			break;
		case gametype_territorial_lord:
			wincond = "territorial lord";
			break;
		case gametype_wood_gnome:
			wincond = "wood gnome";
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
			 it->second->last_stats.land, it->second->last_stats.buildings,
			 it->second->last_stats.economystrength);

		wllog
			(DL_DUMP, "            points: %i, milbuildingslost: %i, "
			 "civbuildingslost: %i",
			 it->second->last_stats.points, it->second->last_stats.milbuildingslost,
			 it->second->last_stats.civbuildingslost);
		it++;
	}

	wllog(DL_DUMP, "");

	if(m_wlserver_ip)
		ggz_free(m_wlserver_ip);
}


// State change hook. Just to monitor state changes
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
	if (not client) {
		wllog(DL_INFO, "joinEvent without client ?!?\n"); 
		return;
	}

	wllog(DL_INFO, "joinEvent: %s", client->name.c_str());
	wlproto.send_greeter(client);

	if (not client->spectator and client->number == 0)
	{
		// This is the host
		// Take peer ip as host ip
		char * ip;
		int ret;
		//  Do not use IP provided by client. Instead, determine peer IP address.
		struct sockaddr addr;
		socklen_t addrsize = static_cast<socklen_t>(sizeof(struct sockaddr));
		//addrsize = 256;
		//addr = static_cast<struct sockaddr *>(malloc(addrsize));
		ret = getpeername(client->fd, &addr, &addrsize);

		//  FIXME: IPv4 compatibility?
		if (addr.sa_family == AF_INET6) {
			ip = static_cast<char *>(ggz_malloc(INET6_ADDRSTRLEN));
			inet_ntop
				(AF_INET6,
				 static_cast<void *>
					(&(reinterpret_cast<struct sockaddr_in6 *>(&addr))->sin6_addr),
					 ip,
					 INET6_ADDRSTRLEN);
		} else if(addr.sa_family == AF_INET) {
			ip = static_cast<char *>(ggz_malloc(INET_ADDRSTRLEN));
			inet_ntop
				(AF_INET,
				 static_cast<void *>
					(&(reinterpret_cast<struct sockaddr_in *>(&addr))->sin_addr),
					 ip,
					 INET_ADDRSTRLEN);
		} else {
			ip = NULL;
			wllog(DL_ERROR, "GAME: unreachable -> done!");
			ggz_write_int(client->fd, op_unreachable);
			changeState(GGZGameServer::done);
		}

		m_wlserver_ip = ggz_strdup(ip);
		ggz_free(ip);

		{
			// test for connectablity ???
			// This code only tests if the string m_wlserver_ip is usable.
			addrinfo * ai = 0;
			if (getaddrinfo(m_wlserver_ip, "7396", 0, &ai)) {
				wllog(DL_ERROR, "GAME: unreachable -> done!");
				ggz_write_int(client->fd, op_unreachable);
				changeState(GGZGameServer::done);
			}
			freeaddrinfo(ai);
		}

		wllog(DL_INFO, "GAME: reachable -> waiting!");
		changeState(GGZGameServer::waiting);
	}
	wllog
		(DL_DUMP, "Player %i \"%s\" (%s)", client->number,
		 client->name.c_str(), (client->spectator?"spectator":"player"));
}

void WidelandsServer::dataEvent(Client* client)
{
	if (not client) {
		wllog(DL_INFO, "dataEvent without client ?!?\n");
		return;
	}
	wlproto.process_data(client);
}


// Player leave event
void WidelandsServer::leaveEvent(Client * client)
{
	if (not client) {
		wllog(DL_INFO, "leaveEvent without client ?!?\n");
		return;
	}
	wllog(DL_INFO, "leaveEvent \"%s\"", client->name.c_str());
}

// Spectator join event (ignored)
void WidelandsServer::spectatorJoinEvent(Client * client)
{
	if (not client) {
		wllog(DL_INFO, "spectatorJoinEvent without client ?!?\n");
		return;
	}
	wllog
		(DL_INFO, "spectatorJoinEvent \"%s\"", client->name.c_str());
}

// Spectator leave event (ignored)
void WidelandsServer::spectatorLeaveEvent(Client * client)
{
	if (not client) {
		wllog(DL_INFO, "spectatorLeaveEvent without client ?!?\n");
		return;
	}
	wllog(DL_INFO, "spectatorLeaveEvent \"%s\"", client->name.c_str());
}

// Spectator data event (ignored)
void WidelandsServer::spectatorDataEvent(Client * client)
{
	if (not client) {
		wllog(DL_INFO, "spectatorDataEvent without client ?!?\n");
		return;
	}
	wllog(DL_INFO, "spectatorDataEvent  \"%s\"", client->name.c_str());
}

// Error handling event
void WidelandsServer::errorEvent()
{
	wllog(DL_ERROR, "errorEvent");
}

void WidelandsServer::set_state_done()
{
	if (wlstat.have_stats())
		game_done();
	changeState(GGZGameServer::done); 
}

void WidelandsServer::game_done()
{
	GGZGameResult results[g_wls->players()];
	int score[players()], teams[players()];
	for (int p = 0; p < players(); p++) {
		teams[p] = 0;
		results[p] = GGZ_GAME_NONE;
		score[p] = 0;
	}

	if (m_reported)
		return;

	if (wlproto.m_map.gametype() == gametype_defeatall)
	{
		wllog(DL_INFO, "gametype defeat all");
		std::map<std::string, WidelandsPlayer*>::iterator it =
			g_wls->m_players.begin();
		int number = 0;
		while(it != g_wls->m_players.end())
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
				else {
					if (it->second->last_stats.result == gamestatresult_winner) {
						results[it->second->ggz_player_number()] = GGZ_GAME_WIN;
						score[it->second->ggz_player_number()] = 7;
					}
					else if
						(it->second->last_stats.result == gamestatresult_looser)
					{
						results[it->second->ggz_player_number()] = GGZ_GAME_LOSS;
						score[it->second->ggz_player_number()] = -3;
					} else
						results[it->second->ggz_player_number()] = GGZ_GAME_FORFEIT;
				}
			}
			it++;
		}
		if (number < 2)
			wllog(DL_INFO, "Less than two ggz players in game. Do not report");
		if (wlproto.m_result_gametime < (30 * 60 * 1000))
			wllog(DL_INFO, "Game lasted less than 30 minutes. Do not report");
		// TODO feed teams to reportGame
		if (number > 1 and wlproto.m_result_gametime > (30 * 60 * 1000))
			reportGame(NULL, results, score);
	}
	else if (wlproto.m_map.gametype() == gametype_tribes_together)
	{
		wllog(DL_WARN, "gametype tribes together. deprecated");
	}
	else if (wlproto.m_map.gametype() == gametype_collectors)
	{
		wllog(DL_INFO, "gametype collectors");

		std::map<std::string,WidelandsPlayer*>::iterator it =
			g_wls->m_players.begin();
		int number = 0;
		while(it != g_wls->m_players.end())
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
					if (it->second->last_stats.result == gamestatresult_winner) {
						results[it->second->ggz_player_number()] = GGZ_GAME_WIN;
						score[it->second->ggz_player_number()] = 1;
					}
					else if
						(it->second->last_stats.result == gamestatresult_looser)
					{
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
	}
	else if (wlproto.m_map.gametype() == gametype_endless)
		wllog(DL_INFO, "gametype_endless");
	else if (wlproto.m_map.gametype() == gametype_territorial_lord)
		wllog(DL_WARN, "gametype: territorial lord - no reporting for this one");
	else if (wlproto.m_map.gametype() == gametype_wood_gnome)
		wllog(DL_WARN, "gametype: wood gnome - no reporting for this one");
	else
		wllog(DL_ERROR, "Error: Unknow gametype! cannot report game");
	m_reported = true;
}

WidelandsPlayer* WidelandsServer::get_player_by_name
	(std::string name, bool create)
{
	WidelandsPlayer * p = m_players[name];
	if (not p and create)
	{
		wllog(DL_DEBUG, "create new player structure for %s", name.c_str());
		p = new WidelandsPlayer(name);
		m_players[name] = p;

		for(int i=0; i < players(); i++)
		{
			Seat * s;
			if ((s = seat(i)) and s->client and s->client->name == name)
			{
				p->set_ggz_player_number(s->client->number);
			}
		}
		if (p->ggz_player_number() < 0)
			wllog(DL_DEBUG, "%s still has no ggz player number", name.c_str());
	}
	return p;
}

WidelandsPlayer* WidelandsServer::get_player_by_wlid(int id)
{
	if (id < 0)
		return NULL;

	WidelandsPlayer * player = NULL;

	std::map<std::string,WidelandsPlayer*>::iterator it = m_players.begin();

	while(it != m_players.end())
	{
	if (it->second->wl_player_number() == id)
		player = it->second;
	++it;
	}

	return player;

}

WidelandsPlayer* WidelandsServer::get_player_by_ggzid(int id)
{
	Seat * s = seat(id);
	if (not s or not s->client)
		return 0;
	WidelandsPlayer * p = m_players[seat(id)->client->name];
	if (p and p->ggz_player_number() < 0)
		p->set_ggz_player_number(id);
	return p;
}


#include <cstdarg>
#include <cstdio>
#include <sstream>

/*
* class _wexception implementation
*/
#undef wexception
_wlggzexception::_wlggzexception
	(char const * const file, uint32_t const line, char const * const fmt, ...)
	throw ()
	{
		char buffer[512];
		{
			va_list va;
			va_start(va, fmt);
			vsnprintf(buffer, sizeof(buffer), fmt, va);
			va_end(va);
		}
			std::ostringstream ost;
			ost << '[' << file << ':' << line << "] " << buffer;
			m_what = ost.str();
	}
	
	_wlggzexception::~_wlggzexception() throw () {}
	
	char const * _wlggzexception::what() const throw ()
	{
		return m_what.c_str();
	}
	
