/*
 * Copyright (C) 2002-2004, 2008 by the Widelands Development Team
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

#ifndef GAME_SERVER_CONNECTION_H
#define GAME_SERVER_CONNECTION_H

#include <SDL_net.h>

#include <map>
#include <string>
#include <cstring>
#include <vector>

#include <stdint.h>

class Game_Server_Protocol_Packet;

/*
 * This class defines a connection to a widelands
 * game server. It is used to
 *  - connect to a game server
 *  - chat there, list games, users and so on...
 *  - send commands to the game server
 */
typedef void (*ServerMessage_Handler)(std::string, void* data);
typedef void (*CriticalError_Handler)(std::string, void* data);
typedef void (*UserEntered_Handler)(std::string, std::string, bool, void*);
typedef void (*RoomInfo_Handler)(std::vector<std::string >, void*);
typedef void (*UserInfo_Handler)(std::string, std::string, std::string, void*);
typedef void (*ChatMessage_Handler)(std::string, std::string, uint8_t, void*);
typedef void (*Disconnet_Handler)(void*);

struct Game_Server_Connection {
	Game_Server_Connection(std::string const & host, uint32_t port);
	~Game_Server_Connection();

	void connect();

	void send(Game_Server_Protocol_Packet *);
	void handle_data();

	//  set callback functions (handlers)
	void set_server_message_handler(ServerMessage_Handler, void *);
	void set_user_entered_handler  (UserEntered_Handler,   void *);
	void set_get_room_info_handler (RoomInfo_Handler,      void *);
	void set_get_user_info_handler (UserInfo_Handler,      void *);
	void set_chat_message_handler  (ChatMessage_Handler,   void *);
	void set_critical_error_handler(CriticalError_Handler, void *);
	void set_disconnect_handler    (Disconnet_Handler,     void *);

	//  call callback functions
	void server_message(std::string str);
	void user_entered(std::string str, std::string, uint8_t);
	void get_room_info(std::vector<std::string>);
	void get_user_info(std::string, std::string, std::string);
	void chat_message(std::string, std::string, uint8_t);
	void critical_error(std::string str);

	//  set user data
	void set_username(char const * const name) {m_username = name;}
	char const * get_username() {return m_username.c_str();}
	void set_group(char const * const name) {m_group = name;}
	char const * get_group() {return m_group.c_str();}
	void set_room(char const * name);
	char const * get_room() {return m_room.c_str();}

private:
	//  connection data
	std::map<uint32_t, Game_Server_Protocol_Packet*> m_pending_packets;
	TCPsocket m_socket;
	SDLNet_SocketSet m_socketset;
	std::string m_host;
	uint32_t m_port;
	uint32_t m_last_packet_index;

	//  user data
	std::string m_username;
	std::string m_group;
	std::string m_room;

	//  callback functions
	ServerMessage_Handler m_smh;
	void * m_smhd;
	UserEntered_Handler m_ueh;
	void * m_uehd;
	RoomInfo_Handler m_rih;
	void * m_rihd;
	UserInfo_Handler m_uih;
	void * m_uihd;
	ChatMessage_Handler m_cmh;
	void * m_cmhd;
	CriticalError_Handler m_ceh;
	void * m_cehd;
	Disconnet_Handler m_dch;
	void * m_dchd;
};

#endif
