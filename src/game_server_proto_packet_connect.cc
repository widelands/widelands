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

#include "game_server_proto_packet_connect.h"

#include "game_server_connection.h"
#include "game_server_proto.h"
#include "i18n.h"
#include "network_buffer.h"
#include "wexception.h"


Game_Server_Protocol_Packet_Connect::Game_Server_Protocol_Packet_Connect() {}


Game_Server_Protocol_Packet_Connect::~Game_Server_Protocol_Packet_Connect() {}

/*
 * Get this packets id
 */
uint16_t Game_Server_Protocol_Packet_Connect::get_id() {
   return GGSPP_CONNECT;
}

/*
 * Write To network
 */
void Game_Server_Protocol_Packet_Connect::send(Network_Buffer* buffer) {
   uint16_t version = (GSP_MAJOR_VERSION << 8) | GSP_MINOR_VERSION;
   buffer->put_16(version);
   buffer->put_string("widelands");
}

/*
 * Handle reply
 */
void Game_Server_Protocol_Packet_Connect::handle_reply(Game_Server_Connection* gsc, Network_Buffer* buf) {
   uint8_t retcode = buf->get_8();
   uint16_t version = buf->get_16();

   char buffer[1024];

	switch (retcode) {
	case WELCOME: // Everything is ok
         break;

	case PROTOCOL_TO_OLD:
		snprintf
			(buffer, sizeof(buffer),
			 _("Server delivers a connection Error. Your Protocol (%i.%02i) is "
			   "too old, Server runs %i.%02i\n")
			 .c_str(),
			 GSP_MAJOR_VERSION, GSP_MINOR_VERSION,
			 version &0xff00, version &0x00ff);
         gsc->critical_error(buffer);
         break;

	case SERVER_FULL:
		snprintf(buffer, sizeof(buffer), _("Server is full!\n").c_str());
         gsc->critical_error(buffer);
         break;

	case GAME_NOT_SERVED:
		snprintf
			(buffer, sizeof(buffer),
			 _("This server doesn't serve widelands!\n").c_str());
         gsc->critical_error(buffer);
         break;

	}
}
