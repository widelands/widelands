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

#include "game.h"
#include "player.h"
#include "playercommand.h"
#include "playerdescrgroup.h"
#include "tribe.h"
#include "network.h"
#include "wexception.h"
#include "fullscreen_menu_launchgame.h"

enum {
	NETCMD_UNUSED=0,
	NETCMD_HELLO,
	NETCMD_SELECTMAP,
	NETCMD_PLAYERINFO,
	NETCMD_START,
	NETCMD_ADVANCETIME,
	NETCMD_PLAYERCOMMAND
};

/* A note on simulation timing:
In a network game, in addition to the regular game time the concept of network
time (NetGame::net_game_time) is introduced. Network time is always ahead of game
time and defines how far the game time may advance using regular simulation.
Whenever a player command is issued, it is scheduled at the current network time so
it is guaranteed that all players will handle it at the appropriate time because
they must not have advanced the game time past that point. When the host decides that
up to some point later than current network time it will not schedule any more player
commands, than the network time will be advanced. Note that only the host has the
authority to do this. */


/*** class NetGame ***/

NetGame::NetGame ()
{
	game=0;
	net_game_time=0;
	playernum=0;
	
	players_changed=false;
}

NetGame::~NetGame ()
{
}

void NetGame::run ()
{
	game=new Game();
	game->run_multi_player (this);
	delete game;

	game=0;
}

// Return the maximum amount of time the game logic is allowed to advance.
// After that, new player commands may be scheduled and must be taken
// into account.
int NetGame::get_max_frametime ()
{
	int game_time=game->get_gametime();
	
	assert (game_time<=net_game_time);

	return net_game_time - game_time;
}

/*** class NetHost ***/

NetHost::NetHost ()
{
	IPaddress myaddr;
	
	// create a listening socket
	SDLNet_ResolveHost (&myaddr, NULL, WIDELANDS_PORT);
	svsock=SDLNet_TCP_Open(&myaddr);
	
	sockset=SDLNet_AllocSocketSet(16);
	serializer=new Serializer();
	
	playernum=1;
}

NetHost::~NetHost ()
{
	SDLNet_FreeSocketSet (sockset);
	
	// close all open sockets
	if (svsock!=0)
		SDLNet_TCP_Close (svsock);
	
	for (unsigned int i=0;i<clients.size();i++)
		SDLNet_TCP_Close (clients[i].sock);
}

// Whenever at the host a new map is selected, NetHost::update_map will
// notify the other players so that they load the map as well
void NetHost::update_map ()
{
	Map* map=game->get_map();
	
	serializer->begin_packet ();
	serializer->putchar (NETCMD_SELECTMAP);
	serializer->putstr (map?map->get_filename():"");
	serializer->end_packet ();
	
	for (unsigned int i=0;i<clients.size();i++)
		serializer->send (clients[i].sock);
	
	playerdescr[0]->set_player_type (Player::playerLocal);
	
	send_player_info ();
}

void NetHost::send_player_info ()
{
	unsigned char playerenabled, playerhuman;
	Player* pl;
	int i;
	
	playerenabled=0;
	playerhuman=0;
	
	for (i=0;i<MAX_PLAYERS;i++)
		if ((pl=game->get_player(i+1))!=0) {
			playerenabled|=1<<i;
			
			if (pl->get_type()!=Player::playerAI)
				playerhuman|=1<<i;
		}
	
	serializer->begin_packet ();
	serializer->putchar (NETCMD_PLAYERINFO);
	serializer->putchar (playerenabled);
	serializer->putchar (playerhuman);
	serializer->end_packet ();
	
	for (unsigned int i=0;i<clients.size();i++)
		serializer->send (clients[i].sock);
}

// When the game starts, first shut down the server socket. No new connections are accepted then.
// After that, notify the other players that we are starting.
void NetHost::begin_game ()
{
	SDLNet_TCP_Close (svsock);
	svsock=0;
	
	serializer->begin_packet ();
	serializer->putchar (NETCMD_START);
	serializer->end_packet ();
	
	for (unsigned int i=0;i<clients.size();i++)
		serializer->send (clients[i].sock);
}

