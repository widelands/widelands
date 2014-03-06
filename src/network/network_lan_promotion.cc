/*
 * Copyright (C) 2004-2010,2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "network/network_lan_promotion.h"

#include <cstdio>
#include <cstring>

#include "build_info.h"
#include "compile_diagnostics.h"
#include "constants.h"
#include "container_iterate.h"

/*** class LAN_Base ***/

LAN_Base::LAN_Base ()
{

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); //  open the socket

	int32_t opt = 1;
	//  the cast to char* is because microsoft wants it that way
	setsockopt
		(sock, SOL_SOCKET, SO_BROADCAST,
		 reinterpret_cast<char *>(&opt), sizeof(opt));

#ifndef _WIN32

	//  get a list of all local broadcast addresses
	struct if_nameindex * ifnames = if_nameindex();
	struct ifreq ifr;

	for (int32_t i = 0; ifnames[i].if_index; ++i) {
		strncpy (ifr.ifr_name, ifnames[i].if_name, IFNAMSIZ);

GCC_DIAG_OFF("-Wold-style-cast")
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
			continue;

		if (!(ifr.ifr_flags & IFF_BROADCAST))
			continue;

		if (ioctl(sock, SIOCGIFBRDADDR, &ifr) < 0)
			continue;
GCC_DIAG_ON("-Wold-style-cast")

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

GCC_DIAG_OFF("-Wold-style-cast")
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(port);
GCC_DIAG_ON("-Wold-style-cast")

	::bind (sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

bool LAN_Base::avail ()
{
	fd_set fds;
	timeval tv;

GCC_DIAG_OFF("-Wold-style-cast")
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
GCC_DIAG_ON("-Wold-style-cast")

	tv.tv_sec  = 0;
	tv.tv_usec = 0;

	return select(sock + 1, &fds, nullptr, nullptr, &tv) == 1;
}

ssize_t LAN_Base::recv
	(void * const buf, size_t const len, sockaddr_in * const addr)
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

void LAN_Base::send
	(void const * const buf, size_t const len, sockaddr_in const * const addr)
{
	sendto
		(sock,
		 static_cast<const DATATYPE *>(buf),
		 len,
		 0,
		 reinterpret_cast<const sockaddr *>(addr),
		 sizeof(sockaddr_in));
}

void LAN_Base::broadcast
	(void const * const buf, size_t const len, uint16_t const port)
{
	container_iterate_const(std::list<in_addr_t>, broadcast_addresses, i) {
		sockaddr_in addr;
		addr.sin_family      = AF_INET;
		addr.sin_addr.s_addr = *i.current;
GCC_DIAG_OFF("-Wold-style-cast")
		addr.sin_port        = htons(port);
GCC_DIAG_ON("-Wold-style-cast")

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

	strncpy
		(gameinfo.gameversion, build_id().c_str(), sizeof(gameinfo.gameversion));

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

		if
			(!strncmp(magic, "QUERY", 6)
			 &&
			 magic[6] == LAN_PROMOTION_PROTOCOL_VERSION)
			send (&gameinfo, sizeof(gameinfo), &addr);
	}
}

void LAN_Game_Promoter::set_map (char const * map)
{
	strncpy (gameinfo.map, map, sizeof(gameinfo.map));

	needupdate = true;
}

/*** class LAN_Game_Finder ***/

LAN_Game_Finder::LAN_Game_Finder () :
	callback(nullptr)
{
	bind (WIDELANDS_LAN_DISCOVERY_PORT);

	reset();
}

void LAN_Game_Finder::reset ()
{
	char magic[8];

	opengames.clear();

	strncpy (magic, "QUERY", 8);
	magic[6] = LAN_PROMOTION_PROTOCOL_VERSION;

	broadcast (magic, 8, WIDELANDS_LAN_PROMOTION_PORT);
}


void LAN_Game_Finder::run ()
{
	while (avail()) {
		Net_Game_Info info;
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

		//  if the game already is in the list, update the information
		//  otherwise just append it to the list
		for (wl_const_range<std::list<Net_Open_Game *> > i(opengames);; ++i)
			if (i.empty()) {
				opengames.push_back (new Net_Open_Game);
GCC_DIAG_OFF("-Wold-style-cast")
				opengames.back()->address = addr.sin_addr.s_addr;
				opengames.back()->port    = htons(WIDELANDS_PORT);
GCC_DIAG_ON("-Wold-style-cast")
				opengames.back()->info    = info;
				callback (GameOpened, opengames.back(), userdata);
				break;
			} else if (0 == strncmp(i.front()->info.hostname, info.hostname, 128))
			{
				i.front()->info = info;
				callback (GameUpdated, i.front(), userdata);
				break;
			}
	}
}

void LAN_Game_Finder::set_callback
	(void (* const cb)(int32_t, Net_Open_Game const *, void *), void * const ud)
{
	callback = cb;
	userdata = ud;
}
