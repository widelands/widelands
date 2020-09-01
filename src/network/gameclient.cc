/*
 * Copyright (C) 2008-2020 by the Widelands Development Team
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

#include "network/gameclient.h"

#include <memory>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "game_io/game_loader.h"
#include "io/fileread.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filewrite.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "map_io/widelands_map_loader.h"
#include "network/internet_gaming.h"
#include "network/netclient.h"
#include "network/netclientproxy.h"
#include "network/network_gaming_messages.h"
#include "network/network_protocol.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/launch_mpg.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

struct GameClientImpl {
	bool internet_;

	GameSettings settings;

	std::string localplayername;

	std::unique_ptr<NetClientInterface> net;

	/// Currently active modal panel. Receives an end_modal on disconnect
	UI::Panel* modal;

	/// Current game. Only non-null if a game is actually running.
	Widelands::Game* game;

	NetworkTime time;

	/// \c true if we received a message indicating that the server is waiting
	/// Send a time message as soon as we caught up to networktime
	bool server_is_waiting;

	/// Data for the last time message we sent.
	uint32_t lasttimestamp;
	uint32_t lasttimestamp_realtime;

	/// The real target speed, in milliseconds per second.
	/// This is always set by the server
	uint32_t realspeed;

	/**
	 * The speed desired by the local player.
	 */
	uint32_t desiredspeed;

	/// Backlog of chat messages
	std::vector<ChatMessage> chatmessages;

	/** File that is eventually transferred via the network if not found at the other side */
	std::unique_ptr<NetTransferFile> file_;

	void send_hello();
	void send_player_command(Widelands::PlayerCommand*);

	bool run_map_menu(GameClient* parent);
	void run_game(InteractiveGameBase* igb);

	InteractiveGameBase* init_game(GameClient* parent, UI::ProgressWindow&);
};

void GameClientImpl::send_hello() {
	SendPacket s;
	s.unsigned_8(NETCMD_HELLO);
	s.unsigned_8(NETWORK_PROTOCOL_VERSION);
	s.string(localplayername);
	s.string(build_id());
	net->send(s);
}

void GameClientImpl::send_player_command(Widelands::PlayerCommand* pc) {
	SendPacket s;
	s.unsigned_8(NETCMD_PLAYERCOMMAND);
	s.signed_32(game->get_gametime());
	pc->serialize(s);
	net->send(s);
}

/**
 * Show and run() the fullscreen menu for setting map and mapsettings.
 *
 *  @return true to indicate that run is done.
 */
bool GameClientImpl::run_map_menu(GameClient* parent) {
	FullscreenMenuLaunchMPG lgm(parent, parent);
	lgm.set_chat_provider(*parent);
	modal = &lgm;
	FullscreenMenuBase::MenuTarget code = lgm.run<FullscreenMenuBase::MenuTarget>();
	modal = nullptr;
	if (code == FullscreenMenuBase::MenuTarget::kBack) {
		// if this is an internet game, tell the metaserver that client is back in the lobby.
		if (internet_) {
			InternetGaming::ref().set_game_done();
		}
		return true;
	}
	return false;
}

/**
 * Show progress dialog and load map or saved game.
 */
InteractiveGameBase* GameClientImpl::init_game(GameClient* parent, UI::ProgressWindow& loader) {
	modal = &loader;

	Notifications::publish(UI::NoteLoadingMessage(_("Preparing game…")));

	game->set_game_controller(parent);
	uint8_t const pn = settings.playernum + 1;
	game->save_handler().set_autosave_filename(
	   (boost::format("%s_netclient%u") % kAutosavePrefix % static_cast<unsigned int>(pn)).str());
	InteractiveGameBase* igb;
	if (pn > 0) {
		igb = new InteractivePlayer(*game, get_config_section(), pn, true, parent);
	} else {
		igb = new InteractiveSpectator(*game, get_config_section(), true, parent);
	}

	game->set_ibase(igb);
	if (settings.savegame) {  // savegame
		game->init_savegame(settings);
	} else {  //  new map
		game->init_newgame(settings);
	}
	return igb;
}

/**
 * Run the actual game and cleanup when done.
 */
