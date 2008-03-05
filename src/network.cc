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

/*  Everything here is for handling in-game networking.

    What does not belong here:
    Game setup, lobby, LAN game finding, internet gaming, GGZ....
*/

#include "network.h"

#include "computer_player.h"
#include "fullscreen_menu_launchgame.h"
#include "game.h"
#include "game_tips.h"
#include "gamesettings.h"
#include "i18n.h"
#include "interactive_player.h"
#include "network_ggz.h"
#include "network_lan_promotion.h"
#include "network_system.h"
#include "player.h"
#include "playercommand.h"
#include "playerdescrgroup.h"
#include "tribe.h"
#include "wexception.h"
#include "wlapplication.h"

#include "ui_progresswindow.h"
#include "ui_table.h"
#include "ui_window.h"

#include "log.h"

#include <algorithm>
#include <climits>

using Widelands::Player;

#define SYNCREPORT_INTERVAL 1000
#define CLIENT_TIMESTAMP_INTERVAL 500
#define SERVER_TIMESTAMP_INTERVAL 100

#define PROTOCOL_VERSION 1

enum {
	NETCMD_UNUSED = 0,

	// Bidirectional messages
	NETCMD_HELLO = 16, // must not be 1, since old versions didn't send PROTOCOL_VERSION
	NETCMD_TIME = 22,
	NETCMD_PLAYERCOMMAND = 24,

	// Server->Client messages
	NETCMD_SETTING_MAP = 4,
	NETCMD_SETTING_TRIBES = 5,
	NETCMD_SETTING_ALLPLAYERS = 6,
	NETCMD_SETTING_PLAYER = 7,
	NETCMD_PING = 10,
	NETCMD_LAUNCH = 20,
	NETCMD_SETSPEED = 21,
	NETCMD_WAIT = 23,
	NETCMD_SYNCREQUEST = 25,

	// Client->Server messages
	NETCMD_PONG = 11,
	NETCMD_SETTING_CHANGETRIBE = 17,
	NETCMD_SYNCREPORT = 26
};


class Cmd_NetCheckSync : public Widelands::Command {
private:
	SyncCallback * m_callback;

public:
	Cmd_NetCheckSync (int32_t dt, SyncCallback* cb) : Command (dt) {m_callback=cb;}

	virtual void execute (Widelands::Game *);

	virtual int32_t get_id() {return QUEUE_CMD_NETCHECKSYNC;}
};


void Cmd_NetCheckSync::execute (Widelands::Game *) {
	m_callback->syncreport();
}

struct NetStatusWindow : public UI::Window {
	NetStatusWindow (UI::Panel*);

	void add_player (int32_t);
	void set_ready (int32_t);

private:
	struct Entry {
		UI::Table<void *>::Entry_Record * entry;
		int32_t                               plnum;
	};

	UI::Table<void *> table;

	std::vector<Entry> entries;
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


/**
 * Keeping track of network time: This class answers the question of how
 * far the local simulation time should proceed, given the history of network
 * time messages.
 *
 * In general, the time progresses as fast as given by the speed, but we
 * introduce some elasticity to catch up with the network time if necessary,
 * and we never progress past the received network time.
 */
class NetworkTime {
public:
	NetworkTime();

	void reset(int32_t ntime);
	void fastforward();

	void think(uint32_t speed);
	int32_t time() const;
	int32_t networktime() const;
	bool recv(int32_t ntime);

private:
	int32_t m_networktime;
	int32_t m_time;

	int32_t m_lastframe;

	/// This is an attempt to measure how far behind the network time we are.
	uint32_t m_latency;
};

NetworkTime::NetworkTime()
{
	reset(0);
}

void NetworkTime::reset(int32_t ntime)
{
	m_networktime = m_time = ntime;
	m_lastframe = WLApplication::get()->get_time();
	m_latency = 0;
}

void NetworkTime::fastforward()
{
	m_time = m_networktime;
	m_lastframe = WLApplication::get()->get_time();
}

void NetworkTime::think(uint32_t speed)
{
	int32_t curtime = WLApplication::get()->get_time();
	int32_t delta = curtime - m_lastframe;
	m_lastframe = curtime;

	// in case weird things are happening with the system time
	// (e.g. debugger, extremely slow simulation, ...)
	if (delta < 0)
		delta = 0;
	else if (delta > 1000)
		delta = 1000;

	delta = (delta * speed) / 1000;

	int32_t behind = m_networktime - m_time;

	// Play catch up
	uint32_t speedup = 0;
	if (m_latency > 10*delta)
		speedup = m_latency/3; // just try to kill as much of the latency as possible if we're that far behind
	else if (m_latency > delta)
		speedup = delta/8; // speed up by 12.5%
	if (delta + speedup > behind)
		speedup = behind - delta;

	delta += speedup;
	m_latency -= speedup;

	if (delta > behind)
		delta = behind;

	m_time += delta;
}

int32_t NetworkTime::time() const
{
	return m_time;
}

int32_t NetworkTime::networktime() const
{
	return m_networktime;
}

bool NetworkTime::recv(int32_t ntime)
{
	if (ntime - m_networktime < 0) {
		log("NetworkTime: running backwards\n");
		return false;
	}

	uint32_t behind = m_networktime - m_time;

	if (behind < m_latency)
		m_latency = behind;
	else
		m_latency = ((m_latency * 7) + behind) / 8;

#if 0
	log
		("NetworkTime: New networktime %i (local time %i), behind %i, latency %u\n",
		 ntime, m_time, m_networktime - m_time, m_latency);
#endif

	m_networktime = ntime;

	return true;
}



/*** class NetHost ***/

class HostGameSettingsProvider : public GameSettingsProvider {
	NetHost* h;
public:
	HostGameSettingsProvider(NetHost* _h)
		: h(_h) {}

