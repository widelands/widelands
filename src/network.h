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
#include <SDL/SDL_net.h>


#define WIDELANDS_PORT		7396


class Game;
class PlayerCommand;
class PlayerDescriptionGroup;
class Serializer;
class Deserializer;

class Fullscreen_Menu_LaunchGame;


class NetGame {
    public:
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
	
	int get_max_frametime();
	
	virtual bool is_host ()=0;
	virtual void begin_game ()=0;

	virtual void handle_network ()=0;

	virtual void send_player_command (PlayerCommand*)=0;

    protected:
	Game*		game;
	
	int		playernum;
	int		net_game_time;
	
	bool		players_changed;
	
	PlayerDescriptionGroup*	playerdescr[MAX_PLAYERS];
	Fullscreen_Menu_LaunchGame*	launch_menu;
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
	
    private:
	void send_player_info ();

	struct Client {
		TCPsocket		sock;
		Deserializer*		deserializer;
		int			playernum;
	};
	
	TCPsocket			svsock;
	SDLNet_SocketSet		sockset;

	std::queue<PlayerCommand*>	cmds;
	std::vector<Client>		clients;
	
	Serializer*			serializer;
};

class NetClient:public NetGame {
    public:
	NetClient (IPaddress*);
	virtual ~NetClient ();

	virtual bool is_host () { return false; }
	virtual void begin_game ();
	
	virtual void handle_network ();
	
	virtual void send_player_command (PlayerCommand*);
    
    private:
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
	
	void putstr (const char*);
    
    private:
	std::vector<unsigned char>	buffer;
};

class Deserializer {
    public:
	Deserializer ();
	~Deserializer ();
	
	void read_packet (TCPsocket);
	
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
	
	void getstr (char*, int);
    
    private:
	std::queue<unsigned char>	queue;
};

#endif