void GameClientImpl::run_game(InteractiveGameBase* igb) {
	time.reset(game->get_gametime());
	lasttimestamp = game->get_gametime();
	lasttimestamp_realtime = SDL_GetTicks();

	modal = igb;

	game->run(settings.savegame ?
	             Widelands::Game::StartGameType::kSaveGame :
	             settings.scenario ? Widelands::Game::StartGameType::kMultiPlayerScenario :
	                                 Widelands::Game::StartGameType::kMap,
	          "", false, (boost::format("netclient_%d") % static_cast<int>(settings.usernum)).str());

	// if this is an internet game, tell the metaserver that the game is done.
	if (internet_) {
		InternetGaming::ref().set_game_done();
	}
	modal = nullptr;
	game = nullptr;
}

GameClient::GameClient(const std::pair<NetAddress, NetAddress>& host,
                       const std::string& playername,
                       bool internet,
                       const std::string& gamename)
   : d(new GameClientImpl) {

	d->internet_ = internet;

	if (internet) {
		assert(!gamename.empty());
		d->net = NetClientProxy::connect(host.first, gamename);
	} else {
		d->net = NetClient::connect(host.first);
	}
	if ((!d->net || !d->net->is_connected()) && host.second.is_valid()) {
		// First IP did not work? Try the second IP
		if (internet) {
			d->net = NetClientProxy::connect(host.second, gamename);
		} else {
			d->net = NetClient::connect(host.second);
		}
	}
	if (!d->net || !d->net->is_connected()) {
		throw WLWarning(_("Could not establish connection to host"),
		                _("Widelands could not establish a connection to the given address. "
		                  "Either no Widelands server was running at the supposed port or "
		                  "the server shut down as you tried to connect."));
	}

	d->settings.playernum = UserSettings::not_connected();
	d->settings.usernum = -2;
	d->localplayername = playername;
	d->modal = nullptr;
	d->game = nullptr;
	d->realspeed = 0;
	d->desiredspeed = 1000;
	d->file_ = nullptr;

	// Get the default win condition script
	d->settings.win_condition_script = d->settings.win_condition_scripts.front();
}

GameClient::~GameClient() {
	assert(d->net != nullptr);
	if (d->net->is_connected()) {
		disconnect("CLIENT_LEFT_GAME", "", true, false);
	}

	delete d;
}

void GameClient::run() {

	d->send_hello();
	d->settings.multiplayer = true;

	// Fill the list of possible system messages
	NetworkGamingMessages::fill_map();

	if (d->run_map_menu(this)) {
		return;  // did not select a Map ...
	}

	d->server_is_waiting = true;

	Widelands::Game game;
	game.set_write_syncstream(get_config_bool("write_syncstreams", true));

	try {
		std::vector<std::string> tipstexts{"general_game", "multiplayer"};
		if (has_players_tribe()) {
			tipstexts.push_back(get_players_tribe());
		}
		UI::ProgressWindow& loader_ui = game.create_loader_ui(tipstexts, false);

		d->game = &game;
		InteractiveGameBase* igb = d->init_game(this, loader_ui);
		d->run_game(igb);

	} catch (...) {
		WLApplication::emergency_save(game);
		d->game = nullptr;
		disconnect("CLIENT_CRASHED");
		// We will bounce back to the main menu, so we better log out
		if (d->internet_) {
			InternetGaming::ref().logout("CLIENT_CRASHED");
		}
		throw;
	}
}

void GameClient::think() {
	handle_network();

	if (d->game) {
		// TODO(Klaus Halfmann): what kind of time tricks are done here?
		if (d->realspeed == 0 || d->server_is_waiting) {
			d->time.fastforward();
		} else {
			d->time.think(d->realspeed);
		}

		if (d->server_is_waiting &&
		    d->game->get_gametime() == static_cast<uint32_t>(d->time.networktime())) {
			send_time();
			d->server_is_waiting = false;
		} else if (d->game->get_gametime() != d->lasttimestamp) {
			uint32_t curtime = SDL_GetTicks();
			if (curtime - d->lasttimestamp_realtime > CLIENT_TIMESTAMP_INTERVAL) {
				send_time();
			}
		}
	}
}

/**
 * Send PlayerCommand to server.
 *
 * @param pc will always be deleted in the end.
 */
