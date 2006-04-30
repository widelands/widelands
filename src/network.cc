/*
 * Copyright (C) 2004-2005 by the Widelands Development Team
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

/*  Everything here is for handling in-game networking.

    What does not belong here:
    Game setup, lobby, LAN game finding, internet gaming, GGZ....
*/

#include <algorithm>
#include "fullscreen_menu_launchgame.h"
#include "game.h"
#include "i18n.h"
#include "interactive_player.h"
#include "network.h"
#include "network_ggz.h"
#include "network_lan_promotion.h"
#include "network_system.h"
#include "player.h"
#include "playercommand.h"
#include "playerdescrgroup.h"
#include "tribe.h"
#include "ui_table.h"
#include "ui_window.h"
#include "wexception.h"

#define CHECK_SYNC_INTERVAL	2000
#define	DELAY_PROBE_INTERVAL	10000
#define MINIMUM_NETWORK_DELAY	10	/* to avoid unneccessary network congestion */
#define INITIAL_NETWORK_DELAY	500


enum {
	NETCMD_UNUSED=0,
	NETCMD_HELLO,
	NETCMD_DISCONNECT,
	NETCMD_DISCONNECT_PLAYER,
	NETCMD_SELECTMAP,
	NETCMD_PLAYERINFO,
	NETCMD_PREGAME_STATUS,
	NETCMD_BEGIN_PREGAME,
	NETCMD_BEGIN_GAME,
	NETCMD_READY,
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


class NetStatusWindow:public UIWindow {
    public:
	NetStatusWindow (UIPanel*);

	void add_player (int);
	void set_ready (int);

    private:
	struct Entry {
		UITable_Entry*	entry;
		int		plnum;
	};

	UITable*		table;

	std::vector<Entry>	entries;
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
	phase=PH_SETUP;

	players_changed=false;

