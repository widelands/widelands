/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__GAME_SERVER_PROTO_PACKET_CONNECT_H
#define __S__GAME_SERVER_PROTO_PACKET_CONNECT_H

#include <string>
#include "game_server_proto_packet.h"

/*
 * Checks versioning with the game server
 */
class Game_Server_Protocol_Packet_Connect : public Game_Server_Protocol_Packet {
   public:
      Game_Server_Protocol_Packet_Connect(void);
      virtual ~Game_Server_Protocol_Packet_Connect();

      virtual ushort get_id(void);

	virtual void recv(Game_Server_Connection *, Network_Buffer *) {}
      virtual void send(Network_Buffer* buffer);
      virtual void write_reply(Network_Buffer*) { }
      virtual void handle_reply(Game_Server_Connection*, Network_Buffer*);
};




#endif // __S__GAME_SERVER_PROTO_PACKET_HELLO_H
