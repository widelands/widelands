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
#include "game_server_proto_packet_chatmessage.h"
#include "util.h"
#include "wexception.h"

/*
 * Constructor
 */
Game_Server_Protocol_Packet_ChatMessage::Game_Server_Protocol_Packet_ChatMessage( uchar flags, std::string<wchar_t> msg ) {
   m_flags = flags;
   m_msg = msg;
}

/*
 * Destructor
 */
Game_Server_Protocol_Packet_ChatMessage::~Game_Server_Protocol_Packet_ChatMessage( void ) {
}

/*
 * Get this packets id
 */
ushort Game_Server_Protocol_Packet_ChatMessage::get_id(void) {
   return GGSPP_CHATMESSAGE;
}

/*
 * Write To network
 */
void Game_Server_Protocol_Packet_ChatMessage::send(Network_Buffer* buffer) {
   buffer->put_8(m_flags);
   buffer->put_string(L"");  // Is ignored by server
   buffer->put_string(m_msg);
}

/*
 * Handle reply
 */
void Game_Server_Protocol_Packet_ChatMessage::handle_reply(Game_Server_Connection* gsc, Network_Buffer* buf) {
   uchar answer = buf->get_8();

   if( answer != CM_ACK ) {
      wchar_t buffer[1024];

      swprintf(buffer, 1024, L"Server replied illegally to ChatMessage package. Should have send %i, but send %i. Ignored\n", 
            CM_ACK, answer);
   
      gsc->server_message( buffer );
   }
}

/*
 * Read from network 
 */
void Game_Server_Protocol_Packet_ChatMessage::recv(Game_Server_Connection* gsc, Network_Buffer* buffer) {
   m_flags = buffer->get_8();
   std::string<wchar_t> user = buffer->get_string();
   std::string<wchar_t> msg = buffer->get_string();

   gsc->chat_message( user, msg, m_flags );
}

/*
 * Write reply
 */
void Game_Server_Protocol_Packet_ChatMessage::write_reply( Network_Buffer* buf ) {
   buf->put_8( CM_ACK );
}

