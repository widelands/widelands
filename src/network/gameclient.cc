/*
 * Copyright (C) 2008-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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
#include "network/participantlist.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/launch_mpg.h"
#include "ui_fsmenu/main.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

struct AddOnsMismatchException : WLWarning {
	explicit AddOnsMismatchException(const std::string& msg) : WLWarning("", "%s", msg.c_str()) {
	}
};

struct GameClientImpl {
	bool internet_;

	GameSettings settings;

	std::string localplayername;

	std::unique_ptr<NetClientInterface> net;

	std::unique_ptr<ParticipantList> participants;

	/// Currently active modal panel. Receives an end_modal on disconnect
	UI::Panel* modal;
	UI::Panel* panel_whose_mutex_needs_resetting_on_game_start;

	/// Current game. Only non-null if a game is actually running.
	Widelands::Game* game;

	NetworkTime time;

	/// \c true if we received a message indicating that the server is waiting
	/// Send a time message as soon as we caught up to networktime
	bool server_is_waiting;

	/// Data for the last time message we sent.
	Time lasttimestamp;
	uint32_t lasttimestamp_realtime;

	/// The real target speed, in milliseconds per second.
	/// This is always set by the server
	uint32_t realspeed;

	/**
	 * The speed desired by the local player.
	 */
	uint32_t desiredspeed;

	bool should_write_replay;

	/// Backlog of chat messages
	std::vector<ChatMessage> chatmessages;

	/** File that is eventually transferred via the network if not found at the other side */
	std::unique_ptr<NetTransferFile> file_;

	AddOns::AddOnsGuard addons_guard_;

	// Whether disconnect() has been called previously.
	bool disconnect_called_;

	void send_hello() const;
	void send_player_command(Widelands::PlayerCommand* /*pc*/) const;

	void run_game(InteractiveGameBase* igb);

	InteractiveGameBase* init_game(GameClient* parent, UI::ProgressWindow& /*loader*/);
};

void GameClientImpl::send_hello() const {
	SendPacket s;
	s.unsigned_8(NETCMD_HELLO);
	s.unsigned_8(NETWORK_PROTOCOL_VERSION);
	s.string(localplayername);
	s.string(build_id());
	net->send(s);
}

void GameClientImpl::send_player_command(Widelands::PlayerCommand* pc) const {
	SendPacket s;
	s.unsigned_8(NETCMD_PLAYERCOMMAND);
	s.unsigned_32(game->get_gametime().get());
	pc->serialize(s);
	net->send(s);
}

/**
 * Show progress dialog and load map or saved game.
 */
InteractiveGameBase* GameClientImpl::init_game(GameClient* parent, UI::ProgressWindow& loader) {
	modal = &loader;

	Notifications::publish(UI::NoteLoadingMessage(_("Preparing game…")));

	game->set_game_controller(parent->get_pointer());
	uint8_t const pn = settings.playernum + 1;
	game->save_handler().set_autosave_filename(
	   format("%s_netclient%u", kAutosavePrefix, static_cast<unsigned int>(pn)));
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

	game->run(settings.savegame ? Widelands::Game::StartGameType::kSaveGame :
	          settings.scenario ? Widelands::Game::StartGameType::kMultiPlayerScenario :
                                 Widelands::Game::StartGameType::kMap,
	          "", format("netclient_%d", static_cast<int>(settings.usernum)));

	// if this is an internet game, tell the metaserver that the game is done.
	if (internet_) {
		InternetGaming::ref().set_game_done();
	}
	modal = nullptr;
	game = nullptr;
}

