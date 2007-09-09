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

#include "game_server_connection.h"

#include "error.h"
#include "game_server_proto.h"
#include "game_server_proto_packet.h"
#include "game_server_proto_packet_getroominfo.h"
#include "game_server_proto_packet_userentered.h"
#include "game_server_proto_packet_chatmessage.h"
#include "game_server_proto_packet_ping.h"
#include "network_buffer.h"
#include "wexception.h"


Game_Server_Connection::Game_Server_Connection(std::string host, uint port) {
   m_host = host;
   m_port = port;
   m_socket = 0;
   m_last_packet_index = FIRST_CLIENT_PACKET_INDEX;

   m_smh = 0;
   m_smhd = 0;
}


Game_Server_Connection::~Game_Server_Connection() {
   if (m_socket) {
      SDLNet_TCP_Close(m_socket);
      SDLNet_FreeSocketSet(m_socketset);
	}

   m_socket = 0;
}

/*
 * Connect to the server,
 * throws wexception on error
 */
void Game_Server_Connection::connect() {
   IPaddress myaddr;

   // Connect to the server
   SDLNet_ResolveHost (&myaddr, m_host.c_str(), m_port);

   m_socket = SDLNet_TCP_Open(&myaddr);
	if (m_socket==0)
		throw wexception("Game_Server_Connection::connect: SDLNet_TCP_Open failed: %s", SDLNet_GetError());

   // Create the socket set and add this socket
   m_socketset = SDLNet_AllocSocketSet(1);
   if (!m_socketset)
		throw wexception("Game_Server_Connection::connect: SDLNet_AllocSocketSet failed: %s", SDLNet_GetError());
   SDLNet_TCP_AddSocket(m_socketset, m_socket);

}

/*
 * Send this packet over the line
 */
void Game_Server_Connection::send(Game_Server_Protocol_Packet* packet) {
   ushort id = packet->get_id();
   uint   index = m_last_packet_index++;
   ushort flags = 0;

   if (m_last_packet_index > LAST_CLIENT_PACKET_INDEX) // Hopefully this wrap never occures
      m_last_packet_index = FIRST_CLIENT_PACKET_INDEX;

   // This packet is replied to
   m_pending_packets.insert(std::pair<uint, Game_Server_Protocol_Packet*>(index, packet));

   // Write this data onto the stream
   Network_Buffer buf;
   buf.put_16(id);
   buf.put_32(index);
   buf.put_16(flags);

   packet->send(&buf);
   buf.finish();

   SDLNet_TCP_Send(m_socket, buf.get_data(), buf.size());
   log("Game_Server_Connection: Send %i bytes over the line!\n", buf.size());
}

/*
 * Check if there is incomming data and if so, handle it
 * accordingly
 */
void Game_Server_Connection::handle_data() {
   // Check if data is available,
   // we only handle one packet per call
	if (SDLNet_CheckSockets(m_socketset, 0) > 0) {
      // There is data


      Network_Buffer buf;
      if (buf.fill(m_socket) == -1) {
         // Upsy, no data. But rather a disconnect
         (*m_dch)(m_dchd);
         return;
		}

      log("Read %i bytes from the net!\n", buf.size());
      // Get the header
      ushort id = buf.get_16();
      uint   index = buf.get_32();
      ushort flags = buf.get_16();

		if (IS_ANSWER(flags)) {
			if (not m_pending_packets.count(index)) {
            log("Game_Server_Connection: WARNING Unknown response packet with id %i, dropped\n", index);
            return;
			}

         Game_Server_Protocol_Packet* pp = m_pending_packets[index];

			if (pp->get_id() != id)  {
            log("Game_Server_Connection: WARNING Response packet with wrong id (has: %i, should: %i), dropped\n", pp->get_id(), id);
            return;
			}

         pp->handle_reply(this, &buf);

         delete pp;
         m_pending_packets.erase(m_pending_packets.find(index));
		} else {
         // server requests
         Game_Server_Protocol_Packet* pp = 0;
			switch (id) {
			case GGSPP_USERENTERED:
				pp = new Game_Server_Protocol_Packet_UserEntered();      break;
			case GGSPP_CHATMESSAGE:
				pp = new Game_Server_Protocol_Packet_ChatMessage(0, ""); break;
			case GGSPP_PING:
				pp = new Game_Server_Protocol_Packet_Ping();             break;
			default:
				log
					("Game_Server_Connection: WARNING unknown protocol packet id in "
					 "server request, dropped!\n");
			}
			if (pp) {
            pp->recv(this, &buf);

            log("Received a server packet: %i, %i\n", id, index);
            Network_Buffer reply;
            reply.put_16(id);
            reply.put_32(index);
            reply.put_16(GSP_ANSWER);
            pp->write_reply(&reply);
            reply.finish();

            SDLNet_TCP_Send(m_socket, reply.get_data(), reply.size());

			}
		}
	}
}

/*
 * Set Room
 *
 * The room has changed, request informations about the room members
 */
void Game_Server_Connection::set_room(const char* room) {
   m_room = room;
   Game_Server_Protocol_Packet_GetRoomInfo* gri = new Game_Server_Protocol_Packet_GetRoomInfo(room);
   send(gri);
}

/*
 * Callback stuff below
 */
void Game_Server_Connection::set_server_message_handler(ServerMessage_Handler func, void* data) {
   m_smh = func;
   m_smhd = data;
}
void Game_Server_Connection::server_message(std::string msg) {
   (*m_smh)(msg, m_smhd);
}

void Game_Server_Connection::set_user_entered_handler(UserEntered_Handler func, void* data) {
   m_ueh = func;
   m_uehd = data;
}
void Game_Server_Connection::user_entered(std::string name, std::string room, uchar b) {
   (*m_ueh)(name, room, b, m_uehd);
}

void Game_Server_Connection::set_critical_error_handler(CriticalError_Handler func, void* data) {
   m_ceh= func;
   m_cehd= data;
}
void Game_Server_Connection::critical_error(std::string msg) {
   (*m_ceh)(msg, m_cehd);
}

void Game_Server_Connection::set_get_room_info_handler(RoomInfo_Handler rih, void* data) {
   m_rih = rih;
   m_rihd = data;
}
void Game_Server_Connection::get_room_info(std::vector<std::string > users) {
   (*m_rih)(users, m_rihd);
}

void Game_Server_Connection::set_get_user_info_handler(UserInfo_Handler uih, void* data) {
   m_uih = uih;
   m_uihd = data;
}
void Game_Server_Connection::get_user_info(std::string username, std::string game, std::string room) {
   (*m_uih)(username, game, room, m_uihd);
}

void Game_Server_Connection::set_chat_message_handler(ChatMessage_Handler cmh, void* data) {
   m_cmh = cmh;
   m_cmhd = data;
}
void Game_Server_Connection::chat_message(std::string user, std::string msg, uchar flags) {
   (*m_cmh)(user, msg, flags, m_cmhd);
}

void Game_Server_Connection::set_disconnect_handler(Disconnet_Handler dch, void* data) {
   m_dch = dch;
   m_dchd = data;
}