	virtual const GameSettings& settings() {return h->settings();}

	virtual bool canChangeMap() {return true;}
	virtual bool canChangePlayerState(uint8_t number) {return number != 0;}
	virtual bool canChangePlayerTribe(uint8_t number) {
		if (number == 0)
			return true;
		if (number >= settings().players.size())
			return false;
		return settings().players[number].state == PlayerSettings::stateComputer;
	}

	virtual bool canLaunch() {return h->canLaunch();}

	virtual void setMap(const std::string& mapname, const std::string& mapfilename, uint32_t maxplayers) {
		h->setMap(mapname, mapfilename, maxplayers);
	}
	virtual void setPlayerState(uint8_t number, PlayerSettings::State state) {
		if (number == 0 || number >= settings().players.size())
			return;

		h->setPlayerState(number, state);
	}
	virtual void nextPlayerState(uint8_t number) {
		if (number == 0 || number >= settings().players.size())
			return;

		PlayerSettings::State newstate = PlayerSettings::stateClosed;
		switch (h->settings().players[number].state) {
		case PlayerSettings::stateClosed:
			newstate = PlayerSettings::stateOpen;
			break;
		case PlayerSettings::stateOpen:
		case PlayerSettings::stateHuman:
			newstate = PlayerSettings::stateComputer;
			break;
		case PlayerSettings::stateComputer:
			newstate = PlayerSettings::stateOpen;
			break;
		}

		setPlayerState(number, newstate);
	}

	virtual void setPlayerTribe(uint8_t number, const std::string& tribe) {
		if (number >= h->settings().players.size())
			return;

		if (number == 0 || settings().players[number].state == PlayerSettings::stateComputer)
			h->setPlayerTribe(number, tribe);
	}
};

struct Client {
	TCPsocket sock;
	Deserializer deserializer;
	int32_t playernum; // -1 as long as the client hasn't said Hi.
	md5_checksum syncreport;
	bool syncreport_arrived;
	int32_t time; // last time report
};

struct NetHostImpl {
	GameSettings settings;

	LAN_Game_Promoter * promoter;
	TCPsocket svsock;
	SDLNet_SocketSet sockset;

	/// List of connected clients. Note that clients are not in the same
	/// order as players. In fact, a client is not assigned to a player as long
	/// as he hasn't said Hi yet.
	std::vector<Client> clients;

	/// The game itself; only non-null while game is running
	Widelands::Game* game;

	/// If we were to send out a plain networktime packet, this would be the time.
	/// However, we have not yet committed to this networktime.
	int32_t pseudo_networktime;
	int32_t last_heartbeat;

	/// The networktime we committed to by sending it across the network.
	int32_t committed_networktime;

	/// This is the time for local simulation
	NetworkTime time;

	/// The real speed used for simulation, in milliseconds per second
	uint32_t realspeed;
	int32_t lastframe;

	/// The speed we want to have
	uint32_t speed;

	/// All currently running computer players, *NOT* in one-one correspondence
	/// with \ref Player objects
	std::vector<Computer_Player *> computerplayers;