GameClient::GameClient(FsMenu::MenuCapsule& c,
                       std::shared_ptr<GameController>& ptr,
                       const std::pair<NetAddress, NetAddress>& host,
                       const std::string& playername,
                       bool internet,
                       const std::string& gamename)
   : d(new GameClientImpl), capsule_(c), pointer_(ptr) {

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
	d->panel_whose_mutex_needs_resetting_on_game_start = nullptr;
	d->should_write_replay = true;
	d->game = nullptr;
	d->realspeed = 0;
	d->desiredspeed = 1000;
	d->file_ = nullptr;
	d->disconnect_called_ = false;

	// Get the default win condition script
	d->settings.win_condition_script = d->settings.win_condition_scripts.front();

	d->participants.reset(new ParticipantList(&(d->settings), d->game, d->localplayername));
	participants_ = d->participants.get();

	run();
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
	d->modal = new FsMenu::LaunchMPG(capsule_, *this, *this, *this, d->internet_);

	// The main menu's think() loop creates a mutex lock that would permanently block the Game
	// from locking this mutex. So when the game starts we'll need to break the lock by force.
	for (UI::Panel* p = d->modal; p != nullptr; p = p->get_parent()) {
		if (p->is_modal()) {
			d->panel_whose_mutex_needs_resetting_on_game_start = p;
			return;
		}
	}
	NEVER_HERE();
}

void GameClient::set_write_replay(bool replay) {
	d->should_write_replay = replay;
}

void GameClient::do_run(RecvPacket& packet) {
	d->server_is_waiting = true;

	Widelands::Game game;
	game.set_write_replay(d->should_write_replay);
	game.set_write_syncstream(get_config_bool("write_syncstreams", true));
	game.logic_rand_seed(packet.unsigned_32());

	game.enabled_addons().clear();
	for (size_t i = packet.unsigned_32(); i != 0u; --i) {
		const std::string name = packet.string();
		bool found = false;
		for (const auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == name) {
				game.enabled_addons().push_back(pair.first);
				found = true;
				break;
			}
		}
		if (!found) {
			throw wexception("Add-on %s required by host not found", name.c_str());
		}
	}

	capsule_.set_visible(false);
	try {
		std::vector<std::string> tipstexts{"general_game", "multiplayer"};
		if (has_players_tribe()) {
			tipstexts.push_back(get_players_tribe());
		}
		UI::ProgressWindow& loader_ui = game.create_loader_ui(
		   tipstexts, true, d->settings.map_theme, d->settings.map_background, true);

		d->game = &game;
		InteractiveGameBase* igb = d->init_game(this, loader_ui);
		if (d->panel_whose_mutex_needs_resetting_on_game_start != nullptr) {
			d->panel_whose_mutex_needs_resetting_on_game_start->clear_current_think_mutex();
			d->panel_whose_mutex_needs_resetting_on_game_start = nullptr;
		}
		d->run_game(igb);

	} catch (const WLWarning& e) {
		WLApplication::emergency_save(&capsule_.menu(), game, e.what(), 1, true, false);
		d->game = nullptr;
	} catch (const std::exception& e) {
		FsMenu::MainMenu& parent = capsule_.menu();  // make includes script happy
		WLApplication::emergency_save(&parent, game, e.what());
		d->game = nullptr;
		disconnect("CLIENT_CRASHED");
		if (d->internet_) {
			InternetGaming::ref().logout("CLIENT_CRASHED");
		}
	}
	d->modal = nullptr;

	// Quit
	capsule_.die();
}

