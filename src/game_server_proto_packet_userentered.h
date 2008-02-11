/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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

#ifndef GAME_SERVER_PROTO_PACKET_USERENTERED_H
#define GAME_SERVER_PROTO_PACKET_USERENTERED_H

#include "game_server_proto_packet.h"

#include <string>

/*
 * A new user entered a certain room
 */
struct Game_Server_Protocol_Packet_UserEntered :
public Game_Server_Protocol_Packet
{
	Game_Server_Protocol_Packet_UserEntered();
	virtual ~Game_Server_Protocol_Packet_UserEntered();

	virtual uint16_t get_id();

	virtual void send                                  (Network_Buffer *) {}
	virtual void recv        (Game_Server_Connection *, Network_Buffer *);
	virtual void write_reply                           (Network_Buffer *);
	virtual void handle_reply(Game_Server_Connection *, Network_Buffer *) {};

private:
	std::string m_name;
};




#endif
