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

#include <string>
#include "error.h"
#include "game_server_connection.h"
#include "game_server_proto.h"
#include "game_server_proto_packet_getuserinfo.h"
#include "unicode.h"
#include "util.h"
#include "wexception.h"

/*
 * Constructor
 */
Game_Server_Protocol_Packet_GetUserInfo::Game_Server_Protocol_Packet_GetUserInfo( std::wstring user ) {
   m_username = user;
}

/*
 * Destructor
 */
Game_Server_Protocol_Packet_GetUserInfo::~Game_Server_Protocol_Packet_GetUserInfo( void ) {
}

/*
 * Get this packets id
 */
ushort Game_Server_Protocol_Packet_GetUserInfo::get_id(void) {
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

   if(flags == UI_UNKNOWN) {
      wchar_t buffer[1024];

      swprintf(buffer, 1024, L"The User %ls is currently not logged in or unknown to the server.\n", 
            m_username.c_str());

      gsc->server_message( buffer );
      return;
   }

   assert(flags == UI_ACK); 

   std::wstring game = buf->get_string();
   std::wstring room = buf->get_string();
   gsc->get_user_info(m_username, game, room);
}

