/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "build_info.h"
#include "config.h"
#include "game_io/game_loader.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "map_io/widelands_map_loader.h"
#include "network/internet_gaming.h"
#include "network/network_gaming_messages.h"
#include "network/network_protocol.h"
#include "network/network_system.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/launch_mpg.h"
#include "wlapplication.h"
#include "wui/game_tips.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

struct GameClientImpl {
	GameSettings settings;

	std::string localplayername;

	std::unique_ptr<NetClient> net;

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
};

GameClient::GameClient(const NetAddress& host, const std::string& playername, bool internet)
   : d(new GameClientImpl), internet_(internet) {
	d->net = NetClient::connect(host);
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
	file_ = nullptr;

	// Get the default win condition script
	d->settings.win_condition_script = d->settings.win_condition_scripts.front();
}

GameClient::~GameClient() {
	assert(d->net != nullptr);
	if (d->net->is_connected())
		disconnect("CLIENT_LEFT_GAME", "", true, false);

	delete d;
}

void GameClient::run() {
	SendPacket s;
	s.unsigned_8(NETCMD_HELLO);
	s.unsigned_8(NETWORK_PROTOCOL_VERSION);
	s.string(d->localplayername);
	s.string(build_id());
	d->net->send(s);

	d->settings.multiplayer = true;

	// Fill the list of possible system messages
	NetworkGamingMessages::fill_map();
	{
		FullscreenMenuLaunchMPG lgm(this, this);
		lgm.set_chat_provider(*this);
		d->modal = &lgm;
		FullscreenMenuBase::MenuTarget code = lgm.run<FullscreenMenuBase::MenuTarget>();
		d->modal = nullptr;
		if (code == FullscreenMenuBase::MenuTarget::kBack) {
			// if this is an internet game, tell the metaserver that client is back in the lobby.
			if (internet_)
				InternetGaming::ref().set_game_done();
			return;
		}
	}

	d->server_is_waiting = true;

	Widelands::Game game;
	game.set_write_syncstream(g_options.pull_section("global").get_bool("write_syncstreams", true));

	try {
		UI::ProgressWindow* loader_ui = new UI::ProgressWindow("images/loadscreens/progress.png");
		std::vector<std::string> tipstext;
		tipstext.push_back("general_game");
		tipstext.push_back("multiplayer");
		try {
			tipstext.push_back(get_players_tribe());
		} catch (NoTribe) {
		}
		GameTips tips(*loader_ui, tipstext);

		loader_ui->step(_("Preparing game"));

		d->game = &game;
		game.set_game_controller(this);
		uint8_t const pn = d->settings.playernum + 1;
		game.save_handler().set_autosave_filename(
		   (boost::format("wl_autosave_netclient%u") % static_cast<unsigned int>(pn)).str());
		InteractiveGameBase* igb;
		if (pn > 0)
			igb = new InteractivePlayer(game, g_options.pull_section("global"), pn, true);
		else
			igb = new InteractiveSpectator(game, g_options.pull_section("global"), true);
		game.set_ibase(igb);
		igb->set_chat_provider(*this);
		if (!d->settings.savegame) {  //  new map
			game.init_newgame(loader_ui, d->settings);
		} else {  // savegame
			game.init_savegame(loader_ui, d->settings);
		}
		d->time.reset(game.get_gametime());
		d->lasttimestamp = game.get_gametime();
		d->lasttimestamp_realtime = SDL_GetTicks();

		d->modal = game.get_ibase();
		game.run(loader_ui, d->settings.savegame ? Widelands::Game::Loaded : d->settings.scenario ?
		                                           Widelands::Game::NewMPScenario :
		                                           Widelands::Game::NewNonScenario,
		         "", false,
		         (boost::format("netclient_%d") % static_cast<int>(d->settings.usernum)).str());

		// if this is an internet game, tell the metaserver that the game is done.
		if (internet_)
			InternetGaming::ref().set_game_done();
		d->modal = nullptr;
		d->game = nullptr;
	} catch (...) {
		d->modal = nullptr;
		WLApplication::emergency_save(game);
		d->game = nullptr;
		disconnect("CLIENT_CRASHED");
		// We will bounce back to the main menu, so we better log out
		if (internet_) {
			InternetGaming::ref().logout("CLIENT_CRASHED");
		}
		throw;
	}
}