	statuswnd=0;
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

void NetGame::begin_game ()
{
	int i;

	phase=PH_PREGAME;

	statuswnd=new NetStatusWindow(game->get_ipl());
	statuswnd->center_to_parent ();

	for (i=0;i<MAX_PLAYERS;i++)
	    if (player_human & (1<<i))
		statuswnd->add_player (i+1);

	statuswnd->set_ready (playernum);
	player_ready=1<<(playernum-1);
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

void NetGame::disconnect_player (int plnum)
{
	printf ("Player %d has been disconnected\n", plnum);

	// TODO: Let the computer take over this player
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
	Player* pl;
	int i;

	player_enabled=0;
	player_human=0;

	for (i=0;i<MAX_PLAYERS;i++)
		if ((pl=game->get_player(i+1))!=0) {
			player_enabled|=1<<i;

			if (pl->get_type()!=Player::playerAI)
				player_human|=1<<i;
		}

	serializer->begin_packet ();
	serializer->putchar (NETCMD_PLAYERINFO);
	serializer->putchar (player_enabled);
	serializer->putchar (player_human);
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
	serializer->putchar (NETCMD_BEGIN_PREGAME);
	serializer->putlong (common_rand_seed);
	serializer->end_packet ();

	for (unsigned int i=0;i<clients.size();i++)
		serializer->send (clients[i].sock);

	NetGame::begin_game ();
}

void NetHost::handle_network ()
{
	TCPsocket sock;
	unsigned int i,j;

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
		game->add_player (i, Player::playerRemote, tribe, _("I have no name").c_str());

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
			if (SDLNet_SocketReady(clients[i].sock)) {
				if (!clients[i].deserializer->read_packet(clients[i].sock))
				    continue;

				// the network connection to this player has been closed
				SDLNet_TCP_DelSocket (sockset, clients[i].sock);
				SDLNet_TCP_Close (clients[i].sock);

				disconnect_player (clients[i].playernum);

				serializer->begin_packet ();
				serializer->putchar (NETCMD_DISCONNECT_PLAYER);
				serializer->putchar (clients[i].playernum);
				serializer->end_packet ();

				clients.erase (clients.begin()+i);

				for (i=0;i<clients.size();i++)
				    serializer->send (clients[i].sock);

				break;
			}

	// if so, deserialize player commands
	for (i=0;i<clients.size();i++)
		while (clients[i].deserializer->avail())
			switch (clients[i].deserializer->getchar()) {
			    case NETCMD_READY:
				assert (phase==PH_PREGAME);
				assert (statuswnd!=0);

				statuswnd->set_ready (clients[i].playernum);

				player_ready|=1<<(clients[i].playernum-1);

				serializer->begin_packet ();
				serializer->putchar (NETCMD_PREGAME_STATUS);
				serializer->putchar (player_ready);

				if (player_ready==player_human) {
				    serializer->putchar (NETCMD_BEGIN_GAME);

				    phase=PH_INGAME;

				    delete statuswnd;
				    statuswnd=0;
				}

				serializer->end_packet ();

				for (j=0;j<clients.size();j++)
				    serializer->send (clients[j].sock);
				break;
			    case NETCMD_PONG:
				if (clients[i].lag>0) {
				    printf ("Duplicate pong received\n");
				    continue;
				}

				clients[i].lag=std::max( (SDL_GetTicks() - last_ping_sent), (ulong)1);
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
					char buffer[256];
					Chat_Message msg;
//					uchar plrnum =  clients[i].deserializer->getchar();

					clients[i].deserializer->getstr (buffer, 256);

					msg.msg=buffer;
					msg.plrnum=clients[i].playernum;

					send_chat_message_int (msg);
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
	if (phase==PH_INGAME && net_game_time - game->get_gametime() < net_delay/2) {
		serializer->begin_packet ();

		// send any outstanding player commands
		while (!cmds.empty()) {
			log ("%d player commands queued\n", (int)cmds.size());

			PlayerCommand* cmd=cmds.front();
			cmds.pop ();

			log ("player command from player %d\n", cmd->get_sender());

			serializer->putchar (NETCMD_PLAYERCOMMAND);
			cmd->serialize (serializer);

			cmd->set_duetime (net_game_time);
			game->enqueue_command (cmd);
		}

		// update network time
		net_game_time=std::max((game->get_gametime()+net_delay), net_game_time);

		serializer->putchar (NETCMD_ADVANCETIME);
		serializer->putlong (net_game_time);

		serializer->end_packet ();

		// send the packet to all peers
		for (i=0;i<clients.size();i++)
			serializer->send (clients[i].sock);
	}

	// see if it is time to check network lag again
	if (phase==PH_INGAME && SDL_GetTicks()>=next_ping_due) {
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
	serializer->putchar (msg.plrnum);
	serializer->putstr (msg.msg.c_str());
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
	// FIXME: send_chat_message should take a string rather than a Chat_Message
	msg.plrnum=playernum;

	send_chat_message_int (msg);
}

void NetHost::send_game_message (const char* msg)
{
	Chat_Message cm;

	cm.plrnum=0;	// not a player but 'the game'
	cm.msg=msg;

	send_chat_message_int (cm);
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

	if (sock!=0)
	    SDLNet_TCP_Close (sock);
}

void NetClient::begin_game ()
{
	NetGame::begin_game ();

	serializer->begin_packet ();
	serializer->putchar (NETCMD_READY);
	serializer->end_packet ();
	serializer->send (sock);
}

void NetClient::handle_network ()
{
	int i;

	// What does this do here? It probably doesn't belong here.
	NetGGZ::ref()->data();

	// check if data is available on the socket
	while (sock!=0 && SDLNet_CheckSockets(sockset, 0) > 0) {
		if (!deserializer->read_packet(sock))
		    continue;

		// lost network connection
		SDLNet_TCP_DelSocket (sockset, sock);
		SDLNet_TCP_Close (sock);
		sock=0;

		disconnect ();
	}

	while (deserializer->avail())
	        switch (deserializer->getchar()) {
		    case NETCMD_DISCONNECT:
			SDLNet_TCP_DelSocket (sockset, sock);
			SDLNet_TCP_Close (sock);
			sock=0;

			disconnect ();
			break;
		    case NETCMD_DISCONNECT_PLAYER:
			disconnect_player (deserializer->getchar());
			break;
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
			player_enabled=deserializer->getchar();
			player_human=deserializer->getchar();

			for (i=0;i<MAX_PLAYERS;i++)
			    if (i!=playernum-1)
				if (player_enabled & (1<<i)) {
				    playerdescr[i]->set_player_type ((player_human&(1<<i))?Player::playerRemote:Player::playerAI);
				    playerdescr[i]->enable_player (true);
				}
				else
				    playerdescr[i]->enable_player (false);
			break;
		    case NETCMD_PREGAME_STATUS:
			{
			    uchar ready=deserializer->getchar();

			    assert (phase==PH_PREGAME);
			    assert (statuswnd!=0);

			    for (i=0;i<MAX_PLAYERS;i++)
				if (ready & ~player_ready & (1<<i))
				    statuswnd->set_ready (i+1);
			}
			break;
		    case NETCMD_BEGIN_PREGAME:
			common_rand_seed=deserializer->getlong();
			game->logic_rand_seed (common_rand_seed);

			assert (launch_menu!=0);
			launch_menu->start_clicked();

			phase=PH_PREGAME;
			break;
		    case NETCMD_BEGIN_GAME:
			assert (statuswnd!=0);
			delete statuswnd;
			statuswnd=0;

			phase=PH_INGAME;
			break;
		    case NETCMD_PING:	// got a ping, reply with a pong
			serializer->begin_packet ();
			serializer->putchar (NETCMD_PONG);
			serializer->end_packet ();

			if (sock!=0)
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
				char buffer[256];
				uchar player;
				Chat_Message msg;

				player=deserializer->getchar();
            			deserializer->getstr (buffer, 256);

        			msg.plrnum=player;
        			msg.msg=buffer;
        			chat_msg_queue.push (msg);
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

	if (sock!=0)
		serializer->send (sock);
}

void NetClient::send_chat_message (Chat_Message msg)
{
	serializer->begin_packet ();
	serializer->putchar (NETCMD_CHATMESSAGE);

// don't send player number because we cannot send a chat message
// from someone else (other than us)
//	serializer->putchar(msg.plrnum);

	serializer->putstr (msg.msg.c_str());
	serializer->end_packet ();

	if (sock!=0)
		serializer->send (sock);
}

void NetClient::syncreport (uint sync)
{
	serializer->begin_packet ();
	serializer->putchar (NETCMD_SYNCREPORT);
	serializer->putlong (sync);
	serializer->end_packet ();

	if (sock!=0)
		serializer->send (sock);
}

void NetClient::disconnect ()
{
    int i;

    for (i=1;i<=MAX_PLAYERS;i++)
	if (game->get_player(i)!=0 && game->get_player(i)->get_type()==Player::playerRemote)
	    disconnect_player (i);

    // Since we are now independent of the host, we are not bound to network
    // time anymore (nor are we receiving NETCMD_ADVANCETIME packets).
    net_game_time=INT_MAX;
}

/*** class NetStatusWindow ***/

NetStatusWindow::NetStatusWindow (UIPanel* parent)
	:UIWindow(parent, 0, 0, 256, 192, _("Starting network game").c_str())
{
    table=new UITable(this, 0, 0, 256, 192);
    table->add_column (_("Player").c_str(), UITable::STRING, 192);
    table->add_column (_("Status").c_str(), UITable::STRING, 64);
}

void NetStatusWindow::add_player (int num)
{
    char buffer[64];
    Entry entry;

    snprintf (buffer, 64, "%s %d", _("Player").c_str(), num);

    entry.plnum=num;
    entry.entry=new UITable_Entry(table, 0);
    entry.entry->set_string (0, buffer);
    entry.entry->set_string (1, _("Waiting").c_str());

    entries.push_back (entry);
}

void NetStatusWindow::set_ready (int num)
{
    unsigned int i;

    for (i=0;i<entries.size();i++)
	if (entries[i].plnum==num)
		    entries[i].entry->set_string (1, _("Ready").c_str());
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
		putchar (*str++);

	putchar (0);
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

int Deserializer::read_packet (TCPsocket sock)
{
	unsigned char buffer[256];
	int length,amount,i;

	// read packet length (including length field)
	if (SDLNet_TCP_Recv(sock, buffer, 2) < 2)
		return -1;

	length=(buffer[0]<<8) | buffer[1];
	length-=2;	// subtract 2 bytes for the length field

	assert (length>=0);

	// read packet data in chunks of 256 bytes
	while (length>0) {
		amount=std::min( length, 256 );

		SDLNet_TCP_Recv (sock, buffer, amount);

		for (i=0;i<amount;i++)
			queue.push (buffer[i]);

		length-=amount;
	}

	return 0;
}

short Deserializer::getshort ()
{
	short val;

	val=getchar() << 8;
	val|=getchar() & 0xFF;

	return val;
}

long Deserializer::getlong ()
{
	long val;

	val=getchar() << 24;
	val|=(getchar() & 0xFF) << 16;
	val|=(getchar() & 0xFF) << 8;
	val|=getchar() & 0xFF;

	return val;
}

void Deserializer::getstr (char* buffer, int maxlength)
{
	int i;

	for (i=0;(buffer[i]=getchar())!=0;i++)
		if (i==maxlength)
			throw wexception("Deserializer: string too long");
}

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
	throw wexception("Cmd_NetCheckSync is not supposed to be written to a file");
}

void Cmd_NetCheckSync::Read(FileRead* file, Editor_Game_Base* egb, Widelands_Map_Map_Object_Loader* ld)
{
}
