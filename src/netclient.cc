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

#include "netclient.h"

#include "fullscreen_menu_launchgame.h"
#include "game.h"
#include "game_tips.h"
#include "i18n.h"
#include "interactive_player.h"
#include "interactive_spectator.h"
#include "layered_filesystem.h"
#include "network_protocol.h"
#include "network_system.h"
#include "playercommand.h"
#include "warning.h"
#include "wexception.h"
#include "wlapplication.h"

#include "ui_messagebox.h"
#include "ui_progresswindow.h"


struct NetClientImpl {
	GameSettings settings;
	int32_t playernum; // is -1 as long as not assigned to a position (else 0-based)
	std::string localplayername;

	/// The socket that connects us to the host
	TCPsocket sock;

	/// Socket set used for selection
	SDLNet_SocketSet sockset;

	/// Deserializer acts as a buffer for packets (reassembly/splitting up)
	Deserializer deserializer;

	/// Currently active modal panel. Receives an end_modal on disconncet
	UI::Panel * modal;

	/// Current game. Only non-null if a game is actually running.
	Widelands::Game * game;

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

	/**
	 * The speed desired by the local player.
	 */
	uint32_t desiredspeed;

	/// Backlog of chat messages
	std::vector<ChatMessage> chatmessages;
};

NetClient::NetClient (IPaddress * const svaddr, std::string const & playername)
: d(new NetClientImpl)
{
	d->sock = SDLNet_TCP_Open(svaddr);
	if (d->sock == 0)
		throw warning
			(_("Could not establish connection to host"),
			 _
			 	("Widelands could not establish a connection to the given "
			 	 "adress.\n"
			 	 "Either no Widelands server was running at the supposed port or\n"
			 	 "the server shut down as you tried to connect."));

	d->sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (d->sockset, d->sock);

	d->playernum = -2;          // -2 == not connected
	d->settings.playernum = -2; // ""
	d->settings.usernum = -2; // ""
	d->localplayername = playername;
	d->modal = 0;
	d->game = 0;
	d->realspeed = 0;
	d->desiredspeed = 1000;
}

NetClient::~NetClient ()
{
	if (d->sock != 0)
		disconnect(_("Client has left the game."), true, false);

	SDLNet_FreeSocketSet (d->sockset);

	delete d;
	d = 0;
}

void NetClient::run ()
{
	SendPacket s;
	s.Unsigned8(NETCMD_HELLO);
	s.Unsigned8(NETWORK_PROTOCOL_VERSION);
	s.String(d->localplayername);
	s.send(d->sock);

	d->settings.multiplayer = true;
	setScenario(false); //  FIXME no scenario for multiplayer
	{
		Fullscreen_Menu_LaunchGame lgm(this, this);
		lgm.setChatProvider(this);
		d->modal = &lgm;
		int32_t code = lgm.run();
		d->modal = 0;
		if (code <= 0)
			return;
	}

	d->server_is_waiting = true;

	Widelands::Game game;
	try {
		UI::ProgressWindow loaderUI("pics/progress.png");
		GameTips tips (loaderUI);

		loaderUI.step(_("Preparing game"));

		d->game = &game;
		game.set_game_controller(this);
		uint8_t pn = d->playernum + 1;
		Interactive_GameBase * igb;
		if (pn > 0)
			igb = new Interactive_Player(game, pn, false, true);
		else
			igb = new Interactive_Spectator(&game, true);
		game.set_iabase(igb);
		igb->set_chat_provider(this);
		if (!d->settings.savegame) //  new map
			game.init_newgame(loaderUI, d->settings);
		else // savegame
			game.init_savegame(loaderUI, d->settings);
		d->time.reset(game.get_gametime());
		d->lasttimestamp = game.get_gametime();
		d->lasttimestamp_realtime = WLApplication::get()->get_time();

		d->modal = game.get_iabase();
		game.run
			(loaderUI,
			 d->settings.savegame ?
			 Widelands::Game::Loaded : Widelands::Game::NewNonScenario);
		d->modal = 0;
		d->game = 0;
	} catch (...) {
		d->modal = 0;
		WLApplication::emergency_save(game);
		d->game = 0;
		disconnect(_("Client crashed and performed an emergency save."));
		throw;
	}
}

