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
#include "playercommand.h"
#include "network.h"
#include "wexception.h"

enum {
	NETCMD_UNUSED=0,
	NETCMD_ADVANCETIME,
	NETCMD_PLAYERCOMMAND
};

/*** class NetGame ***/

NetGame::NetGame ()
{
	net_game_time=0;
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
	
	SDLNet_ResolveHost (&myaddr, NULL, WIDELANDS_PORT);
	svsock=SDLNet_TCP_Open(&myaddr);
	
	sockset=SDLNet_AllocSocketSet(16);
	serializer=new Serializer();
	
	playernum=1;
	
	// FIXME: temporarily wait for my buddy here
	Client peer;
	
	while ((peer.sock=SDLNet_TCP_Accept(svsock))==0);
	
	SDLNet_TCP_AddSocket (sockset, peer.sock);
	peer.deserializer=new Deserializer();
	clients.push_back (peer);
}

NetHost::~NetHost ()
{
}

// When the game starts, shut down the server socket.
// No new connections are accepted then.
void NetHost::begin_game ()
{
	SDLNet_TCP_Close (svsock);
	svsock=0;
}

void NetHost::handle_network ()
{
	unsigned int i;
	
	while (SDLNet_CheckSockets(sockset, 0) > 0)
		for (i=0;i<clients.size();i++)
			if (SDLNet_SocketReady(clients[i].sock))
				clients[i].deserializer->read_packet (clients[i].sock);

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

		while (!cmds.empty()) {
			log ("%d player commands queued\n", cmds.size());

			PlayerCommand* cmd=cmds.front();
			cmds.pop ();
	    
			serializer->putchar (NETCMD_PLAYERCOMMAND);
			cmd->serialize (serializer);
	    
			cmd->set_duetime (net_game_time);
			game->enqueue_command (cmd);
		}

		net_game_time=game->get_gametime()+500;
	
		serializer->putchar (NETCMD_ADVANCETIME);
		serializer->putlong (net_game_time);
	
		serializer->end_packet ();
	
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
	
	playernum=2;
	
	serializer=new Serializer();
	deserializer=new Deserializer();
}

NetClient::~NetClient ()
{
	delete serializer;
	delete deserializer;
}

void NetClient::begin_game ()
{
}

void NetClient::handle_network ()
{
	while (SDLNet_CheckSockets(sockset, 0) > 0)
		deserializer->read_packet (sock);
	
	while (deserializer->avail())
	        switch (deserializer->getchar()) {
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
	buffer.push_back (0);
	buffer.push_back (0);
}

void Serializer::end_packet ()
{
	int length=buffer.size();
	
	assert (length<0x10000);
	
	buffer[0]=length >> 8;
	buffer[1]=length & 0xFF;
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
	
	if (SDLNet_TCP_Recv(sock, buffer, 2) < 2)
		throw wexception("Error reading from socket");

	length=(buffer[0]<<8) | buffer[1];
	length-=2;

	assert (length>=0);
	
	while (length>0) {
		amount=length <? 256;
		
		SDLNet_TCP_Recv (sock, buffer, amount);
		
		for (i=0;i<amount;i++)
			queue.push (buffer[i]);
		
		length-=amount;
	}
}