void GameClient::send_player_command(Widelands::PlayerCommand* pc) {
	assert(d->game);

	// TODDO(Klaus Halfmann)should this be an assert?
	if (pc->sender() == d->settings.playernum + 1)  //  allow command for current player only
	{
		log_info("[Client]: send playercommand at time %i\n", d->game->get_gametime());

		d->send_player_command(pc);

		d->lasttimestamp = d->game->get_gametime();
		d->lasttimestamp_realtime = SDL_GetTicks();
	} else {
		log_warn("[Client]: Playercommand is not for current player? %i\n", pc->sender());
	}

	delete pc;
}

int32_t GameClient::get_frametime() {
	return d->time.time() - d->game->get_gametime();
}

GameController::GameType GameClient::get_game_type() {
	return GameController::GameType::kNetClient;
}

void GameClient::report_result(uint8_t player_nr,
                               Widelands::PlayerEndResult result,
                               const std::string& info) {
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

const GameSettings& GameClient::settings() {
	return d->settings;
}

void GameClient::set_scenario(bool) {
}

bool GameClient::can_change_map() {
	return false;
}

bool GameClient::can_change_player_state(uint8_t const) {
	return false;
}

bool GameClient::can_change_player_tribe(uint8_t number) {
	return can_change_player_team(number);
}

bool GameClient::can_change_player_team(uint8_t number) {
	return (number == d->settings.playernum) && !d->settings.scenario && !d->settings.savegame;
}

bool GameClient::can_change_player_init(uint8_t) {
	return false;
}

bool GameClient::can_launch() {
	return false;
}

void GameClient::set_player_state(uint8_t, PlayerSettings::State) {
	// client is not allowed to do this
}

void GameClient::set_player_ai(uint8_t, const std::string&, bool const /* random_ai */) {
	// client is not allowed to do this
}

void GameClient::next_player_state(uint8_t) {
	// client is not allowed to do this
}

void GameClient::set_map(const std::string&, const std::string&, uint32_t, bool) {
	// client is not allowed to do this
}

void GameClient::set_player_tribe(uint8_t number,
                                  const std::string& tribe,
                                  bool const random_tribe) {
	if ((number != d->settings.playernum)) {
		return;
	}

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGETRIBE);
	s.unsigned_8(number);
	s.string(tribe);
	s.unsigned_8(random_tribe ? 1 : 0);
	d->net->send(s);
}

void GameClient::set_player_team(uint8_t number, Widelands::TeamNumber team) {
	if ((number != d->settings.playernum)) {
		return;
	}

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGETEAM);
	s.unsigned_8(number);
	s.unsigned_8(team);
	d->net->send(s);
}

void GameClient::set_player_closeable(uint8_t, bool) {
	//  client is not allowed to do this
}

void GameClient::set_player_shared(PlayerSlot number, Widelands::PlayerNumber shared) {
	if ((number != d->settings.playernum)) {
		return;
	}

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGESHARED);
	s.unsigned_8(number);
	s.unsigned_8(shared);
	d->net->send(s);
}

void GameClient::set_player_init(uint8_t number, uint8_t initialization_index) {
	if ((number != d->settings.playernum)) {
		return;
	}

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGEINIT);
	s.unsigned_8(number);
	s.unsigned_8(initialization_index);
	d->net->send(s);
}

void GameClient::set_player_name(uint8_t, const std::string&) {
	// until now the name is set before joining - if you allow a change in
	// launchgame-menu, here properly should be a set_name function
}

void GameClient::set_player(uint8_t, const PlayerSettings&) {
	// do nothing here - the request for a positionchange is send in
	// set_player_number(uint8_t) to the host.
}

void GameClient::set_peaceful_mode(bool peace) {
	d->settings.peaceful = peace;
}

bool GameClient::is_peaceful_mode() {
	return d->settings.peaceful;
}

void GameClient::set_custom_starting_positions(bool c) {
	d->settings.custom_starting_positions = c;
}

bool GameClient::get_custom_starting_positions() {
	return d->settings.custom_starting_positions;
}

std::string GameClient::get_win_condition_script() {
	return d->settings.win_condition_script;
}

void GameClient::set_win_condition_script(const std::string&) {
	// Clients are not allowed to change this
	NEVER_HERE();
}

