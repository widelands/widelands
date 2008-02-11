/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef GAME_SERVER_PROTO_PACKET_H
#define GAME_SERVER_PROTO_PACKET_H

#include <stdint.h>

class Game_Server_Connection;
class Network_Buffer;

/*
 * This is the abstract base class
 * of a Game Server Protocoll packet
 */
struct Game_Server_Protocol_Packet {
	virtual ~Game_Server_Protocol_Packet() {}

	virtual uint16_t get_id() = 0;

	virtual void recv         (Game_Server_Connection*, Network_Buffer *) = 0;
	virtual void send                                  (Network_Buffer *) = 0;
	virtual void write_reply                           (Network_Buffer *) = 0;
	virtual void handle_reply(Game_Server_Connection *, Network_Buffer *) = 0;
};

#endif
