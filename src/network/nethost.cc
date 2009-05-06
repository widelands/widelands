/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#include "nethost.h"

#include "build_info.h"
#include "chat.h"
#include "computer_player.h"
#include "ui/ui_fs_menus/fullscreen_menu_launchgame.h"
#include "game.h"
#include "wui/game_tips.h"
#include "i18n.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"
#include "network_lan_promotion.h"
#include "network_protocol.h"
#include "network_system.h"
#include "player.h"
#include "playercommand.h"
#include "profile/profile.h"
#include "tribe.h"
#include "wexception.h"
#include "wlapplication.h"

#include "ui_basic/progresswindow.h"



struct HostGameSettingsProvider : public GameSettingsProvider {
	HostGameSettingsProvider(NetHost * const _h) : h(_h) {}

	virtual void setScenario(bool) {}; //  FIXME no scenario for multiplayer

	virtual GameSettings const & settings() {return h->settings();}

	virtual bool canChangeMap() {return true;}
	virtual bool canChangePlayerState(uint8_t number) {
		return number != settings().playernum;
	}
	virtual bool canChangePlayerTribe(uint8_t number) {
		if (number == settings().playernum)
			return true;
		if (number >= settings().players.size())
			return false;
		return settings().players[number].state == PlayerSettings::stateComputer;
	}
	virtual bool canChangePlayerInit(uint8_t const number) {
		return number < settings().players.size();
	}

	virtual bool canLaunch() {return h->canLaunch();}

	virtual void setMap
		(std::string const &       mapname,
		 std::string const &       mapfilename,
		 uint32_t            const maxplayers,
		 bool                const savegame = false)
	{
		h->setMap(mapname, mapfilename, maxplayers, savegame);
	}
	virtual void setPlayerState
		(uint8_t const number, PlayerSettings::State const state)
	{
		if (number >= settings().players.size())
			return;

		h->setPlayerState(number, state);
	}
	virtual void nextPlayerState(uint8_t const number) {
		if
			(number == settings().playernum ||
			 number >= settings().players.size())
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

		h->setPlayerState(number, newstate, true);
	}

	virtual void setPlayerTribe(uint8_t const number, std::string const & tribe)
	{
		if (number >= h->settings().players.size())
			return;

		if
			(number == settings().playernum ||
			 settings().players[number].state == PlayerSettings::stateComputer)
			h->setPlayerTribe(number, tribe);
	}

	virtual void setPlayerInit(uint8_t const number, uint8_t const index) {
		if (number >= h->settings().players.size())
			return;

		h->setPlayerInit(number, index);
	}

	virtual void setPlayerAI(uint8_t, std::string const &) {
		// not implemented
	}

	virtual void setPlayerName(uint8_t const number, std::string const & name) {
		if (number >= h->settings().players.size())
			return;
		h->setPlayerName(number, name);
	}

	virtual void setPlayer(uint8_t const number, PlayerSettings const ps) {
		if (number >= h->settings().players.size())
			return;
		h->setPlayer(number, ps);
	}

	virtual void setPlayerNumber(int32_t const number) {
		if (number >= static_cast<int32_t>(h->settings().players.size()))
			return;
		h->setPlayerNumber(number);
	}

private:
	NetHost * h;
};

struct HostChatProvider : public ChatProvider {
	HostChatProvider(NetHost * const _h) : h(_h) {}

	void send(std::string const & msg) {
		ChatMessage c;
		c.time = WLApplication::get()->get_time();
		c.playern = h->getLocalPlayerposition();
		c.sender = h->getLocalPlayername();
		c.msg = msg;
		h->send(c);
	}

	std::vector<ChatMessage> const & getMessages() const {
		return messages;
	}

	void receive(ChatMessage const & msg) {
		messages.push_back(msg);
		ChatProvider::send(msg);
	}

private:
	NetHost                * h;
	std::vector<ChatMessage> messages;
};

struct Client {
	TCPsocket sock;
	Deserializer deserializer;
	int32_t playernum; // -1 as long as the client is not connected to a position
	int32_t usernum;
	std::string build_id;
	md5_checksum syncreport;
	bool syncreport_arrived;
	int32_t time; // last time report
	uint32_t desiredspeed;
};

struct NetHostImpl {
	GameSettings settings;
	std::string localplayername;
	uint32_t localdesiredspeed;
	HostChatProvider chat;

	LAN_Game_Promoter * promoter;
	TCPsocket svsock;
	SDLNet_SocketSet sockset;

	/// List of connected clients. Note that clients are not in the same
	/// order as players. In fact, a client must not be assigned to a player.
	std::vector<Client> clients;

	/// The game itself; only non-null while game is running
	Widelands::Game * game;

	/// If we were to send out a plain networktime packet, this would be the
	/// time. However, we have not yet committed to this networktime.
	int32_t pseudo_networktime;
	int32_t last_heartbeat;

