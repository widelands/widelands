/*
 * Copyright (C) 2008 by the Widelands Development Team
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
#include "network_protocol.h"
#include "network_system.h"
#include "playercommand.h"
#include "wexception.h"
#include "wlapplication.h"

#include "ui_progresswindow.h"


struct NetClientImpl {
	GameSettings settings;
	int32_t playernum; // is -1 until we are assigned our number, then it's 0-based
	std::string localplayername;

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

NetClient::NetClient (IPaddress* svaddr, const std::string& playername)
: d(new NetClientImpl)
{
	d->sock = SDLNet_TCP_Open(svaddr);
	if (d->sock == 0)
		throw wexception("[Client] SDLNet_TCP_Open failed: %s", SDLNet_GetError());

	d->sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (d->sockset, d->sock);

	d->playernum = -1;
	d->localplayername = playername;
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
	s.Unsigned8(NETWORK_PROTOCOL_VERSION);
	s.String(d->localplayername);
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

	if (number == d->playernum)
		d->localplayername = player.name;
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
			if (version != NETWORK_PROTOCOL_VERSION) {
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