void GameClient::set_player_number(uint8_t const number) {
	// If the playernumber we want to switch to is our own, there is no need
	// for sending a request to the host.
	if (number == d->settings.playernum) {
		return;
	}
	// Same if the player is not selectable
	if (number < d->settings.players.size() &&
	    (d->settings.players.at(number).state == PlayerSettings::State::kClosed ||
	     d->settings.players.at(number).state == PlayerSettings::State::kComputer)) {
		return;
	}

	// Send request
	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGEPOSITION);
	s.unsigned_8(number);
	d->net->send(s);
}

uint32_t GameClient::real_speed() {
	return d->realspeed;
}

uint32_t GameClient::desired_speed() {
	return d->desiredspeed;
}

void GameClient::set_desired_speed(uint32_t speed) {
	if (speed > std::numeric_limits<uint16_t>::max()) {
		speed = std::numeric_limits<uint16_t>::max();
	}

	if (speed != d->desiredspeed) {
		d->desiredspeed = speed;

		SendPacket s;
		s.unsigned_8(NETCMD_SETSPEED);
		s.unsigned_16(d->desiredspeed);
		d->net->send(s);
	}
}

// Network games cannot be paused
bool GameClient::is_paused() {
	return false;
}

void GameClient::set_paused(bool /* paused */) {
}

void GameClient::receive_one_player(uint8_t const number, StreamRead& packet) {
	if (number >= d->settings.players.size()) {
		throw DisconnectException("PLAYER_UPDATE_FOR_N_E_P");
	}

	PlayerSettings& player = d->settings.players.at(number);
	player.state = static_cast<PlayerSettings::State>(packet.unsigned_8());
	player.name = packet.string();
	player.tribe = packet.string();
	player.random_tribe = packet.unsigned_8() == 1;
	player.initialization_index = packet.unsigned_8();
	player.ai = packet.string();
	player.random_ai = packet.unsigned_8() == 1;
	player.team = packet.unsigned_8();
	player.shared_in = packet.unsigned_8();
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer, number));
}

void GameClient::receive_one_user(uint32_t const number, StreamRead& packet) {
	if (number > d->settings.users.size()) {
		throw DisconnectException("USER_UPDATE_FOR_N_E_U");
	}

	// This might happen, if a users connects after the game starts.
	if (number == d->settings.users.size()) {
		d->settings.users.push_back(*new UserSettings());
	}

	d->settings.users.at(number).name = packet.string();
	d->settings.users.at(number).position = packet.signed_32();
	d->settings.users.at(number).ready = packet.unsigned_8() == 1;

	if (static_cast<int32_t>(number) == d->settings.usernum) {
		d->localplayername = d->settings.users.at(number).name;
		d->settings.playernum = d->settings.users.at(number).position;
	}
	Notifications::publish(
	   NoteGameSettings(NoteGameSettings::Action::kUser, d->settings.playernum, number));
}

void GameClient::send(const std::string& msg) {
	SendPacket s;
	s.unsigned_8(NETCMD_CHAT);
	s.string(msg);
	d->net->send(s);
}

const std::vector<ChatMessage>& GameClient::get_messages() const {
	return d->chatmessages;
}

void GameClient::send_time() {
	assert(d->game);

	log_info("[Client]: sending timestamp: %i\n", d->game->get_gametime());

	SendPacket s;
	s.unsigned_8(NETCMD_TIME);
	s.signed_32(d->game->get_gametime());
	d->net->send(s);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = SDL_GetTicks();
}

void GameClient::sync_report_callback() {
	assert(d->net != nullptr);
	if (d->net->is_connected()) {
		SendPacket s;
		s.unsigned_8(NETCMD_SYNCREPORT);
		s.signed_32(d->game->get_gametime());
		s.data(d->game->get_sync_hash().data, 16);
		d->net->send(s);
	}
}

void GameClient::handle_disconnect(RecvPacket& packet) {
	uint8_t number = packet.unsigned_8();
	std::string reason = packet.string();
	if (number == 1) {
		disconnect(reason, "", false);
	} else {
		std::string arg = packet.string();
		disconnect(reason, arg, false);
	}
}

/**
 * Hello from the other side
 */