void NetHost::handle_network ()
{
	TCPsocket sock;
	unsigned int i;
	
	// if we are in the game initiation phase, check for new connections
	while (svsock!=0 && (sock=SDLNet_TCP_Accept(svsock))!=0) {
		Player* pl=0;
		
		for (i=1;i<=MAX_PLAYERS;i++)
			if ((pl=game->get_player(i))!=0 && pl->get_type()==Player::playerAI) break;
		
		if (pl==0) {
			// sorry, but there no room on this map for any more players
			SDLNet_TCP_Close (sock);
			continue;
		}
		
		SDLNet_TCP_AddSocket (sockset, sock);
		
		const char* tribe=pl->get_tribe()->get_name();
		
		game->remove_player (i);
		game->add_player (i, Player::playerRemote, tribe);
		
		Client peer;
		peer.sock=sock;
		peer.deserializer=new Deserializer();
		peer.playernum=i;
		clients.push_back (peer);
		
		players_changed=true;
		
		playerdescr[i-1]->set_player_type (Player::playerRemote);
		
		serializer->begin_packet ();
		serializer->putchar (NETCMD_HELLO);
		serializer->putchar (i);

		Map* map=game->get_map();
		serializer->putchar (NETCMD_SELECTMAP);
		serializer->putstr (map?map->get_filename():"");
		serializer->end_packet ();
		serializer->send (peer.sock);
		
		send_player_info ();
	}
	
	// check if we hear anything from our peers
	while (SDLNet_CheckSockets(sockset, 0) > 0)
		for (i=0;i<clients.size();i++)
			if (SDLNet_SocketReady(clients[i].sock))
				clients[i].deserializer->read_packet (clients[i].sock);

	// if so, deserialize player commands
	for (i=0;i<clients.size();i++)
		while (clients[i].deserializer->avail())
			cmds.push (PlayerCommand::deserialize(clients[i].deserializer));
	
	// Do not send out packets too often.
	// The length of the interval should be set
	// according to the network speed and lag.
	// The values used here should be alright for a
	// slow dialup connection.
	if (net_game_time - game->get_gametime() < 250) {
		serializer->begin_packet ();

		// send any outstanding player commands
		while (!cmds.empty()) {
			log ("%d player commands queued\n", cmds.size());

			PlayerCommand* cmd=cmds.front();
			cmds.pop ();
			
			log ("player command from player %d\n", cmd->get_sender());
	    
			serializer->putchar (NETCMD_PLAYERCOMMAND);
			cmd->serialize (serializer);
	    
			cmd->set_duetime (net_game_time);
			game->enqueue_command (cmd);
		}

		// update network time
		net_game_time=game->get_gametime()+500;
	
		serializer->putchar (NETCMD_ADVANCETIME);
		serializer->putlong (net_game_time);
	
		serializer->end_packet ();
	
		// send the packet to all peers
		for (i=0;i<clients.size();i++)
			serializer->send (clients[i].sock);
	}
}

void NetHost::send_player_command (PlayerCommand* cmd)
{
	cmds.push (cmd);
}

/*** class NetClient ***/

NetClient::NetClient (IPaddress* svaddr)
{
	sock=SDLNet_TCP_Open(svaddr);
	if (sock==0)
		throw wexception("SDLNet_TCP_Open failed: %s", SDLNet_GetError());
	
	sockset=SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (sockset, sock);
	
	serializer=new Serializer();
	deserializer=new Deserializer();
	
	deserializer->read_packet (sock);
	if (deserializer->getchar()!=NETCMD_HELLO)
		throw wexception("Invalid network data received");
	
	playernum=deserializer->getchar();
}

NetClient::~NetClient ()
{
	delete serializer;
	delete deserializer;
	
	SDLNet_FreeSocketSet (sockset);
	SDLNet_TCP_Close (sock);
}

