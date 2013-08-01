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

#ifndef NETWORK_LAN_PROMOTION_H
#define NETWORK_LAN_PROMOTION_H

#include <list>

#ifndef _WIN32
#include <sys/socket.h>
#endif
#include <sys/types.h>

#include "network/network_system.h"

#define LAN_PROMOTION_PROTOCOL_VERSION 1

#define LAN_GAME_CLOSED                0
#define LAN_GAME_OPEN                  1

struct Net_Game_Info {
	char          magic       [6];
	uint8_t version;
	uint8_t state;

	char          gameversion[32];
	char          hostname   [128];
	char          map        [32];
};

struct Net_Open_Game {
	in_addr_t     address;
	in_port_t     port;
	Net_Game_Info info;
};

struct LAN_Base {
protected:
	LAN_Base ();
	~LAN_Base ();

	void bind (uint16_t);

	bool avail ();

	ssize_t recv (void *, size_t, sockaddr_in *);

	void send (void const *, size_t, sockaddr_in const *);
	void broadcast (void const *, size_t, uint16_t);

private:
	int32_t                  sock;

	std::list<in_addr_t> broadcast_addresses;
};

struct LAN_Game_Promoter : public LAN_Base {
	LAN_Game_Promoter ();
	~LAN_Game_Promoter ();

	void run ();

	void set_map (char const *);

private:
	Net_Game_Info gameinfo;
	bool          needupdate;
};

struct LAN_Game_Finder:LAN_Base {
	enum {
		GameOpened,
		GameClosed,
		GameUpdated
	};

	LAN_Game_Finder ();

	void reset ();
	void run ();

	void set_callback (void(*)(int32_t, Net_Open_Game const *, void *), void *);

private:
	std::list<Net_Open_Game *> opengames;

	void (*callback) (int32_t, Net_Open_Game const *, void *);
	void                     * userdata;
};

#endif