	/// The networktime we committed to by sending it across the network.
	int32_t committed_networktime;

	/// This is the time for local simulation
	NetworkTime time;

	/// Whether we're waiting for all clients to report back.
	bool waiting;
	int32_t lastframe;

	/**
	 * The speed, in milliseconds per second, that is effective as long
	 * as we're not \ref waiting.
	 */
	uint32_t networkspeed;

	/// All currently running computer players, *NOT* in one-one correspondence
	/// with \ref Player objects
	std::vector<Computer_Player *> computerplayers;

	/// \c true if a syncreport is currently in flight
	bool syncreport_pending;
	int32_t syncreport_time;
	md5_checksum syncreport;
	bool syncreport_arrived;

	NetHostImpl(NetHost * const h) : chat(h) {}
};

NetHost::NetHost (std::string const & playername)
: d(new NetHostImpl(this))
{
	log("[Host] starting up.\n");

	d->localplayername = playername;

	// create a listening socket
	IPaddress myaddr;
	SDLNet_ResolveHost (&myaddr, 0, WIDELANDS_PORT);
	d->svsock = SDLNet_TCP_Open(&myaddr);

	d->sockset = SDLNet_AllocSocketSet(16);
	d->promoter = new LAN_Game_Promoter();
	d->game = 0;
	d->pseudo_networktime = 0;
	d->waiting = true;
	d->networkspeed = 1000;
	d->localdesiredspeed = 1000;
	d->syncreport_pending = false;
	d->syncreport_time = 0;

	Widelands::Tribe_Descr::get_all_tribe_infos(d->settings.tribes);
	setMultiplayerGameSettings();
	d->settings.playernum = -1;
	d->settings.usernum = 0;
	UserSettings hostuser;
	hostuser.name = playername;
	hostuser.position = -1;
	d->settings.users.push_back(hostuser);
}

NetHost::~NetHost ()
{
	clearComputerPlayers();

	while (d->clients.size() > 0) {
		disconnectClient(0, _("Server has left the game."));
		reaper();
	}

	SDLNet_FreeSocketSet (d->sockset);

	// close all open sockets
	if (d->svsock != 0)
		SDLNet_TCP_Close (d->svsock);

	delete d;
	d = 0;
}

std::string const & NetHost::getLocalPlayername() const
{
	return d->localplayername;
}

int16_t NetHost::getLocalPlayerposition()
{
	return d->settings.users[0].position;
}

void NetHost::clearComputerPlayers()
{
	for (uint32_t i = 0; i < d->computerplayers.size(); ++i)
		delete d->computerplayers[i];
	d->computerplayers.clear();
}

void NetHost::initComputerPlayer(Widelands::Player_Number p)
{
	d->computerplayers.push_back
		(Computer_Player::getImplementation(d->game->get_player(p)->getAI())
		 ->instantiate(*d->game, p));
}

void NetHost::initComputerPlayers()
{
	const Widelands::Player_Number nr_players = d->game->map().get_nrplayers();
	iterate_players_existing(p, nr_players, *d->game, plr) {
		if (p == d->settings.playernum + 1)
			continue;

		uint32_t client;
		for (client = 0; client < d->clients.size(); ++client)
			if (d->clients[client].playernum + 1 == p)
				break;

		if (client >= d->clients.size())
			initComputerPlayer(p);
	}
}

