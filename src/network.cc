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

#include <algorithm>
#include "network_system.h"
#include "game.h"
#include "player.h"
#include "playercommand.h"
#include "playerdescrgroup.h"
#include "tribe.h"
#include "network.h"
#include "network_lan_promotion.h"
#include "wexception.h"
#include "fullscreen_menu_launchgame.h"


#define CHECK_SYNC_INTERVAL	2000
#define	DELAY_PROBE_INTERVAL	10000
#define MINIMUM_NETWORK_DELAY	10	/* to avoid unneccessary network congestion */
#define INITIAL_NETWORK_DELAY	500


enum {
	NETCMD_UNUSED=0,
	NETCMD_HELLO,
	NETCMD_SELECTMAP,
	NETCMD_PLAYERINFO,
	NETCMD_START,
	NETCMD_PING,
	NETCMD_PONG,
	NETCMD_ADVANCETIME,
	NETCMD_PLAYERCOMMAND,
	NETCMD_SYNCREPORT,
	NETCMD_CHATMESSAGE
};


class Cmd_NetCheckSync:public BaseCommand {
    private:
	NetGame*	netgame;
	
    public:
	Cmd_NetCheckSync (int dt, NetGame* ng) : BaseCommand (dt) { netgame=ng; }
       
	virtual void execute (Game* g);
	
	// Write these commands to a file (for savegames)
	virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
	virtual void Read(FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);

	virtual int get_id(void) { return QUEUE_CMD_NETCHECKSYNC; }
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
	
	game->enqueue_command (
	    new Cmd_NetCheckSync(game->get_gametime()+CHECK_SYNC_INTERVAL, this));
	
	game->run_multi_player (this);
	delete game;

	game=0;
}

bool NetGame::have_chat_message ()
{
	return !chat_msg_queue.empty();
}

NetGame::Chat_Message NetGame::get_chat_message ()
{
	assert (!chat_msg_queue.empty());
	
	Chat_Message msg=chat_msg_queue.front();
	
	chat_msg_queue.pop ();
	
	return msg;
}

// Return the maximum amount of time the game logic is allowed to advance.
// After that, new player commands may be scheduled and must be taken
// into account.
uint NetGame::get_max_frametime ()
{
	uint game_time=game->get_gametime();
	
	assert (game_time<=net_game_time);

	return net_game_time - game_time;
}

/*** class NetHost ***/

NetHost::NetHost ()
{
	IPaddress myaddr;
	int i;
	
	// create a listening socket
	SDLNet_ResolveHost (&myaddr, NULL, WIDELANDS_PORT);
	svsock=SDLNet_TCP_Open(&myaddr);
	
	sockset=SDLNet_AllocSocketSet(16);
	serializer=new Serializer();
	
	playernum=1;
	
	net_delay=INITIAL_NETWORK_DELAY;
	next_ping_due=0;
	
	for (i=0;i<8;i++)
	    net_delay_history[i]=INITIAL_NETWORK_DELAY;

	promoter=new LAN_Game_Promoter();
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
	
	promoter->set_map (map?map->get_name():"none");
	
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
	delete promoter;
	promoter=0;
	
	SDLNet_TCP_Close (svsock);
	svsock=0;
	
	common_rand_seed=rand();
	game->logic_rand_seed (common_rand_seed);
	
	serializer->begin_packet ();
	serializer->putchar (NETCMD_START);
	serializer->putlong (common_rand_seed);
	serializer->end_packet ();
	
	for (unsigned int i=0;i<clients.size();i++)
		serializer->send (clients[i].sock);
}