	/// \c true if a syncreport is currently in flight
	bool syncreport_pending;
	int32_t syncreport_time;
	md5_checksum syncreport;
	bool syncreport_arrived;
};

NetHost::NetHost ()
: d(new NetHostImpl)
{
	log("[Host] starting up.\n");

	// create a listening socket
	IPaddress myaddr;
	SDLNet_ResolveHost (&myaddr, NULL, WIDELANDS_PORT);
	d->svsock = SDLNet_TCP_Open(&myaddr);

	d->sockset = SDLNet_AllocSocketSet(16);
	d->promoter = new LAN_Game_Promoter();
	d->game = 0;
	d->pseudo_networktime = 0;
	d->realspeed = 0;
	d->speed = 1000;
	d->syncreport_pending = false;
	d->syncreport_time = 0;

	Widelands::Tribe_Descr::get_all_tribenames(d->settings.tribes);
}

NetHost::~NetHost ()
{
	clearComputerPlayers();

	SDLNet_FreeSocketSet (d->sockset);

	// close all open sockets
	if (d->svsock != 0)
		SDLNet_TCP_Close (d->svsock);

	for (uint32_t i = 0; i < d->clients.size(); ++i)
		SDLNet_TCP_Close (d->clients[i].sock);

	delete d;
	d = 0;
}

void NetHost::clearComputerPlayers()
{
	for (uint32_t i = 0; i < d->computerplayers.size(); ++i)
		delete d->computerplayers[i];
	d->computerplayers.clear();
}

void NetHost::initComputerPlayers()
{
	const Widelands::Player_Number nr_players = d->game->map().get_nrplayers();
	iterate_players_existing(p, nr_players, *d->game, plr) {
		if (p == 1)
			continue;

		uint32_t client;
		for (client = 0; client < d->clients.size(); ++client)
			if (d->clients[client].playernum+1 == p)
				break;

		if (client >= d->clients.size())
			d->computerplayers.push_back(new Computer_Player(*d->game, p));
	}
}

void NetHost::run()
{
	HostGameSettingsProvider hp(this);
	Fullscreen_Menu_LaunchGame lgm(&hp, this);
	const int32_t code = lgm.run();

	if (code <= 0)
		return;

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients[i].playernum == -1) {
			log("[Host]: say goodbye to client %i, game is starting without him\n", i);
			disconnectClient(i);
		}
	}

	SendPacket s;
	s.Unsigned8(NETCMD_LAUNCH);
	broadcast(s);

	Widelands::Game game;
	try {
		UI::ProgressWindow loaderUI("pics/progress.png");
		GameTips tips (loaderUI);

		loaderUI.step(_("Preparing game"));

		d->game = &game;
		game.set_game_controller(this);
		game.set_iabase(new Interactive_Player(game, 1));
		game.init(loaderUI, d->settings);
		d->pseudo_networktime = game.get_gametime();
		d->time.reset(d->pseudo_networktime);
		d->lastframe = WLApplication::get()->get_time();
		d->last_heartbeat = d->lastframe;

		d->committed_networktime = d->pseudo_networktime;

		for (uint32_t i = 0; i < d->clients.size(); ++i)
			d->clients[i].time = d->committed_networktime-1;

		// The call to checkHungClients ensures that the game leaves the
		// wait mode (d->realspeed == 0) when there are no clients
		checkHungClients();
		initComputerPlayers();
		game.run(loaderUI);
		clearComputerPlayers();
	} catch (...) {
		WLApplication::emergency_save(game);
		clearComputerPlayers();
		d->game = 0;
		throw;
	}
	d->game = 0;
}

void NetHost::think()
{
	handle_network();

	if (d->game) {
		int32_t curtime = WLApplication::get()->get_time();
		int32_t delta = curtime - d->lastframe;
		d->lastframe = curtime;

		if (d->realspeed) {
			int32_t diff = (delta * d->realspeed) / 1000;
			d->pseudo_networktime += diff;
		}

		d->time.think(d->realspeed);

		if (d->pseudo_networktime != d->committed_networktime)
		{
			if (d->pseudo_networktime - d->committed_networktime < 0) {
				d->pseudo_networktime = d->committed_networktime;
			} else if (curtime - d->last_heartbeat >= SERVER_TIMESTAMP_INTERVAL) {
				d->last_heartbeat = curtime;

				SendPacket s;
				s.Unsigned8(NETCMD_TIME);
				s.Signed32(d->pseudo_networktime);
				broadcast(s);

				committedNetworkTime(d->pseudo_networktime);

				checkHungClients();
			}
		}

		for (uint32_t i = 0; i < d->computerplayers.size(); ++i)
			d->computerplayers[i]->think();
	}
}

void NetHost::sendPlayerCommand(Widelands::PlayerCommand* pc)
{
	pc->set_duetime(d->committed_networktime+1);

	SendPacket s;
	s.Unsigned8(NETCMD_PLAYERCOMMAND);
	s.Signed32(pc->get_duetime());
	pc->serialize(s);
	broadcast(s);
	d->game->enqueue_command(pc);

	committedNetworkTime(d->committed_networktime+1);
}

int32_t NetHost::getFrametime()
{
	return d->time.time() - d->game->get_gametime();
}

std::string NetHost::getGameDescription()
{
	return "network player 0";
}

const GameSettings& NetHost::settings()
{
	return d->settings;
}

bool NetHost::canLaunch()
{
	return d->settings.mapname.size() != 0 && d->settings.players.size() >= 1;
}

void NetHost::setMap(const std::string& mapname, const std::string& mapfilename, uint32_t maxplayers)
{
	d->settings.mapname = mapname;
	d->settings.mapfilename = mapfilename;

	uint32_t oldplayers = d->settings.players.size();

	while (oldplayers > maxplayers) {
		oldplayers--;
		disconnectPlayer(oldplayers);
	}

	d->settings.players.resize(maxplayers);

	while (oldplayers < maxplayers) {
		PlayerSettings& player = d->settings.players[oldplayers];
		player.state = (oldplayers == 0) ? PlayerSettings::stateHuman : PlayerSettings::stateOpen;
		player.tribe = d->settings.tribes[0];
		char buf[200];
		snprintf(buf, sizeof(buf), "%s %u", _("Player"), oldplayers+1);
		player.name = buf;
		oldplayers++;
	}

	// Broadcast new map info
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_MAP);
	writeSettingMap(s);
	broadcast(s);

	// Broadcast new player settings
	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLPLAYERS);
	writeSettingAllPlayers(s);
	broadcast(s);
}

void NetHost::setPlayerState(uint8_t number, PlayerSettings::State state)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings& player = d->settings.players[number];

	if (player.state == state)
		return;

	player.state = state;

	// Broadcast change
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);

	if (player.state != PlayerSettings::stateHuman)
		disconnectPlayer(number);
}