void NetHost::run()
{
	HostGameSettingsProvider hp(this);
	{
		Fullscreen_Menu_LaunchGame lgm(&hp, this);
		lgm.setChatProvider(d->chat);
		const int32_t code = lgm.run();

		if (code <= 0)
			return;
	}

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients[i].playernum == -2)
			disconnectClient
				(i, _("The game has started just after you tried to connect."));
	}

	SendPacket s;
	s.Unsigned8(NETCMD_LAUNCH);
	broadcast(s);

	Widelands::Game game;
	try {
		UI::ProgressWindow loaderUI("pics/progress.png");
		std::vector<std::string> tipstext;
		tipstext.push_back("general_game");
		tipstext.push_back("multiplayer");
		std::string tribe = hp.getPlayersTribe();
		if (!tribe.empty())
			tipstext.push_back(tribe);
		GameTips tips (loaderUI, tipstext);

		loaderUI.step(_("Preparing game"));

		uint8_t const pn = d->settings.playernum + 1;
		d->game = &game;
		game.set_game_controller(this);
		Interactive_GameBase * igb;
		if (pn > 0)
			igb =
				new Interactive_Player
					(game, g_options.pull_section("global"), pn, false, true);
		else
			igb =
				new Interactive_Spectator
					(*d->game, g_options.pull_section("global"), true);
		igb->set_chat_provider(d->chat);
		game.set_ibase(igb);
		if (!d->settings.savegame) //  new game
			game.init_newgame(loaderUI, d->settings);
		else // savegame
			game.init_savegame(loaderUI, d->settings);
		d->pseudo_networktime = game.get_gametime();
		d->time.reset(d->pseudo_networktime);
		d->lastframe = WLApplication::get()->get_time();
		d->last_heartbeat = d->lastframe;

		d->committed_networktime = d->pseudo_networktime;

		for (uint32_t i = 0; i < d->clients.size(); ++i)
			d->clients[i].time = d->committed_networktime - 1;

		// The call to checkHungClients ensures that the game leaves the
		// wait mode when there are no clients
		checkHungClients();
		initComputerPlayers();
		game.run
			(loaderUI,
			 d->settings.savegame ?
			 Widelands::Game::Loaded : Widelands::Game::NewNonScenario);
		clearComputerPlayers();
	} catch (...) {
		WLApplication::emergency_save(game);
		clearComputerPlayers();
		d->game = 0;

		while (d->clients.size() > 0) {
			disconnectClient
				(0, _("Server has crashed and performed an emergency save."));
			reaper();
		}
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

		if (!d->waiting) {
			int32_t diff = (delta * d->networkspeed) / 1000;
			d->pseudo_networktime += diff;
		}

		d->time.think(realSpeed()); // must be called even when d->waiting

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

void NetHost::sendPlayerCommand(Widelands::PlayerCommand & pc)
{
	pc.set_duetime(d->committed_networktime + 1);

	SendPacket s;
	s.Unsigned8(NETCMD_PLAYERCOMMAND);
	s.Signed32(pc.duetime());
	pc.serialize(s);
	broadcast(s);
	d->game->enqueue_command(&pc);

	committedNetworkTime(d->committed_networktime + 1);
}

/**
 * All chat messages go through this function.
 * The message is sent to clients as needed, and it is forwarded
 * to our local \ref ChatProvider.
 */
void NetHost::send(ChatMessage msg)
{
	if (msg.msg.size() == 0)
		return;

	// Make sure that msg is free of richtext formation tags. Such tags could not
	// just be abused by the user, but could also break the whole text formation.
	for (uint32_t i = 0; i < msg.msg.size(); ++i)
		if (msg.msg.substr(i, 1) == "<")
			msg.msg.replace(i, 1, "{");

	SendPacket s;
	s.Unsigned8(NETCMD_CHAT);
	s.Signed16(msg.playern);
	s.String(msg.sender);
	s.String(msg.msg);
	broadcast(s);

	d->chat.receive(msg);

	log("[Host]: chat: %s\n", msg.toPlainString().c_str());
}

void NetHost::sendSystemChat(char const * const fmt, ...)
{
	char buffer[500];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	ChatMessage c;
	c.time = WLApplication::get()->get_time();
	c.msg = buffer;
	c.playern = -2; // == System message
	// c.sender remains empty to indicate a system message
	send(c);
}

int32_t NetHost::getFrametime()
{
	return d->time.time() - d->game->get_gametime();
}

std::string NetHost::getGameDescription()
{
	char buf[200];
	snprintf
		(buf, sizeof(buf),
		 "network player %i (host)", d->settings.users[0].position);
	return buf;
}

GameSettings const & NetHost::settings()
{
	return d->settings;
}

bool NetHost::canLaunch()
{
	return d->settings.mapname.size() != 0 && d->settings.players.size() >= 1;
}

void NetHost::setMap
	(std::string const &       mapname,
	 std::string const &       mapfilename,
	 uint32_t            const maxplayers,
	 bool                const savegame)
{
	d->settings.mapname = mapname;
	d->settings.mapfilename = mapfilename;
	d->settings.savegame = savegame;

	uint32_t oldplayers = d->settings.players.size();

	SendPacket s;

	while (oldplayers > maxplayers) {
		--oldplayers;
		for (uint32_t i = 1; i < d->settings.users.size(); ++i) {
			if (d->settings.users[i].position == oldplayers) {
				d->settings.users[i].position = -1;

				// for local settings
				uint32_t j = 0;
				for (; j < d->clients.size(); ++j)
					if (d->clients[j].usernum == static_cast<int32_t>(i))
						break;
				d->clients[j].playernum = -1;

				// Broadcast change
				s.reset();
				s.Unsigned8(NETCMD_SETTING_USER);
				s.Unsigned32(i);
				writeSettingUser(s, i);
				broadcast(s);
			}
		}
	}

	d->settings.players.resize(maxplayers);

	while (oldplayers < maxplayers) {
		PlayerSettings & player = d->settings.players[oldplayers];
		player.state                = PlayerSettings::stateOpen;
		player.tribe                = d->settings.tribes[0].name;
		player.initialization_index = 0;
		++oldplayers;
	}

	// Broadcast new map info
	s.reset();
	s.Unsigned8(NETCMD_SETTING_MAP);
	writeSettingMap(s);
	broadcast(s);

	// Broadcast new player settings
	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLPLAYERS);
	writeSettingAllPlayers(s);
	broadcast(s);
}

