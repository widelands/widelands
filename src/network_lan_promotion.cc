/*
 * Copyright (C) 2004-2008 by the Widelands Development Team
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

#include "network_lan_promotion.h"

#include "build_id.h"
#include "constants.h"

#include <stdio.h>
#include <string.h>

/*** class LAN_Base ***/

LAN_Base::LAN_Base ()
{

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); //  open the socket

	int32_t opt = 1;
	//  the cast to char* is because microsoft wants it that way
	setsockopt
		(sock, SOL_SOCKET, SO_BROADCAST,
		 reinterpret_cast<char *>(&opt), sizeof(opt));

#ifndef WIN32

	//  get a list of all local broadcast addresses
	struct if_nameindex* ifnames=if_nameindex();
	struct ifreq ifr;

	for (int32_t i = 0; ifnames[i].if_index; ++i) {
		strncpy (ifr.ifr_name, ifnames[i].if_name, IFNAMSIZ);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
			continue;

		if (!(ifr.ifr_flags & IFF_BROADCAST))
			continue;

		if (ioctl(sock, SIOCGIFBRDADDR, &ifr) < 0)
			continue;

		broadcast_addresses.push_back
			(reinterpret_cast<sockaddr_in *>(&ifr.ifr_broadaddr)
			 ->sin_addr.s_addr);
	}

	if_freenameindex (ifnames);
#else
	//  As Microsoft does not seem to support if_nameindex, we just broadcast to
	//  INADDR_BROADCAST.
	broadcast_addresses.push_back (INADDR_BROADCAST);
#endif
}

LAN_Base::~LAN_Base ()
{
	closesocket (sock);
}

void LAN_Base::bind (uint16_t port)
{
	sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(port);

	::bind (sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

bool LAN_Base::avail ()
{
	fd_set fds;
	timeval tv;

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	tv.tv_sec  = 0;
	tv.tv_usec = 0;

	return select(sock + 1, &fds, 0, 0, &tv) == 1;
}

ssize_t LAN_Base::recv (void* buf, size_t len, sockaddr_in* addr)
{
	socklen_t addrlen = sizeof(sockaddr_in);
	return
		recvfrom
		(sock,
		 static_cast<DATATYPE *>(buf),
		 len,
		 0,
		 reinterpret_cast<sockaddr *>(addr),
		 &addrlen);
}

void LAN_Base::send (const void* buf, size_t len, const sockaddr_in* addr)
{
	sendto
		(sock,
		 static_cast<const DATATYPE *>(buf),
		 len,
		 0,
		 reinterpret_cast<const sockaddr *>(addr),
		 sizeof(sockaddr_in));
}

void LAN_Base::broadcast (const void* buf, size_t len, uint16_t port)
{
	for
		(std::list<in_addr_t>::iterator i = broadcast_addresses.begin();
		 i != broadcast_addresses.end();
		 ++i)
	{
		sockaddr_in addr;
		addr.sin_family      = AF_INET;
		addr.sin_addr.s_addr = *i;
		addr.sin_port        = htons(port);

		sendto
			(sock,
			 static_cast<const DATATYPE *>(buf),
			 len,
			 0,
			 reinterpret_cast<const sockaddr *>(&addr),
			 sizeof(addr));
	}
}

/*** class LAN_Game_Promoter ***/

LAN_Game_Promoter::LAN_Game_Promoter ()
{
	bind (WIDELANDS_LAN_PROMOTION_PORT);

	needupdate = true;

	memset (&gameinfo, 0, sizeof(gameinfo));
	strcpy (gameinfo.magic, "GAME");

	gameinfo.version = LAN_PROMOTION_PROTOCOL_VERSION;
	gameinfo.state = LAN_GAME_OPEN;

	strncpy (gameinfo.gameversion, BUILD_ID, sizeof(gameinfo.gameversion));

	gethostname (gameinfo.hostname, sizeof(gameinfo.hostname));
}

LAN_Game_Promoter::~LAN_Game_Promoter ()
{
	gameinfo.state = LAN_GAME_CLOSED;

	broadcast (&gameinfo, sizeof(gameinfo), WIDELANDS_LAN_DISCOVERY_PORT);
}

void LAN_Game_Promoter::run ()
{
	if (needupdate) {
		needupdate = false;

		broadcast (&gameinfo, sizeof(gameinfo), WIDELANDS_LAN_DISCOVERY_PORT);
	}

	while (avail()) {
		char magic[8];
		sockaddr_in addr;

		if (recv(magic, 8, &addr) < 8)
			continue;

		printf ("Received %s packet\n", magic);

		if (!strncmp(magic, "QUERY", 6) && magic[6]==LAN_PROMOTION_PROTOCOL_VERSION)
			send (&gameinfo, sizeof(gameinfo), &addr);
	}
}

void LAN_Game_Promoter::set_map (const char* map)
{
	strncpy (gameinfo.map, map, sizeof(gameinfo.map));

	needupdate = true;
}

/*** class LAN_Game_Finder ***/

LAN_Game_Finder::LAN_Game_Finder ()
{
	callback = 0;

	bind (WIDELANDS_LAN_DISCOVERY_PORT);

	reset();
}

void LAN_Game_Finder::reset ()
{
	char magic[8];

	opengames.clear();

	strncpy (magic, "QUERY", 8);
	magic[6]=LAN_PROMOTION_PROTOCOL_VERSION;

	broadcast (magic, 8, WIDELANDS_LAN_PROMOTION_PORT);
}

LAN_Game_Finder::~LAN_Game_Finder () {}

void LAN_Game_Finder::run ()
{
	while (avail()) {
		LAN_Game_Info info;
		sockaddr_in addr;

		if
			(recv
			 (&info, sizeof(info), &addr) < static_cast<int32_t>(sizeof(info)))
			continue;

		printf ("Received %s packet\n", info.magic);

		if (strncmp(info.magic, "GAME", 6))
			continue;

		if (info.version != LAN_PROMOTION_PROTOCOL_VERSION)
			continue;

		std::list<LAN_Open_Game *>::iterator i;
		// if the game already is in the list, update the information
		for (i = opengames.begin(); i != opengames.end(); ++i) {
			if ((*i)->address == addr.sin_addr.s_addr) {
				(*i)->info=info;

				callback (GameUpdated, *i, userdata);
				break;
			}
		}

		//  otherwise just append it to the list
		if (i==opengames.end()) {
			opengames.push_back (new LAN_Open_Game);

			opengames.back()->address = addr.sin_addr.s_addr;
			opengames.back()->port    = htons(WIDELANDS_PORT);
			opengames.back()->info    = info;

			callback (GameOpened, opengames.back(), userdata);
		}
	}
}

void LAN_Game_Finder::set_callback (void (*cb)(int32_t, const LAN_Open_Game*, void*), void* ud)
{
	callback = cb;
	userdata = ud;
}
