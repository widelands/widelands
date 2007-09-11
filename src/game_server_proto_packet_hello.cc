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

#include "game_server_proto_packet_hello.h"

#include "game_server_connection.h"
#include "game_server_proto.h"
#include "network_buffer.h"
#include "wexception.h"

#include <cassert>

Game_Server_Protocol_Packet_Hello::Game_Server_Protocol_Packet_Hello
(const std::string & name)
: m_name(name)
{}


Game_Server_Protocol_Packet_Hello::~Game_Server_Protocol_Packet_Hello() {}

/*
 * Get this packets id
 */
ushort Game_Server_Protocol_Packet_Hello::get_id() {
   return GGSPP_HELLO;
}

/*
 * Write To network
 */
void Game_Server_Protocol_Packet_Hello::send(Network_Buffer* buffer) {
   buffer->put_8(FEATURES_NONE);
   buffer->put_string(m_name);
}

/*
 * Handle reply
 */
void Game_Server_Protocol_Packet_Hello::handle_reply(Game_Server_Connection* gsc, Network_Buffer* buf) {
#ifdef DEBUG
   uchar retcode = buf->get_8();
#else
   buf->get_8();
#endif

   m_name = buf->get_string();
   std::string m_group = buf->get_string();
   std::string m_room= buf->get_string();
   std::string m_motd= buf->get_string();

   assert(retcode == USER_WELCOME); // no other code defined at the moment

   gsc->set_username(m_name.c_str());
   gsc->set_group(m_group.c_str());
   gsc->set_room(m_room.c_str());
   gsc->server_message(m_motd);
}