void NetHost::setPlayerState
	(uint8_t const number, PlayerSettings::State const state, bool const host)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players[number];

	if (player.state == state)
		return;

	SendPacket s;

	if (player.state == PlayerSettings::stateHuman) {
		uint32_t i = 1; //  0 is host an host has no client -> segfault.
		for (; i < d->settings.users.size(); ++i) {
			if (d->settings.users[i].position == number)
				break;
		}
		if (i < d->settings.users.size()) {
			d->settings.users[i].position = -1;
			if (host) // did host player send the user to lobby?
				sendSystemChat
					(_("Host sent player %s to the lobby!"),
					 d->settings.users[i].name.c_str());

			// for local settings
			uint32_t j = 0;
			for (; j < d->clients.size(); ++j)
				if (d->clients[j].usernum == static_cast<int32_t>(i))
					break;
			Client & client = d->clients[j];
			client.playernum = -1;

			// Broadcast change
			s.Unsigned8(NETCMD_SETTING_USER);
			s.Unsigned32(i);
			writeSettingUser(s, i);
			broadcast(s);
		}
	}

	player.state = state;

	if (player.state == PlayerSettings::stateComputer)
		player.name = getComputerPlayerName(number);

	// Broadcast change
	s.reset();
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


void NetHost::setPlayerTribe(uint8_t const number, std::string const & tribe)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players[number];

	if (player.tribe == tribe)
		return;

	container_iterate_const(std::vector<TribeBasicInfo>, d->settings.tribes, i)
		if (i.current->name == player.tribe) {
			player.tribe = tribe;
			if (i.current->initializations.size() <= player.initialization_index)
				player.initialization_index = 0;

			//  broadcast changes
			SendPacket s;
			s.Unsigned8(NETCMD_SETTING_PLAYER);
			s.Unsigned8(number);
			writeSettingPlayer(s, number);
			broadcast(s);
			return;
		}
	log
		("Player %u attempted to change to tribe %s; not a valid tribe\n",
		 number, tribe.c_str());
}

void NetHost::setPlayerInit(uint8_t const number, uint8_t const index)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players[number];

	if (player.initialization_index == index)
		return;

	container_iterate_const(std::vector<TribeBasicInfo>, d->settings.tribes, i)
		if (i.current->name == player.tribe) {
			if (index < i.current->initializations.size()) {
				player.initialization_index = index;

				//  broadcast changes
				SendPacket s;
				s.Unsigned8(NETCMD_SETTING_PLAYER);
				s.Unsigned8(number);
				writeSettingPlayer(s, number);
				broadcast(s);
				return;
			} else
				log
					("Attempted to change to out-of-range initialization index %u "
					 "for player %u.\n", index, number);
			return;
		}
	assert(false);
}


void NetHost::setPlayerName(uint8_t const number, std::string const & name)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players[number];

	if (player.name == name)
		return;

	player.name = name;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}


void NetHost::setPlayer(uint8_t const number, PlayerSettings const ps)
{
	if (number >= d->settings.players.size())
		return;

	PlayerSettings & player = d->settings.players[number];
	player = ps;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	writeSettingPlayer(s, number);
	broadcast(s);
}

void NetHost::setPlayerNumber(int32_t const number)
{
	if (number >= static_cast<int32_t>(d->settings.players.size()))
		return;

	d->settings.playernum = number;
	d->settings.users[0].position = number;

	// Broadcast changes
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_USER);
	s.Unsigned32(0);
	writeSettingUser(s, 0);
	broadcast(s);
}

void NetHost::setMultiplayerGameSettings()
{
	d->settings.scenario = false;
	d->settings.multiplayer = true;
}


uint32_t NetHost::realSpeed()
{
	if (d->waiting)
		return 0;
	return d->networkspeed;
}

uint32_t NetHost::desiredSpeed()
{
	return d->localdesiredspeed;
}

void NetHost::setDesiredSpeed(uint32_t const speed)
{
	if (speed != d->localdesiredspeed) {
		d->localdesiredspeed = speed;
		updateNetworkSpeed();
	}
}


// Send the packet to all properly connected clients
void NetHost::broadcast(SendPacket & packet)
{
	container_iterate_const(std::vector<Client>, d->clients, i)
		if (i.current->playernum != -2)
			packet.send(i.current->sock);
}

void NetHost::writeSettingMap(SendPacket & packet)
{
	packet.String(d->settings.mapname);
	packet.String(d->settings.mapfilename);
	packet.Unsigned8(d->settings.savegame ? 1 : 0);
}