void NetClient::think()
{
	handle_network();

	if (d->game) {
		if (d->realspeed == 0 || d->server_is_waiting)
			d->time.fastforward();
		else
			d->time.think(d->realspeed);

		if (d->server_is_waiting && d->game->get_gametime() == d->time.networktime()) {
			sendTime();
			d->server_is_waiting = false;
		} else if (d->game->get_gametime() != d->lasttimestamp) {
			int32_t curtime = WLApplication::get()->get_time();
			if (curtime - d->lasttimestamp_realtime > CLIENT_TIMESTAMP_INTERVAL)
				sendTime();
		}
	} else {
		// Just syncing so both are the same. This is only needed,
		// because the launchgamemenu needs d->settings.playernum.
		d->settings.playernum = d->playernum;
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
	snprintf(buf, sizeof(buf), "network player %i", d->playernum);
	return buf;
}

GameSettings const & NetClient::settings()
{
	return d->settings;
}

void NetClient::setScenario(bool set)
{
	d->settings.scenario = set;
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

bool NetClient::canChangePlayerInit(uint8_t)
{
	return false;
}

bool NetClient::canLaunch()
{
	return false;
}

void NetClient::setMap(std::string const &, std::string const &, uint32_t, bool)
{
	// client is not allowed to do this
}

void NetClient::setPlayerState(uint8_t, PlayerSettings::State)
{
	// client is not allowed to do this
}

void NetClient::setPlayerAI(uint8_t, std::string const &)
{
	// client is not allowed to do this
}

void NetClient::nextPlayerState(uint8_t)
{
	// client is not allowed to do this
}

void NetClient::setPlayerTribe(uint8_t number, std::string const & tribe)
{
	if (number != d->playernum)
		return;

	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGETRIBE);
	s.String(tribe);
	s.send(d->sock);
}

void NetClient::setPlayerInit(uint8_t, uint8_t)
{
	//  client is not allowed to do this
}

void NetClient::setPlayerName(uint8_t, std::string const &)
{
	// until now the name is set before joining - if you allow a change in
	// launchgame-menu, here properly should be a set_name function
}

void NetClient::setPlayer(uint8_t, PlayerSettings)
{
	// do nothing here - the request for a positionchange is send in
	// setPlayerNumber(int32_t) to the host.
}

void NetClient::setPlayerNumber(int32_t number)
{
	if (number == d->playernum)
		return;

	// Send request
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGEPOSITION);
	s.Signed32(number);
	s.send(d->sock);
}

uint32_t NetClient::realSpeed()
{
	return d->realspeed;
}

uint32_t NetClient::desiredSpeed()
{
	return d->desiredspeed;
}

void NetClient::setDesiredSpeed(uint32_t speed)
{
	if (speed > std::numeric_limits<uint16_t>::max())
		speed = std::numeric_limits<uint16_t>::max();

	if (speed != d->desiredspeed) {
		d->desiredspeed = speed;

		SendPacket s;
		s.Unsigned8(NETCMD_SETSPEED);
		s.Unsigned16(d->desiredspeed);
		s.send(d->sock);
	}
}


void NetClient::recvOnePlayer
	(uint8_t const number, Widelands::StreamRead & packet)
{
	if (number >= d->settings.players.size())
		throw DisconnectException
			(_("Server sent a player update for a player that does not exist."));

	PlayerSettings & player = d->settings.players[number];
	player.state = static_cast<PlayerSettings::State>(packet.Unsigned8());
	player.name = packet.String();
	player.tribe = packet.String();
	player.initialization_index = packet.Unsigned8();

	if (number == d->playernum)
		d->localplayername = player.name;
}

void NetClient::recvOneUser
	(uint32_t const number, Widelands::StreamRead & packet)
{
	if (number > d->settings.users.size())
		throw DisconnectException
			(_("Server sent an user update for a user that does not exist."));

	// This might happen, if a users connects after the game starts.
	if (number == d->settings.users.size()) {
		UserSettings newuser;
		d->settings.users.push_back(newuser);
	}

	d->settings.users[number].name = packet.String();
	d->settings.users[number].position = packet.Signed32();
	if (number == d->settings.usernum) {
		d->localplayername = d->settings.users[number].name;
		d->settings.playernum = d->settings.users[number].position;
		d->playernum = d->settings.users[number].position;
	}
}

void NetClient::send(std::string const & msg)
{
	SendPacket s;
	s.Unsigned8(NETCMD_CHAT);
	s.String(msg);
	s.send(d->sock);
}