void GameClient::handle_hello(RecvPacket& packet) {
	if (d->settings.usernum != -2) {           // TODO(Klaus Halfmann): if the host is the client ?.
		throw ProtocolException(NETCMD_HELLO);  // I am talking with myself? Bad idea
	}
	uint8_t const version = packet.unsigned_8();
	if (version != NETWORK_PROTOCOL_VERSION) {
		throw DisconnectException("DIFFERENT_PROTOCOL_VERS");
	}
	d->settings.usernum = packet.unsigned_32();  // TODO(Klaus Halfmann): usernum is int8_t.
	d->settings.playernum = -1;
}

/**
 * Give a pong for a ping
 */
void GameClient::handle_ping(RecvPacket&) {
	SendPacket s;
	s.unsigned_8(NETCMD_PONG);
	d->net->send(s);

	log_info("[Client] Pong!\n");
}

/**
 * New Map name was sent.
 */
void GameClient::handle_setting_map(RecvPacket& packet) {
	d->settings.mapname = packet.string();
	d->settings.mapfilename = g_fs->FileSystem::fix_cross_file(packet.string());
	d->settings.savegame = packet.unsigned_8() == 1;
	d->settings.scenario = packet.unsigned_8() == 1;
	log_info("[Client] SETTING_MAP '%s' '%s'\n", d->settings.mapname.c_str(),
	         d->settings.mapfilename.c_str());

	// New map was set, so we clean up the buffer of a previously requested file
	d->file_.reset(nullptr);
}

/**
 *
 */
// TODO(Klaus Halfmann): refactor this until it can be understood, move into impl.
void GameClient::handle_new_file(RecvPacket& packet) {
	std::string path = g_fs->FileSystem::fix_cross_file(packet.string());
	uint32_t bytes = packet.unsigned_32();
	std::string md5 = packet.string();

	// Check whether the file or a file with that name already exists
	if (g_fs->file_exists(path)) {
		// If the file is a directory, we have to rename the file and replace it with the version
		// of the host. If it is a zipped file, we can check, whether the host and the client have
		// got the same file.
		if (!g_fs->is_directory(path)) {
			FileRead fr;
			fr.open(*g_fs, path);
			if (bytes == fr.get_size()) {
				std::unique_ptr<char[]> complete(new char[bytes]);
				if (!complete) {
					throw wexception("Out of memory");
				}
				fr.data_complete(complete.get(), bytes);
				// TODO(Klaus Halfmann): compute MD5 on the fly in FileRead...
				SimpleMD5Checksum md5sum;
				md5sum.data(complete.get(), bytes);
				md5sum.finish_checksum();
				std::string localmd5 = md5sum.get_checksum().str();
				if (localmd5 == md5) {
					// everything is alright we now have the file.
					return;
				}
			}
		}
		// Don't overwrite the file, better rename the original one
		try {
			g_fs->fs_rename(path, backup_file_name(path));
		} catch (const FileError& e) {
			log_err("file error in GameClient::handle_packet: case NETCMD_FILE_PART: "
			        "%s\n",
			        e.what());
			// TODO(Arty): What now? It just means the next step will fail
			// or possibly result in some corrupt file
		}
	}

	// Yes we need the file!
	SendPacket s;
	s.unsigned_8(NETCMD_NEW_FILE_AVAILABLE);
	d->net->send(s);

	d->file_.reset(new NetTransferFile());
	d->file_->bytes = bytes;
	d->file_->filename = path;
	d->file_->md5sum = md5;
	size_t position = path.rfind(g_fs->file_separator(), path.size() - 2);
	if (position != std::string::npos) {
		path.resize(position);
		g_fs->ensure_directory_exists(path);
	}
}

/**
 *
 */