void NetHost::writeSettingPlayer(SendPacket & packet, uint8_t const number)
{
	PlayerSettings & player = d->settings.players[number];
	packet.Unsigned8(static_cast<uint8_t>(player.state));
	packet.String(player.name);
	packet.String(player.tribe);
	packet.Unsigned8(player.initialization_index);
}

void NetHost::writeSettingAllPlayers(SendPacket & packet)
{
	packet.Unsigned8(d->settings.players.size());
	for (uint8_t i = 0; i < d->settings.players.size(); ++i)
		writeSettingPlayer(packet, i);
}

void NetHost::writeSettingUser(SendPacket & packet, uint32_t const number)
{
	packet.String(d->settings.users[number].name);
	packet.Signed32(d->settings.users[number].position);
}

void NetHost::writeSettingAllUsers(SendPacket & packet)
{
	packet.Unsigned8(d->settings.users.size());
	for (uint32_t i = 0; i < d->settings.users.size(); ++i)
		writeSettingUser(packet, i);
}

/**
 *
 * \return a name for the given player.
 */
std::string NetHost::getComputerPlayerName(uint32_t const playernum)
{
	std::string name;
	uint32_t suffix = playernum + 1;
	do {
		char buf[200];
		snprintf(buf, sizeof(buf), "%s %u", _("Computer"), suffix++);
		name = buf;
	} while (haveUserName(name, playernum));
	return name;
}


/**
 * Checks whether a user with the given name exists already.
 *
 * If \p ignoreplayer is non-negative, the user with this number will
 * be ignored.
 */
bool NetHost::haveUserName(std::string const & name, int32_t ignoreplayer) {
	for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
		if (static_cast<int32_t>(i) != ignoreplayer) {
			UserSettings const & user = d->settings.users[i];
			if (user.name == name)
				return true;
		}
	}

	// Computer players are not handled like human users,
	// so make sure no cp owns this name.
	if
		(ignoreplayer <= static_cast<int32_t>(d->settings.users.size())
		 && ignoreplayer >= 0)
		ignoreplayer = d->settings.users[ignoreplayer].position;
	for (uint32_t i = 0; i < d->settings.players.size(); ++i) {
		if (static_cast<int32_t>(i) != ignoreplayer) {
			PlayerSettings const & player = d->settings.players[i];
			if (player.name == name)
				return true;
		}
	}


	return false;
}


/// Respond to a client's Hello message.
void NetHost::welcomeClient
	(uint32_t const number, std::string const & playername)
{
	assert(number < d->clients.size());

	Client & client = d->clients[number];

	assert(client.playernum == -2);
	assert(client.sock);

	// The client gets its own initial data set.
	client.playernum = -1;
	client.usernum = d->settings.users.size();
	UserSettings newuser;
	d->settings.users.push_back(newuser);

	// Assign the player a name, preferably the name chosen by the client
	std::string effective_name = playername;

	if (effective_name.size() == 0)
		effective_name = _("Player");

	if (haveUserName(effective_name, client.usernum)) {
		uint32_t i = 2;
		do {
			char buf[32];
			snprintf(buf, sizeof(buf), " %u", i++);
			effective_name = playername + buf;
		} while (haveUserName(effective_name, client.usernum));
	}

	d->settings.users[client.usernum].name = effective_name;
	d->settings.users[client.usernum].position = -1;

	log("[Host]: client %u: welcome to usernum %u\n", number, client.usernum);

	SendPacket s;
	s.Unsigned8(NETCMD_HELLO);
	s.Unsigned8(NETWORK_PROTOCOL_VERSION);
	s.Unsigned32(client.usernum);
	s.send(client.sock);

	// even if the network protocol is the same, the data might be different.
	if (client.build_id != build_id())
		sendSystemChat
			(_("WARNING: %s uses version: %s, while Host uses version: %s"),
			 effective_name.c_str(), client.build_id.c_str(), build_id().c_str());

	s.reset();
	s.Unsigned8(NETCMD_SETTING_MAP);
	writeSettingMap(s);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_TRIBES);
	s.Unsigned8(d->settings.tribes.size());
	for (uint8_t i = 0; i < d->settings.tribes.size(); ++i) {
		s.String(d->settings.tribes[i].name);
		s.Unsigned8(d->settings.tribes[i].initializations.size());
		for (uint8_t j = 0; j < d->settings.tribes[i].initializations.size(); ++j)
			s.String(d->settings.tribes[i].initializations[j].first);
	}
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLPLAYERS);
	writeSettingAllPlayers(s);
	s.send(client.sock);

	s.reset();
	s.Unsigned8(NETCMD_SETTING_ALLUSERS);
	writeSettingAllUsers(s);
	s.send(client.sock);

	// Broadcast new information about the player to everybody
	s.reset();
	s.Unsigned8(NETCMD_SETTING_USER);
	s.Unsigned32(client.usernum);
	writeSettingUser(s, client.usernum);
	broadcast(s);

	sendSystemChat(_("%s has joined the game"), effective_name.c_str());
}