void NetHost::handle_network ()
{
	TCPsocket sock;
	unsigned int i;
	
	if (promoter!=0)
	    promoter->run ();
	
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
		game->add_player (i, Player::playerRemote, tribe, "I have no name");
		
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
			switch (clients[i].deserializer->getchar()) {
			    case NETCMD_PONG:
				if (clients[i].lag>0) {
				    printf ("Duplicate pong received\n");
				    continue;
				}
				
				clients[i].lag=(SDL_GetTicks() - last_ping_sent) >? 1;
				pongs_received++;
				
				if (pongs_received==clients.size())
					update_network_delay ();
				
				break;
			    case NETCMD_PLAYERCOMMAND:
				cmds.push (PlayerCommand::deserialize(clients[i].deserializer));
				break;
			    case NETCMD_SYNCREPORT:
				clients[i].syncreports.push (
					clients[i].deserializer->getlong());
				break;
			    case NETCMD_CHATMESSAGE:
				{
					wchar_t buffer[256];
					uchar plrnum =  clients[i].deserializer->getchar();
               clients[i].deserializer->getwstr (buffer, 256);
               
               Chat_Message m;
               m.msg = buffer;
               m.plrnum = plrnum;
               
					send_chat_message_int (m);
				}
				break;
			    default:
				throw wexception("Invalid network data received");
			}
	
	// Do not send out packets too often.
	// The length of the interval should be set
	// according to the network speed and lag.
	// The values used here should be alright for a
	// slow dialup connection.
	if (net_game_time - game->get_gametime() < net_delay/2) {
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
		net_game_time=(game->get_gametime()+net_delay) >? net_game_time;
	
		serializer->putchar (NETCMD_ADVANCETIME);
		serializer->putlong (net_game_time);
	
		serializer->end_packet ();
	
		// send the packet to all peers
		for (i=0;i<clients.size();i++)
			serializer->send (clients[i].sock);
	}
    
	// see if it is time to check network lag again
	if (SDL_GetTicks()>=next_ping_due) {
		printf ("Ping!\n");
		
		last_ping_sent=SDL_GetTicks();
		next_ping_due=last_ping_sent + DELAY_PROBE_INTERVAL;
		pongs_received=0;
	    
		serializer->begin_packet ();
		serializer->putchar (NETCMD_PING);
		serializer->end_packet ();
	    
    		// send the packet to all peers
		for (i=0;i<clients.size();i++) {
			serializer->send (clients[i].sock);
			
			clients[i].lag=0;
		}
	}
}

void NetHost::send_chat_message_int (const Chat_Message msg)
{
	unsigned int i;
	
	serializer->begin_packet ();
	serializer->putchar (NETCMD_CHATMESSAGE);
	serializer->putchar(msg.plrnum);
	serializer->putwstr (msg.msg.c_str());
	serializer->end_packet ();
	
	for (i=0;i<clients.size();i++)
		serializer->send (clients[i].sock);

	chat_msg_queue.push (msg);
}

void NetHost::update_network_delay ()
{
	ulong tmp[8];
	unsigned int i;
	
	for (i=7;i>0;i--)
	    net_delay_history[i]=net_delay_history[i-1];
	
	net_delay_history[0]=MINIMUM_NETWORK_DELAY;
	
	for (i=0;i<clients.size();i++)
		if (clients[i].lag>net_delay_history[0])
		    net_delay_history[0]=clients[i].lag;
	
	// add a safety margin (25%)
	net_delay_history[0]+=net_delay_history[0]/4;
	
	for (i=0;i<8;i++)
	    tmp[i]=net_delay_history[i];
	
	std::sort (tmp, tmp+8);
	
	// forget the two slowest and the two fastest probes
	// average the remaining four
	
	net_delay=0;
	
	for (i=2;i<6;i++)
	    net_delay+=tmp[i];
	
	net_delay/=4;
	
	printf ("network delay is now %dms\n", (int) net_delay);
}

void NetHost::send_player_command (PlayerCommand* cmd)
{
	cmds.push (cmd);
}

void NetHost::send_chat_message (Chat_Message msg)
{
	send_chat_message_int (msg);
}

void NetHost::syncreport (uint sync)
{
    unsigned int i;
    
    mysyncreports.push (sync);
    
    // TODO: Check whether the list of pending syncreports is getting too
    // long. This might happen if a client is not sending syncreports.
    
    // Now look whether there is at least one syncreport from everyone.
    // If so, make sure they match.
    for (i=0;i<clients.size();i++)
	if (clients[i].syncreports.empty())
	    return;
    
    sync=mysyncreports.front();
    mysyncreports.pop();
    
    for (i=0;i<clients.size();i++) {
	if (clients[i].syncreports.front()!=sync)
	    throw wexception("Synchronization lost");
	    // TODO: handle this more gracefully
	
	clients[i].syncreports.pop();
    }
    
    printf ("synchronization is good so far\n");
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
	NetGGZ::ref()->data();

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
			common_rand_seed=deserializer->getlong();
			game->logic_rand_seed (common_rand_seed);
			
			assert (launch_menu!=0);
			launch_menu->start_clicked();
			break;
		    case NETCMD_PING:	// got a ping, reply with a pong
			serializer->begin_packet ();
			serializer->putchar (NETCMD_PONG);
			serializer->end_packet ();
			serializer->send (sock);
			printf ("Pong!\n");
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
		    case NETCMD_CHATMESSAGE:
			{
				wchar_t buffer[256];
			   char player = deserializer->getchar();
            
				deserializer->getwstr (buffer, 256);
				Chat_Message t;
            t.plrnum = player;
            t.msg = buffer;
            chat_msg_queue.push (t);
			}
			break;
		    default:
			throw wexception("Invalid network data received");
		}
}

