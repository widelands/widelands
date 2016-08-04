/*
 * Copyright (C) 2004, 2007-2008 by the Widelands Development Team
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

#ifndef WL_NETWORK_NETWORK_LAN_PROMOTION_H
#define WL_NETWORK_NETWORK_LAN_PROMOTION_H

#include <list>

#ifndef _WIN32
#include <sys/socket.h>
#endif
#include <sys/types.h>

#include "network/network_system.h"

#define LAN_PROMOTION_PROTOCOL_VERSION 1

#define LAN_GAME_CLOSED 0
#define LAN_GAME_OPEN 1

struct NetGameInfo {
	char magic[6];
	uint8_t version;
	uint8_t state;

	char gameversion[32];
	char hostname[128];
	char map[32];
};

struct NetOpenGame {
	in_addr_t address;
	in_port_t port;
	NetGameInfo info;
};

struct LanBase {
protected:
	LanBase();
	~LanBase();

	void bind(uint16_t);

	bool avail();

	ssize_t receive(void*, size_t, sockaddr_in*);

	void send(void const*, size_t, sockaddr_in const*);
	void broadcast(void const*, size_t, uint16_t);

private:
	int32_t sock;

	std::list<in_addr_t> broadcast_addresses;
};

struct LanGamePromoter : public LanBase {
	LanGamePromoter();
	~LanGamePromoter();

	void run();

	void set_map(char const*);

private:
	NetGameInfo gameinfo;
	bool needupdate;
};

struct LanGameFinder : LanBase {
	enum { GameOpened, GameClosed, GameUpdated };

	LanGameFinder();

	void reset();
	void run();

	void set_callback(void (*)(int32_t, NetOpenGame const*, void*), void*);

private:
	std::list<NetOpenGame*> opengames;

	void (*callback)(int32_t, NetOpenGame const*, void*);
	void* userdata;
};

#endif  // end of include guard: WL_NETWORK_NETWORK_LAN_PROMOTION_H