void NetHost::setPlayerTribe(uint8_t number, const std::string& tribe)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings& player = d->settings.players[number];

	if (player.tribe == tribe)
		return;

	if (std::find(d->settings.tribes.begin(), d->settings.tribes.end(), tribe) == d->settings.tribes.end()) {
		log("Player %u attempted to change to tribe %s; not a valid tribe\n", number, tribe.c_str());
		return;
	}

	player.tribe = tribe;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


// Send the packet to all properly connected clients
void NetHost::broadcast(SendPacket& packet)
{
	for
		(std::vector<Client>::iterator it = d->clients.begin();
		 it != d->clients.end();
		 ++it)
	{
		if (it->playernum == -1)
			continue;

		packet.send(it->sock);
	}
}

void NetHost::writeSettingMap(SendPacket& packet)
{
	packet.String(d->settings.mapname);
	packet.String(d->settings.mapfilename);
}

void NetHost::writeSettingPlayer(SendPacket& packet, uint8_t number)
{
	PlayerSettings& player = d->settings.players[number];
	packet.Unsigned8(static_cast<uint8_t>(player.state));
	packet.String(player.name);
	packet.String(player.tribe);
}


void NetHost::writeSettingAllPlayers(SendPacket& packet)
{
	packet.Unsigned8(d->settings.players.size());
	for (uint8_t i = 0; i < d->settings.players.size(); ++i)
		writeSettingPlayer(packet, i);
}


/// Respond to a client's Hello message.
void NetHost::welcomeClient(uint32_t number)
{
	assert(number < d->clients.size());

	Client& client = d->clients[number];

	assert(client.playernum == -1);
	assert(client.sock);

	// Find a free player slot
	uint8_t playernum;
	for (playernum = 0; playernum < d->settings.players.size(); ++playernum)
		if (d->settings.players[playernum].state == PlayerSettings::stateOpen)
			break;

	if (playernum >= d->settings.players.size()) {
		log("[Host]: client %u: no open player slot\n", number);
		disconnectClient(number);
		return;
	}

	setPlayerState(playernum, PlayerSettings::stateHuman);

	// The client gets its own initial data set, so we set the player number
	// after the broadcast related to setPlayerState have gone through.
	client.playernum = playernum;

	log("[Host]: client %u: welcome to playernum %u\n", number, playernum);

	SendPacket s;
	s.Unsigned8(NETCMD_HELLO);
	s.Unsigned8(PROTOCOL_VERSION);
	s.Unsigned8(playernum);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_MAP);
	writeSettingMap(s);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_TRIBES);
	s.Unsigned8(d->settings.tribes.size());
	for (uint8_t i = 0; i < d->settings.tribes.size(); ++i)
		s.String(d->settings.tribes[i]);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLPLAYERS);
	writeSettingAllPlayers(s);
	s.send(client.sock);
}

void NetHost::committedNetworkTime(int32_t time)
{
	assert(time - d->committed_networktime > 0);

	d->committed_networktime = time;
	d->time.recv(time);

	if (!d->syncreport_pending && d->committed_networktime - d->syncreport_time >= SYNCREPORT_INTERVAL)
		requestSyncReports();
}

void NetHost::recvClientTime(uint32_t number, int32_t time)
{
	assert(number < d->clients.size());

	Client& client = d->clients[number];

	if (time - client.time < 0) {
		log("[Host]: Client %i time running backwards\n", number);
		disconnectClient(number);
		return;
	}
	if (d->committed_networktime - time < 0) {
		log("[Host]: Client %i rushes past committed networktime\n", number);
		disconnectClient(number);
		return;
	}
	if (d->syncreport_pending && !client.syncreport_arrived) {
		if (time - d->syncreport_time > 0) {
			log
				("[Host]: client %i tries to skip syncreport (time %i, syncreport at %i)\n",
				 number, time, d->syncreport_time);
			disconnectClient(number);
			return;
		}
	}

	client.time = time;
	log("[Host]: Client %i: Time %i\n", number, time);

	if (d->realspeed == 0) {
		log
			("[Host]: Client %i reports time %i (networktime = %i) during hang\n",
			 number, time, d->committed_networktime);
		checkHungClients();
	}
}


void NetHost::checkHungClients()
{
	int nrready = 0;
	int nrdelayed = 0;
	int nrhung = 0;

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients[i].playernum == -1)
			continue;

		int32_t delta = d->committed_networktime - d->clients[i].time;

		if (delta == 0) {
			nrready++;
		} else {
			nrdelayed++;
			if (delta > (5*CLIENT_TIMESTAMP_INTERVAL*static_cast<int32_t>(d->speed))/1000) {
				log("[Host]: Client %i hung\n", i);
				nrhung++;
			}
		}
	}

	if (d->realspeed) {
		if (nrhung) {
			log("[Host]: %i clients hung. Entering wait mode\n", nrhung);

			// Brake and wait
			setRealSpeed(0);

			SendPacket s;
			s.Unsigned8(NETCMD_WAIT);
			broadcast(s);
		}
	} else {
		if (nrdelayed == 0) {
			setRealSpeed(d->speed);
			if (!d->syncreport_pending)
				requestSyncReports();
		}
	}
}