void NetClient::send_player_command (PlayerCommand* cmd)
{
	// send the packet to the server instead of queuing it locally
	serializer->begin_packet ();
	serializer->putchar (NETCMD_PLAYERCOMMAND);
	cmd->serialize (serializer);
	serializer->end_packet ();
	serializer->send (sock);
}

void NetClient::send_chat_message (Chat_Message msg)
{
	serializer->begin_packet ();
	serializer->putchar (NETCMD_CHATMESSAGE);
	serializer->putchar(msg.plrnum);
	serializer->putwstr (msg.msg.c_str());
	serializer->end_packet ();
	serializer->send (sock);
}

void NetClient::syncreport (uint sync)
{
	serializer->begin_packet ();
	serializer->putchar (NETCMD_SYNCREPORT);
	serializer->putlong (sync);
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

void Serializer::putwchar (wchar_t wc)
{
	// use UTF-8 encoding so preserve space in the packet
	
	if (wc<0x80)
		buffer.push_back (wc);
	else if (wc<0x800) {
		buffer.push_back (0xC0 | (wc>>6));
		buffer.push_back (0x80 | (wc&0x3F));
	}
	else if (wc<0x10000) {
		buffer.push_back (0xE0 | (wc>>12));
		buffer.push_back (0x80 | ((wc>>6)&0x3F));
		buffer.push_back (0x80 | (wc&0x3F));
	}
	else if (wc<0x200000) {
		buffer.push_back (0xF0 | (wc>>18));
		buffer.push_back (0x80 | ((wc>>12)&0x3F));
		buffer.push_back (0x80 | ((wc>>6)&0x3F));
		buffer.push_back (0x80 | (wc&0x3F));
	}
	else if (wc<0x4000000) {
		buffer.push_back (0xF8 | (wc>>24));
		buffer.push_back (0x80 | ((wc>>18)&0x3F));
		buffer.push_back (0x80 | ((wc>>12)&0x3F));
		buffer.push_back (0x80 | ((wc>>6)&0x3F));
		buffer.push_back (0x80 | (wc&0x3F));
	}
	else {
		buffer.push_back (0xFC | (wc>>30));
		buffer.push_back (0x80 | ((wc>>24)&0x3F));
		buffer.push_back (0x80 | ((wc>>18)&0x3F));
		buffer.push_back (0x80 | ((wc>>12)&0x3F));
		buffer.push_back (0x80 | ((wc>>6)&0x3F));
		buffer.push_back (0x80 | (wc&0x3F));
	}
}

void Serializer::putstr (const char* str)
{
	while (*str)
		putchar (*str++);
	
	putchar (0);
}

void Serializer::putwstr (const wchar_t* str)
{
	while (*str)
		putwchar (*str++);
	
	putwchar (0);
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

wchar_t Deserializer::getwchar ()
{
	wchar_t chr,tmp;
	int n;

	chr=queue.front();
	queue.pop ();
	
	if (chr<128)
	    return chr;
	
	if (chr<0xC0 || chr>=0xFE)
	    throw wexception("Illegal UTF-8 character encountered");
	
	for (n=1;chr&(0x40>>n);n++);
	
	chr&=0x3F >> n;
	chr<<=n*6;
	
	while (n-->0) {
		tmp=queue.front();
		queue.pop ();
		
		if ((tmp&0xC0)!=0x80)
			throw wexception("Illegal UTF-8 character encountered");
		
		chr|=tmp << (n*6);
	}
	
	return chr;
}

void Deserializer::getstr (char* buffer, int maxlength)
{
	int i;
	
	for (i=0;(buffer[i]=getchar())!=0;i++)
		if (i==maxlength)
			throw wexception("Deserializer: string too long");
}

void Deserializer::getwstr (wchar_t* buffer, int maxlength)
{
	int i;
	
	for (i=0;(buffer[i]=getwchar())!=0;i++)
		if (i==maxlength)
			throw wexception("Deserializer: string too long");
}


/*** class Cmd_NetCheckSync ***/

void Cmd_NetCheckSync::execute (Game* g)
{
	// because the random number generator is made dependant
	// on the command queue, it is a good indicator for synchronization
	
	netgame->syncreport (g->logic_rand());
	
	g->enqueue_command (new Cmd_NetCheckSync(get_duetime()+CHECK_SYNC_INTERVAL, netgame));
}

void Cmd_NetCheckSync::Write(FileWrite* file, Editor_Game_Base* egb, Widelands_Map_Map_Object_Saver* sv)
{
	// this command should not be written to a file
	throw wexception("Cdm_NetCheckSync is not supposed to be written to a file");
}

void Cmd_NetCheckSync::Read(FileRead* file, Editor_Game_Base* egb, Widelands_Map_Map_Object_Loader* ld)
{
}

static NetGGZ *ggzobj = 0;
#ifdef HAVE_GGZ
static GGZMod *mod;
static GGZServer *ggzserver;
#endif

NetGGZ::NetGGZ()
{
	use_ggz = 0;
	fd = -1;
	ip_address = NULL;
}

NetGGZ* NetGGZ::ref()
{
	if(!ggzobj) ggzobj = new NetGGZ();
	return ggzobj;
}

void NetGGZ::init()
{
	use_ggz = true;
	printf(">> GGZ: initialized\n");

	//initcore(); /* FIXME: embedded ggzcore */
}

bool NetGGZ::used()
{
	return use_ggz;
	printf(">> GGZ: is used\n");
}

bool NetGGZ::connect()
{
#ifdef HAVE_GGZ
	int ret;

	if(!used()) return false;

	printf("GGZ ## connect\n");
	mod = ggzmod_new(GGZMOD_GAME);
	ggzmod_set_handler(mod, GGZMOD_EVENT_SERVER, &NetGGZ::ggzmod_server);
	ret = ggzmod_connect(mod);
	if(ret)
	{
		printf("GGZ ## connection failed\n");
		return false;
	}

	int fd = ggzmod_get_fd(mod);
	printf("GGZ ## connection fd %i\n", fd);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 999 * 1000;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	while(ggzmod_get_state(mod) != GGZMOD_STATE_PLAYING)
	{
		select(fd + 1, &fdset, NULL, NULL, &timeout);
		ggzmod_dispatch(mod);
		//printf("GGZ ## timeout!\n");
	}

	return true;
#else
	return false;
#endif
}

#ifdef HAVE_GGZ
void NetGGZ::ggzmod_server(GGZMod *mod, GGZModEvent e, void *data)
{
	printf("GGZ ## ggzmod_server\n");
	int fd = *(int*)data;
	ggzobj->fd = fd;
	printf("GGZ ## got fd: %i\n", fd);
	ggzmod_set_state(mod, GGZMOD_STATE_PLAYING);
}
#endif

void NetGGZ::data()
{
#ifdef HAVE_GGZ
	int op;
	char *ip;
	char *greeter;
	int greeterversion;
	char ipaddress[17];

	if(!used()) return;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	int ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
	if(ret <= 0) return;
	printf("GGZ ## select() returns: %i for fd %i\n", ret, fd);

	ggz_read_int(fd, &op);
	printf("GGZ ## received opcode: %i\n", op);

	switch(op)
	{
		case op_greeting:
			ggz_read_string_alloc(fd, &greeter);
			ggz_read_int(fd, &greeterversion);
			printf("GGZ ## server is: '%s' '%i'\n", greeter, greeterversion);
			ggz_free(greeter);
			break;
		case op_request_ip:
			printf("GGZ ## ip request!\n");
			snprintf(ipaddress, sizeof(ipaddress), "%i.%i.%i.%i", 127, 0, 0, 1);
			ggz_write_int(fd, op_reply_ip);
			ggz_write_string(fd, ipaddress);
			break;
		case op_broadcast_ip:
			ggz_read_string_alloc(fd, &ip);
			printf("GGZ ## ip broadcast: '%s'\n", ip);
			ip_address = ggz_strdup(ip);
			ggz_free(ip);
			break;
		default:
			printf("GGZ ## opcode unknown!\n");
	}
#endif
}

bool NetGGZ::host()
{
#ifdef HAVE_GGZ
	int spectator, seat;

	if(!used()) return false;

	do
	{
		ggzmod_dispatch(mod);
		ggzmod_get_player(mod, &spectator, &seat);
	}
	while(seat == -1);

	printf("GGZ ## host? seat=%i\n", seat);
	if(!seat) return true;
	return false;
#else
	return false;
#endif
}

const char *NetGGZ::ip()
{
	return ip_address;
}

void NetGGZ::initcore()
{
#ifdef HAVE_GGZ
	GGZOptions opt;
	int ret;

	printf("GGZCORE ## initialization\n");
	ggzcore_login = true;
	use_ggz = false; /* FIXME: use_ggz != use_ggzcore */

	opt.flags = (GGZOptionFlags)(GGZ_OPT_PARSER | GGZ_OPT_MODULES);
	ret = ggzcore_init(opt);

	ggzserver = ggzcore_server_new();
	ggzcore_server_add_event_hook(ggzserver, GGZ_CONNECTED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_NEGOTIATED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_LOGGED_IN, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_ENTERED, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_CONNECT_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_NEGOTIATE_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_LOGIN_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_ENTER_FAIL, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_ROOM_LIST, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_TYPE_LIST, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_NET_ERROR, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_PROTOCOL_ERROR, &NetGGZ::callback_server);

	ggzcore_server_set_hostinfo(ggzserver, "live.ggzgamingzone.org", 5688, 0);
	ggzcore_server_connect(ggzserver);

	printf("GGZCORE ## start loop\n");
	while(ggzcore_login)
	{
		if(ggzcore_server_data_is_pending(ggzserver))
			ggzcore_server_read_data(ggzserver, ggzcore_server_get_fd(ggzserver));
	}
#endif
}

#ifdef HAVE_GGZ
GGZHookReturn NetGGZ::callback_server(unsigned id, void *data, void *user)
{
	printf("GGZCORE ## callback: %i\n", id);
	ggzobj->event_server(id, data);

	return GGZ_HOOK_OK;
}
#endif

#ifdef HAVE_GGZ
GGZHookReturn NetGGZ::callback_room(unsigned id, void *data, void *user)
{
	printf("GGZCORE/room ## callback: %i\n", id);
	ggzobj->event_room(id, data);

	return GGZ_HOOK_OK;
}
#endif

void NetGGZ::event_server(unsigned int id, void *data)
{
#ifdef HAVE_GGZ
	GGZRoom *room;
	int num, i;
	int joined;

	switch(id)
	{
		case GGZ_CONNECTED:
			printf("GGZCORE ## -- connected\n");
			break;
		case GGZ_NEGOTIATED:
			printf("GGZCORE ## -- negotiated\n");
			ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN_GUEST, "widelands#ggz", "");
			ggzcore_server_login(ggzserver);
			break;
		case GGZ_LOGGED_IN:
			printf("GGZCORE ## -- logged in\n");
			ggzcore_server_list_rooms(ggzserver, -1, 1);
			break;
		case GGZ_ENTERED:
			printf("GGZCORE ## -- entered\n");
			room = ggzcore_server_get_cur_room(ggzserver);
			ggzcore_room_add_event_hook(room, GGZ_TABLE_LIST, &NetGGZ::callback_room);
			ggzcore_room_list_tables(room, -1, 0);
			break;
		case GGZ_ROOM_LIST:
			printf("GGZCORE ## -- (room list)\n");
			num = ggzcore_server_get_num_rooms(ggzserver);
			joined = 0;
			for(i = 0; i < num; i++)
			{
				room = ggzcore_server_get_nth_room(ggzserver, i);
				if(!strcmp(ggzcore_room_get_name(room), "TicTacToe"))
				{
					ggzcore_server_join_room(ggzserver, i);
					joined = 1;
					break;
				}
			}
			if(!joined)
			{
				printf("GGZCORE ## couldn't find room! :(\n");
			}
			break;
		case GGZ_TYPE_LIST:
			printf("GGZCORE ## -- (type list)\n");
			break;
		case GGZ_CONNECT_FAIL:
		case GGZ_NEGOTIATE_FAIL:
		case GGZ_LOGIN_FAIL:
		case GGZ_ENTER_FAIL:
		case GGZ_NET_ERROR:
		case GGZ_PROTOCOL_ERROR:
			printf("GGZCORE ## -- error! :(\n");
			break;
	}
#endif
}

void NetGGZ::event_room(unsigned int id, void *data)
{
#ifdef HAVE_GGZ
	GGZRoom *room;
	int i, num;
	GGZTable *table;

	switch(id)
	{
		case GGZ_TABLE_LIST:
			printf("GGZCORE/room ## -- table list\n");
			room = ggzcore_server_get_cur_room(ggzserver);
			num = ggzcore_room_get_num_tables(room);
			for(i = 0; i < num; i++)
			{
				table = ggzcore_room_get_nth_table(room, i);
				printf("GGZCORE/room ## table: %s\n", ggzcore_table_get_desc(table));
				tablelist.push_back(ggzcore_table_get_desc(table));
			}
			ggzcore_login = false;
			break;
	}
#endif
}

std::list<std::string> NetGGZ::tables()
{
	return tablelist;
}

