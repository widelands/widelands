/*
 * Copyright (C) 2008-2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "network/netclient.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <config.h>

#include "build_info.h"
#include "game_io/game_loader.h"
#include "helper.h"
#include "i18n.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "map_io/widelands_map_loader.h"
#include "network/internet_gaming.h"
#include "network/network_gaming_messages.h"
#include "network/network_protocol.h"
#include "network/network_system.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/launchMPG.h"
#include "warning.h"
#include "wexception.h"
#include "wlapplication.h"
#include "wui/game_tips.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"


struct NetClientImpl {
	GameSettings settings;

	std::string localplayername;

	/// The socket that connects us to the host
	TCPsocket sock;

	/// Socket set used for selection
	SDLNet_SocketSet sockset;

	/// Deserializer acts as a buffer for packets (reassembly/splitting up)
	Deserializer deserializer;

	/// Currently active modal panel. Receives an end_modal on disconnect
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

NetClient::NetClient
	(IPaddress * const svaddr, const std::string & playername, bool internet)
: d(new NetClientImpl), m_internet(internet), m_dedicated_access(false), m_dedicated_temp_scenario(false)
{
	d->sock = SDLNet_TCP_Open(svaddr);
	if (d->sock == nullptr)
		throw warning
			(_("Could not establish connection to host"),
			 _
			 	("Widelands could not establish a connection to the given "
			 	 "address.\n"
			 	 "Either no Widelands server was running at the supposed port or\n"
			 	 "the server shut down as you tried to connect."));

	d->sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (d->sockset, d->sock);

	d->settings.playernum = UserSettings::notConnected();
	d->settings.usernum = -2;
	d->localplayername = playername;
	d->modal = nullptr;
	d->game = nullptr;
	d->realspeed = 0;
	d->desiredspeed = 1000;
	file = nullptr;

	// Temporarily register win condition scripts to get the default
	std::set<std::string> win_condition_scripts =
	   filter(g_fs->ListDirectory("scripting/win_conditions"),
	          [](const std::string& fn) {return boost::ends_with(fn, ".lua");});
	assert(win_condition_scripts.size());
	d->settings.win_condition_script = *win_condition_scripts.begin();
}

NetClient::~NetClient ()
{
	if (d->sock != nullptr)
		disconnect("CLIENT_LEFT_GAME", "", true, false);

	SDLNet_FreeSocketSet (d->sockset);

	delete d;
}

void NetClient::run ()
{
	SendPacket s;
	s.Unsigned8(NETCMD_HELLO);
	s.Unsigned8(NETWORK_PROTOCOL_VERSION);
	s.String(d->localplayername);
	s.String(build_id());
	s.send(d->sock);

	d->settings.multiplayer = true;

	// Fill the list of possible system messages
	NetworkGamingMessages::fill_map();
	{
		Fullscreen_Menu_LaunchMPG lgm(this, this);
		lgm.setChatProvider(*this);
		d->modal = &lgm;
		int32_t code = lgm.run();
		d->modal = nullptr;
		if (code == 1) { // Only possible if server is dedicated - client pressed "start game" button
			SendPacket subs;
			subs.Unsigned8(NETCMD_LAUNCH);
			subs.send(d->sock);

			// Reopen the menu - perhaps the start is denied or other problems occur
			d->modal = &lgm;
			code = lgm.run();
			d->modal = nullptr;
		}
		if (code <= 0) {
			// if this is an internet game, tell the metaserver that client is back in the lobby.
			if (m_internet)
				InternetGaming::ref().set_game_done();
			return;
		}
	}

	d->server_is_waiting = true;

	Widelands::Game game;
#ifndef NDEBUG
	game.set_write_syncstream(true);
#endif

	try {
		UI::ProgressWindow * loaderUI = new UI::ProgressWindow("pics/progress.png");
		std::vector<std::string> tipstext;
		tipstext.push_back("general_game");
		tipstext.push_back("multiplayer");
		try {
			tipstext.push_back(getPlayersTribe());
		} catch (No_Tribe) {}
		GameTips tips (*loaderUI, tipstext);

		loaderUI->step(_("Preparing game"));

		d->game = &game;
		game.set_game_controller(this);
		uint8_t const pn = d->settings.playernum + 1;
		Interactive_GameBase * igb;
		if (pn > 0)
			igb =
				new Interactive_Player
					(game, g_options.pull_section("global"),
					 pn, d->settings.scenario, true);
		else
			igb =
				new Interactive_Spectator
					(game, g_options.pull_section("global"), true);
		game.set_ibase(igb);
		igb->set_chat_provider(*this);
		if (!d->settings.savegame) //  new map
			game.init_newgame(loaderUI, d->settings);
		else // savegame
			game.init_savegame(loaderUI, d->settings);
		d->time.reset(game.get_gametime());
		d->lasttimestamp = game.get_gametime();
		d->lasttimestamp_realtime = WLApplication::get()->get_time();

		d->modal = game.get_ibase();
		game.run
			(loaderUI,
			 d->settings.savegame ?
			 Widelands::Game::Loaded
			 : d->settings.scenario ?
			 Widelands::Game::NewMPScenario : Widelands::Game::NewNonScenario,
			 "", false);

		// if this is an internet game, tell the metaserver that the game is done.
		if (m_internet)
			InternetGaming::ref().set_game_done();
		d->modal = nullptr;
		d->game = nullptr;
	} catch (...) {
		d->modal = nullptr;
		WLApplication::emergency_save(game);
		d->game = nullptr;
		disconnect("CLIENT_CRASHED");
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

		if
			(d->server_is_waiting &&
			 d->game->get_gametime() == d->time.networktime())
		{
			sendTime();
			d->server_is_waiting = false;
		} else if (d->game->get_gametime() != d->lasttimestamp) {
			int32_t curtime = WLApplication::get()->get_time();
			if (curtime - d->lasttimestamp_realtime > CLIENT_TIMESTAMP_INTERVAL)
				sendTime();
		}
	}
}

void NetClient::sendPlayerCommand(Widelands::PlayerCommand & pc)
{
	assert(d->game);
	if (pc.sender() != d->settings.playernum + 1)
	{
		delete &pc;
		return;
	}

	log("[Client]: send playercommand at time %i\n", d->game->get_gametime());

	SendPacket s;
	s.Unsigned8(NETCMD_PLAYERCOMMAND);
	s.Signed32(d->game->get_gametime());
	pc.serialize(s);
	s.send(d->sock);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = WLApplication::get()->get_time();

	delete &pc;
}

int32_t NetClient::getFrametime()
{
	return d->time.time() - d->game->get_gametime();
}

std::string NetClient::getGameDescription()
{
	char buf[200];
	snprintf(buf, sizeof(buf), "network player %i", d->settings.playernum);
	return buf;
}

void NetClient::report_result
	(uint8_t player_nr, Widelands::PlayerEndResult result, const std::string & info)
{
	// Send to game
	Widelands::PlayerEndStatus pes;
	Widelands::Player* player = d->game->get_player(player_nr);
	assert(player);
	pes.player = player->player_number();
	pes.time = d->game->get_gametime();
	pes.result = result;
	pes.info = info;
	d->game->player_manager()->add_player_end_status(pes);
}


const GameSettings & NetClient::settings()
{
	return d->settings;
}

void NetClient::setScenario(bool scenario)
{
	// only accessible, if server is a dedicated server and access is granted
	if (!m_dedicated_access)
		return;
	m_dedicated_temp_scenario = scenario;
}

bool NetClient::canChangeMap()
{
	// only true, if server is a dedicated server and access is granted
	return m_dedicated_access;
}

bool NetClient::canChangePlayerState(uint8_t const number)
{
	if (!m_dedicated_access) // normal case
		return false;

	// dedicated server, access granted
	if (d->settings.savegame)
		return d->settings.players.at(number).state != PlayerSettings::stateClosed;
	else if (d->settings.scenario)
			return
				((d->settings.players.at(number).state == PlayerSettings::stateOpen
				  ||
				  d->settings.players.at(number).state == PlayerSettings::stateHuman)
				 &&
				 d->settings .players.at(number).closeable)
				||
				d->settings  .players.at(number).state == PlayerSettings::stateClosed;
	return true;
}

bool NetClient::canChangePlayerTribe(uint8_t number)
{
	return canChangePlayerTeam(number);
}

bool NetClient::canChangePlayerTeam(uint8_t number)
{
	if (!m_dedicated_access) // normal case
		return (number == d->settings.playernum) && !d->settings.scenario && !d->settings.savegame;
	else { // dedicated server, access granted
		if (d->settings.scenario || d->settings.savegame)
			return false;
		if (number >= d->settings.players.size())
			return false;
		if (number == d->settings.playernum)
			return true;
		return
			d->settings.players.at(number).state == PlayerSettings::stateComputer;
	}
}

bool NetClient::canChangePlayerInit(uint8_t number)
{
	if (!m_dedicated_access) // normal case
		return false;
	else { // dedicated server, access granted
		if (d->settings.scenario || d->settings.savegame)
			return false;
		return number < d->settings.players.size();
	}
}

bool NetClient::canLaunch()
{
	// only true, if server is a dedicated server and access is granted
	if (!m_dedicated_access)
		return false;
	if (d->settings.mapname.empty())
		return false;
	if (d->settings.players.size() < 1)
		return false;
	if (d->game)
		return false;

	// if there is one client that is currently receiving a file, we can not launch.
	for (uint8_t i = 0; i < d->settings.users.size(); ++i) {
		if (d->settings.users[i].position == d->settings.users[i].notConnected())
			continue;
		if (!d->settings.users[i].ready)
			return false;
	}

	// all players must be connected to a controller (human/ai) or be closed.
	for (size_t i = 0; i < d->settings.players.size(); ++i) {
		if (d->settings.players.at(i).state == PlayerSettings::stateOpen)
			return false;
	}
	return true;
}

void NetClient::setMap
	(const std::string & name,
	 const std::string & path,
	 uint32_t /* players */,
	 bool savegame)
{
	// only accessible, if server is a dedicated server and access is granted
	if (!m_dedicated_access)
		return;
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_MAP);
	s.String(name);
	s.String(path);
	s.Unsigned8(savegame ? 1 : 0);
	s.Unsigned8(m_dedicated_temp_scenario ? 1 : 0);
	s.send(d->sock);
}