void NetHost::setRealSpeed(uint32_t speed)
{
	if (speed > std::numeric_limits<uint16_t>::max())
		speed = std::numeric_limits<uint16_t>::max();

	if (speed == d->realspeed)
		return;

	d->realspeed = speed;

	SendPacket s;
	s.Unsigned8(NETCMD_SETSPEED);
	s.Unsigned16(speed);
	broadcast(s);
}

/**
 * Request sync reports from all clients at the next possible time.
 */
void NetHost::requestSyncReports()
{
	assert(!d->syncreport_pending);

	d->syncreport_pending = true;
	d->syncreport_arrived = false;
	d->syncreport_time = d->committed_networktime+1;

	for (uint32_t i = 0; i < d->clients.size(); ++i)
		d->clients[i].syncreport_arrived = false;

	log("[Host]: Requesting sync reports for time %i\n", d->syncreport_time);

	SendPacket s;
	s.Unsigned8(NETCMD_SYNCREQUEST);
	s.Signed32(d->syncreport_time);
	broadcast(s);

	d->game->enqueue_command(new Cmd_NetCheckSync(d->syncreport_time, this));

	committedNetworkTime(d->syncreport_time);
}

/**
 * Check whether all sync reports have arrived, and if so, compare.
 */
void NetHost::checkSyncReports()
{
	assert(d->syncreport_pending);

	if (!d->syncreport_arrived)
		return;

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients[i].playernum != -1 && !d->clients[i].syncreport_arrived)
			return;
	}

	d->syncreport_pending = false;
	log("[Host]: comparing syncreports for time %i\n", d->syncreport_time);

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		Client& client = d->clients[i];
		if (client.playernum == -1)
			continue;

		if (client.syncreport != d->syncreport) {
			log
				("[Host] lost synchronization with client %u!\n"
				 "I have:     %s\n"
				 "Client has: %s\n",
				 i, d->syncreport.str().c_str(), client.syncreport.str().c_str());
			disconnectClient(i);
		}
	}
}

void NetHost::syncreport()
{
	assert(d->game->get_gametime() == d->syncreport_time);

	d->syncreport = d->game->get_sync_hash();
	d->syncreport_arrived = true;

	checkSyncReports();
}


void NetHost::handle_network ()
{
	TCPsocket sock;

	if (d->promoter != 0)
		d->promoter->run ();

	// if we are in the game initiation phase, check for new connections
	while (d->svsock != 0 && (sock = SDLNet_TCP_Accept(d->svsock)) != 0) {
		log("[Host] Received a connection request\n");

		SDLNet_TCP_AddSocket (d->sockset, sock);

		Client peer;
		peer.sock = sock;
		peer.playernum = -1;
		peer.syncreport_arrived = false;
		d->clients.push_back (peer);

		// Now we wait for the client to say Hi in the right language,
		// unless the game has already started
		if (d->game) {
			log("[Host]: client trying to connect, game is already running\n");
			disconnectClient(d->clients.size()-1);
		}
	}

	// check if we hear anything from our clients
	while (SDLNet_CheckSockets(d->sockset, 0) > 0) {
		for (size_t i = 0; i < d->clients.size(); ++i) {
			while (SDLNet_SocketReady(d->clients[i].sock)) {
				if (d->clients[i].deserializer.read(d->clients[i].sock))
					continue;

				// the network connection to this player has been closed
				log("[Host] client %i has left\n", i);
				disconnectClient(i);
				break;
			}
		}
	}

	// Now process client messages
	for (size_t i = 0; i < d->clients.size(); ++i) {
		Client& client = d->clients[i];
		while (client.sock && client.deserializer.avail()) {
			RecvPacket r(client.deserializer);
			uint8_t cmd = r.Unsigned8();

			if (client.playernum == -1) {
				if (d->game) {
					log("[Host]: client %i pre-connected but game is already running\n", i);
					disconnectClient(i);
					break;
				}
				if (cmd != NETCMD_HELLO) {
					log("[Host]: client %i: HELLO expected instead of %u\n", i, cmd);
					disconnectClient(i);
					break;
				}
				uint8_t version = r.Unsigned8();
				if (version != PROTOCOL_VERSION) {
					log("[Host]: client %i: talks unsupported version %u\n", i, version);
					disconnectClient(i);
					break;
				}

				welcomeClient(i);
				continue;
			}

			switch (cmd) {
			case NETCMD_PONG:
				log("[Host] client %i: got pong\n", i);
				break;

			case NETCMD_SETTING_CHANGETRIBE:
				// Don't be harsh about packets of this type arriving
				// out of order - the client might just have had bad luck with the timing.
				if (!d->game) {
					std::string tribe = r.String();
					setPlayerTribe(client.playernum, tribe);
				}
				break;

			case NETCMD_TIME:
				if (!d->game) {
					log("[Host] client %i: sent time while game not running\n", i);
					disconnectClient(i);
					break;
				}
				recvClientTime(i, r.Signed32());
				break;

			case NETCMD_PLAYERCOMMAND: {
				if (!d->game) {
					log("[Host] client %i: sent playercommand while game not running\n", i);
					disconnectClient(i);
					break;
				}
				int32_t time = r.Signed32();
				Widelands::PlayerCommand* plcmd = Widelands::PlayerCommand::deserialize(r);
				log
					("[Host] client %i (%i) sent player command for %i, time = %i\n",
					 i, client.playernum, plcmd->get_sender(), time);
				recvClientTime(i, time);
				if (plcmd->get_sender() != client.playernum+1) {
					log("[Host] client %i: trying to send playercommand for %i\n", i, plcmd->get_sender());
					disconnectClient(i);
					break;
				}
				sendPlayerCommand(plcmd);
			} break;

			case NETCMD_SYNCREPORT: {
				if (!d->game || !d->syncreport_pending || client.syncreport_arrived) {
					log("[Host] client %i: unexpected syncreport\n", i);
					disconnectClient(i);
					break;
				}
				int32_t time = r.Signed32();
				r.Data(client.syncreport.data, 16);
				client.syncreport_arrived = true;
				recvClientTime(i, time);
				checkSyncReports();
				break;
			}

			default:
				log("[Host] client %i: sent bad cmd %u\n", i, cmd);
				disconnectClient(i);
				break;
			}
		}
	}

	reaper();
}