void GameClient::think() {
	handle_network();

	if (d->game) {
		if (d->realspeed == 0 || d->server_is_waiting)
			d->time.fastforward();
		else
			d->time.think(d->realspeed);

		if (d->server_is_waiting &&
		    d->game->get_gametime() == static_cast<uint32_t>(d->time.networktime())) {
			send_time();
			d->server_is_waiting = false;
		} else if (d->game->get_gametime() != d->lasttimestamp) {
			uint32_t curtime = SDL_GetTicks();
			if (curtime - d->lasttimestamp_realtime > CLIENT_TIMESTAMP_INTERVAL)
				send_time();
		}
	}
}

void GameClient::send_player_command(Widelands::PlayerCommand& pc) {
	assert(d->game);
	if (pc.sender() != d->settings.playernum + 1) {
		delete &pc;
		return;
	}

	log("[Client]: send playercommand at time %i\n", d->game->get_gametime());

	SendPacket s;
	s.unsigned_8(NETCMD_PLAYERCOMMAND);
	s.signed_32(d->game->get_gametime());
	pc.serialize(s);
	d->net->send(s);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = SDL_GetTicks();

	delete &pc;
}

int32_t GameClient::get_frametime() {
	return d->time.time() - d->game->get_gametime();
}

GameController::GameType GameClient::get_game_type() {
	return GameController::GameType::NETCLIENT;
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
	if ((number != d->settings.playernum))
		return;

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGETRIBE);
	s.unsigned_8(number);
	s.string(tribe);
	s.unsigned_8(random_tribe ? 1 : 0);
	d->net->send(s);
}

void GameClient::set_player_team(uint8_t number, Widelands::TeamNumber team) {
	if ((number != d->settings.playernum))
		return;

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGETEAM);
	s.unsigned_8(number);
	s.unsigned_8(team);
	d->net->send(s);
}

void GameClient::set_player_closeable(uint8_t, bool) {
	//  client is not allowed to do this
}

void GameClient::set_player_shared(uint8_t number, uint8_t player) {
	if ((number != d->settings.playernum))
		return;

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGESHARED);
	s.unsigned_8(number);
	s.unsigned_8(player);
	d->net->send(s);
}

void GameClient::set_player_init(uint8_t number, uint8_t) {
	if ((number != d->settings.playernum))
		return;

	// Host will decide what to change, therefore the init is not send, just the request to change
	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGEINIT);
	s.unsigned_8(number);
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
	if (number == d->settings.playernum)
		return;
	// Same if the player is not selectable
	if (number < d->settings.players.size() &&
	    (d->settings.players.at(number).state == PlayerSettings::stateClosed ||
	     d->settings.players.at(number).state == PlayerSettings::stateComputer))
		return;

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
	if (speed > std::numeric_limits<uint16_t>::max())
		speed = std::numeric_limits<uint16_t>::max();

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
	if (number >= d->settings.players.size())
		throw DisconnectException("PLAYER_UPDATE_FOR_N_E_P");

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
}

void GameClient::receive_one_user(uint32_t const number, StreamRead& packet) {
	if (number > d->settings.users.size())
		throw DisconnectException("USER_UPDATE_FOR_N_E_U");

	// This might happen, if a users connects after the game starts.
	if (number == d->settings.users.size()) {
		UserSettings newuser;
		d->settings.users.push_back(newuser);
	}

	d->settings.users.at(number).name = packet.string();
	d->settings.users.at(number).position = packet.signed_32();
	d->settings.users.at(number).ready = packet.unsigned_8() == 1;
	if (static_cast<int32_t>(number) == d->settings.usernum) {
		d->localplayername = d->settings.users.at(number).name;
		d->settings.playernum = d->settings.users.at(number).position;
	}
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

	log("[Client]: sending timestamp: %i\n", d->game->get_gametime());

	SendPacket s;
	s.unsigned_8(NETCMD_TIME);
	s.signed_32(d->game->get_gametime());
	d->net->send(s);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = SDL_GetTicks();
}