void NetClient::setPlayerState(uint8_t, PlayerSettings::State)
{
	// client is not allowed to do this
}

void NetClient::setPlayerAI(uint8_t, const std::string &, bool const /* random_ai */)
{
	// client is not allowed to do this
}

void NetClient::nextPlayerState(uint8_t number)
{
	// only accessible, if server is a dedicated server and access is granted
	if (!m_dedicated_access)
		return;
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_PLAYER);
	s.Unsigned8(number);
	s.send(d->sock);
}

void NetClient::setPlayerTribe(uint8_t number, const std::string & tribe, bool const random_tribe)
{
	if ((number != d->settings.playernum) && !m_dedicated_access)
		return;

	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGETRIBE);
	s.Unsigned8(number);
	s.String(tribe);
	s.Unsigned8(random_tribe ? 1 : 0);
	s.send(d->sock);
}

void NetClient::setPlayerTeam(uint8_t number, Widelands::TeamNumber team)
{
	if ((number != d->settings.playernum) && !m_dedicated_access)
		return;

	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGETEAM);
	s.Unsigned8(number);
	s.Unsigned8(team);
	s.send(d->sock);
}

void NetClient::setPlayerCloseable(uint8_t, bool)
{
	//  client is not allowed to do this
}

void NetClient::setPlayerShared(uint8_t number, uint8_t player)
{
	if ((number != d->settings.playernum) && !m_dedicated_access)
		return;

	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGESHARED);
	s.Unsigned8(number);
	s.Unsigned8(player);
	s.send(d->sock);
}