void NetClient::begin_game ()
{
}

void NetClient::handle_network ()
{
	// check if data is available on the socket
	while (SDLNet_CheckSockets(sockset, 0) > 0)
		deserializer->read_packet (sock);
	
	while (deserializer->avail())
	        switch (deserializer->getchar()) {
		    case NETCMD_SELECTMAP:
			{
				char buffer[256];
				deserializer->getstr (buffer,sizeof(buffer));
				log ("Map '%s' selected\n", buffer);
				
				game->load_map (buffer);
				playerdescr[playernum-1]->set_player_type (Player::playerLocal);
				launch_menu->refresh ();
			}
			break;
		    case NETCMD_PLAYERINFO:
			{
				unsigned char enabled,human;
				int i;
				
				enabled=deserializer->getchar();
				human=deserializer->getchar();
				
				for (i=0;i<MAX_PLAYERS;i++)
					if (i!=playernum-1)
						if (enabled & (1<<i)) {
					    		playerdescr[i]->set_player_type ((human&(1<<i))?Player::playerRemote:Player::playerAI);
					    		playerdescr[i]->enable_player (true);
						}
						else
							playerdescr[i]->enable_player (false);
			}
			break;
		    case NETCMD_START:
			assert (launch_menu!=0);
			launch_menu->start_clicked();
			break;
		    case NETCMD_ADVANCETIME:
			net_game_time=deserializer->getlong();
			break;
		    case NETCMD_PLAYERCOMMAND:
			{
				PlayerCommand* cmd=PlayerCommand::deserialize(deserializer);
				cmd->set_duetime(net_game_time);
				game->enqueue_command (cmd);
			}
			break;
		    default:
			throw wexception("Invalid network data received");
		}
}

void NetClient::send_player_command (PlayerCommand* cmd)
{
	// send the packet to the server instead of queuing it locally
	// note that currently clients can only send player commands, this may change in the future
	serializer->begin_packet ();
	cmd->serialize (serializer);
	serializer->end_packet ();
	serializer->send (sock);
}

/*** class Serializer ***/

Serializer::Serializer ()
{
}

Serializer::~Serializer ()
{
}

void Serializer::begin_packet ()
{
	buffer.clear ();
	buffer.push_back (0);	// this will finally be the length of the packet
	buffer.push_back (0);
}

void Serializer::end_packet ()
{
	int length=buffer.size();
	
	assert (length<0x10000);
	
	// update packet length
	buffer[0]=length >> 8;
	buffer[1]=length & 0xFF;
}

void Serializer::putstr (const char* str)
{
	while (*str)
		buffer.push_back (*str++);
	
	buffer.push_back (0);
}

void Serializer::send (TCPsocket sock)
{
	SDLNet_TCP_Send (sock, &(buffer[0]), buffer.size());
}

/*** class Deserializer ***/

Deserializer::Deserializer ()
{
}

Deserializer::~Deserializer ()
{
}

void Deserializer::read_packet (TCPsocket sock)
{
	unsigned char buffer[256];
	int length,amount,i;
	
	// read packet length (including length field)
	if (SDLNet_TCP_Recv(sock, buffer, 2) < 2)
		throw wexception("Error reading from socket");

	length=(buffer[0]<<8) | buffer[1];
	length-=2;	// subtract 2 bytes for the length field

	assert (length>=0);
	
	// read packet data in chunks of 256 bytes
	while (length>0) {
		amount=length <? 256;
		
		SDLNet_TCP_Recv (sock, buffer, amount);
		
		for (i=0;i<amount;i++)
			queue.push (buffer[i]);
		
		length-=amount;
	}
}

void Deserializer::getstr (char* buffer, int maxlength)
{
	int i;
	
	for (i=0;(buffer[i]=getchar())!=0;i++)
		if (i==maxlength)
			throw wexception("Deserializer: string too long");
}