void NetHost::disconnectPlayer(uint8_t number)
{
	bool needai = false;

	for (uint32_t index = 0; index < d->clients.size(); ++index) {
		Client& client = d->clients[index];
		if (client.playernum != static_cast<int32_t>(number))
			continue;

		client.playernum = -1;
		disconnectClient(index);

		setPlayerState(number, PlayerSettings::stateOpen);
		needai = true;
	}

	if (needai && d->game)
		d->computerplayers.push_back(new Computer_Player(*d->game, number+1));
}

void NetHost::disconnectClient(uint32_t number)
{
	assert(number < d->clients.size());

	Client& client = d->clients[number];
	if (client.playernum != -1)
		disconnectPlayer(client.playernum);

	if (client.sock) {
		SDLNet_TCP_DelSocket (d->sockset, client.sock);
		SDLNet_TCP_Close (client.sock);
		client.sock = 0;
	}

	if (d->game)
		checkHungClients();
}

// Reap (i.e. delete) all disconnected clients.
void NetHost::reaper()
{
	uint32_t index = 0;
	while (index < d->clients.size()) {
		if (d->clients[index].sock) {
			index++;
		} else {
			d->clients.erase(d->clients.begin() + index);
		}
	}
}


/*** class NetClient ***/

struct NetClientImpl {
	GameSettings settings;
	int32_t playernum; // is -1 until we are assigned our number, then it's 0-based

	/// The socket that connects us to the host
	TCPsocket sock;

	/// Socket set used for selection
	SDLNet_SocketSet sockset;

	/// Deserializer acts as a buffer for packets (reassembly/splitting up)
	Deserializer deserializer;

	/// Currently active modal panel. Receives an end_modal on disconncet
	UI::Panel* modal;

	/// Current game. Only non-null if a game is actually running.
	Widelands::Game* game;

	NetworkTime time;

	/// \c true if we received a message indicating that the server is waiting
	/// Send a time message as soon as we caught up to networktime
	bool server_is_waiting;

	/// Data for the last time message we sent.
	int32_t lasttimestamp;
	int32_t lasttimestamp_realtime;

	/// The real target speed, in milliseconds per second.
	/// This is always set by the server
	uint32_t realspeed;
};

NetClient::NetClient (IPaddress* svaddr)
: d(new NetClientImpl)
{
	d->sock = SDLNet_TCP_Open(svaddr);
	if (d->sock == 0)
		throw wexception("[Client] SDLNet_TCP_Open failed: %s", SDLNet_GetError());

	d->sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (d->sockset, d->sock);

	d->playernum = -1;
	d->modal = 0;
	d->game = 0;
	d->realspeed = 0;
}

NetClient::~NetClient ()
{
	SDLNet_FreeSocketSet (d->sockset);

	if (d->sock != 0)
		SDLNet_TCP_Close (d->sock);

	delete d;
	d = 0;
}

void NetClient::run ()
{
	SendPacket s;
	s.Unsigned8(NETCMD_HELLO);
	s.Unsigned8(PROTOCOL_VERSION);
	s.send(d->sock);

	Fullscreen_Menu_LaunchGame lgm(this, this);
	d->modal = &lgm;
	int32_t code = lgm.run();
	d->modal = 0;
	if (code <= 0)
		return;

	d->server_is_waiting = true;

	Widelands::Game game;
	try {
		UI::ProgressWindow loaderUI("pics/progress.png");
		GameTips tips (loaderUI);

		loaderUI.step(_("Preparing game"));

		d->game = &game;
		game.set_game_controller(this);
		game.set_iabase(new Interactive_Player(game, d->playernum+1));
		game.init(loaderUI, d->settings);
		d->time.reset(game.get_gametime());
		d->lasttimestamp = game.get_gametime();
		d->lasttimestamp_realtime = WLApplication::get()->get_time();
		game.run(loaderUI);
	} catch (...) {
		WLApplication::emergency_save(game);
		throw;
	}
}

void NetClient::think()
{
	handle_network();

	if (d->game) {
		if (d->realspeed == 0 || d->server_is_waiting) {
			d->time.fastforward();
		} else {
			d->time.think(d->realspeed);
		}

		if (d->server_is_waiting && d->game->get_gametime() == d->time.networktime()) {
			sendTime();
			d->server_is_waiting = false;
		} else if (d->game->get_gametime() != d->lasttimestamp) {
			int32_t curtime = WLApplication::get()->get_time();
			if (curtime - d->lasttimestamp_realtime > CLIENT_TIMESTAMP_INTERVAL)
				sendTime();
		}
	}
}

