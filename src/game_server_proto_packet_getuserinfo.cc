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

#include "game_server_proto_packet_getuserinfo.h"

#include "game_server_connection.h"
#include "game_server_proto.h"
#include "i18n.h"
#include "network_buffer.h"
#include "wexception.h"

#include <cassert>

Game_Server_Protocol_Packet_GetUserInfo::Game_Server_Protocol_Packet_GetUserInfo
(const std::string & username)
: m_username(username)
{}


Game_Server_Protocol_Packet_GetUserInfo::
~Game_Server_Protocol_Packet_GetUserInfo
()
{}

/*
 * Get this packets id
 */
ushort Game_Server_Protocol_Packet_GetUserInfo::get_id() {
   return GGSPP_GETUSERINFO;
}

/*
 * Write To network
 */
void Game_Server_Protocol_Packet_GetUserInfo::send(Network_Buffer* buffer) {
   buffer->put_string(m_username);
}

/*
 * Handle reply
 */
void Game_Server_Protocol_Packet_GetUserInfo::handle_reply(Game_Server_Connection* gsc, Network_Buffer* buf) {
   uchar flags = buf->get_8();

	if (flags == UI_UNKNOWN) {
      char buffer[1024];

		snprintf
			(buffer, sizeof(buffer),
			 _("The User %s is currently not logged in or unknown to the "
			   "server.\n")
			 .c_str(),
			 m_username.c_str());

      gsc->server_message(buffer);
      return;
	}

   assert(flags == UI_ACK);

   std::string game = buf->get_string();
   std::string room = buf->get_string();
   gsc->get_user_info(m_username, game, room);
}