// TODO(Klaus Halfmann): refactor this until it can be understood, move into impl.
void GameClient::handle_file_part(RecvPacket& packet) {
	// Only go on, if we are waiting for a file part at the moment. It can happen, that an
	// "unrequested" part is send by the server if the map was changed just a moment ago
	// and there was an outstanding request from the client.
	if (!d->file_) {
		return;  // silently ignore
	}

	uint32_t part = packet.unsigned_32();
	uint32_t size = packet.unsigned_32();

	FilePart fp;

	char buf[NETFILEPARTSIZE];
	assert(size <= NETFILEPARTSIZE);

	// TODO(Klaus Halfmann): read directly into FilePart?
	if (packet.data(buf, size) != size) {
		log_warn("Readproblem. Will try to go on anyways\n");
	}
	memcpy(fp.part, &buf[0], size);
	d->file_->parts.push_back(fp);

	// Write file to disk as soon as all parts arrived
	uint32_t left = (d->file_->bytes - NETFILEPARTSIZE * part);
	if (left <= NETFILEPARTSIZE) {

		// Send an answer. We got everything
		SendPacket s;
		s.unsigned_8(NETCMD_FILE_PART);
		s.unsigned_32(part);
		s.string(d->file_->md5sum);
		d->net->send(s);

		FileWrite fw;
		left = d->file_->bytes;
		uint32_t i = 0;
		// Put all data together
		while (left > 0) {
			uint32_t writeout = (left > NETFILEPARTSIZE) ? NETFILEPARTSIZE : left;
			fw.data(d->file_->parts[i].part, writeout, FileWrite::Pos::null());
			left -= writeout;
			++i;
		}
		// Now really write the file
		fw.write(*g_fs, d->file_->filename.c_str());

		// Check for consistence
		FileRead fr;
		fr.open(*g_fs, d->file_->filename);

		std::unique_ptr<char[]> complete(new char[d->file_->bytes]);

		fr.data_complete(complete.get(), d->file_->bytes);
		SimpleMD5Checksum md5sum;
		md5sum.data(complete.get(), d->file_->bytes);
		md5sum.finish_checksum();
		std::string localmd5 = md5sum.get_checksum().str();
		if (localmd5 != d->file_->md5sum) {
			// Something went wrong! We have to rerequest the file.
			s.reset();
			s.unsigned_8(NETCMD_NEW_FILE_AVAILABLE);
			d->net->send(s);
			// Notify the players
			s.reset();
			s.unsigned_8(NETCMD_CHAT);
			s.string(_("/me 's file failed md5 checksumming."));
			d->net->send(s);
			try {
				g_fs->fs_unlink(d->file_->filename);
			} catch (const FileError& e) {
				log_err("file error in GameClient::handle_packet: case NETCMD_FILE_PART: "
				        "%s\n",
				        e.what());
			}
		}
		// Check file for validity
		bool invalid = false;
		if (d->settings.savegame) {
			// Saved game check - does Widelands recognize the file as saved game?
			Widelands::Game game;
			try {
				Widelands::GameLoader gl(d->file_->filename, game);
			} catch (...) {
				invalid = true;
			}
		} else {
			// Map check - does Widelands recognize the file as map?
			Widelands::Map map;
			std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(d->file_->filename);
			if (!ml) {
				invalid = true;
			}
		}
		if (invalid) {
			try {
				g_fs->fs_unlink(d->file_->filename);
				// Restore original file, if there was one before
				if (g_fs->file_exists(backup_file_name(d->file_->filename))) {
					g_fs->fs_rename(backup_file_name(d->file_->filename), d->file_->filename);
				}
			} catch (const FileError& e) {
				log_err("file error in GameClient::handle_packet: case NETCMD_FILE_PART: "
				        "%s\n",
				        e.what());
			}
			s.reset();
			s.unsigned_8(NETCMD_CHAT);
			s.string(_("/me checked the received file. Although md5 check summing succeeded, "
			           "I can not handle the file."));
			d->net->send(s);
		}
	}
}

/**
 *
 */
void GameClient::handle_setting_tribes(RecvPacket& packet) {
	d->settings.tribes.clear();
	for (uint8_t i = packet.unsigned_8(); i; --i) {
		Widelands::TribeBasicInfo info = Widelands::get_tribeinfo(packet.string());

		// Get initializations (we have to do this locally, for translations)
		LuaInterface lua;
		info.initializations.clear();
		for (uint8_t j = packet.unsigned_8(); j > 0; --j) {
			std::string const initialization_script = packet.string();
			std::unique_ptr<LuaTable> t = lua.run_script(initialization_script);
			t->do_not_warn_about_unaccessed_keys();
			std::set<std::string> tags;
			if (t->has_key("map_tags")) {
				std::unique_ptr<LuaTable> tt = t->get_table("map_tags");
				for (int key : tt->keys<int>()) {
					tags.insert(tt->get_string(key));
				}
			}
			info.initializations.push_back(Widelands::TribeBasicInfo::Initialization(
			   initialization_script, t->get_string("descname"), t->get_string("tooltip"), tags));
		}
		d->settings.tribes.push_back(info);
	}
}