void NetClient::sendPlayerCommand(Widelands::PlayerCommand* pc)
{
	assert(d->game);

	log("[Client]: send playercommand at time %i\n", d->game->get_gametime());

	SendPacket s;
	s.Unsigned8(NETCMD_PLAYERCOMMAND);
	s.Signed32(d->game->get_gametime());
	pc->serialize(s);
	s.send(d->sock);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = WLApplication::get()->get_time();

	delete pc;
}

int32_t NetClient::getFrametime()
{
	return d->time.time() - d->game->get_gametime();
}

std::string NetClient::getGameDescription()
{
	char buf[200];
	snprintf(buf, sizeof(buf), "network player %u", d->playernum);
	return buf;
}

const GameSettings& NetClient::settings()
{
	return d->settings;
}

bool NetClient::canChangeMap()
{
	return false;
}

bool NetClient::canChangePlayerState(uint8_t)
{
	return false;
}

bool NetClient::canChangePlayerTribe(uint8_t number)
{
	return number == d->playernum;
}

bool NetClient::canLaunch()
{
	return false;
}

void NetClient::setMap(const std::string&, const std::string&, uint32_t)
{
	// client is not allowed to do this
}

void NetClient::setPlayerState(uint8_t, PlayerSettings::State)
{
	// client is not allowed to do this
}

void NetClient::nextPlayerState(uint8_t)
{
	// client is not allowed to do this
}

void NetClient::setPlayerTribe(uint8_t number, const std::string& tribe)
{
	if (number != d->playernum)
		return;

	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGETRIBE);
	s.String(tribe);
	s.send(d->sock);
}

void NetClient::recvOnePlayer(uint8_t number, Widelands::StreamRead& packet)
{
	if (number >= d->settings.players.size()) {
		log("[Client]: recvOnePlayer out of bounds\n");
		disconnect();
		return;
	}

	PlayerSettings& player = d->settings.players[number];
	player.state = static_cast<PlayerSettings::State>(packet.Unsigned8());
	player.name = packet.String();
	player.tribe = packet.String();
}

void NetClient::sendTime()
{
	assert(d->game);

	log("[Client]: sending timestamp: %i\n", d->game->get_gametime());

	SendPacket s;
	s.Unsigned8(NETCMD_TIME);
	s.Signed32(d->game->get_gametime());
	s.send(d->sock);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = WLApplication::get()->get_time();
}

void NetClient::syncreport()
{
	if (d->sock) {
		SendPacket s;
		s.Unsigned8(NETCMD_SYNCREPORT);
		s.Signed32(d->game->get_gametime());
		s.Data(d->game->get_sync_hash().data, 16);
		s.send(d->sock);
	}
}

void NetClient::handle_network ()
{
	// check if data is available on the socket
	while (d->sock != 0 && SDLNet_CheckSockets(d->sockset, 0) > 0) {
		if (d->deserializer.read(d->sock))
			continue;

		// lost network connection
		SDLNet_TCP_DelSocket (d->sockset, d->sock);
		SDLNet_TCP_Close (d->sock);
		d->sock = 0;
		log("[Client] Connection Lost\n");
		disconnect();
		return;
	}

	while (d->deserializer.avail()) {
		RecvPacket packet(d->deserializer);
		uint8_t cmd = packet.Unsigned8();

		if (d->playernum == -1) {
			if (cmd != NETCMD_HELLO) {
				log("[Client] Expected HELLO, got cmd %u (likely incompatible versions)\n", cmd);
				disconnect();
				return;
			}
			uint8_t version = packet.Unsigned8();
			if (version != PROTOCOL_VERSION) {
				log("[Client] Server has version %u\n", version);
				disconnect();
				return;
			}
			d->playernum = packet.Unsigned8();
			continue;
		}

		switch (cmd) {
		case NETCMD_PING: {
			SendPacket s;
			s.Unsigned8(NETCMD_PONG);
			s.send(d->sock);

			log ("[Client] Pong!\n");
			break;
		}

		case NETCMD_SETTING_MAP:
			d->settings.mapname = packet.String();
			d->settings.mapfilename = packet.String();
			log("[Client] SETTING_MAP '%s' '%s'\n", d->settings.mapname.c_str(), d->settings.mapfilename.c_str());
			break;

		case NETCMD_SETTING_TRIBES: {
			uint8_t count = packet.Unsigned8();
			d->settings.tribes.clear();
			for (uint8_t i = 0; i < count; ++i)
				d->settings.tribes.push_back(packet.String());
			break;
		}

		case NETCMD_SETTING_ALLPLAYERS: {
			d->settings.players.resize(packet.Unsigned8());
			for (uint8_t i = 0; i < d->settings.players.size(); ++i)
				recvOnePlayer(i, packet);
			break;
		}
		case NETCMD_SETTING_PLAYER: {
			uint8_t player = packet.Unsigned8();
			recvOnePlayer(player, packet);
			break;
		}
		case NETCMD_LAUNCH: {
			if (!d->modal) {
				log("[Client] received LAUNCH, but no modal screen\n");
				disconnect();
				return;
			}
			d->modal->end_modal(1);
			break;
		}
		case NETCMD_SETSPEED:
			d->realspeed = packet.Unsigned16();
			log("[Client] speed: %u.%03u\n", d->realspeed / 1000, d->realspeed % 1000);
			break;
		case NETCMD_TIME: {
			if (!d->time.recv(packet.Signed32())) {
				disconnect();
				return;
			}
			break;
		}
		case NETCMD_WAIT:
			log("[Client]: server is waiting.\n");
			d->server_is_waiting = true;
			break;
		case NETCMD_PLAYERCOMMAND: {
			if (!d->game) {
				log("[Client]: received PLAYERCOMMAND while game not running\n");
				disconnect();
				return;
			}

			int32_t time = packet.Signed32();
			Widelands::PlayerCommand* plcmd = Widelands::PlayerCommand::deserialize(packet);
			plcmd->set_duetime(time);
			d->game->enqueue_command(plcmd);
			if (!d->time.recv(time)) {
				disconnect();
				return;
			}
			break;
		}
		case NETCMD_SYNCREQUEST: {
			if (!d->game) {
				log("[Client]: received SYNCREQUEST while game not running\n");
				disconnect();
				return;
			}
			int32_t time = packet.Signed32();
			if (!d->time.recv(time)) {
				disconnect();
				return;
			}
			d->game->enqueue_command(new Cmd_NetCheckSync(time, this));
			break;
		}
		default:
			log("[Client] Invalid network data received (cmd = %u)\n", cmd);
			disconnect();
			return;
		}
	}
}