void GameClient::think() {
	NoteThreadSafeFunction::instantiate([this]() { handle_network(); }, true, false);

	while (!pending_player_commands_.empty()) {
		MutexLock m(MutexLock::ID::kCommands);
		do_send_player_command(pending_player_commands_.front());
		pending_player_commands_.pop_front();
	}

	if (d->game != nullptr) {
		// TODO(Klaus Halfmann): what kind of time tricks are done here?
		if (d->realspeed == 0 || d->server_is_waiting) {
			d->time.fastforward();
		} else {
			d->time.think(d->realspeed);
		}

		if (d->server_is_waiting && d->game->get_gametime() == d->time.networktime()) {
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
	pending_player_commands_.push_back(pc);
}

void GameClient::do_send_player_command(Widelands::PlayerCommand* pc) {
	assert(d->game);

	// TODO(Klaus Halfmann): should this be an assert?
	if (pc->sender() == d->settings.playernum + 1)  //  allow command for current player only
	{
		verb_log_info("[Client]: enqueue playercommand at time %i\n", d->game->get_gametime().get());

		d->send_player_command(pc);

		d->lasttimestamp = d->game->get_gametime();
		d->lasttimestamp_realtime = SDL_GetTicks();
	} else {
		log_warn("[Client]: Playercommand is not for current player? %i\n", pc->sender());
	}

	delete pc;
}

Duration GameClient::get_frametime() {
	return Time(d->time.time()) - d->game->get_gametime();
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

void GameClient::set_scenario(bool /*set*/) {
}

bool GameClient::can_change_map() {
	return false;
}

bool GameClient::can_change_player_state(uint8_t const /*number*/) {
	return false;
}

bool GameClient::can_change_player_tribe(uint8_t number) {
	return can_change_player_team(number);
}

bool GameClient::can_change_player_team(uint8_t number) {
	return (number == d->settings.playernum) && !d->settings.scenario && !d->settings.savegame;
}

bool GameClient::can_change_player_init(uint8_t /*number*/) {
	return false;
}

bool GameClient::can_launch() {
	return false;
}

void GameClient::set_player_state(uint8_t /*number*/, PlayerSettings::State /* state */) {
	// client is not allowed to do this
}

void GameClient::set_player_ai(uint8_t /*number*/,
                               const std::string& /* ai */,
                               bool const /* random_ai */) {
	// client is not allowed to do this
}

void GameClient::next_player_state(uint8_t /* state */) {
	// client is not allowed to do this
}

void GameClient::set_map(const std::string& /*mapname*/,
                         const std::string& /*mapfilename*/,
                         const std::string& /*map_theme*/,
                         const std::string& /*map_bg*/,
                         uint32_t /*maxplayers*/,
                         bool /*savegame*/) {
	// client is not allowed to do this
}

void GameClient::send_cheating_info() {
	SendPacket packet;
	packet.unsigned_8(NETCMD_SYSTEM_MESSAGE_CODE);
	packet.string("CHEAT");
	packet.string(d->localplayername);
	packet.string("");
	packet.string("");
	d->net->send(packet);
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

void GameClient::set_player_color(const uint8_t number, const RGBColor& c) {
	if ((number != d->settings.playernum)) {
		return;
	}

	SendPacket s;
	s.unsigned_8(NETCMD_SETTING_CHANGECOLOR);
	s.unsigned_8(number);
	s.unsigned_8(c.r);
	s.unsigned_8(c.g);
	s.unsigned_8(c.b);
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

void GameClient::set_player_closeable(uint8_t /*number*/, bool /*closeable*/) {
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

void GameClient::set_player_name(uint8_t /*number*/, const std::string& /* name */) {
	// until now the name is set before joining - if you allow a change in
	// launchgame-menu, here properly should be a set_name function
}

void GameClient::set_player(uint8_t /*number*/, const PlayerSettings& /* settings */) {
	// do nothing here - the request for a positionchange is send in
	// set_player_number(uint8_t) to the host.
}

void GameClient::set_flag(GameSettings::Flags flag, bool state) {
	if (state) {
		d->settings.flags |= flag;
	} else {
		d->settings.flags &= ~flag;
	}
}

bool GameClient::get_flag(GameSettings::Flags flag) {
	return (d->settings.flags & flag) != 0;
}

std::string GameClient::get_win_condition_script() {
	return d->settings.win_condition_script;
}

int32_t GameClient::get_win_condition_duration() {
	return d->settings.win_condition_duration;
}

void GameClient::set_win_condition_script(const std::string& /* wc */) {
	// Clients are not allowed to change this
	NEVER_HERE();
}

void GameClient::set_win_condition_duration(const int32_t /* duration */) {
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
	player.random_tribe = (packet.unsigned_8() != 0u);
	player.initialization_index = packet.unsigned_8();
	player.ai = packet.string();
	player.random_ai = (packet.unsigned_8() != 0u);
	player.team = packet.unsigned_8();
	player.shared_in = packet.unsigned_8();
	player.color.r = packet.unsigned_8();
	player.color.g = packet.unsigned_8();
	player.color.b = packet.unsigned_8();
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer, number));
}

void GameClient::receive_one_user(uint32_t const number, StreamRead& packet) {
	if (number > d->settings.users.size()) {
		throw DisconnectException("USER_UPDATE_FOR_N_E_U");
	}

	// This might happen, if a users connects after the game starts.
	if (number == d->settings.users.size()) {
		d->settings.users.emplace_back();
	}

	d->settings.users.at(number).name = packet.string();
	d->settings.users.at(number).position = packet.signed_32();
	d->settings.users.at(number).ready = (packet.unsigned_8() != 0u);

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

	verb_log_info("[Client]: sending timestamp: %i", d->game->get_gametime().get());

	SendPacket s;
	s.unsigned_8(NETCMD_TIME);
	s.unsigned_32(d->game->get_gametime().get());
	d->net->send(s);

	d->lasttimestamp = d->game->get_gametime();
	d->lasttimestamp_realtime = SDL_GetTicks();
}

void GameClient::sync_report_callback() {
	assert(d->net != nullptr);
	if (d->net->is_connected()) {
		SendPacket s;
		s.unsigned_8(NETCMD_SYNCREPORT);
		s.unsigned_32(d->game->get_gametime().get());
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

	d->addons_guard_.reset();
	std::vector<AddOns::AddOnState> new_g_addons;
	std::map<std::string, std::shared_ptr<AddOns::AddOnInfo>> disabled_installed_addons;
	for (const auto& pair : AddOns::g_addons) {
		disabled_installed_addons[pair.first->internal_name] = pair.first;
	}
	std::set<std::string> missing_addons;
	std::map<std::string, std::pair<std::string /* installed */, std::string /* host */>>
	   wrong_version_addons;
	for (size_t i = packet.unsigned_32(); i != 0u; --i) {
		const std::string name = packet.string();
		disabled_installed_addons.erase(name);
		const AddOns::AddOnVersion v = AddOns::string_to_version(packet.string());
		AddOns::AddOnVersion found;
		for (const auto& pair : AddOns::g_addons) {
			if (pair.first->internal_name == name) {
				found = pair.first->version;
				new_g_addons.emplace_back(pair.first, true);
				break;
			}
		}
		if (found.empty()) {
			missing_addons.insert(name);
		} else if (found != v) {
			wrong_version_addons[name] = std::make_pair(
			   AddOns::version_to_string(found, false), AddOns::version_to_string(v, false));
		}
	}
	if (!missing_addons.empty() || !wrong_version_addons.empty()) {
		const size_t nr = missing_addons.size() + wrong_version_addons.size();
		std::string message = format(
		   ngettext("%u add-on mismatch detected:\n", "%u add-on mismatches detected:\n", nr), nr);
		for (const std::string& name : missing_addons) {
			message = format(_("%1$s\n· ‘%2$s’ required by host not found"), message, name);
		}
		for (const auto& pair : wrong_version_addons) {
			message = format(_("%1$s\n· ‘%2$s’ installed at version %3$s but host uses version %4$s"),
			                 message, pair.first, pair.second.first, pair.second.second);
		}
		throw AddOnsMismatchException(message);
	}
	for (const auto& pair : disabled_installed_addons) {
		new_g_addons.emplace_back(pair.second, false);
	}
	AddOns::g_addons = new_g_addons;
}

/**
 * Give a pong for a ping
 */
void GameClient::handle_ping(RecvPacket& /* packet */) {
	SendPacket s;
	s.unsigned_8(NETCMD_PONG);
	d->net->send(s);

	verb_log_info("[Client] Pong!");
}

/**
 * New Map name was sent.
 */
void GameClient::handle_setting_map(RecvPacket& packet) {
	d->settings.mapname = packet.string();
	d->settings.mapfilename = g_fs->FileSystem::fix_cross_file(packet.string());
	d->settings.map_theme = packet.string();
	d->settings.map_background = packet.string();
	d->settings.savegame = (packet.unsigned_8() != 0u);
	d->settings.scenario = (packet.unsigned_8() != 0u);
	verb_log_info("[Client] SETTING_MAP '%s' '%s'", d->settings.mapname.c_str(),
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
	size_t position = path.rfind(FileSystem::file_separator(), path.size() - 2);
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
		fw.write(*g_fs, d->file_->filename);

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
			s.string(_("/me ’s file failed md5 checksumming."));
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
		// Notify UI to refresh the map data
		Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));
	}
}

/**
 *
 */
void GameClient::handle_setting_tribes(RecvPacket& packet) {
	d->settings.tribes.clear();
	Widelands::AllTribes all_tribes = Widelands::get_all_tribeinfos(nullptr);
	for (uint8_t i = packet.unsigned_8(); i != 0u; --i) {
		Widelands::TribeBasicInfo info = Widelands::get_tribeinfo(packet.string(), all_tribes);

		// Get initializations (we have to do this locally, for translations)
		LuaInterface lua;
		info.initializations.clear();
		for (uint8_t j = packet.unsigned_8(); j > 0; --j) {
			std::string const initialization_script = packet.string();
			std::unique_ptr<LuaTable> t = lua.run_script(initialization_script);
			t->do_not_warn_about_unaccessed_keys();
			// TODO(hessenfarmer): Needs to be pulled out as it is duplicated to tribe_basic_info.cc
			std::set<std::string> tags;
			std::set<std::string> incompatible_wc;
			if (t->has_key("map_tags")) {
				std::unique_ptr<LuaTable> tt = t->get_table("map_tags");
				for (int key : tt->keys<int>()) {
					tags.insert(tt->get_string(key));
				}
			}
			if (t->has_key("incompatible_wc")) {
				std::unique_ptr<LuaTable> w = t->get_table("incompatible_wc");
				for (int key : w->keys<int>()) {
					incompatible_wc.insert(w->get_string(key));
				}
			}
			info.initializations.emplace_back(
			   initialization_script, t->get_string("descname"), t->get_string("tooltip"), tags,
			   incompatible_wc,
			   !t->has_key("uses_map_starting_position") || t->get_bool("uses_map_starting_position"));
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
	if (participants_ != nullptr) {
		participants_->participants_updated();
	}
}

/**
 *
 */
void GameClient::handle_playercommand(RecvPacket& packet) {
	if (d->game == nullptr) {
		throw DisconnectException("PLAYERCMD_WO_GAME");
	}

	const Time time(packet.unsigned_32());
	Widelands::PlayerCommand& plcmd = *Widelands::PlayerCommand::deserialize(packet);
	plcmd.set_duetime(time);
	d->game->enqueue_command(&plcmd);
	d->time.receive(time);
}

/**
 *
 */
void GameClient::handle_syncrequest(RecvPacket& packet) {
	if (d->game == nullptr) {
		throw DisconnectException("SYNCREQUEST_WO_GAME");
	}
	const Time time(packet.unsigned_32());
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
	if (packet.unsigned_8() != CHATTYPE_PUBLIC) {
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
void GameClient::handle_desync(RecvPacket& /* packet */) {
	log_err("[Client] received NETCMD_INFO_DESYNC. Trying to salvage some "
	        "information for debugging.\n");
	if (d->game != nullptr) {
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
		if (participants_ != nullptr) {
			participants_->participants_updated();
		}
	} break;
	case NETCMD_SETTING_ALLUSERS: {
		d->settings.users.resize(packet.unsigned_8());
		for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
			receive_one_user(i, packet);
		}
		if (participants_ != nullptr) {
			participants_->participants_updated();
		}
	} break;
	case NETCMD_SETTING_USER: {
		uint32_t user = packet.unsigned_32();
		receive_one_user(user, packet);
		if (participants_ != nullptr) {
			participants_->participants_updated();
		}
	} break;
	case NETCMD_SET_PLAYERNUMBER: {
		int32_t number = packet.signed_32();
		d->settings.playernum = number;
		d->settings.users.at(d->settings.usernum).position = number;
		if (participants_ != nullptr) {
			participants_->participants_updated();
		}
	} break;
	case NETCMD_WIN_CONDITION:
		d->settings.win_condition_script = g_fs->FileSystem::fix_cross_file(packet.string());
		break;
	case NETCMD_WIN_CONDITION_DURATION:
		d->settings.win_condition_duration = packet.signed_32();
		break;
	case NETCMD_GAMEFLAGS:
		d->settings.flags = packet.unsigned_16();
		break;
	case NETCMD_LAUNCH:
		if ((d->game != nullptr) || ((d->modal != nullptr) && d->modal->is_modal())) {
			throw DisconnectException("UNEXPECTED_LAUNCH");
		}
		do_run(packet);
		break;
	case NETCMD_SETSPEED:
		d->realspeed = packet.unsigned_16();
		verb_log_info("[Client] speed: %u.%03u", d->realspeed / 1000, d->realspeed % 1000);
		break;
	case NETCMD_TIME:
		d->time.receive(Time(packet.unsigned_32()));
		break;
	case NETCMD_WAIT:
		verb_log_info("[Client]: server is waiting.");
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
		// Process all available packets
		std::unique_ptr<RecvPacket> packet = d->net->try_receive();
		while (packet) {
			handle_packet(*packet);
			packet = d->net->try_receive();
		}
		// Check if the connection is still open
		if (!d->net->is_connected()) {
			disconnect("CONNECTION_LOST", "", false);
			return;
		}
	} catch (const AddOnsMismatchException& e) {
		disconnect("SOMETHING_WRONG", e.what());
	} catch (const WLWarning& e) {
		// disconnect() should have been called already, but just in case:
		disconnect("SOMETHING_WRONG", e.what());
		// the Warning is intended for the caller
		throw;
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
	log_warn("[Client]: disconnect(%s, %s)", reason.c_str(), arg.c_str());
	if (d->disconnect_called_) {
		return;
	}
	d->disconnect_called_ = true;

	assert(d->net != nullptr);
	if (d->net->is_connected()) {
		if (sendreason) {
			SendPacket s;
			s.unsigned_8(NETCMD_DISCONNECT);
			s.unsigned_8(arg.empty() ? 1 : 2);
			s.string(reason);
			if (!arg.empty()) {
				s.string(arg);
			}
			d->net->send(s);
		}

		d->net->close();
	}

	if (showmsg) {
		if (d->game != nullptr) {
			if (reason == "KICKED" || reason == "SERVER_LEFT" || reason == "SERVER_CRASHED") {
				throw WLWarning("", "%s",
				                arg.empty() ? NetworkGamingMessages::get_message(reason).c_str() :
                                          NetworkGamingMessages::get_message(reason, arg).c_str());
			}
			throw wexception("%s", arg.empty() ?
                                   NetworkGamingMessages::get_message(reason).c_str() :
                                   NetworkGamingMessages::get_message(reason, arg).c_str());
		}
		capsule_.menu().show_messagebox(
		   _("Disconnected"),
		   format(_("The connection with the host was lost for the following reason:\n%s"),
		          (arg.empty() ? NetworkGamingMessages::get_message(reason) :
                               NetworkGamingMessages::get_message(reason, arg))));
	}

	// TODO(Klaus Halfmann): Some of the modal windows are now handled by unique_ptr resulting in a
	// double free.
	if (d->modal != nullptr) {
		d->modal->die();
	}
	d->modal = nullptr;
}
