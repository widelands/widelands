/*
 * Copyright (C) 2004-2006 by the Widelands Development Team
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

#include "constants.h"
#include "md5.h"
#include "streamread.h"
#include "streamwrite.h"

#include "types.h"

#include <SDL_net.h>

#include <queue>
#include <string>
#include <vector>

class Game;
class PlayerCommand;
class PlayerDescriptionGroup;
class Serializer;
class Deserializer;

class Fullscreen_Menu_LaunchGame;

class LAN_Game_Promoter;

class NetStatusWindow;

struct NetGame {
	struct Chat_Message {
		uint plrnum;
		std::string msg;
	};

	NetGame ();
	virtual ~NetGame ();

	uint get_playernum () {return playernum;}

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
	virtual void begin_game ();

	virtual void handle_network ()=0;

	virtual void send_player_command (PlayerCommand*)=0;
	virtual void send_chat_message (Chat_Message)=0;

	bool have_chat_message();
	Chat_Message get_chat_message();

	virtual void syncreport (const md5_checksum&)=0;

protected:
	void disconnect_player (int);

	enum {
		PH_SETUP,
		PH_PREGAME,
		PH_INGAME
	}                            phase;

	Game                       * game;

	uint                          playernum;
	ulong                        net_game_time;

	uint                         common_rand_seed;

	bool                         players_changed;

	uchar                        player_enabled;
	uchar                        player_human;
	uchar                        player_ready;

	PlayerDescriptionGroup     * playerdescr[MAX_PLAYERS];
	Fullscreen_Menu_LaunchGame * launch_menu;

	NetStatusWindow            * statuswnd;

	std::queue<Chat_Message>     chat_msg_queue;
};

struct NetHost:public NetGame {
	NetHost ();
	virtual ~NetHost ();

	void update_map ();

	virtual bool is_host () {return true;}
	virtual void begin_game ();

	virtual void handle_network ();

	virtual void send_player_command (PlayerCommand*);
	virtual void send_chat_message (Chat_Message);

	virtual void syncreport (const md5_checksum&);

private:
	void send_game_message (const char*);
	void send_chat_message_int (const Chat_Message);
	void send_player_info ();
	void update_network_delay ();

	struct Client {
		TCPsocket        sock;
		Deserializer   * deserializer;
		int              playernum;
		std::queue<md5_checksum> syncreports;
		ulong            lag;
	};

	LAN_Game_Promoter         * promoter;

	TCPsocket                   svsock;
	SDLNet_SocketSet            sockset;

	std::queue<PlayerCommand *> cmds;
	std::vector<Client>         clients;

	Serializer                * serializer;

	std::queue<md5_checksum>    mysyncreports;

	ulong                       net_delay;
	ulong                       net_delay_history[8];

	ulong                       next_ping_due;
	ulong                       last_ping_sent;
	uint                        pongs_received;
};

struct NetClient:public NetGame {
	NetClient (IPaddress*);
	virtual ~NetClient ();

	virtual bool is_host () {return false;}
	virtual void begin_game ();

	virtual void handle_network ();

	virtual void send_player_command (PlayerCommand*);
	virtual void send_chat_message (Chat_Message);

	virtual void syncreport (const md5_checksum&);

private:
	void disconnect ();

	TCPsocket        sock;
	SDLNet_SocketSet sockset;

	Serializer     * serializer;
	Deserializer   * deserializer;
};

struct Serializer : public StreamWrite {
	Serializer ();
	~Serializer ();

	void begin_packet ();
	void end_packet ();

	void send (TCPsocket);

	void Data(const void * const data, const size_t size);

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

	void putstr (const char*);

private:
	std::vector<unsigned char> buffer;
};

struct Deserializer : public StreamRead {
	Deserializer ();
	~Deserializer ();

	int read_packet (TCPsocket);

	bool avail ()
	{return !queue.empty();}

	size_t Data(void* const data, const size_t bufsize);
	bool EndOfFile();

	char getchar ();
	short getshort ();
	long getlong ();

	void getstr (char*, int);

private:
	std::queue<unsigned char> queue;
};

#endif