void GameClient::syncreport() {
	assert(d->net != nullptr);
	if (d->net->is_connected()) {
		SendPacket s;
		s.unsigned_8(NETCMD_SYNCREPORT);
		s.signed_32(d->game->get_gametime());
		s.data(d->game->get_sync_hash().data, 16);
		d->net->send(s);
	}
}

/**
 * Handle one packet received from the host.
 *
 * \note The caller must handle exceptions by closing the connection.
 */
void GameClient::handle_packet(RecvPacket& packet) {
	uint8_t cmd = packet.unsigned_8();

	if (cmd == NETCMD_DISCONNECT) {
		uint8_t number = packet.unsigned_8();
		std::string reason = packet.string();
		if (number == 1)
			disconnect(reason, "", false);
		else {
			std::string arg = packet.string();
			disconnect(reason, arg, false);
		}
		return;
	}

	if (d->settings.usernum == -2) {
		if (cmd != NETCMD_HELLO)
			throw ProtocolException(cmd);
		uint8_t const version = packet.unsigned_8();
		if (version != NETWORK_PROTOCOL_VERSION)
			throw DisconnectException("DIFFERENT_PROTOCOL_VERS");
		d->settings.usernum = packet.unsigned_32();
		d->settings.playernum = -1;
		return;
	}

	switch (cmd) {
	case NETCMD_PING: {
		SendPacket s;
		s.unsigned_8(NETCMD_PONG);
		d->net->send(s);

		log("[Client] Pong!\n");
		break;
	}

	case NETCMD_SETTING_MAP: {
		d->settings.mapname = packet.string();
		d->settings.mapfilename = g_fs->FileSystem::fix_cross_file(packet.string());
		d->settings.savegame = packet.unsigned_8() == 1;
		d->settings.scenario = packet.unsigned_8() == 1;
		log("[Client] SETTING_MAP '%s' '%s'\n", d->settings.mapname.c_str(),
		    d->settings.mapfilename.c_str());

		// New map was set, so we clean up the buffer of a previously requested file
		if (file_)
			delete file_;
		break;
	}

	case NETCMD_NEW_FILE_AVAILABLE: {
		std::string path = g_fs->FileSystem::fix_cross_file(packet.string());
		uint32_t bytes = packet.unsigned_32();
		std::string md5 = packet.string();

		// Check whether the file or a file with that name already exists
		if (g_fs->file_exists(path)) {
			// If the file is a directory, we have to rename the file and replace it with the version
			// of the
			// host. If it is a ziped file, we can check, whether the host and the client have got the
			// same file.
			if (!g_fs->is_directory(path)) {
				FileRead fr;
				fr.open(*g_fs, path);
				if (bytes == fr.get_size()) {
					std::unique_ptr<char[]> complete(new char[bytes]);
					if (!complete)
						throw wexception("Out of memory");

					fr.data_complete(complete.get(), bytes);
					SimpleMD5Checksum md5sum;
					md5sum.data(complete.get(), bytes);
					md5sum.finish_checksum();
					std::string localmd5 = md5sum.get_checksum().str();
					if (localmd5 == md5)
						// everything is alright we already have the file.
						return;
				}
			}
			// Don't overwrite the file, better rename the original one
			g_fs->fs_rename(path, backup_file_name(path));
		}

		// Yes we need the file!
		SendPacket s;
		s.unsigned_8(NETCMD_NEW_FILE_AVAILABLE);
		d->net->send(s);

		if (file_)
			delete file_;

		file_ = new NetTransferFile();
		file_->bytes = bytes;
		file_->filename = path;
		file_->md5sum = md5;
		size_t position = path.rfind(g_fs->file_separator(), path.size() - 2);
		if (position != std::string::npos) {
			path.resize(position);
			g_fs->ensure_directory_exists(path);
		}
		break;
	}

	case NETCMD_FILE_PART: {
		// Only go on, if we are waiting for a file part at the moment. It can happen, that an
		// "unrequested"
		// part is send by the server if the map was changed just a moment ago and there was an
		// outstanding
		// request from the client.
		if (!file_)
			return;  // silently ignore

		uint32_t part = packet.unsigned_32();
		uint32_t size = packet.unsigned_32();

		// Send an answer
		SendPacket s;
		s.unsigned_8(NETCMD_FILE_PART);
		s.unsigned_32(part);
		s.string(file_->md5sum);
		d->net->send(s);

		FilePart fp;

		char buf[NETFILEPARTSIZE];
		assert(size <= NETFILEPARTSIZE);

		if (packet.data(buf, size) != size)
			log("Readproblem. Will try to go on anyways\n");
		memcpy(fp.part, &buf[0], size);
		file_->parts.push_back(fp);

		// Write file to disk as soon as all parts arrived
		uint32_t left = (file_->bytes - NETFILEPARTSIZE * part);
		if (left <= NETFILEPARTSIZE) {
			FileWrite fw;
			left = file_->bytes;
			uint32_t i = 0;
			// Put all data together
			while (left > 0) {
				uint32_t writeout = (left > NETFILEPARTSIZE) ? NETFILEPARTSIZE : left;
				fw.data(file_->parts[i].part, writeout, FileWrite::Pos::null());
				left -= writeout;
				++i;
			}
			// Now really write the file
			fw.write(*g_fs, file_->filename.c_str());

			// Check for consistence
			FileRead fr;
			fr.open(*g_fs, file_->filename);

			std::unique_ptr<char[]> complete(new char[file_->bytes]);

			fr.data_complete(complete.get(), file_->bytes);
			SimpleMD5Checksum md5sum;
			md5sum.data(complete.get(), file_->bytes);
			md5sum.finish_checksum();
			std::string localmd5 = md5sum.get_checksum().str();
			if (localmd5 != file_->md5sum) {
				// Something went wrong! We have to rerequest the file.
				s.reset();
				s.unsigned_8(NETCMD_NEW_FILE_AVAILABLE);
				d->net->send(s);
				// Notify the players
				s.reset();
				s.unsigned_8(NETCMD_CHAT);
				s.string(_("/me 's file failed md5 checksumming."));
				d->net->send(s);
				g_fs->fs_unlink(file_->filename);
			}
			// Check file for validity
			bool invalid = false;
			if (d->settings.savegame) {
				// Saved game check - does Widelands recognize the file as saved game?
				Widelands::Game game;
				try {
					Widelands::GameLoader gl(file_->filename, game);
				} catch (...) {
					invalid = true;
				}
			} else {
				// Map check - does Widelands recognize the file as map?
				Widelands::Map map;
				std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(file_->filename);
				if (!ml)
					invalid = true;
			}
			if (invalid) {
				g_fs->fs_unlink(file_->filename);
				// Restore original file, if there was one before
				if (g_fs->file_exists(backup_file_name(file_->filename)))
					g_fs->fs_rename(backup_file_name(file_->filename), file_->filename);
				s.reset();
				s.unsigned_8(NETCMD_CHAT);
				s.string(_("/me checked the received file. Although md5 check summing succeeded, "
				           "I can not handle the file."));
				d->net->send(s);
			}
		}
		break;
	}

	case NETCMD_SETTING_TRIBES: {
		d->settings.tribes.clear();
		for (uint8_t i = packet.unsigned_8(); i; --i) {
			TribeBasicInfo info = Widelands::get_tribeinfo(packet.string());

			// Get initializations (we have to do this locally, for translations)
			LuaInterface lua;
			info.initializations.clear();
			for (uint8_t j = packet.unsigned_8(); j; --j) {
				std::string const initialization_script = packet.string();
				std::unique_ptr<LuaTable> t = lua.run_script(initialization_script);
				t->do_not_warn_about_unaccessed_keys();
				info.initializations.push_back(TribeBasicInfo::Initialization(
				   initialization_script, t->get_string("descname"), t->get_string("tooltip")));
			}
			d->settings.tribes.push_back(info);
		}
		break;
	}

	case NETCMD_SETTING_ALLPLAYERS: {
		d->settings.players.resize(packet.unsigned_8());
		for (uint8_t i = 0; i < d->settings.players.size(); ++i)
			receive_one_player(i, packet);
		break;
	}
	case NETCMD_SETTING_PLAYER: {
		uint8_t player = packet.unsigned_8();
		receive_one_player(player, packet);
		break;
	}
	case NETCMD_SETTING_ALLUSERS: {
		d->settings.users.resize(packet.unsigned_8());
		for (uint32_t i = 0; i < d->settings.users.size(); ++i)
			receive_one_user(i, packet);
		break;
	}
	case NETCMD_SETTING_USER: {
		uint32_t user = packet.unsigned_32();
		receive_one_user(user, packet);
		break;
	}
	case NETCMD_SET_PLAYERNUMBER: {
		int32_t number = packet.signed_32();
		d->settings.playernum = number;
		d->settings.users.at(d->settings.usernum).position = number;
		break;
	}
	case NETCMD_WIN_CONDITION: {
		d->settings.win_condition_script = packet.string();
		break;
	}

	case NETCMD_LAUNCH: {
		if (!d->modal || d->game) {
			throw DisconnectException("UNEXPECTED_LAUNCH");
		}
		d->modal->end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
		break;
	}

	case NETCMD_SETSPEED:
		d->realspeed = packet.unsigned_16();
		log("[Client] speed: %u.%03u\n", d->realspeed / 1000, d->realspeed % 1000);
		break;

	case NETCMD_TIME:
		d->time.receive(packet.signed_32());
		break;

	case NETCMD_WAIT:
		log("[Client]: server is waiting.\n");
		d->server_is_waiting = true;
		break;

	case NETCMD_PLAYERCOMMAND: {
		if (!d->game)
			throw DisconnectException("PLAYERCMD_WO_GAME");

		int32_t const time = packet.signed_32();
		Widelands::PlayerCommand& plcmd = *Widelands::PlayerCommand::deserialize(packet);
		plcmd.set_duetime(time);
		d->game->enqueue_command(&plcmd);
		d->time.receive(time);
		break;
	}

	case NETCMD_SYNCREQUEST: {
		if (!d->game)
			throw DisconnectException("SYNCREQUEST_WO_GAME");
		int32_t const time = packet.signed_32();
		d->time.receive(time);
		d->game->enqueue_command(new CmdNetCheckSync(time, this));
		break;
	}

	case NETCMD_CHAT: {
		ChatMessage c;
		c.time = time(nullptr);
		c.playern = packet.signed_16();
		c.sender = packet.string();
		c.msg = packet.string();
		if (packet.unsigned_8())
			c.recipient = packet.string();
		d->chatmessages.push_back(c);
		Notifications::publish(c);
		break;
	}

	case NETCMD_SYSTEM_MESSAGE_CODE: {
		ChatMessage c;
		c.time = time(nullptr);
		std::string code = packet.string();
		std::string arg1 = packet.string();
		std::string arg2 = packet.string();
		std::string arg3 = packet.string();
		c.msg = NetworkGamingMessages::get_message(code, arg1, arg2, arg3);
		c.playern = UserSettings::none();  //  == System message
		// c.sender remains empty to indicate a system message
		d->chatmessages.push_back(c);
		Notifications::publish(c);
		break;
	}

	case NETCMD_INFO_DESYNC:
		log("[Client] received NETCMD_INFO_DESYNC. Trying to salvage some "
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
void GameClient::handle_network() {
	// if this is an internet game, handle the metaserver network
	if (internet_)
		InternetGaming::ref().handle_metaserver_communication();
	try {
		assert(d->net != nullptr);
		// Check if the connection is still open
		if (!d->net->is_connected()) {
			disconnect("CONNECTION_LOST", "", false);
			return;
		}
		// Process all available packets
		RecvPacket packet;
		while (d->net->try_receive(&packet)) {
			handle_packet(packet);
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
	log("[Client]: disconnect(%s, %s)\n", reason.c_str(), arg.c_str());

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

	if (showmsg) {
		std::string msg;
		if (arg.empty())
			msg = NetworkGamingMessages::get_message(reason);
		else
			msg = NetworkGamingMessages::get_message(reason, arg);

		if (trysave) {
			/** TRANSLATORS: %s contains an error message. */
			msg = (boost::format(_("%s An automatic savegame will be created.")) % msg).str();
		}

		UI::WLMessageBox mmb(
		   d->modal, _("Disconnected from Host"), msg, UI::WLMessageBox::MBoxType::kOk);
		mmb.run<UI::Panel::Returncodes>();
	}

	if (trysave)
		WLApplication::emergency_save(*d->game);

	if (d->modal) {
		d->modal->end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
		d->modal = nullptr;
	}
}