void NetClient::setPlayerInit(uint8_t number, uint8_t)
{
	if ((number != d->settings.playernum) && !m_dedicated_access)
		return;

	// Host will decide what to change, therefore the init is not send, just the request to change
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGEINIT);
	s.Unsigned8(number);
	s.send(d->sock);
}

void NetClient::setPlayerName(uint8_t, const std::string &)
{
	// until now the name is set before joining - if you allow a change in
	// launchgame-menu, here properly should be a set_name function
}

void NetClient::setPlayer(uint8_t, PlayerSettings)
{
	// do nothing here - the request for a positionchange is send in
	// setPlayerNumber(uint8_t) to the host.
}

std::string NetClient::getWinConditionScript() {
	return d->settings.win_condition_script;
}

void NetClient::setWinConditionScript(std::string) {
	// Clients are not allowed to change this
}

void NetClient::nextWinCondition() {
	// only accessible, if server is a dedicated server and access is granted
	if (!m_dedicated_access)
		return;
	SendPacket s;
	s.Unsigned8(NETCMD_WIN_CONDITION);
	s.send(d->sock);
}

void NetClient::setPlayerNumber(uint8_t const number)
{
	// If the playernumber we want to switch to is our own, there is no need
	// for sending a request to the host.
	if (number == d->settings.playernum)
		return;
	// Same if the player is not selectable
	if
		(number < d->settings.players.size()
		 &&
		 (d->settings.players.at(number).state == PlayerSettings::stateClosed
		  ||
		  d->settings.players.at(number).state == PlayerSettings::stateComputer))
		return;

	// Send request
	SendPacket s;
	s.Unsigned8(NETCMD_SETTING_CHANGEPOSITION);
	s.Unsigned8(number);
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

// Network games cannot be paused
bool NetClient::isPaused()
{
	return false;
}

void NetClient::setPaused(bool /* paused */)
{
}

void NetClient::recvOnePlayer
	(uint8_t const number, Widelands::StreamRead & packet)
{
	if (number >= d->settings.players.size())
		throw DisconnectException("PLAYER_UPDATE_FOR_N_E_P");

	PlayerSettings & player = d->settings.players.at(number);
	player.state = static_cast<PlayerSettings::State>(packet.Unsigned8());
	player.name = packet.String();
	player.tribe = packet.String();
	player.random_tribe = packet.Unsigned8() == 1;
	player.initialization_index = packet.Unsigned8();
	player.ai = packet.String();
	player.random_ai = packet.Unsigned8() == 1;
	player.team = packet.Unsigned8();
	player.shared_in = packet.Unsigned8();
}

void NetClient::recvOneUser
	(uint32_t const number, Widelands::StreamRead & packet)
{
	if (number > d->settings.users.size())
		throw DisconnectException("USER_UPDATE_FOR_N_E_U");

	// This might happen, if a users connects after the game starts.
	if (number == d->settings.users.size()) {
		UserSettings newuser;
		d->settings.users.push_back(newuser);
	}

	d->settings.users.at(number).name     = packet.String  ();
	d->settings.users.at(number).position = packet.Signed32();
	d->settings.users.at(number).ready    = packet.Unsigned8() == 1;
	if (static_cast<int32_t>(number) == d->settings.usernum) {
		d->localplayername = d->settings.users.at(number).name;
		d->settings.playernum = d->settings.users.at(number).position;
	}
}

void NetClient::send(const std::string & msg)
{
	SendPacket s;
	s.Unsigned8(NETCMD_CHAT);
	s.String(msg);
	s.send(d->sock);
}

const std::vector<ChatMessage> & NetClient::getMessages() const
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
		uint8_t number = packet.Unsigned8();
		std::string reason = packet.String();
		if (number == 1)
			disconnect(reason,  "", false);
		else {
			std::string arg = packet.String();
			disconnect(reason, arg, false);
		}
		return;
	}

	if (d->settings.usernum == -2) {
		if (cmd != NETCMD_HELLO)
			throw ProtocolException(cmd);
		uint8_t const version = packet.Unsigned8();
		if (version != NETWORK_PROTOCOL_VERSION)
			throw DisconnectException("DIFFERENT_PROTOCOL_VERS");
		d->settings.usernum = packet.Unsigned32();
		d->settings.playernum = -1;
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

	case NETCMD_SETTING_MAP: {
		d->settings.mapname = packet.String();
		d->settings.mapfilename = g_fs->FileSystem::fixCrossFile(packet.String());
		d->settings.savegame = packet.Unsigned8() == 1;
		d->settings.scenario = packet.Unsigned8() == 1;
		log
			("[Client] SETTING_MAP '%s' '%s'\n",
			 d->settings.mapname.c_str(), d->settings.mapfilename.c_str());

		// New map was set, so we clean up the buffer of a previously requested file
		if (file)
			delete file;
		break;
	}

	case NETCMD_DEDICATED_MAPS: {
		DedicatedMapInfos info;
		info.path     = g_fs->FileSystem::fixCrossFile(packet.String());
		info.players  = packet.Unsigned8();
		info.scenario = packet.Unsigned8() == 1;
		d->settings.maps.push_back(info);
		break;
	}

	case NETCMD_DEDICATED_SAVED_GAMES: {
		DedicatedMapInfos info;
		info.path    = g_fs->FileSystem::fixCrossFile(packet.String());
		info.players = packet.Unsigned8();
		d->settings.saved_games.push_back(info);
		break;
	}

	case NETCMD_NEW_FILE_AVAILABLE: {
		std::string path = g_fs->FileSystem::fixCrossFile(packet.String());
		uint32_t bytes   = packet.Unsigned32();
		std::string md5  = packet.String();

		// Check whether the file or a file with that name already exists
		if (g_fs->FileExists(path)) {
			// If the file is a directory, we have to rename the file and replace it with the version of the
			// host. If it is a ziped file, we can check, whether the host and the client have got the same file.
			if (!g_fs->IsDirectory(path)) {
				FileRead fr;
				fr.Open(*g_fs, path.c_str());
				if (bytes == fr.GetSize()) {
					std::unique_ptr<char[]> complete(new char[bytes]);
					if (!complete) throw wexception("Out of memory");

					fr.DataComplete(complete.get(), bytes);
					SimpleMD5Checksum md5sum;
					md5sum.Data(complete.get(), bytes);
					md5sum.FinishChecksum();
					std::string localmd5 = md5sum.GetChecksum().str();
					if (localmd5 == md5)
						// everything is alright we already have the file.
						return;
				}
			}
			// Don't overwrite the file, better rename the original one
			g_fs->Rename(path, backupFileName(path));
		}

		// Yes we need the file!
		SendPacket s;
		s.Unsigned8(NETCMD_NEW_FILE_AVAILABLE);
		s.send(d->sock);

		if (file)
			delete file;

		file = new NetTransferFile();
		file->bytes = bytes;
		file->filename = path;
		file->md5sum = md5;
		size_t position = path.rfind(g_fs->fileSeparator(), path.size() - 2);
		if (position != std::string::npos) {
			path.resize(position);
			g_fs->EnsureDirectoryExists(path);
		}
		break;
	}

	case NETCMD_FILE_PART: {
		// Only go on, if we are waiting for a file part at the moment. It can happen, that an "unrequested"
		// part is send by the server if the map was changed just a moment ago and there was an outstanding
		// request from the client.
		if (!file)
			return; // silently ignore

		uint32_t part = packet.Unsigned32();
		uint32_t size = packet.Unsigned32();

		// Send an answer
		SendPacket s;
		s.Unsigned8(NETCMD_FILE_PART);
		s.Unsigned32(part);
		s.String(file->md5sum);
		s.send(d->sock);

		FilePart fp;

		char buf[NETFILEPARTSIZE];
		assert(size <= NETFILEPARTSIZE);

		if (packet.Data(buf, size) != size)
			log("Readproblem. Will try to go on anyways\n");
		memcpy(fp.part, &buf[0], size);
		file->parts.push_back(fp);

		// Write file to disk as soon as all parts arrived
		uint32_t left = (file->bytes - NETFILEPARTSIZE * part);
		if (left <= NETFILEPARTSIZE) {
			FileWrite fw;
			left = file->bytes;
			uint32_t i = 0;
			// Put all data together
			while (left > 0) {
				uint32_t writeout
					= (left > NETFILEPARTSIZE) ? NETFILEPARTSIZE : left;
				fw.Data(file->parts[i].part, writeout, FileWrite::Pos::Null());
				left -= writeout;
				++i;
			}
			// Now really write the file
			fw.Write(*g_fs, file->filename.c_str());

			// Check for consistence
			FileRead fr;
			fr.Open(*g_fs, file->filename.c_str());

			std::unique_ptr<char[]> complete(new char[file->bytes]);
			if (!complete) throw wexception("Out of memory");

			fr.DataComplete(complete.get(), file->bytes);
			SimpleMD5Checksum md5sum;
			md5sum.Data(complete.get(), file->bytes);
			md5sum.FinishChecksum();
			std::string localmd5 = md5sum.GetChecksum().str();
			if (localmd5 != file->md5sum) {
				// Something went wrong! We have to rerequest the file.
				s.reset();
				s.Unsigned8(NETCMD_NEW_FILE_AVAILABLE);
				s.send(d->sock);
				// Notify the players
				s.reset();
				s.Unsigned8(NETCMD_CHAT);
				s.String(_("/me 's file failed md5 checksumming."));
				s.send(d->sock);
				g_fs->Unlink(file->filename);
			}
			// Check file for validity
			bool invalid = false;
			if (d->settings.savegame) {
				// Saved game check - does Widelands recognize the file as saved game?
				Widelands::Game game;
				try {
					Widelands::Game_Loader gl(file->filename, game);
				} catch (...) {
					invalid = true;
				}
			} else {
				// Map check - does Widelands recognize the file as map?
				Widelands::Map map;
				std::unique_ptr<Widelands::Map_Loader> ml = map.get_correct_loader(file->filename);
				if (!ml)
					invalid = true;
			}
			if (invalid) {
				g_fs->Unlink(file->filename);
				// Restore original file, if there was one before
				if (g_fs->FileExists(backupFileName(file->filename)))
					g_fs->Rename(backupFileName(file->filename), file->filename);
				s.reset();
				s.Unsigned8(NETCMD_CHAT);
				s.String
					(_
					  ("/me checked the received file. Although md5 check summing succeeded, "
					   "I can not handle the file."));
				s.send(d->sock);
			}
		}
		break;
	}

	case NETCMD_SETTING_TRIBES: {
		d->settings.tribes.clear();
		for (uint8_t i = packet.Unsigned8(); i; --i) {
			TribeBasicInfo info;
			info.name = packet.String();

			// Get initializations (we have to do this locally, for translations)
			LuaInterface lua;
			for (uint8_t j = packet.Unsigned8(); j; --j) {
				std::string const initialization_script = packet.String();
				std::unique_ptr<LuaTable> t = lua.run_script(initialization_script);
				info.initializations.push_back
					(TribeBasicInfo::Initialization(initialization_script, t->get_string("name")));
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
		d->settings.playernum = number;
		d->settings.users.at(d->settings.usernum).position = number;
		break;
	}
	case NETCMD_WIN_CONDITION: {
		d->settings.win_condition_script = packet.String();
		break;
	}

	case NETCMD_LAUNCH: {
		if (!d->modal || d->game)
			throw DisconnectException("UNEXPECTED_LAUNCH");
		d->modal->end_modal(2);
		break;
	}
	case NETCMD_SETSPEED:
		d->realspeed = packet.Unsigned16();
		log
			("[Client] speed: %u.%03u\n",
			 d->realspeed / 1000, d->realspeed % 1000);
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
			throw DisconnectException("PLAYERCMD_WO_GAME");

		int32_t const time = packet.Signed32();
		Widelands::PlayerCommand & plcmd =
			*Widelands::PlayerCommand::deserialize(packet);
		plcmd.set_duetime(time);
		d->game->enqueue_command(&plcmd);
		d->time.recv(time);
		break;
	}
	case NETCMD_SYNCREQUEST: {
		if (!d->game)
			throw DisconnectException("SYNCREQUEST_WO_GAME");
		int32_t const time = packet.Signed32();
		d->time.recv(time);
		d->game->enqueue_command(new Cmd_NetCheckSync(time, this));
		break;
	}
	case NETCMD_CHAT: {
		ChatMessage c;
		c.time = time(nullptr);
		c.playern = packet.Signed16();
		c.sender = packet.String();
		c.msg = packet.String();
		if (packet.Unsigned8())
			c.recipient = packet.String();
		d->chatmessages.push_back(c);
		ChatProvider::send(c); // NoteSender<ChatMessage>
		break;
	}
	case NETCMD_SYSTEM_MESSAGE_CODE: {
		ChatMessage c;
		c.time = time(nullptr);
		std::string code = packet.String();
		std::string arg1 = packet.String();
		std::string arg2 = packet.String();
		std::string arg3 = packet.String();
		c.msg = NetworkGamingMessages::get_message(code, arg1, arg2, arg3);
		c.playern = UserSettings::none(); //  == System message
		// c.sender remains empty to indicate a system message
		d->chatmessages.push_back(c);
		ChatProvider::send(c);
		break;
	}
	case NETCMD_DEDICATED_ACCESS: {
		m_dedicated_access = true;
		break;
	}
	case NETCMD_INFO_DESYNC:
		log
			("[Client] received NETCMD_INFO_DESYNC. Trying to salvage some "
			 "information for debugging.\n");
		if (d->game)
			d->game->save_syncstream(true);
		break;
	default:
		throw ProtocolException(cmd);
	}
}