void NetClient::disconnect ()
{
	if (d->sock) {
		SDLNet_TCP_DelSocket (d->sockset, d->sock);
		SDLNet_TCP_Close (d->sock);
		d->sock = 0;
	}

	if (d->modal) {
		d->modal->end_modal(0);
		d->modal = 0;
	}
}

/*** class NetStatusWindow ***/

// NetStatusWindow::NetStatusWindow (UI::Panel* parent) :
// UI::Window(parent, 0, 0, 256, 192, _("Starting network game")),
// table(this, 0, 0, 256, 192)
// {
// 	table.add_column (_("Player"), 192);
// 	table.add_column (_("Status"),  64);
// }
//
// void NetStatusWindow::add_player (int32_t const num) {
// 	char buffer[64];
//
// 	snprintf (buffer, sizeof(buffer), _("Player %d"), num);
//
// 	Entry entry = {&table.add(), num};
// 	entry.entry->set_string (0, buffer);
// 	entry.entry->set_string (1, _("Waiting"));
//
// 	entries.push_back (entry);
// }
//
// void NetStatusWindow::set_ready (int32_t num)
// {
// 	for (size_t i = 0; i < entries.size(); ++i)
// 		if (entries[i].plnum == num)
// 			entries[i].entry->set_string (1, _("Ready"));
// }

/*** class SendPacket ***/

SendPacket::SendPacket () {}

void SendPacket::Data(const void * const data, const size_t size)
{
	if (!buffer.size()) {
		buffer.push_back (0); //  this will finally be the length of the packet
		buffer.push_back (0);
	}
	for (size_t idx = 0; idx < size; ++idx)
		buffer.push_back(static_cast<const uint8_t *>(data)[idx]);
}

void SendPacket::send (TCPsocket sock)
{
	uint32_t length = buffer.size();

	assert (length<0x10000);

	// update packet length
	buffer[0] = length >> 8;
	buffer[1] = length & 0xFF;

	if (sock)
		SDLNet_TCP_Send (sock, &(buffer[0]), buffer.size());
}

void SendPacket::reset ()
{
	buffer.clear();
}


/*** class RecvPacket ***/

RecvPacket::RecvPacket (Deserializer& des)
{
	uint16_t size = des.queue[0] << 8 | des.queue[1];

	// The following should be caught by Deserializer::read and ::avail
	assert(des.queue.size() >= static_cast<size_t>(size));
	assert(size >= 2);

	buffer.insert(buffer.end(), &des.queue[2], &des.queue[size]);
	m_index = 0;

	des.queue.erase(des.queue.begin(), des.queue.begin() + size);
}

size_t RecvPacket::Data(void* const data, const size_t bufsize)
{
	if (m_index+bufsize > buffer.size())
		throw wexception("Packet too short");

	for (size_t read = 0; read < bufsize; ++read)
		static_cast<uint8_t *>(data)[read] = buffer[m_index++];

	return bufsize;
}

bool RecvPacket::EndOfFile() const
{
	return m_index < buffer.size();
}

bool Deserializer::read (TCPsocket sock)
{
	uint8_t buffer[512];
	int32_t bytes;

	bytes = SDLNet_TCP_Recv(sock, buffer, sizeof(buffer));
	if (bytes <= 0)
		return false;

	queue.insert(queue.end(), &buffer[0], &buffer[bytes]);

	if (queue.size() >= 2) {
		uint16_t size = queue[0] << 8 | queue[1];
		if (size < 2)
			return false;
	}

	return true;
}

/**
 * Returns true if an entire packet is available
 */
bool Deserializer::avail() const
{
	if (queue.size() < 2)
		return false;

	uint16_t size = queue[0] << 8 | queue[1];
	if (size < 2)
		return false;

	return queue.size() >= static_cast<size_t>(size);
}