/**
 *
 */
void GameClient::handle_setting_allplayers(RecvPacket& packet) {
	d->settings.players.resize(packet.unsigned_8());
	for (uint8_t i = 0; i < d->settings.players.size(); ++i) {
		receive_one_player(i, packet);
	}
	// Map changes are finished here
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));
}

/**
 *
 */
void GameClient::handle_playercommand(RecvPacket& packet) {
	if (!d->game) {
		throw DisconnectException("PLAYERCMD_WO_GAME");
	}

	int32_t const time = packet.signed_32();
	Widelands::PlayerCommand& plcmd = *Widelands::PlayerCommand::deserialize(packet);
	plcmd.set_duetime(time);
	d->game->enqueue_command(&plcmd);
	d->time.receive(time);
}

/**
 *
 */
void GameClient::handle_syncrequest(RecvPacket& packet) {
	if (!d->game) {
		throw DisconnectException("SYNCREQUEST_WO_GAME");
	}
	int32_t const time = packet.signed_32();
	d->time.receive(time);
	d->game->enqueue_command(new CmdNetCheckSync(time, [this] { sync_report_callback(); }));
	d->game->report_sync_request();
}

/**
 *
 */
void GameClient::handle_chat(RecvPacket& packet) {
	ChatMessage c("");
	c.playern = packet.signed_16();
	c.sender = packet.string();
	c.msg = packet.string();
	if (packet.unsigned_8()) {
		c.recipient = packet.string();
	}
	d->chatmessages.push_back(c);
	Notifications::publish(c);
}

/**
 *
 */
void GameClient::handle_system_message(RecvPacket& packet) {
	const std::string code = packet.string();
	const std::string arg1 = packet.string();
	const std::string arg2 = packet.string();
	const std::string arg3 = packet.string();
	ChatMessage c(NetworkGamingMessages::get_message(code, arg1, arg2, arg3));
	c.playern = UserSettings::none();  //  == System message
	                                   // c.sender remains empty to indicate a system message
	d->chatmessages.push_back(c);
	Notifications::publish(c);
}

/**
 *
 */
void GameClient::handle_desync(RecvPacket&) {
	log_err("[Client] received NETCMD_INFO_DESYNC. Trying to salvage some "
	        "information for debugging.\n");
	if (d->game) {
		d->game->save_syncstream(true);
		// We don't know our playernumber, so report as -1
		d->game->report_desync(-1);
	}
}

/**
 * Handle one packet received from the host.
 *
 * \note The caller must handle exceptions by closing the connection.
 */