void NetHost::committedNetworkTime(int32_t const time)
{
	assert(time - d->committed_networktime > 0);

	d->committed_networktime = time;
	d->time.recv(time);

	if
		(!d->syncreport_pending &&
		 d->committed_networktime - d->syncreport_time >= SYNCREPORT_INTERVAL)
		requestSyncReports();
}

void NetHost::recvClientTime(uint32_t const number, int32_t const time)
{
	assert(number < d->clients.size());

	Client & client = d->clients[number];

	if (time - client.time < 0)
		throw DisconnectException
			(_("Client reports time to host that is running backwards."));
	if (d->committed_networktime - time < 0)
		throw DisconnectException
			(_("Client simulates beyond the game time allowed by the host."));
	if (d->syncreport_pending && !client.syncreport_arrived) {
		if (time - d->syncreport_time > 0)
			throw DisconnectException
				(_("Client did not submit sync report in time."));
	}

	client.time = time;
	log("[Host]: Client %i: Time %i\n", number, time);

	if (d->waiting) {
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
		if (d->clients[i].playernum == -2)
			continue;

		int32_t delta = d->committed_networktime - d->clients[i].time;

		if (delta == 0)
			++nrready;
		else {
			++nrdelayed;
			if
				(delta
				 >
				 (5                         *
				  CLIENT_TIMESTAMP_INTERVAL *
				  static_cast<int32_t>(d->networkspeed))
				 /
				 1000)
			{
				log("[Host]: Client %i hung\n", i);
				++nrhung;
			}
		}
	}

	if (!d->waiting) {
		if (nrhung) {
			log("[Host]: %i clients hung. Entering wait mode\n", nrhung);

			// Brake and wait
			d->waiting = true;
			broadcastRealSpeed(0);

			SendPacket s;
			s.Unsigned8(NETCMD_WAIT);
			broadcast(s);
		}
	} else {
		if (nrdelayed == 0) {
			d->waiting = false;
			broadcastRealSpeed(d->networkspeed);
			if (!d->syncreport_pending)
				requestSyncReports();
		}
	}
}


void NetHost::broadcastRealSpeed(uint32_t const speed)
{
	assert(speed <= std::numeric_limits<uint16_t>::max());

	SendPacket s;
	s.Unsigned8(NETCMD_SETSPEED);
	s.Unsigned16(speed);
	broadcast(s);
}


/**
 * This is the algorithm that decides upon the effective network speed,
 * given the desired speed of all clients.
 *
 * This function is supposed to be the only code that ever changes
 * \ref NetHostImpl::networkspeed.
 *
 * The current implementation picks the median, or the average of
 * lower and upper median.
 */
void NetHost::updateNetworkSpeed()
{
	uint32_t oldnetworkspeed = d->networkspeed;
	std::vector<uint32_t> speeds;

	speeds.push_back(d->localdesiredspeed);
	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients[i].playernum >= 0)
			speeds.push_back(d->clients[i].desiredspeed);
	}
	std::sort(speeds.begin(), speeds.end());

	d->networkspeed =
		speeds.size() % 2 ? speeds[speeds.size() / 2] :
		(speeds[speeds.size() / 2] + speeds[(speeds.size() / 2) - 1]) / 2;

	if (d->networkspeed > std::numeric_limits<uint16_t>::max())
		d->networkspeed = std::numeric_limits<uint16_t>::max();

	if (d->networkspeed != oldnetworkspeed && !d->waiting)
		broadcastRealSpeed(d->networkspeed);
}


/**
 * Request sync reports from all clients at the next possible time.
 */
