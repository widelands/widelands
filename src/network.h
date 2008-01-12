/*
 * Copyright (C) 2004-2008 by the Widelands Development Team
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
#include "widelands_streamread.h"
#include "widelands_streamwrite.h"

#include <SDL_net.h>

#include <queue>
#include <string>
#include <vector>

namespace Widelands {
struct Game;
class PlayerCommand;
};
class PlayerDescriptionGroup;
class Serializer;
class Deserializer;

class Fullscreen_Menu_LaunchGame;

class LAN_Game_Promoter;

class NetStatusWindow;

struct NetGame {
	struct Chat_Message {
		uint32_t plrnum;
		std::string msg;
	};

	NetGame ();
	virtual ~NetGame ();

	uint32_t get_playernum () {return playernum;}

	bool get_players_changed ()
	{
		bool ch=players_changed;
		players_changed=false;
		return ch;
	}

	void set_player_description_group (int32_t plnum, PlayerDescriptionGroup* pdg)
	{
		playerdescr[plnum-1]=pdg;
	}

	void set_launch_menu (Fullscreen_Menu_LaunchGame* lgm)
	{
		launch_menu=lgm;
	}

	void run ();

	uint32_t get_max_frametime();

	virtual bool is_host ()=0;
	virtual void begin_game ();

	virtual void handle_network ()=0;

	virtual void send_player_command (Widelands::PlayerCommand *) = 0;
	virtual void send_chat_message (Chat_Message)=0;

	bool have_chat_message();
	Chat_Message get_chat_message();

	virtual void syncreport (const md5_checksum&)=0;

protected:
	void disconnect_player (int32_t);

	enum {
		PH_SETUP,
		PH_PREGAME,
		PH_INGAME
	}                            phase;

	Widelands::Game                       * game;

	uint32_t                          playernum;
	uint32_t                        net_game_time;

	uint32_t                         common_rand_seed;

	bool                         players_changed;

	uint8_t                        player_enabled;
	uint8_t                        player_human;
	uint8_t                        player_ready;

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

	virtual void send_player_command (Widelands::PlayerCommand *);
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
		int32_t              playernum;
		std::queue<md5_checksum> syncreports;
		uint32_t            lag;
	};

	LAN_Game_Promoter         * promoter;

	TCPsocket                   svsock;
	SDLNet_SocketSet            sockset;

	std::queue<Widelands::PlayerCommand *> cmds;
	std::vector<Client>         clients;

	Serializer                * serializer;

	std::queue<md5_checksum>    mysyncreports;

	uint32_t                       net_delay;
	uint32_t                       net_delay_history[8];

	uint32_t                       next_ping_due;
	uint32_t                       last_ping_sent;
	uint32_t                        pongs_received;
};

struct NetClient:public NetGame {
	NetClient (IPaddress*);
	virtual ~NetClient ();

	virtual bool is_host () {return false;}
	virtual void begin_game ();

	virtual void handle_network ();

	virtual void send_player_command (Widelands::PlayerCommand *);
	virtual void send_chat_message (Chat_Message);

	virtual void syncreport (const md5_checksum&);

private:
	void disconnect ();

	TCPsocket        sock;
	SDLNet_SocketSet sockset;

	Serializer     * serializer;
	Deserializer   * deserializer;
};

struct Serializer : public Widelands::StreamWrite {
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

	void putshort (int16_t v)
	{
		buffer.push_back (v >> 8);
		buffer.push_back (v & 0xFF);
	}

	void putlong (int32_t v)
	{
		buffer.push_back (v >> 24);
		buffer.push_back ((v>>16) & 0xFF);
		buffer.push_back ((v>>8) & 0xFF);
		buffer.push_back (v & 0xFF);
	}

	void putstr (const char*);

private:
	std::vector<uint8_t> buffer;
};

struct Deserializer : public Widelands::StreamRead {
	Deserializer ();
	~Deserializer ();

	int32_t read_packet (TCPsocket);

	bool avail () const throw () {return not queue.empty();}

	size_t Data(void* const data, const size_t bufsize);
	bool EndOfFile() const;

	char getchar ();
	int16_t getshort ();
	int32_t getlong ();

	void getstr (char*, int32_t);

private:
	std::queue<uint8_t> queue;
};

#endif