void GameClient::handle_packet(RecvPacket& packet) {
	uint8_t cmd = packet.unsigned_8();

	switch (cmd) {
	case NETCMD_DISCONNECT:
		return handle_disconnect(packet);
	case NETCMD_HELLO:
		return handle_hello(packet);
	case NETCMD_PING:
		return handle_ping(packet);
	case NETCMD_SETTING_MAP:
		return handle_setting_map(packet);
	case NETCMD_NEW_FILE_AVAILABLE:
		return handle_new_file(packet);
	case NETCMD_FILE_PART:
		return handle_file_part(packet);
	case NETCMD_SETTING_TRIBES:
		return handle_setting_tribes(packet);
	case NETCMD_SETTING_ALLPLAYERS:
		return handle_setting_allplayers(packet);
	case NETCMD_SETTING_PLAYER: {
		uint8_t player = packet.unsigned_8();
		receive_one_player(player, packet);
	} break;
	case NETCMD_SETTING_ALLUSERS: {
		d->settings.users.resize(packet.unsigned_8());
		for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
			receive_one_user(i, packet);
		}
	} break;
	case NETCMD_SETTING_USER: {
		uint32_t user = packet.unsigned_32();
		receive_one_user(user, packet);
	} break;
	case NETCMD_SET_PLAYERNUMBER: {
		int32_t number = packet.signed_32();
		d->settings.playernum = number;
		d->settings.users.at(d->settings.usernum).position = number;
	} break;
	case NETCMD_WIN_CONDITION:
		d->settings.win_condition_script = g_fs->FileSystem::fix_cross_file(packet.string());
		break;
	case NETCMD_PEACEFUL_MODE:
		d->settings.peaceful = packet.unsigned_8();
		break;
	case NETCMD_CUSTOM_STARTING_POSITIONS:
		d->settings.custom_starting_positions = packet.unsigned_8();
		break;
	case NETCMD_LAUNCH:
		if (!d->modal || d->game) {
			throw DisconnectException("UNEXPECTED_LAUNCH");
		}
		d->modal->end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
		break;
	case NETCMD_SETSPEED:
		d->realspeed = packet.unsigned_16();
		log_info("[Client] speed: %u.%03u\n", d->realspeed / 1000, d->realspeed % 1000);
		break;
	case NETCMD_TIME:
		d->time.receive(packet.signed_32());
		break;
	case NETCMD_WAIT:
		log_info("[Client]: server is waiting.\n");
		d->server_is_waiting = true;
		break;
	case NETCMD_PLAYERCOMMAND:
		return handle_playercommand(packet);
	case NETCMD_SYNCREQUEST:
		return handle_syncrequest(packet);
	case NETCMD_CHAT:
		return handle_chat(packet);
	case NETCMD_SYSTEM_MESSAGE_CODE:
		return handle_system_message(packet);
	case NETCMD_INFO_DESYNC:
		return handle_desync(packet);
	default:
		throw ProtocolException(cmd);
	}
}

/**
 * Handle all incoming network traffic.
 */
void GameClient::handle_network() {
	// if this is an internet game, handle the metaserver network
	if (d->internet_) {
		InternetGaming::ref().handle_metaserver_communication();
	}
	try {
		assert(d->net != nullptr);
		// Check if the connection is still open
		if (!d->net->is_connected()) {
			disconnect("CONNECTION_LOST", "", false);
			return;
		}
		// Process all available packets
		std::unique_ptr<RecvPacket> packet = d->net->try_receive();
		while (packet) {
			handle_packet(*packet);
			packet = d->net->try_receive();
		}
	} catch (const DisconnectException& e) {
		disconnect(e.what());
	} catch (const ProtocolException& e) {
		disconnect("PROTOCOL_EXCEPTION", e.what());
	} catch (const std::exception& e) {
		disconnect("SOMETHING_WRONG", e.what());
	}
}

void GameClient::disconnect(const std::string& reason,
                            const std::string& arg,
                            bool const sendreason,
                            bool const showmsg) {
	log_info("[Client]: disconnect(%s, %s)\n", reason.c_str(), arg.c_str());

	assert(d->net != nullptr);
	if (d->net->is_connected()) {
		if (sendreason) {
			SendPacket s;
			s.unsigned_8(NETCMD_DISCONNECT);
			s.unsigned_8(arg.size() < 1 ? 1 : 2);
			s.string(reason);
			if (!arg.empty()) {
				s.string(arg);
			}
			d->net->send(s);
		}

		d->net->close();
	}

	bool const trysave = showmsg && d->game;

	if (showmsg && d->modal) {  // can only show a message with a valid modal parent window
		std::string msg;
		if (arg.empty()) {
			msg = NetworkGamingMessages::get_message(reason);
		} else {
			msg = NetworkGamingMessages::get_message(reason, arg);
		}

		if (trysave) {
			/** TRANSLATORS: %s contains an error message. */
			msg = (boost::format(_("%s An automatic savegame will be created.")) % msg).str();
		}

		UI::WLMessageBox mmb(
		   d->modal, _("Disconnected from Host"), msg, UI::WLMessageBox::MBoxType::kOk);
		mmb.run<UI::Panel::Returncodes>();
	}

	if (trysave) {
		WLApplication::emergency_save(*d->game);
	}

	// TODO(Klaus Halfmann): Some of the modal windows are now handled by unique_ptr resulting in a
	// double free.
	if (d->modal) {
		d->modal->end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	}
	d->modal = nullptr;
}