void NetHost::requestSyncReports()
{
	assert(!d->syncreport_pending);

	d->syncreport_pending = true;
	d->syncreport_arrived = false;
	d->syncreport_time = d->committed_networktime + 1;

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
		if (d->clients[i].playernum != -2 && !d->clients[i].syncreport_arrived)
			return;
	}

	d->syncreport_pending = false;
	log("[Host]: comparing syncreports for time %i\n", d->syncreport_time);

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		Client & client = d->clients[i];
		if (client.playernum == -2)
			continue;

		if (client.syncreport != d->syncreport) {
			log
				("[Host] lost synchronization with client %u!\n"
				 "I have:     %s\n"
				 "Client has: %s\n",
				 i, d->syncreport.str().c_str(), client.syncreport.str().c_str());

			d->game->save_syncstream(true);

			SendPacket s;
			s.Unsigned8(NETCMD_INFO_DESYNC);
			broadcast(s);

			disconnectClient(i, _("Client and host have become desynchronized."));
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

	// Check for new connections.
	while (d->svsock != 0 && (sock = SDLNet_TCP_Accept(d->svsock)) != 0) {
		log("[Host] Received a connection request\n");

		SDLNet_TCP_AddSocket (d->sockset, sock);

		Client peer;
		peer.sock = sock;
		peer.playernum = -2;
		peer.syncreport_arrived = false;
		peer.desiredspeed = 1000;
		d->clients.push_back(peer);

		// Now we wait for the client to say Hi in the right language,
		// unless the game has already started
		if (d->game) {
			// the following lines are needed to avoid segfaults in
			// disconnectClient
			UserSettings newuser;
			newuser.name = _("New User"); // shown in later disconnect msg.
			d->clients[d->clients.size() - 1].usernum = d->settings.users.size();
			d->settings.users.push_back(newuser);

			disconnectClient
				(d->clients.size() - 1, _("The game has already started."));
		}
	}

	// Check if we hear anything from our clients
	while (SDLNet_CheckSockets(d->sockset, 0) > 0) {
		for (size_t i = 0; i < d->clients.size(); ++i) {
			try {
				Client & client = d->clients[i];

				while (client.sock && SDLNet_SocketReady(client.sock)) {
					if (!client.deserializer.read(client.sock)) {
						disconnectClient(i, _("Connection to client lost."), false);
						break;
					}

					//  Handle all available packets immediately after each read, so
					//  that we do not miss any commands (especially DISCONNECT...).
					while (client.sock && client.deserializer.avail()) {
						RecvPacket r(client.deserializer);
						handle_packet(i, r);
					}
				}
			} catch (DisconnectException const & e) {
				disconnectClient(i, e.what());
			} catch (std::exception const & e) {
				std::string reason = _("Client sent malformed commands: ");
				reason += e.what();
				disconnectClient(i, reason);
			}
		}
	}

	reaper();
}


/**
 * Handle a single received packet.
 *
 * The caller must catch exceptions and disconnect the client as appropriate.
 *
 * \param i the client number
 * \param r the received packet
 */
void NetHost::handle_packet(uint32_t const i, RecvPacket & r)
{
	Client & client = d->clients[i];
	uint8_t const cmd = r.Unsigned8();

	if (cmd == NETCMD_DISCONNECT) {
		std::string reason = r.String();
		disconnectClient(i, reason, false);
		return;
	}

	if (client.playernum == -2) {
		if (d->game)
			throw DisconnectException
				(_("Game is running already, but client has not connected fully"));
		if (cmd != NETCMD_HELLO)
			throw DisconnectException
				(_
				 	("First command sent by client is %u instead of HELLO. "
				 	 "Most likely the client is running an incompatible version."),
				 cmd);
		uint8_t version = r.Unsigned8();
		if (version != NETWORK_PROTOCOL_VERSION)
			throw DisconnectException
				(_("Server uses a different protocol version."));

		std::string playername = r.String();
		client.build_id = r.String();

		welcomeClient(i, playername);
		return;
	}

	switch (cmd) {
	case NETCMD_PONG:
		log("[Host] client %i: got pong\n", i);
		break;

	case NETCMD_SETTING_CHANGETRIBE:
		//  Do not be harsh about packets of this type arriving out of order -
		//  the client might just have had bad luck with the timing.
		if (!d->game) {
			std::string tribe = r.String();
			setPlayerTribe(client.playernum, tribe);
		}
		break;

	case NETCMD_SETTING_CHANGEPOSITION:
		if (!d->game) {
			int32_t pos = r.Signed32();
			if (pos == -1) {
				setPlayerState(client.playernum, PlayerSettings::stateOpen);
				log("[Host] client %i went to lobby.\n", i);
			} else if (client.playernum == -1) {
				int8_t maxplayers = d->settings.players.size();
				if (pos >= maxplayers)
					break;
				PlayerSettings position = d->settings.players[pos];
				if (position.state == PlayerSettings::stateOpen) {
					setPlayerState(pos, PlayerSettings::stateHuman);
					setPlayerName(pos, d->settings.users[client.usernum].name);
					log("[Host] client %i switched to position %i.\n", i, pos);
				}
			} else {
				PlayerSettings position = d->settings.players[pos];
				PlayerSettings player   = d->settings.players[client.playernum];
				int8_t maxplayers = d->settings.players.size();
				if
					((pos < maxplayers) &
					 (position.state == PlayerSettings::stateOpen))
				{
					const PlayerSettings oldOnPos = position;
					setPlayer(pos, player);
					setPlayer(client.playernum, oldOnPos);
					log("[Host] client %i switched to position %i.\n", i, pos);
				} else
					break;
			}
			// local settings
			d->settings.users[client.usernum].position = pos;
			client.playernum = pos;

			// Send new user information
			SendPacket s;
			s.Unsigned8(NETCMD_SET_PLAYERNUMBER);
			s.Signed32(pos);
			s.send(client.sock);

			// Broadcast new information about the User to everybody
			s.reset();
			s.Unsigned8(NETCMD_SETTING_USER);
			s.Unsigned32(client.usernum);
			writeSettingUser(s, client.usernum);
			broadcast(s);
		}
		break;

	case NETCMD_TIME:
		if (!d->game)
			throw DisconnectException
				(_("Client sent TIME command even though game is not running."));
		recvClientTime(i, r.Signed32());
		break;

	case NETCMD_PLAYERCOMMAND: {
		if (!d->game)
			throw DisconnectException
				(_
				 	("Client sent PLAYERCOMMAND command even though game is not "
				 	 "running."));
		int32_t time = r.Signed32();
		Widelands::PlayerCommand & plcmd =
			*Widelands::PlayerCommand::deserialize(r);
		log
			("[Host] client %i (%i) sent player command %i for %i, time = %i\n",
			 i, client.playernum, plcmd.id(), plcmd.sender(), time);
		recvClientTime(i, time);
		if (plcmd.sender() != client.playernum + 1)
			throw DisconnectException
				(_
				 	("Client %i (%i) sent a playercommand (%i) for a different "
				 	 "player (%i)."),
				 i, client.playernum, plcmd.id(), plcmd.sender());
		sendPlayerCommand(plcmd);
	} break;

	case NETCMD_SYNCREPORT: {
		if (!d->game || !d->syncreport_pending || client.syncreport_arrived)
			throw DisconnectException
				(_("Client sent unexpected synchronization report."));
		int32_t time = r.Signed32();
		r.Data(client.syncreport.data, 16);
		client.syncreport_arrived = true;
		recvClientTime(i, time);
		checkSyncReports();
		break;
	}

	case NETCMD_CHAT: {
		ChatMessage c;
		c.time = WLApplication::get()->get_time();
		c.playern = d->settings.users[client.usernum].position;
		c.sender = d->settings.users[client.usernum].name;
		c.msg = r.String();
		send(c);
		break;
	}

	case NETCMD_SETSPEED: {
		client.desiredspeed = r.Unsigned16();
		updateNetworkSpeed();
		break;
	}

	default:
		throw DisconnectException
			(_("Client sent unknown command number %u"), cmd);
	}
}


