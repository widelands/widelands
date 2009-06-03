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

// GGZ includes
#include <ggz.h>

// System includes
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Constructor: inherit from ggzgameserver
WidelandsServer::WidelandsServer()
: GGZGameServer()
{
	std::cout << "WidelandsServer: launched!" << std::endl;
	m_ip = NULL;
}

// Destructor
WidelandsServer::~WidelandsServer()
{
	if(m_ip)
		ggz_free(m_ip);
}

// State change hook
void WidelandsServer::stateEvent()
{
	std::cout << "WidelandsServer: stateEvent" << std::endl;
}

// Player join hook
void WidelandsServer::joinEvent(Client *client)
{
	std::cout << "WidelandsServer: joinEvent" << std::endl;

	// Send greeter
	int channel = fd(client->number);
	ggz_write_int(channel, op_greeting);
	ggz_write_string(channel, "widelands server");
	ggz_write_int(channel, WIDELANDS_PROTOCOL);

	if(client->number == 0)
		ggz_write_int(channel, op_request_ip);
	else
	{
		ggz_write_int(channel, op_broadcast_ip);
		ggz_write_string(channel, m_ip);
	}
}

// Player leave event
void WidelandsServer::leaveEvent(Client *client)
{
	std::cout << "WidelandsServer: leaveEvent" << std::endl;
}

// Spectator join event (ignored)
void WidelandsServer::spectatorJoinEvent(Client *client)
{
}

// Spectator leave event (ignored)
void WidelandsServer::spectatorLeaveEvent(Client *client)
{
}

// Spectator data event (ignored)
void WidelandsServer::spectatorDataEvent(Client *client)
{
}

// Game data event
void WidelandsServer::dataEvent(Client *client)
{
	int opcode;

	std::cout << "WidelandsServer: dataEvent" << std::endl;

	// Read data
	int channel = fd(client->number);

	ggz_read_int(channel, &opcode);
	switch(opcode)
	{
		case op_reply_ip:
			char *ip;

			struct sockaddr *addr;
			socklen_t addrsize;
			int ret;

			// Do not use IP provided by client
			// instead, determine peer IP address
			ggz_read_string_alloc(channel, &ip);
			std::cout << "IP: " << ip << std::endl;
			//m_ip = ggz_strdup(ip);
			ggz_free(ip);

			addrsize = 256;
			addr = (struct sockaddr*)malloc(addrsize);
			ret = getpeername(channel, addr, &addrsize);

			// FIXME: IPv4 compatibility?
			if(addr->sa_family == AF_INET6)
			{
				ip = (char*)ggz_malloc(INET6_ADDRSTRLEN);
				inet_ntop(AF_INET6, (void*)&(((struct sockaddr_in6*)addr)->sin6_addr),
					ip, INET6_ADDRSTRLEN);
			}
			else if(addr->sa_family == AF_INET)
			{
				ip = (char*)ggz_malloc(INET_ADDRSTRLEN);
				inet_ntop(AF_INET, (void*)&(((struct sockaddr_in*)addr)->sin_addr),
					ip, INET_ADDRSTRLEN);
			}
			else ip = NULL;

			std::cout << "broadcast IP: " << ip << std::endl;
			m_ip = ggz_strdup(ip);
			ggz_free(ip);
			break;
		case op_state_playing:
			std::cout << "GAME: playing!" << std::endl;
			changeState(GGZGameServer::playing);
			break;
		case op_state_done:
			std::cout << "GAME: done!" << std::endl;
			changeState(GGZGameServer::done);
			break;
		default:
			// Discard
			std::cerr << "ERROR!" << std::endl;
			break;
	}
}

// Error handling event
void WidelandsServer::errorEvent()
{
	std::cout << "WidelandsServer: errorEvent" << std::endl;
}

void WidelandsServer::game_start()
{
}

void WidelandsServer::game_stop()
{
}

void WidelandsServer::game_end()
{
}