std::vector<ChatMessage> const & NetClient::getMessages() const
{
	return d->chatmessages;
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


/**
 * Handle one packet received from the host.
 *
 * \note The caller must handle exceptions by closing the connection.
 */
void NetClient::handle_packet(RecvPacket & packet)
{
	uint8_t cmd = packet.Unsigned8();

	if (cmd == NETCMD_DISCONNECT) {
		std::string reason = packet.String();
		disconnect(reason, false);
		return;
	}

	if (d->playernum == -2) {
		if (cmd != NETCMD_HELLO)
			throw DisconnectException
				(_
				 	("Expected a HELLO packet from server, but received command "
				 	 "number %u. Maybe the server is running a different version "
				 	 "of Widelands?"),
				 cmd);
		uint8_t version = packet.Unsigned8();
		if (version != NETWORK_PROTOCOL_VERSION)
			throw DisconnectException(_("Server uses a different protocol version"));
		d->settings.usernum = packet.Unsigned32();
		d->playernum = -1;
		return;
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
		d->settings.mapfilename = g_fs->FileSystem::fixCrossFile(packet.String());
		d->settings.savegame = packet.Unsigned8() == 1;
		log("[Client] SETTING_MAP '%s' '%s'\n", d->settings.mapname.c_str(), d->settings.mapfilename.c_str());
		break;

	case NETCMD_SETTING_TRIBES: {
		d->settings.tribes.clear();
		for (uint8_t i = packet.Unsigned8(); i; --i) {
			TribeBasicInfo info;
			info.name = packet.String();
			for (uint8_t j = packet.Unsigned8(); j; --j) {
				std::string const name = packet.String();
				info.initializations.push_back
					(TribeBasicInfo::Initialization(name, name));
			}
			d->settings.tribes.push_back(info);
		}
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
	case NETCMD_SETTING_ALLUSERS: {
		d->settings.users.resize(packet.Unsigned8());
		for (uint32_t i = 0; i < d->settings.users.size(); ++i)
			recvOneUser(i, packet);
		break;
	}
	case NETCMD_SETTING_USER: {
		uint32_t user = packet.Unsigned32();
		recvOneUser(user, packet);
		break;
	}
	case NETCMD_SET_PLAYERNUMBER: {
		int32_t number = packet.Signed32();
		d->playernum = number;
		d->settings.users[d->settings.usernum].position = number;
		d->settings.playernum = number;
		break;
	}

	case NETCMD_LAUNCH: {
		if (!d->modal || d->game)
			throw DisconnectException(_("Unexpectedly received LAUNCH command from server."));
		d->modal->end_modal(1);
		break;
	}
	case NETCMD_SETSPEED:
		d->realspeed = packet.Unsigned16();
		log("[Client] speed: %u.%03u\n", d->realspeed / 1000, d->realspeed % 1000);
		break;
	case NETCMD_TIME:
		d->time.recv(packet.Signed32());
		break;
	case NETCMD_WAIT:
		log("[Client]: server is waiting.\n");
		d->server_is_waiting = true;
		break;
	case NETCMD_PLAYERCOMMAND: {
		if (!d->game)
			throw DisconnectException(_("Server sent a PLAYERCOMMAND even though no game is running."));

		int32_t time = packet.Signed32();
		Widelands::PlayerCommand & plcmd =
			*Widelands::PlayerCommand::deserialize(packet);
		plcmd.set_duetime(time);
		d->game->enqueue_command(&plcmd);
		d->time.recv(time);
		break;
	}
	case NETCMD_SYNCREQUEST: {
		if (!d->game)
			throw DisconnectException(_("Server sent a SYNCREQUEST even though no game is running."));
		int32_t time = packet.Signed32();
		d->time.recv(time);
		d->game->enqueue_command(new Cmd_NetCheckSync(time, this));
		break;
	}
	case NETCMD_CHAT: {
		ChatMessage c;
		c.time = WLApplication::get()->get_time();
		c.playern = packet.Signed16();
		c.sender = packet.String();
		c.msg = packet.String();
		d->chatmessages.push_back(c);
		ChatProvider::send(c); // NoteSender<ChatMessage>
		break;
	}
	case NETCMD_INFO_DESYNC:
		log("[Client] received NETCMD_INFO_DESYNC. Trying to salvage some information for debugging.\n");
		if (d->game)
			d->game->save_syncstream(true);
		break;
	default:
		throw DisconnectException(_("Server sent an unknown command (command number %u)"), cmd);
	}
}


/**
 * Handle all incoming network traffic.
 */
void NetClient::handle_network ()
{
	try {
		while (d->sock != 0 && SDLNet_CheckSockets(d->sockset, 0) > 0) {
			// Perform only one read operation, then process all packets
			// from this read. This ensures that we process DISCONNECT
			// packets that are followed immediately by connection close.
			if (!d->deserializer.read(d->sock)) {
				disconnect("Connection was lost.", false);
				return;
			}

			// Process all the packets from the last read
			while (d->sock && d->deserializer.avail()) {
				RecvPacket packet(d->deserializer);
				handle_packet(packet);
			}
		}
	} catch (DisconnectException const & e) {
		disconnect(e.what());
	} catch (std::exception const & e) {
		std::string reason = _("Server sent malformed commands: ");
		reason += e.what();
		disconnect(reason);
	}
}


void NetClient::disconnect
	(std::string const & reason, bool const sendreason, bool const showmsg)
{
	log("[Client]: disconnect(%s)\n", reason.c_str());

	if (d->sock) {
		if (sendreason) {
			SendPacket s;
			s.Unsigned8(NETCMD_DISCONNECT);
			s.String(reason);
			s.send(d->sock);
		}

		SDLNet_TCP_DelSocket (d->sockset, d->sock);
		SDLNet_TCP_Close (d->sock);
		d->sock = 0;
	}

	bool const trysave = showmsg && d->game;

	if (showmsg) {
		std::string msg = reason;

		if (trysave)
			msg += _(" An automatic savegame will be created.");

		UI::MessageBox mmb
			(d->modal,
			 "Disconnected from Host",
			 msg,
			 UI::MessageBox::OK);
		mmb.run();
	}

	if (trysave)
		WLApplication::emergency_save(*d->game);

	if (d->modal) {
		d->modal->end_modal(0);
		d->modal = 0;
	}
}