/**
 * Handle all incoming network traffic.
 */
void NetClient::handle_network ()
{
	// if this is an internet game, handle the metaserver network
	if (m_internet)
		InternetGaming::ref().handle_metaserver_communication();
	try {
		while (d->sock != nullptr && SDLNet_CheckSockets(d->sockset, 0) > 0) {
			// Perform only one read operation, then process all packets
			// from this read. This ensures that we process DISCONNECT
			// packets that are followed immediately by connection close.
			if (!d->deserializer.read(d->sock)) {
				disconnect("CONNECTION_LOST", "", false);
				return;
			}

			// Process all the packets from the last read
			while (d->sock && d->deserializer.avail()) {
				RecvPacket packet(d->deserializer);
				handle_packet(packet);
			}
		}
	} catch (const DisconnectException & e) {
		disconnect(e.what());
	} catch (const ProtocolException & e) {
		disconnect("PROTOCOL_EXCEPTION", boost::lexical_cast<std::string>(e.number()));
	} catch (const std::exception & e) {
		disconnect("SOMETHING_WRONG", e.what());
	}
}


void NetClient::disconnect
	(const std::string & reason, const std::string & arg, bool const sendreason, bool const showmsg)
{
	log("[Client]: disconnect(%s, %s)\n", reason.c_str(), arg.c_str());

	if (d->sock) {
		if (sendreason) {
			SendPacket s;
			s.Unsigned8(NETCMD_DISCONNECT);
			s.Unsigned8(arg.size() < 1 ? 1 : 2);
			s.String(reason);
			if (arg.size() > 0)
				s.String(arg);
			s.send(d->sock);
		}

		SDLNet_TCP_DelSocket (d->sockset, d->sock);
		SDLNet_TCP_Close (d->sock);
		d->sock = nullptr;
	}

	bool const trysave = showmsg && d->game;

	if (showmsg) {
		std::string msg;
		if (arg.empty())
			msg = NetworkGamingMessages::get_message(reason);
		else
			msg = NetworkGamingMessages::get_message(reason, arg);

		if (trysave)
			msg += _(" An automatic savegame will be created.");

		UI::WLMessageBox mmb
			(d->modal,
			 _("Disconnected from Host"),
			 msg,
			 UI::WLMessageBox::OK);
		mmb.run();
	}

	if (trysave)
		WLApplication::emergency_save(*d->game);

	if (d->modal) {
		d->modal->end_modal(0);
		d->modal = nullptr;
	}
}
