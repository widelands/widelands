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

#ifndef __S__META_SERVER_CONNECTION_H
#define __S__META_SERVER_CONNECTION_H

class Game_Server_Protocol_Packet;

#include <map>
#include <string>
#include <vector>
#include <SDL_net.h>

#include "types.h"

/*
 * This class defines a connection to a widelands
 * game server. It is used to 
 *  - connect to a game server
 *  - chat there, list games, users and so on...
 *  - send commands to the game server
 */
typedef void (*ServerMessage_Handler)(std::wstring, void* data);
typedef void (*CriticalError_Handler)(std::wstring, void* data);
typedef void (*UserEntered_Handler)(std::wstring, std::wstring, bool, void*);
typedef void (*RoomInfo_Handler)(std::vector<std::wstring >, void*);
typedef void (*UserInfo_Handler)(std::wstring, std::wstring, std::wstring, void*);
typedef void (*ChatMessage_Handler)(std::wstring, std::wstring, uchar, void*);
typedef void (*Disconnet_Handler)( void* );

class Game_Server_Connection {
   public:
      Game_Server_Connection(std::string host, uint port);
      ~Game_Server_Connection(void);

      void connect(void); 

      void send(Game_Server_Protocol_Packet*);
      void handle_data(void);

      // Set callback functions (handlers)
      void set_server_message_handler(ServerMessage_Handler, void*);
      void set_user_entered_handler(UserEntered_Handler, void*);
      void set_get_room_info_handler(RoomInfo_Handler, void*);
      void set_get_user_info_handler(UserInfo_Handler, void*);
      void set_chat_message_handler(ChatMessage_Handler, void*);
      void set_critical_error_handler(CriticalError_Handler, void*);
      void set_disconnect_handler(Disconnet_Handler, void*);
      
      // Call callback functions
      void server_message(std::wstring str);
      void user_entered(std::wstring str, std::wstring, uchar);
      void get_room_info(std::vector<std::wstring >);
      void get_user_info( std::wstring, std::wstring, std::wstring);
      void chat_message( std::wstring, std::wstring, uchar );
      void critical_error(std::wstring str);
      
      // Set user data 
      void set_username(const wchar_t* name) { m_username = name; }
      const wchar_t* get_username(void) { return m_username.c_str(); }
      void set_group(const wchar_t* name) { m_group = name; }
      const wchar_t* get_group(void) { return m_group.c_str(); }
      void set_room(const wchar_t* name); 
      const wchar_t* get_room(void) { return m_room.c_str(); }

   private:
      // Connection data
      std::map<uint,Game_Server_Protocol_Packet*> m_pending_packets;
      TCPsocket m_socket;
      SDLNet_SocketSet m_socketset;
      std::string m_host;
      uint m_port;
      uint m_last_packet_index;

      // User data
      std::wstring m_username;
      std::wstring m_group;
      std::wstring m_room;

      // Callback functions
      ServerMessage_Handler m_smh;
      void* m_smhd; 
      UserEntered_Handler m_ueh;
      void* m_uehd;
      RoomInfo_Handler m_rih;
      void* m_rihd;
      UserInfo_Handler m_uih;
      void* m_uihd;
      ChatMessage_Handler m_cmh;
      void* m_cmhd;
      CriticalError_Handler m_ceh;
      void* m_cehd;
      Disconnet_Handler m_dch;
      void* m_dchd;
};

#endif // __S__META_SERVER_CONNECTION_H