void NetHost::disconnectPlayer
	(uint8_t const number, std::string const & reason, bool const sendreason)
{
	log("[Host]: disconnectPlayer(%u, %s)\n", number, reason.c_str());

	bool needai = false;

	for (uint32_t index = 0; index < d->clients.size(); ++index) {
		Client & client = d->clients[index];
		if (client.playernum != static_cast<int32_t>(number))
			continue;

		client.playernum = -2;
		disconnectClient(index, reason, sendreason);

		setPlayerState(number, PlayerSettings::stateOpen);
		needai = true;
	}

	if (needai && d->game)
		initComputerPlayer(number + 1);
}

void NetHost::disconnectClient
	(uint32_t const number, std::string const & reason, bool const sendreason)
{
	assert(number < d->clients.size());

	Client & client = d->clients[number];
	if (client.playernum >= 0) {
		disconnectPlayer(client.playernum, reason, sendreason);
		// disconnectPlayer calls us recursively
		return;
	}

	sendSystemChat
		(_("%s has left the game (%s)"),
		 d->settings.users[client.usernum].name.c_str(),
		 reason.c_str());

	d->settings.users[client.usernum].name = std::string();
	d->settings.users[client.usernum].position = -2;
	client.playernum = -2; // needed as client.playernum = -1 leads to crashes

	// Broadcast the user changes to everybody
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_USER);
	s.Unsigned32(client.usernum);
	writeSettingUser(s, client.usernum);
	broadcast(s);

	log("[Host]: disconnectClient(%u, %s)\n", number, reason.c_str());

	if (client.sock) {
		if (sendreason) {
			s.reset();
			s.Unsigned8(NETCMD_DISCONNECT);
			s.String(reason);
			s.send(client.sock);
		}

		SDLNet_TCP_DelSocket (d->sockset, client.sock);
		SDLNet_TCP_Close (client.sock);
		client.sock = 0;
	}

	if (d->game)
		checkHungClients();
}

/**
 * The grim reaper. This finally erases disconnected clients from the clients
 * array.
 *
 * Calls this when you're certain that nobody is holding any client indices or
 * iterators, since this function will invalidate them.
 */
void NetHost::reaper()
{
	uint32_t index = 0;
	while (index < d->clients.size())
		if (d->clients[index].sock)
			++index;
		else
			d->clients.erase(d->clients.begin() + index);
}
