/*
 * Copyright (C) 2004 by the Widelands Development Team
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <vector>
#include <queue>
#include <list>
#include <SDL/SDL_net.h>
#include "wexception.h"
#include "types.h"

#ifdef USE_GGZ
#define HAVE_GGZ 1
#endif

#ifdef HAVE_GGZ
#include <ggzmod.h>
#include <ggzcore.h>
#endif

#define WIDELANDS_PORT		7396

class Game;
class PlayerCommand;
class PlayerDescriptionGroup;
class Serializer;
class Deserializer;

class Fullscreen_Menu_LaunchGame;

class LAN_Game_Promoter;

class NetGame {
    public:
	struct Chat_Message {
		uint plrnum;
		std::wstring msg;
	};
	
	NetGame ();
	virtual ~NetGame ();

	int get_playernum () { return playernum; }
	
	bool get_players_changed ()
	{
		bool ch=players_changed;
		players_changed=false;
		return ch;
	}
	
	void set_player_description_group (int plnum, PlayerDescriptionGroup* pdg)
	{
		playerdescr[plnum-1]=pdg;
	}
	
	void set_launch_menu (Fullscreen_Menu_LaunchGame* lgm)
	{
		launch_menu=lgm;
	}
	
	void run ();
	
	uint get_max_frametime();
	
	virtual bool is_host ()=0;
	virtual void begin_game ()=0;

	virtual void handle_network ()=0;

	virtual void send_player_command (PlayerCommand*)=0;
	virtual void send_chat_message (Chat_Message)=0;
	
	bool have_chat_message();
	Chat_Message get_chat_message();
	
	virtual void syncreport (uint)=0;

    protected:
	void disconnect_player (int);
	
	Game*		game;
	
	int		playernum;
	uint		net_game_time;
	
	uint		common_rand_seed;
	
	bool		players_changed;
	
	PlayerDescriptionGroup*	playerdescr[MAX_PLAYERS];
	Fullscreen_Menu_LaunchGame*	launch_menu;

	std::queue<Chat_Message>	chat_msg_queue;
};

class NetHost:public NetGame {
    public:
	NetHost ();
	virtual ~NetHost ();
	
	void update_map ();

	virtual bool is_host () { return true; }
	virtual void begin_game ();
	
	virtual void handle_network ();
	
	virtual void send_player_command (PlayerCommand*);
	virtual void send_chat_message (Chat_Message);
	
	virtual void syncreport (uint);

    private:
	void send_game_message (const wchar_t*);
	void send_chat_message_int ( const Chat_Message );
	void send_player_info ();
	void update_network_delay ();

	struct Client {
		TCPsocket		sock;
		Deserializer*		deserializer;
		int			playernum;
		std::queue<uint>	syncreports;
		ulong			lag;
	};
	
	LAN_Game_Promoter*		promoter;
	
	TCPsocket			svsock;
	SDLNet_SocketSet		sockset;

	std::queue<PlayerCommand*>	cmds;
	std::vector<Client>		clients;
	
	Serializer*			serializer;
	
	std::queue<uint>		mysyncreports;
	
	ulong				net_delay;
	ulong				net_delay_history[8];
	
	ulong				next_ping_due;
	ulong				last_ping_sent;
	uint				pongs_received;
};

class NetClient:public NetGame {
    public:
	NetClient (IPaddress*);
	virtual ~NetClient ();

	virtual bool is_host () { return false; }
	virtual void begin_game ();
	
	virtual void handle_network ();
	
	virtual void send_player_command (PlayerCommand*);
	virtual void send_chat_message (Chat_Message);
    
	virtual void syncreport (uint);

    private:
	void disconnect ();
	
	TCPsocket			sock;
	SDLNet_SocketSet		sockset;

	Serializer*			serializer;
	Deserializer*			deserializer;
};

class Serializer {
    public:
	Serializer ();
	~Serializer ();
	
	void begin_packet ();
	void end_packet ();
	
	void send (TCPsocket);
	
	void putchar (char v)
	{
		buffer.push_back (v);
	}
    
	void putshort (short v)
	{
		buffer.push_back (v >> 8);
		buffer.push_back (v & 0xFF);
	}
    
	void putlong (long v)
	{
		buffer.push_back (v >> 24);
		buffer.push_back ((v>>16) & 0xFF);
		buffer.push_back ((v>>8) & 0xFF);
		buffer.push_back (v & 0xFF);
	}
	
	void putwchar (wchar_t);
	
	void putstr (const char*);
	void putwstr (const wchar_t*);
    
    private:
	std::vector<unsigned char>	buffer;
};

class Deserializer {
    public:
	Deserializer ();
	~Deserializer ();
	
	int read_packet (TCPsocket);
	
	bool avail ()
	{ return !queue.empty(); }
	
	char getchar ()
	{
		char v=queue.front();
		queue.pop();
		return v;
	}
	
	short getshort ()
	{
		short v;
		
		v=queue.front() << 8;
		queue.pop();
		v|=queue.front();
		queue.pop();
		
		return v;
	}
    
	long getlong ()
	{
		long v;
		
		v=queue.front() << 24;
		queue.pop();
		v|=queue.front() << 16;
		queue.pop();
		v|=queue.front() << 8;
		queue.pop();
		v|=queue.front();
		queue.pop();
		
		return v;
	}
	
	wchar_t getwchar ();
	
	void getstr (char*, int);
	void getwstr (wchar_t*, int);
    
    private:
	std::queue<unsigned char>	queue;
};

class NetGGZ {
    public:
	NetGGZ();
	static NetGGZ* ref();

	void init();
	bool connect();

	bool used();
	bool host();
	void data();
	const char *ip();

	std::list<std::string> tables();

	enum Protocol
	{
		op_greeting = 1,
		op_request_ip = 2,
		op_reply_ip = 3,
		op_broadcast_ip = 4
	};

    private:
#ifdef HAVE_GGZ
	static void ggzmod_server(GGZMod *mod, GGZModEvent e, void *data);
	static GGZHookReturn callback_server(unsigned int id, void *data, void *user);
	static GGZHookReturn callback_room(unsigned int id, void *data, void *user);
#endif
	void initcore();
	void event_server(unsigned int id, void *data);
	void event_room(unsigned int id, void *data);

	bool use_ggz;
	int fd;
	char *ip_address;
	bool ggzcore_login;

	std::list<std::string> tablelist;
};

#endif

