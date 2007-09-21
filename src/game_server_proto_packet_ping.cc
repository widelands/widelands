/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "game_server_proto_packet_ping.h"

#include "game_server_connection.h"
#include "game_server_proto.h"
#include "network_buffer.h"
#include "wexception.h"


Game_Server_Protocol_Packet_Ping::Game_Server_Protocol_Packet_Ping() {}


Game_Server_Protocol_Packet_Ping::~Game_Server_Protocol_Packet_Ping() {}

/*
 * Get this packets id
 */
uint16_t Game_Server_Protocol_Packet_Ping::get_id() {
   return GGSPP_PING;
}

/*
 * Recv
 */
void Game_Server_Protocol_Packet_Ping::recv(Game_Server_Connection*, Network_Buffer*) {
}

/*
 * Construct reply
 */
void Game_Server_Protocol_Packet_Ping::write_reply(Network_Buffer* buf) {
   buf->put_8(PING_ACK);
}
