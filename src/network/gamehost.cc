/*
 * Copyright (C) 2008-2021 by the Widelands Development Team
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

#include "network/gamehost.h"

#include <algorithm>
#include <cstdlib>
#include <memory>

#include <SDL_timer.h>
#ifndef _WIN32
#include <unistd.h>  // for usleep
#endif

#include "ai/computer_player.h"
#include "ai/defaultai.h"
#include "base/i18n.h"
#include "base/log.h"
#include "base/md5.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "build_info.h"
#include "chat/chat.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/playersmanager.h"
#include "network/constants.h"
#include "network/host_game_settings_provider.h"
#include "network/internet_gaming.h"
#include "network/nethost.h"
#include "network/nethostproxy.h"
#include "network/network_gaming_messages.h"
#include "network/network_lan_promotion.h"
#include "network/network_player_settings_backend.h"
#include "network/network_protocol.h"
#include "network/participantlist.h"
#include "ui_basic/progresswindow.h"
#include "ui_fsmenu/launch_mpg.h"
#include "ui_fsmenu/main.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/interactive_spectator.h"

struct HostChatProvider : public ChatProvider {
	explicit HostChatProvider(GameHost* const init_host) : h(init_host), kickClient(0) {
	}

	// TODO(k.halfmann): this deserves a refactoring
	void send(const std::string& msg) override {
		ChatMessage c(msg);
		c.playern = h->get_local_playerposition();
		c.sender = h->get_local_playername();
		if (!c.msg.empty() && *c.msg.begin() == '@') {
			// Personal message
			std::string::size_type const space = c.msg.find(' ');
			if (space >= c.msg.size() - 1) {
				return;
			}
			c.recipient = c.msg.substr(1, space - 1);
			c.msg = c.msg.substr(space + 1);
		}

		/* Handle hostcommands like:
		 * /help                : Shows all available commands
		 * /announce <msg>      : Send a chatmessage as announcement (system chat)
		 * /warn <name> <reason>: Warn the user <name> because of <reason>
		 * /kick <name> <reason>: Kick the user <name> because of <reason>
		 * /forcePause         : Force the game to pause.
		 * /endForcedPause      : Puts game back to normal speed.
		 */
		else if (c.msg.size() > 1 && *c.msg.begin() == '/') {

			// Split up in "cmd" "arg1" "arg2"
			std::string cmd, arg1, arg2;
			std::string temp = c.msg.substr(1);  // cut off '/'
			h->split_command_array(temp, cmd, arg1, arg2);
			verb_log_info("%s + \"%s\" + \"%s\"\n", cmd.c_str(), arg1.c_str(), arg2.c_str());

			// let "/me" pass - handled by chat
			if (cmd == "me") {
				h->send(c);
				return;
			}

			// Everything handled from now on will be system stuff - and so will be
			// messages send because of that commands
			c.playern = -2;
			c.sender = "";

			// Help
			if (cmd == "help") {
				c.msg = bformat(
				   "<br>%s<br>%s<br>%s<br>%s<br>%s<br>%s<br>%s", _("Available host commands are:"),
				   /** TRANSLATORS: Available host command */
				   _("/help  -  Shows this help"),
				   /** TRANSLATORS: Available host command */
				   _("/announce <msg>  -  Send a chatmessage as announcement (system chat)"),
				   /** TRANSLATORS: Available host command */
				   _("/warn <name> <reason>  -  Warn the user <name> because of <reason>"),
				   /** TRANSLATORS: Available host command */
				   _("/kick <name> <reason>  -  Kick the user <name> because of <reason>"),
				   /** TRANSLATORS: Available host command */
				   _("/forcePause            -  Force the game to pause."),
				   /** TRANSLATORS: Available host command */
				   _("/endForcedPause        -  Return game to normal speed."));
			}

			// Announce
			else if (cmd == "announce") {
				if (arg1.empty()) {
					c.msg = _("Wrong use, should be: /announce <message>");
				} else {
					if (!arg2.empty()) {
						arg1 += " " + arg2;
					}
					c.msg = "HOST ANNOUNCEMENT: " + arg1;
				}
			}

			// Warn user
			else if (cmd == "warn") {
				if (arg1.empty() || arg2.empty()) {
					c.msg = _("Wrong use, should be: /warn <name> <reason>");
				} else {
					int32_t num = h->check_client(arg1);
					if (num == -1) {
						c.msg = bformat(_("The client %s could not be found."), arg1);
					} else {
						c.msg = bformat("HOST WARNING FOR %s: ", arg1);
						c.msg += arg2;
					}
				}
			}

			// Kick
			else if (cmd == "kick") {
				if (arg1.empty()) {
					c.msg = _("Wrong use, should be: /kick <name> <reason>");
				} else {
					kickUser = arg1;
					if (!arg2.empty()) {
						kickReason = arg2;
					} else {
						kickReason = "No reason given!";
					}
					// Check if client exists
					int32_t num = h->check_client(kickUser);
					if (num == -2) {
						c.msg = _("You can not kick yourself!");
					} else if (num == -1) {
						c.msg = bformat(_("The client %s could not be found."), arg1);
					} else {
						kickClient = num;
						c.msg = bformat(_("Are you sure you want to kick %s?"), arg1) + "<br>";
						c.msg += bformat(_("The stated reason was: %s"), kickReason) + "<br>";
						c.msg += bformat(_("If yes, type: /ack_kick %s"), arg1);
					}
				}
			}

			// Acknowledge kick
			else if (cmd == "ack_kick") {
				if (arg1.empty()) {
					c.msg = _("Kick acknowledgement cancelled: No name given!");
				} else if (!arg2.empty()) {
					c.msg = _("Wrong use, should be: /ack_kick <name>");
				} else {
					if (arg1 == kickUser) {
						h->kick_user(kickClient, kickReason);
						return;
					} else {
						c.msg = _("Kick acknowledgement cancelled: Wrong name given!");
					}
				}
				kickUser = "";
				kickReason = "";
			}

			// Force Pause
			else if (cmd == "forcePause") {
				if (h->forced_pause()) {
					c.msg = _("Pause was already forced - game should be paused.");
				} else {
					c.msg = "HOST FORCED THE GAME TO PAUSE!";
					h->force_pause();
				}
			}

			// End Forced Pause
			else if (cmd == "endForcedPause") {
				if (!h->forced_pause()) {
					c.msg = _("There is no forced pause - nothing to end.");
				} else {
					c.msg = "HOST ENDED THE FORCED GAME PAUSE!";
					h->end_forced_pause();
				}
			}

			// Default
			else {
				c.msg = _("Invalid command! Type /help for a list of commands.");
			}
		}
		h->send(c);
	}

	const std::vector<ChatMessage>& get_messages() const override {
		return messages;
	}

	void receive(const ChatMessage& msg) {
		messages.push_back(msg);
		Notifications::publish(msg);
	}

	bool has_been_set() const override {
		return true;
	}

private:
	GameHost* h;
	std::vector<ChatMessage> messages;
	std::string kickUser;
	uint32_t kickClient;
	std::string kickReason;
};

struct Client {
	NetHostInterface::ConnectionId sock_id;
	// TODO(Notabilis): This should probably be PlayerSlot or Widelands::PlayerNumber
	uint8_t playernum;
	// TODO(Notabilis): usernum is int16_t while UserSettings::position is uint8_t.
	//                  Unify this and replace with PlayerSlot or Widelands::PlayerNumber
	int16_t usernum;
	std::string build_id;
	Md5Checksum syncreport;
	bool syncreport_arrived;
	Time time;  // last time report
	uint32_t desiredspeed;
	time_t hung_since;
	/// The delta time where the last information about the hung client was sent to the other clients
	/// relative
	/// to when the last answer of the client was received.
	time_t lastdelta;
};

struct GameHostImpl {
	GameSettings settings;
	std::string localplayername;
	uint32_t localdesiredspeed;
	// unique_ptr instead of object to break cyclic dependency
	std::unique_ptr<ParticipantList> participants;
	HostChatProvider chat;
	HostGameSettingsProvider hp;
	NetworkPlayerSettingsBackend npsb;

	std::unique_ptr<LanGamePromoter> promoter;
	std::unique_ptr<NetHostInterface> net;

	/// List of connected clients. Note that clients are not in the same
	/// order as players. In fact, a client may not be assigned to a player.
	std::vector<Client> clients;

	/// The game itself; only non-null while game is running
	Widelands::Game* game;

	/// If we were to send out a plain networktime packet, this would be the
	/// time. However, we have not yet committed to this networktime.
	Time pseudo_networktime;
	int32_t last_heartbeat;

	/// The networktime we committed to by sending it across the network.
	Time committed_networktime;

	/// This is the time for local simulation
	NetworkTime time;

	/// Whether we're waiting for all clients to report back.
	bool waiting;
	uint32_t lastframe;

	/**
	 * The speed, in milliseconds per second, that is effective as long
	 * as we're not \ref waiting.
	 */
	uint32_t networkspeed;
	time_t lastpauseping;

	/// All currently running computer players, *NOT* in one-one correspondence
	/// with \ref Player objects
	std::vector<AI::ComputerPlayer*> computerplayers;

	/// \c true if a syncreport is currently in flight
	bool syncreport_pending;
	Time syncreport_time;
	Md5Checksum syncreport;
	bool syncreport_arrived;

	explicit GameHostImpl(GameHost* const h)
	   : localdesiredspeed(0),
	     participants(nullptr),
	     chat(h),
	     hp(h),
	     npsb(&hp),
	     promoter(),
	     net(),
	     game(nullptr),
	     pseudo_networktime(0),
	     last_heartbeat(0),
	     committed_networktime(0),
	     waiting(false),
	     lastframe(0),
	     networkspeed(0),
	     lastpauseping(0),
	     syncreport_pending(false),
	     syncreport_time(0),
	     syncreport(),
	     syncreport_arrived(false) {
	}

	/// Takes ownership of the given pointer
	void set_participant_list(ParticipantList* p) {
		participants.reset(p);
		chat.participants_ = p;
	}
};

GameHost::GameHost(FsMenu::MenuCapsule* c,
                   std::shared_ptr<GameController>& ptr,
                   const std::string& playername,
                   std::vector<Widelands::TribeBasicInfo> tribeinfos,
                   bool internet)
   : capsule_(c),
     pointer_(ptr),
     d(new GameHostImpl(this)),
     internet_(internet),
     forced_pause_(false) {
	verb_log_info("[Host]: starting up.");

	d->localplayername = playername;

	// create a listening socket
	if (internet) {
		// No real listening socket. Instead, connect to the relay server
		d->net = NetHostProxy::connect(InternetGaming::ref().ips(),
		                               InternetGaming::ref().get_local_servername(),
		                               InternetGaming::ref().relay_password());
		if (d->net == nullptr) {
			// Some kind of problem with the relay server. Bad luck :(
			throw WLWarning(_("Failed to host the server!"),
			                _("Widelands could not start hosting a server.\n"
			                  "This should not happen and there is most likely "
			                  "nothing you can do about it. Please report a bug."));
		}
	} else {
		d->net = NetHost::listen(kWidelandsLanPort);
		if (d->net == nullptr) {
			// This might happen when the widelands socket is already in use
			throw WLWarning(_("Failed to start the server!"),
			                _("Widelands could not start a server.\n"
			                  "Probably some other process is already running a server on our port."));
		}
		d->promoter.reset(new LanGamePromoter());
	}
	d->game = nullptr;
	d->pseudo_networktime = Time(0);
	d->waiting = true;
	d->networkspeed = 1000;
	d->localdesiredspeed = 1000;
	d->syncreport_pending = false;
	d->syncreport_time = Time(0);

	assert(!tribeinfos.empty());
	d->settings.tribes = std::move(tribeinfos);

	set_multiplayer_game_settings();
	d->settings.playernum = UserSettings::none();
	d->settings.usernum = 0;
	UserSettings hostuser;
	hostuser.name = playername;
	hostuser.position = UserSettings::none();
	hostuser.ready = true;
	d->settings.users.push_back(hostuser);
	file_.reset(nullptr);  //  Initialize as 0 pointer - unfortunately needed in struct.

	d->set_participant_list(new ParticipantList(&(d->settings), d->game, d->localplayername));

	if (capsule_) {
		run();
	}
}

GameHost::~GameHost() {
	clear_computer_players();

	while (!d->clients.empty()) {
		disconnect_client(0, "SERVER_LEFT");
		reaper();
	}

	// close all open sockets
	d->net.reset();
	d->promoter.reset();
	delete d;
}

const std::string& GameHost::get_local_playername() const {
	return d->localplayername;
}

int16_t GameHost::get_local_playerposition() {
	return d->settings.users.at(0).position;
}

void GameHost::clear_computer_players() {
	for (AI::ComputerPlayer* ai : d->computerplayers) {
		delete ai;
	}
	d->computerplayers.clear();
}

void GameHost::init_computer_player(Widelands::PlayerNumber p) {
	d->computerplayers.push_back(
	   AI::ComputerPlayer::get_implementation(d->game->get_player(p)->get_ai())
	      ->instantiate(*d->game, p));
}

void GameHost::replace_client_with_ai(uint8_t playernumber, const std::string& ai) {
	assert(d->game->get_player(playernumber + 1)->get_ai().empty());
	assert(d->game->get_player(playernumber + 1)->get_ai() ==
	       d->settings.players.at(playernumber).ai);
	// Inform all players about the change
	// Has to be done at first in this method since the calls later on overwrite players[].name
	send_system_message_code("CLIENT_X_REPLACED_WITH", d->settings.players.at(playernumber).name,
	                         AI::ComputerPlayer::get_implementation(ai)->descname);
	set_player_ai(playernumber, ai, false);
	d->game->get_player(playernumber + 1)->set_ai(ai);
	// Activate the ai
	init_computer_player(playernumber + 1);
	set_player_state(playernumber, PlayerSettings::State::kComputer);
	assert(d->game->get_player(playernumber + 1)->get_ai() ==
	       d->settings.players.at(playernumber).ai);
}

void GameHost::init_computer_players() {
	const Widelands::PlayerNumber nr_players = d->game->map().get_nrplayers();
	iterate_players_existing_novar(p, nr_players, *d->game) {
		if (p == d->settings.playernum + 1) {
			continue;
		}

		uint32_t client;
		for (client = 0; client < d->clients.size(); ++client) {
			if (d->clients.at(client).playernum + 1 == p) {
				break;
			}
		}

		if (client >= d->clients.size()) {
			init_computer_player(p);
		}
	}
}

void GameHost::run() {
	game_.reset(new Widelands::Game());
	// Fill the list of possible system messages
	NetworkGamingMessages::fill_map();
	new FsMenu::LaunchMPG(
	   *capsule_, d->hp, *this, d->chat, *game_, internet_, [this]() { run_callback(); });
}

void GameHost::run_direct() {
	game_.reset(new Widelands::Game());
	// Fill the list of possible system messages
	NetworkGamingMessages::fill_map();
	run_callback();
}

// TODO(k.halfmann): refactor into smaller functions
void GameHost::run_callback() {
	// if this is an internet game, tell the metaserver that the game started
	if (internet_) {
		InternetGaming::ref().set_game_playing();
	} else {
		// if it is a LAN game, no longer accept new clients
		dynamic_cast<NetHost*>(d->net.get())->stop_listening();
		d->promoter.reset();
	}

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients.at(i).playernum == UserSettings::not_connected()) {
			disconnect_client(i, "GAME_STARTED_AT_CONNECT");
		}
	}

	SendPacket packet;
	packet.unsigned_8(NETCMD_LAUNCH);
	packet.unsigned_32(game_->enabled_addons().size());
	for (const auto& a : game_->enabled_addons()) {
		packet.string(a->internal_name);
	}
	broadcast(packet);

	game_->set_ai_training_mode(get_config_bool("ai_training", false));
	game_->set_auto_speed(get_config_bool("auto_speed", false));
	game_->set_write_syncstream(get_config_bool("write_syncstreams", true));

	if (capsule_) {
		capsule_->set_visible(false);
	}
	uint8_t player_number = 1;
	try {
		std::vector<std::string> tipstexts{"general_game", "multiplayer"};
		if (d->hp.has_players_tribe()) {
			tipstexts.push_back(d->hp.get_players_tribe());
		}
		game_->create_loader_ui(tipstexts, true, d->settings.map_theme, d->settings.map_background);
		Notifications::publish(UI::NoteLoadingMessage(_("Preparing game…")));

		d->game = game_.get();
		game_->set_game_controller(pointer_);
		InteractiveGameBase* igb;
		player_number = d->settings.playernum + 1;
		game_->save_handler().set_autosave_filename(bformat("%s_nethost", kAutosavePrefix));

		if (d->settings.savegame) {
			// Read and broadcast original win condition
			Widelands::GameLoader gl(d->settings.mapfilename, *game_);
			Widelands::GamePreloadPacket gpdp;
			gl.preload_game(gpdp);

			set_win_condition_script(gpdp.get_win_condition());
		}

		if ((player_number > 0) && (player_number <= UserSettings::highest_playernum())) {
			igb = new InteractivePlayer(*game_, get_config_section(), player_number, true, &d->chat);
		} else {
			igb = new InteractiveSpectator(*game_, get_config_section(), true, &d->chat);
			player_number = 1;  // for the emergency save later
		}
		game_->set_ibase(igb);

		if (!d->settings.savegame) {  // new game
			game_->init_newgame(d->settings);
		} else {  // savegame
			game_->init_savegame(d->settings);
		}
		d->pseudo_networktime = game_->get_gametime();
		d->time.reset(d->pseudo_networktime);
		d->lastframe = SDL_GetTicks();
		d->last_heartbeat = d->lastframe;

		d->committed_networktime = d->pseudo_networktime;

		for (Client& client : d->clients) {
			client.time =
			   d->committed_networktime - Duration(d->committed_networktime.get() > 0 ? 1 : 0);
		}

		// The call to check_hung_clients ensures that the game leaves the
		// wait mode when there are no clients
		check_hung_clients();
		init_computer_players();
		game_->run(d->settings.savegame ? Widelands::Game::StartGameType::kSaveGame :
		           d->settings.scenario ? Widelands::Game::StartGameType::kMultiPlayerScenario :
                                        Widelands::Game::StartGameType::kMap,
		           "", false, "nethost");

		// if this is an internet game, tell the metaserver that the game is done.
		if (internet_) {
			InternetGaming::ref().set_game_done();
		}
		clear_computer_players();
	} catch (const std::exception& e) {
		FsMenu::MainMenu* parent =
		   capsule_ ? &capsule_->menu() : nullptr;  // make includes script happy
		WLApplication::emergency_save(parent, *game_, e.what(), player_number);
		clear_computer_players();

		while (!d->clients.empty()) {
			disconnect_client(0, "SERVER_CRASHED");
			reaper();
		}
		// We will bounce back to the main menu, so we better log out
		if (internet_) {
			InternetGaming::ref().logout("SERVER_CRASHED");
		}
	}
	d->game = nullptr;
	game_.reset();
}

void GameHost::think() {
	NoteThreadSafeFunction::instantiate([this]() { handle_network(); }, true);

	while (!pending_player_commands_.empty()) {
		MutexLock m(MutexLock::ID::kCommands);
		do_send_player_command(pending_player_commands_.front());
		pending_player_commands_.pop_front();
	}

	if (d->game) {
		uint32_t curtime = SDL_GetTicks();
		int32_t delta = curtime - d->lastframe;
		d->lastframe = curtime;

		if (!d->waiting) {
			int32_t diff = (delta * d->networkspeed) / 1000;
			d->pseudo_networktime.increment(Duration(diff));
		}

		d->time.think(real_speed());  // must be called even when d->waiting

		if (d->pseudo_networktime != d->committed_networktime) {
			if (d->pseudo_networktime < d->committed_networktime) {
				d->pseudo_networktime = d->committed_networktime;
			} else if (curtime - d->last_heartbeat >= SERVER_TIMESTAMP_INTERVAL) {
				d->last_heartbeat = curtime;

				SendPacket packet;
				packet.unsigned_8(NETCMD_TIME);
				packet.unsigned_32(d->pseudo_networktime.get());
				broadcast(packet);

				committed_network_time(d->pseudo_networktime);

				check_hung_clients();
			}
		}

		for (AI::ComputerPlayer* cp : d->computerplayers) {
			cp->think();
		}
	}
}

void GameHost::send_player_command(Widelands::PlayerCommand* pc) {
	pending_player_commands_.push_back(pc);
}

void GameHost::do_send_player_command(Widelands::PlayerCommand* pc) {
	pc->set_duetime(d->committed_networktime + Duration(1));

	SendPacket packet;
	packet.unsigned_8(NETCMD_PLAYERCOMMAND);
	packet.unsigned_32(pc->duetime().get());
	pc->serialize(packet);
	broadcast(packet);
	d->game->enqueue_command(pc);

	committed_network_time(d->committed_networktime + Duration(1));
}

/**
 * All chat messages go through this function.
 * If it is a normal message it is sent to clients as needed, and it is
 * forwarded to our local \ref ChatProvider.
 * If it is a personal message it will only be send to the recipient and to
 * the sender (to show that the message was actually sent).
 */
void GameHost::send(ChatMessage msg) {
	if (msg.msg.empty()) {
		// No message: Nothing to do
		return;
	}

	// The set containing all receivers of the message.
	// Being a set ensures that each receiver only gets the message once
	std::set<int32_t> recipients;
	// Whether this is a public, personal, or team message (see protocol definition)
	uint8_t msg_type = CHATTYPE_PUBLIC;
	// Figure out who to send the message to
	// If there is no recipient, it is a broadcast. Send to everyone
	if (!msg.recipient.empty()) {
		// There is a recipient, find it
		msg_type = CHATTYPE_PERSONAL;
		// Add the sender to the recipients so it gets a copy of the message
		if (msg.sender.empty()) {
			// Since there is no sender, it must be a system message
			assert(msg.playern == -2);
		} else {
			// No system message, so get the sending client
			assert(msg.playern != -2);
			const int32_t sender_id = check_client(msg.sender);
			assert(sender_id != -1);
			// The sender will get a copy of the message
			recipients.insert(sender_id);
		}

		// The message is directed somewhere. Figure out where
		if (msg.recipient != "team") {
			// A single player is the recipient. Find it
			const int32_t client_id = check_client(msg.recipient);
			assert(client_id >= -2);
			if (client_id == -1) {
				// Error: Can't find given recipient, so we only will send
				// an error message back to the sender
				msg.sender.clear();
				msg.playern = -2;
				// TODO(Notabilis): Make this a command so it can be localized on the client
				msg.msg = "Failed to send message: Recipient \"";
				msg.msg += msg.recipient + "\" could not be found!";
			} else {
				// Its either a player we found or the host (id -2). Either way, add it
				recipients.insert(client_id);
			}
		} else {
			// It is a team message
			msg_type = CHATTYPE_TEAM;
			// Figure out who is in a team with the recipient and add them
			// Figure out the team of the sender
			if (msg.playern == UserSettings::none()) {
				// The message is from a spectator. Find all other spectators and send it to them
				if (d->settings.playernum == UserSettings::none()) {
					// The host is (one of the) spectators
					recipients.insert(-2);
				}
				for (uint16_t i = 0; i < d->settings.users.size(); ++i) {
					const UserSettings& user = d->settings.users.at(i);
					if (user.position != UserSettings::none()) {
						continue;
					}
					// Search for the matching network connection
					for (uint32_t client = 0; client < d->clients.size(); ++client) {
						if (d->clients.at(client).usernum == static_cast<int16_t>(i)) {
							// Found the matching connection, store it for later use
							recipients.insert(client);
						}
					}
				}
			} else {

				assert(msg.playern >= 0);
				const Widelands::TeamNumber team_sender = d->settings.players[msg.playern].team;
				// Team 0 is the "no team" option.
				// There might be multiple humans controlling that player, though
				if (team_sender == 0) {
					// Search for network clients that are using the player slot
					for (uint32_t client = 0; client < d->clients.size(); ++client) {
						if (d->clients.at(client).playernum == msg.playern) {
							recipients.insert(client);
						}
					}
					// Check if the host is using the same player slot
					if (d->settings.playernum == msg.playern) {
						recipients.insert(-2);
					}
				} else {
					// Player has a team. Search for other human players with the same team
					for (size_t i = 0; i < d->settings.players.size(); ++i) {
						// Ignore whether we are using this player: It might be a shared player
						// The set<> will filter out duplicated receivers anyway
						const PlayerSettings& player = d->settings.players[i];
						if (player.state != PlayerSettings::State::kHuman) {
							// We don't send messages to AIs or empty players
							continue;
						}
						if (player.team == team_sender) {
							if (d->settings.playernum == static_cast<int16_t>(i)) {
								// The host is (one of the) users of this player
								recipients.insert(-2);
							}
							// Search for the matching network connection(s)
							for (uint32_t client = 0; client < d->clients.size(); ++client) {
								if (d->clients.at(client).playernum == static_cast<int16_t>(i)) {
									// Found the matching connection, store it for later use
									recipients.insert(client);
									// Don't break the loop, there might be multiple
									// clients for one (shared) player
								}
							}
						}
					}
				}  // end team is not "no team"
			}     // end team is not spectator
		}        // end team message
	}           // end directed message

	// Assemble message packet
	SendPacket packet;
	packet.unsigned_8(NETCMD_CHAT);
	packet.signed_16(msg.playern);
	packet.string(msg.sender);
	packet.string(msg.msg);
	packet.unsigned_8(msg_type);
	packet.string(msg.recipient);

	// Send to either everyone or the found recipients
	if (recipients.empty()) {
		// Receive it on the host
		d->chat.receive(msg);
		// Send to all clients
		broadcast(packet);
	} else {
		for (const int32_t clientnum : recipients) {
			if (clientnum >= 0) {
				d->net->send(d->clients.at(clientnum).sock_id, packet);
			} else {
				assert(clientnum == -2);
				// Send to host player
				d->chat.receive(msg);
			}
		}
	}
}

/**
 * Checks if client \ref name exists and \returns int32_t :
 *   -   the client number if found
 *   -   -1 if no client was found
 *   -   -2 if the host is the client (has no client number)
 */
int32_t GameHost::check_client(const std::string& name) {
	// Check if the client is the host him-/herself
	if (d->localplayername == name) {
		return -2;
	}

	// Search for the client
	uint16_t i = 0;
	for (; i < d->settings.users.size(); ++i) {
		const UserSettings& user = d->settings.users.at(i);
		if (user.position == UserSettings::not_connected()) {
			// Ignore users that are not fully connected yet or who lost/broke the connection
			continue;
		}
		if (user.name == name) {
			break;
		}
	}
	if (i < d->settings.users.size()) {
		uint32_t client = 0;
		for (; client < d->clients.size(); ++client) {
			if (d->clients.at(client).usernum == static_cast<int16_t>(i)) {
				break;
			}
		}
		if (client >= d->clients.size()) {
			// This should probably not happen since we checked whether the user is connected above
			throw wexception("WARNING: user was found but no client is connected to it!\n");
		}
		return client;  // client found
	} else {
		return -1;  // no client found
	}
}

/**
 * If the host sends a chat message with formation /kick <name> <reason>.
 *
 * This function will handle this command and try to kick the user.
 */
void GameHost::kick_user(uint32_t client, const std::string& reason) {
	disconnect_client(client, "KICKED", true, reason);
}

/// Split up a user entered string in "cmd", "arg1" and "arg2"
/// \note the cmd must begin with "/"
void GameHost::split_command_array(const std::string& cmdarray,
                                   std::string& cmd,
                                   std::string& arg1,
                                   std::string& arg2) {
	assert(cmdarray.size() > 1);

	std::string::size_type const space = cmdarray.find(' ');
	if (space > cmdarray.size()) {  // only cmd
		cmd = cmdarray;
	} else {
		cmd = cmdarray.substr(0, space);
		std::string::size_type const space2 = cmdarray.find(' ', space + 1);
		if (space2 != std::string::npos) {  // cmd + arg1 + arg2
			arg1 = cmdarray.substr(space + 1, space2 - space - 1);
			arg2 = cmdarray.substr(space2 + 1);
		} else if (space + 1 < cmdarray.size()) {  // cmd + arg1
			arg1 = cmdarray.substr(space + 1);
		}
	}
	if (arg1.empty()) {
		arg1 = "";
	}
	if (arg2.empty()) {
		arg2 = "";
	}
}

void GameHost::send_system_message_code(const std::string& code,
                                        const std::string& a,
                                        const std::string& b,
                                        const std::string& c) {
	// First send to all clients
	SendPacket packet;
	packet.unsigned_8(NETCMD_SYSTEM_MESSAGE_CODE);
	packet.string(code);
	packet.string(a);
	packet.string(b);
	packet.string(c);
	broadcast(packet);

	// Now add to our own chatbox
	ChatMessage msg(NetworkGamingMessages::get_message(code, a, b, c));
	msg.playern = UserSettings::none();  //  == System message
	// c.sender remains empty to indicate a system message
	NoteThreadSafeFunction::instantiate([this, &msg]() { d->chat.receive(msg); }, true);
}

Duration GameHost::get_frametime() {
	return d->time.time() - d->game->get_gametime();
}

GameController::GameType GameHost::get_game_type() {
	return GameController::GameType::kNetHost;
}

const GameSettings& GameHost::settings() const {
	return d->settings;
}

bool GameHost::can_launch() {
	if (d->settings.mapname.empty()) {
		return false;
	}
	if (d->settings.players.empty()) {
		return false;
	}
	if (d->game) {
		return false;
	}

	// if there is one client that is currently receiving a file, we can not launch.

	const std::vector<UserSettings>& users = d->settings.users;

	for (const Client& client : d->clients) {
		const int usernum = client.usernum;
		if (usernum == -1) {
			return false;
		}
		if (!users[usernum].ready) {
			return false;
		}
	}

	// all players must be connected to a controller (human/ai) or be closed.
	// but not all should be closed!
	bool one_not_closed = false;  // TODO(k.halfmann): check this logic
	for (PlayerSettings& setting : d->settings.players) {
		if (setting.state != PlayerSettings::State::kClosed) {
			one_not_closed = true;
		}
		if (setting.state == PlayerSettings::State::kOpen) {
			return false;
		}
	}
	return one_not_closed;
}

void GameHost::set_map(const std::string& mapname,
                       const std::string& mapfilename,
                       const std::string& theme,
                       const std::string& bg,
                       uint32_t const maxplayers,
                       bool const savegame) {
	d->settings.mapname = mapname;
	d->settings.mapfilename = mapfilename;
	d->settings.savegame = savegame;
	d->settings.map_theme = theme;
	d->settings.map_background = bg;

	std::vector<PlayerSettings>::size_type oldplayers = d->settings.players.size();

	// Care about the host
	if (static_cast<int32_t>(maxplayers) <= d->settings.playernum &&
	    d->settings.playernum != UserSettings::none()) {
		set_player_number(UserSettings::none());
	}

	// Drop players not matching map any longer
	while (oldplayers > maxplayers) {
		--oldplayers;
		for (uint16_t i = 1; i < d->settings.users.size(); ++i) {
			if (d->settings.users.at(i).position == oldplayers) {
				d->settings.users.at(i).position = UserSettings::none();

				// for local settings
				uint32_t j = 0;
				for (; j < d->clients.size(); ++j) {
					if (d->clients.at(j).usernum == static_cast<int16_t>(i)) {
						break;
					}
				}
				d->clients.at(j).playernum = UserSettings::none();

				// Broadcast change
				broadcast_setting_user(i);
			}
		}
	}

	d->settings.players.resize(maxplayers);

	// Open slots for new players found on the map.
	while (oldplayers < maxplayers) {
		PlayerSettings& player = d->settings.players.at(oldplayers);
		player.state = PlayerSettings::State::kOpen;
		player.name = "";
		player.tribe = d->settings.tribes.at(0).name;
		player.random_tribe = false;
		player.initialization_index = 0;
		player.team = 0;
		player.color = kPlayerColors[oldplayers];
		player.ai = "";
		player.random_ai = false;
		player.closeable = false;
		player.shared_in = 0;

		++oldplayers;
	}

	// Broadcast new map info
	SendPacket packet;
	packet.unsigned_8(NETCMD_SETTING_MAP);
	write_setting_map(packet);
	broadcast(packet);

	// Also broadcast on LAN
	if (d->promoter) {
		d->promoter->set_map(mapname.c_str());
	}

	if (d->settings.savegame) {
		// Free all slots slots for now, so that the scenario data can be loaded properly
		set_player_number(UserSettings::none());
		for (const Client& client : d->clients) {
			switch_to_player(client.usernum, UserSettings::none());
		}
	}

	// Broadcast new player settings
	packet.reset();
	packet.unsigned_8(NETCMD_SETTING_ALLPLAYERS);
	write_setting_all_players(packet);
	broadcast(packet);
	// Map changes are finished here
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kMap));

	// Assign players from savegame
	if (d->settings.savegame) {
		for (uint8_t i = 0; i < d->settings.players.size(); ++i) {
			const PlayerSettings& p = d->settings.players.at(i);
			if (p.tribe.empty()) {
				// Closed slot
				continue;
			}

			std::stringstream playerstream(p.name);
			std::string user_part;
			while (getline(playerstream, user_part, ' ')) {
				for (uint8_t usernum = 0; usernum < d->settings.users.size(); ++usernum) {
					const std::string& username = d->settings.users.at(usernum).name;

					if (user_part == username) {
						switch_to_player(usernum, i);
						break;
					}
				}
			}
		}
	}

	// If possible, offer the map / saved game as transfer
	// TODO(unknown): not yet able to handle directory type maps / savegames, would involve zipping
	// in place or such ...
	if (!g_fs->is_directory(mapfilename)) {
		// Read in the file
		FileRead fr;
		fr.open(*g_fs, mapfilename);
		file_.reset(new NetTransferFile());
		file_->filename = mapfilename;
		uint32_t leftparts = file_->bytes = fr.get_size();
		while (leftparts > 0) {
			uint32_t readout = (leftparts > NETFILEPARTSIZE) ? NETFILEPARTSIZE : leftparts;
			FilePart fp;
			memcpy(fp.part, fr.data(readout), readout);
			file_->parts.push_back(fp);
			leftparts -= readout;
		}
		std::vector<char> complete(file_->bytes);
		fr.set_file_pos(0);
		fr.data_complete(&complete[0], file_->bytes);
		SimpleMD5Checksum md5sum;
		md5sum.data(&complete[0], file_->bytes);
		md5sum.finish_checksum();
		file_->md5sum = md5sum.get_checksum().str();
	} else {
		// reset previously offered map / saved game
		file_.reset(nullptr);
	}

	packet.reset();
	if (write_map_transfer_info(packet, mapfilename)) {
		broadcast(packet);
	}
}

// TODO(k.halfmann): refactor this
void GameHost::set_player_state(uint8_t const number,
                                PlayerSettings::State const state,
                                bool const host) {

	if (number >= d->settings.players.size()) {
		return;  // ignore player numbers out of range
	}

	PlayerSettings& player = d->settings.players.at(number);

	if (player.state == state) {
		return;
	}

	if (player.state == PlayerSettings::State::kHuman) {
		// 0 is host and has no client
		if (d->settings.users.at(0).position == number) {
			d->settings.users.at(0).position = UserSettings::none();
			d->settings.playernum = UserSettings::none();
		}
		for (uint8_t i = 1; i < d->settings.users.size(); ++i) {
			if (d->settings.users.at(i).position == number) {
				d->settings.users.at(i).position = UserSettings::none();
				if (host) {  //  Did host send the user to lobby?
					send_system_message_code("SENT_PLAYER_TO_LOBBY", d->settings.users.at(i).name);
				}

				//  for local settings
				for (std::vector<Client>::iterator j = d->clients.begin();; ++j) {
					assert(j != d->clients.end());
					if (j->usernum == i) {
						j->playernum = UserSettings::none();
						break;
					}
				}
				break;
			}
		}
	}

	player.state = state;

	// Make sure that shared slots have a player number to share in
	if (player.state == PlayerSettings::State::kShared) {
		const PlayerSlot shared = d->settings.find_shared(number);
		if (d->settings.is_shared_usable(number, shared)) {
			set_player_shared(number, shared);
		} else {
			player.state = PlayerSettings::State::kClosed;
		}
	}

	// Update shared positions for other players
	for (size_t i = 0; i < d->settings.players.size(); ++i) {
		if (i == number) {
			// Don't set own state
			continue;
		}
		if (d->settings.players.at(i).state == PlayerSettings::State::kShared) {
			const PlayerSlot shared = d->settings.find_shared(i);
			if (d->settings.is_shared_usable(i, shared)) {
				set_player_shared(i, shared);
			} else {
				set_player_state(i, PlayerSettings::State::kClosed, host);
			}
		}
	}

	// Make sure that slots that are not closeable stay open
	if (player.state == PlayerSettings::State::kClosed && d->settings.uncloseable(number)) {
		player.state = PlayerSettings::State::kOpen;
	}

	if (player.state == PlayerSettings::State::kComputer) {
		player.name = get_computer_player_name(number);
	}

	// Broadcast change to player
	broadcast_setting_player(number);

	// Let clients know whether their slot has changed
	SendPacket packet;
	packet.unsigned_8(NETCMD_SETTING_ALLUSERS);
	write_setting_all_users(packet);
	broadcast(packet);
}

void GameHost::set_player_tribe(uint8_t const number,
                                const std::string& tribe,
                                bool const random_tribe) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);

	// TODO(k.halfmann): check this logic, will tribe "survive" when random is selected?
	if (player.tribe == tribe && player.random_tribe == random_tribe) {
		return;
	}

	std::string actual_tribe = tribe;
	player.random_tribe = random_tribe;

	while (!d->settings.savegame && random_tribe) {
		uint8_t num_tribes = d->settings.tribes.size();
		uint8_t random = (std::rand() % num_tribes);  // NOLINT
		actual_tribe = d->settings.tribes.at(random).name;
		if (player.state != PlayerSettings::State::kComputer ||
		    d->settings.get_tribeinfo(actual_tribe).suited_for_ai) {
			break;
		}
	}

	for (const Widelands::TribeBasicInfo& temp_tribeinfo : d->settings.tribes) {
		if (temp_tribeinfo.name == player.tribe) {
			player.tribe = actual_tribe;
			if (temp_tribeinfo.initializations.size() <= player.initialization_index) {
				player.initialization_index = 0;
			}

			//  broadcast changes
			broadcast_setting_player(number);
			return;  // TODO(k.halfmann): check this logic
		}
	}
	log_warn(
	   "Player %u attempted to change to tribe %s; not a valid tribe\n", number, tribe.c_str());
}

void GameHost::set_player_init(uint8_t const number, uint8_t const index) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);

	if (player.initialization_index == index) {
		return;
	}

	for (const Widelands::TribeBasicInfo& temp_tribeinfo : d->settings.tribes) {
		if (temp_tribeinfo.name == player.tribe) {
			if (index < temp_tribeinfo.initializations.size()) {
				player.initialization_index = index;

				//  broadcast changes
				broadcast_setting_player(number);
				return;
			} else {
				log_warn("Attempted to change to out-of-range initialization index %u "
				         "for player %u.\n",
				         index, number);
			}
			return;
		}
	}
	NEVER_HERE();
}

void GameHost::set_player_ai(uint8_t number, const std::string& name, bool const random_ai) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);
	player.ai = name;
	player.random_ai = random_ai;

	// Broadcast changes
	broadcast_setting_player(number);
}

/**
 * Removes one player from a combined slot, where the name consists
 * of all players in this slot separated by spaces, i.e. "Player1 Player2"
 * \return true if the name has no players left
 */
bool GameHost::remove_player_name(uint8_t const number, const std::string& name) {
	PlayerSettings& p = d->settings.players.at(number);
	std::string temp(p.name);
	temp.erase(p.name.find(name), name.size());
	if (temp.back() == ' ') {
		temp.erase(temp.end() - 1);
	} else if (temp.front() == ' ') {
		temp.erase(temp.begin());
	}
	set_player_name(number, temp);
	return temp.empty();
}

void GameHost::set_player_name(uint8_t const number, const std::string& name) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);

	if (player.name == name) {
		return;
	}

	player.name = name;

	// Broadcast changes
	broadcast_setting_player(number);
}

void GameHost::set_player_closeable(uint8_t const number, bool closeable) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);

	if (player.closeable == closeable) {
		return;
	}

	player.closeable = closeable;

	// There is no need to broadcast a player closeability change, as the host is the only one who
	// uses it.
}

void GameHost::set_player_shared(PlayerSlot number, Widelands::PlayerNumber shared) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);

	if (player.shared_in == shared) {
		return;
	}

	const PlayerSettings& sharedplr = d->settings.players.at(shared - 1);
	assert(PlayerSettings::can_be_shared(sharedplr.state));
	assert(d->settings.is_shared_usable(number, shared));

	player.shared_in = shared;
	player.tribe = sharedplr.tribe;

	// Broadcast changes
	broadcast_setting_player(number);
}

void GameHost::set_player(uint8_t const number, const PlayerSettings& ps) {
	if (number >= d->settings.players.size()) {
		return;
	}

	PlayerSettings& player = d->settings.players.at(number);
	player = ps;

	// Broadcast changes
	broadcast_setting_player(number);
}

void GameHost::set_player_number(uint8_t const number) {
	switch_to_player(0, number);
}

void GameHost::set_win_condition_script(const std::string& wc) {
	d->settings.win_condition_script = wc;

	// Broadcast changes
	SendPacket packet;
	packet.unsigned_8(NETCMD_WIN_CONDITION);
	packet.string(wc);
	broadcast(packet);
}

void GameHost::set_peaceful_mode(bool peace) {
	d->settings.peaceful = peace;

	// Broadcast changes
	SendPacket packet;
	packet.unsigned_8(NETCMD_PEACEFUL_MODE);
	packet.unsigned_8(peace ? 1 : 0);
	broadcast(packet);
}

void GameHost::set_custom_starting_positions(bool c) {
	d->settings.custom_starting_positions = c;

	// Broadcast changes
	SendPacket packet;
	packet.unsigned_8(NETCMD_CUSTOM_STARTING_POSITIONS);
	packet.unsigned_8(c ? 1 : 0);
	broadcast(packet);
}

void GameHost::switch_to_player(uint32_t user, uint8_t number) {
	if (number < d->settings.players.size() &&
	    (d->settings.players.at(number).state != PlayerSettings::State::kOpen &&
	     d->settings.players.at(number).state != PlayerSettings::State::kHuman)) {
		return;
	}

	uint32_t old = d->settings.users.at(user).position;
	std::string name = d->settings.users.at(user).name;
	// Remove clients name from old player slot
	if (old < d->settings.players.size()) {
		if (remove_player_name(old, name)) {
			set_player_state(old, PlayerSettings::State::kOpen);
		}
	}

	if (number < d->settings.players.size()) {
		// Add clients name to new player slot
		PlayerSettings& op = d->settings.players.at(number);
		if (op.state == PlayerSettings::State::kOpen) {
			set_player_state(number, PlayerSettings::State::kHuman);
			set_player_name(number, name);
		} else {
			set_player_name(number, op.name + " " + name);
		}
	}
	d->settings.users.at(user).position = number;
	if (user == 0) {  // host
		d->settings.playernum = number;
	} else {
		for (Client& client : d->clients) {
			if (client.usernum == static_cast<int16_t>(user)) {
				client.playernum = number;
				break;
			}
		}
	}

	// Broadcast the user changes to everybody
	broadcast_setting_user(user);
}

void GameHost::set_player_team(uint8_t number, Widelands::TeamNumber team) {
	if (number >= d->settings.players.size()) {
		return;
	}
	d->settings.players.at(number).team = team;

	// Broadcast changes
	broadcast_setting_player(number);
}

void GameHost::set_player_color(const uint8_t number, const RGBColor& col) {
	if (number >= d->settings.players.size()) {
		return;
	}
	d->settings.players.at(number).color = col;
	broadcast_setting_player(number);
}

void GameHost::set_multiplayer_game_settings() {
	d->settings.scenario = false;
	d->settings.multiplayer = true;
}

void GameHost::set_scenario(bool is_scenario) {
	d->settings.scenario = is_scenario;
}

uint32_t GameHost::real_speed() {
	if (d->waiting) {
		return 0;
	}
	return d->networkspeed;
}

uint32_t GameHost::desired_speed() {
	return d->localdesiredspeed;
}

void GameHost::set_desired_speed(uint32_t const speed) {
	if (speed != d->localdesiredspeed) {
		d->localdesiredspeed = speed;
		update_network_speed();
	}
}

// Network games cannot be paused
bool GameHost::is_paused() {
	return false;
}

void GameHost::set_paused(bool /* paused */) {
}

// Send the packet to all properly connected clients
void GameHost::broadcast(const SendPacket& packet) {
	std::vector<NetHostInterface::ConnectionId> receivers;
	for (const Client& client : d->clients) {
		if (client.playernum != UserSettings::not_connected()) {
			assert(client.sock_id > 0);
			receivers.push_back(client.sock_id);
		}
	}
	d->net->send(receivers, packet);
}

void GameHost::write_setting_map(SendPacket& packet) {
	packet.string(d->settings.mapname);
	packet.string(d->settings.mapfilename);
	packet.string(d->settings.map_theme);
	packet.string(d->settings.map_background);
	packet.unsigned_8(d->settings.savegame ? 1 : 0);
	packet.unsigned_8(d->settings.scenario ? 1 : 0);
}

void GameHost::write_setting_player(SendPacket& packet, uint8_t const number) {
	PlayerSettings& player = d->settings.players.at(number);
	packet.unsigned_8(static_cast<uint8_t>(player.state));
	packet.string(player.name);
	packet.string(player.tribe);
	packet.unsigned_8(player.random_tribe ? 1 : 0);
	packet.unsigned_8(player.initialization_index);
	packet.string(player.ai);
	packet.unsigned_8(player.random_ai ? 1 : 0);
	packet.unsigned_8(player.team);
	packet.unsigned_8(player.shared_in);
	packet.unsigned_8(player.color.r);
	packet.unsigned_8(player.color.g);
	packet.unsigned_8(player.color.b);
	Notifications::publish(NoteGameSettings(NoteGameSettings::Action::kPlayer, number));
}

void GameHost::broadcast_setting_player(uint8_t const number) {
	SendPacket packet;
	packet.unsigned_8(NETCMD_SETTING_PLAYER);
	packet.unsigned_8(number);
	write_setting_player(packet, number);
	broadcast(packet);
	if (d && d->chat.participants_) {
		d->chat.participants_->participants_updated();
	}
}

void GameHost::write_setting_all_players(SendPacket& packet) {
	packet.unsigned_8(d->settings.players.size());
	for (uint8_t i = 0; i < d->settings.players.size(); ++i) {
		write_setting_player(packet, i);
	}
}

void GameHost::write_setting_user(SendPacket& packet, uint32_t const number) {
	packet.string(d->settings.users.at(number).name);
	packet.signed_32(d->settings.users.at(number).position);
	packet.unsigned_8(d->settings.users.at(number).ready ? 1 : 0);
	Notifications::publish(NoteGameSettings(
	   NoteGameSettings::Action::kUser, d->settings.users.at(number).position, number));
}

void GameHost::broadcast_setting_user(uint32_t const number) {
	SendPacket packet;
	packet.unsigned_8(NETCMD_SETTING_USER);
	packet.unsigned_32(number);
	write_setting_user(packet, number);
	broadcast(packet);
	if (d && d->chat.participants_) {
		d->chat.participants_->participants_updated();
	}
}

void GameHost::write_setting_all_users(SendPacket& packet) {
	packet.unsigned_8(d->settings.users.size());
	for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
		write_setting_user(packet, i);
	}
}

/**
 * If possible, this function writes the MapTransferInfo to SendPacket & packet
 *
 * \returns true if the data was written, else false
 */
bool GameHost::write_map_transfer_info(SendPacket& packet, const std::string& mapfilename) {
	// TODO(unknown): not yet able to handle directory type maps / savegames
	if (g_fs->is_directory(mapfilename)) {
		log_warn("Map/Save is a directory! No way for making it available a.t.m.!\n");
		return false;
	}

	// Write the new map/save file information, so client can decide whether it
	// needs the file.
	packet.unsigned_8(NETCMD_NEW_FILE_AVAILABLE);
	packet.string(mapfilename);
	// Scan-build reports that access to bytes here results in a dereference of null pointer.
	// This is a false positive.
	// See https://bugs.launchpad.net/widelands/+bug/1198919
	packet.unsigned_32(file_->bytes);  // NOLINT
	packet.string(file_->md5sum);
	return true;
}

/**
 *
 * \return a name for the given player.
 */
std::string GameHost::get_computer_player_name(uint8_t const playernum) {
	std::string name;
	uint32_t suffix = playernum;
	do {
		name = bformat(_("Computer %u"), static_cast<unsigned int>(++suffix));
	} while (has_user_name(name, playernum));
	return name;
}

/**
 * Checks whether a user with the given name exists already.
 *
 * If \p ignoreplayer < UserSettings::highest_playernum(), the user with this
 * number will be ignored.
 */
bool GameHost::has_user_name(const std::string& name, uint8_t ignoreplayer) {
	for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
		if (i != ignoreplayer && d->settings.users.at(i).name == name) {
			return true;
		}
	}

	// Computer players are not handled like human users,
	// so make sure no cp owns this name.
	if (ignoreplayer < d->settings.users.size()) {
		ignoreplayer = d->settings.users.at(ignoreplayer).position;
	}
	for (uint32_t i = 0; i < d->settings.players.size(); ++i) {
		if (d->settings.savegame && d->settings.players.at(i).state == PlayerSettings::State::kOpen) {
			// This might be this users slot from the configured savegame
			continue;
		}
		if (i != ignoreplayer && d->settings.players.at(i).name == name) {
			return true;
		}
	}

	return false;
}

/// Respond to a client's Hello message.
void GameHost::welcome_client(uint32_t const number, std::string& playername) {
	assert(number < d->clients.size());

	Client& client = d->clients.at(number);

	assert(client.playernum == UserSettings::not_connected());
	assert(client.sock_id > 0);

	// The client gets its own initial data set.
	client.playernum = UserSettings::none();

	if (!d->game) {  // just in case we allow connection of spectators/players after game start
		for (uint32_t i = 0; i < d->settings.users.size(); ++i) {
			if (d->settings.users[i].position == UserSettings::not_connected()) {
				client.usernum = i;
				d->settings.users[i].result = Widelands::PlayerEndResult::kUndefined;
				d->settings.users[i].ready = true;
				break;
			}
		}
	}
	if (client.usernum == -1) {  // add the new client / user to the settings
		client.usernum = d->settings.users.size();
		UserSettings newuser;
		newuser.result = Widelands::PlayerEndResult::kUndefined;
		newuser.ready = true;
		d->settings.users.push_back(newuser);
	}

	// Assign the player a name, preferably the name chosen by the client
	if (playername.empty()) {  // Make sure there is at least a name base.
		playername = "Player";
	}
	std::string effective_name = playername;

	if (has_user_name(effective_name, client.usernum)) {
		uint32_t i = 1;
		do {
			effective_name = bformat("%s%u", playername, i++);
		} while (has_user_name(effective_name, client.usernum));
	}

	d->settings.users.at(client.usernum).name = effective_name;
	d->settings.users.at(client.usernum).position = UserSettings::none();

	verb_log_info("[Host]: Client %u: welcome to usernum %u", number, client.usernum);

	SendPacket packet;
	packet.unsigned_8(NETCMD_HELLO);
	packet.unsigned_8(NETWORK_PROTOCOL_VERSION);
	packet.unsigned_32(client.usernum);
	{
		std::vector<const AddOns::AddOnInfo*> enabled_addons;
		for (const auto& pair : AddOns::g_addons) {
			if (pair.second && pair.first->category != AddOns::AddOnCategory::kTheme) {
				enabled_addons.push_back(pair.first.get());
			}
		}
		packet.unsigned_32(enabled_addons.size());
		for (const AddOns::AddOnInfo* a : enabled_addons) {
			packet.string(a->internal_name);
			packet.string(AddOns::version_to_string(a->version, false));
		}
	}
	d->net->send(client.sock_id, packet);
	// even if the network protocol is the same, the data might be different.
	if (client.build_id != build_id()) {
		send_system_message_code("DIFFERENT_WL_VERSION", effective_name, client.build_id, build_id());
	}
	// Send information about currently selected map / savegame
	packet.reset();

	packet.unsigned_8(NETCMD_SETTING_MAP);
	write_setting_map(packet);
	d->net->send(client.sock_id, packet);

	// If possible, offer the map / savegame as transfer
	if (file_) {
		packet.reset();
		if (write_map_transfer_info(packet, file_->filename)) {
			d->net->send(client.sock_id, packet);
		}
	}

	//  Send the tribe information to the new client.
	packet.reset();
	packet.unsigned_8(NETCMD_SETTING_TRIBES);
	packet.unsigned_8(d->settings.tribes.size());
	for (const Widelands::TribeBasicInfo& tribe : d->settings.tribes) {
		packet.string(tribe.name);
		size_t const nr_initializations = tribe.initializations.size();
		packet.unsigned_8(nr_initializations);
		for (const Widelands::TribeBasicInfo::Initialization& init : tribe.initializations) {
			packet.string(init.script);
		}
	}
	d->net->send(client.sock_id, packet);

	packet.reset();
	packet.unsigned_8(NETCMD_SETTING_ALLPLAYERS);
	write_setting_all_players(packet);
	d->net->send(client.sock_id, packet);

	packet.reset();
	packet.unsigned_8(NETCMD_SETTING_ALLUSERS);
	write_setting_all_users(packet);
	d->net->send(client.sock_id, packet);

	packet.reset();
	packet.unsigned_8(NETCMD_WIN_CONDITION);
	packet.string(d->settings.win_condition_script);
	d->net->send(client.sock_id, packet);

	packet.reset();
	packet.unsigned_8(NETCMD_PEACEFUL_MODE);
	packet.unsigned_8(d->settings.peaceful ? 1 : 0);
	d->net->send(client.sock_id, packet);

	packet.reset();
	packet.unsigned_8(NETCMD_CUSTOM_STARTING_POSITIONS);
	packet.unsigned_8(d->settings.custom_starting_positions ? 1 : 0);
	d->net->send(client.sock_id, packet);

	// Broadcast new information about the player to everybody
	broadcast_setting_user(client.usernum);

	// Check if there is an unoccupied player left and if, assign.
	// Assign the slot with the same username for savegames
	for (uint8_t i = 0; i < d->settings.players.size(); ++i) {
		const PlayerSettings& p = d->settings.players.at(i);
		if (p.state == PlayerSettings::State::kOpen &&
		    (!d->settings.savegame || p.name == effective_name)) {
			switch_to_player(client.usernum, i);
			break;
		}
	}

	send_system_message_code("CLIENT_HAS_JOINED_GAME", effective_name);
}

void GameHost::committed_network_time(const Time& time) {
	assert(time > d->committed_networktime);

	d->committed_networktime = time;
	d->time.receive(time);

	if (!d->syncreport_pending &&
	    d->committed_networktime - d->syncreport_time >= Duration(SYNCREPORT_INTERVAL)) {
		request_sync_reports();
	}
}

void GameHost::receive_client_time(uint32_t const number, const Time& time) {
	assert(number < d->clients.size());

	Client& client = d->clients.at(number);

	if (time < client.time) {
		throw DisconnectException("BACKWARDS_RUNNING_TIME");
	}
	if (d->committed_networktime < time) {
		throw DisconnectException("SIMULATING_BEYOND_TIME");
	}
	if (d->syncreport_pending && !client.syncreport_arrived) {
		if (time > d->syncreport_time) {
			throw DisconnectException("CLIENT_SYNC_REP_TIMEOUT");
		}
	}

	client.time = time;
	verb_log_info("[Host]: Client %i: Time %i", number, time.get());

	if (d->waiting) {
		verb_log_info("[Host]: Client %i reports time %i (networktime = %i) during hang", number,
		              time.get(), d->committed_networktime.get());
		check_hung_clients();
	}
}

void GameHost::check_hung_clients() {
	assert(d->game != nullptr);

	int nrdelayed = 0;
	int nrhung = 0;

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		if (d->clients.at(i).playernum == UserSettings::not_connected()) {
			continue;
		}

		const Duration delta = d->committed_networktime - d->clients.at(i).time;

		if (delta.get() == 0) {
			// reset the hung_since time
			d->clients.at(i).hung_since = 0;
		} else {
			assert(d->game != nullptr);
			++nrdelayed;
			if (delta > Duration(5 * CLIENT_TIMESTAMP_INTERVAL * d->networkspeed / 1000)) {
				verb_log_info("[Host]: Client %i (%s) hung", i,
				              d->settings.users.at(d->clients.at(i).usernum).name.c_str());
				++nrhung;
				if (d->clients.at(i).hung_since == 0) {
					d->clients.at(i).hung_since = time(nullptr);
					d->clients.at(i).lastdelta = 0;
				} else if (time_t deltanow = time(nullptr) - d->clients.at(i).hung_since > 60) {

					// inform the other clients about the problem regulary
					if (deltanow - d->clients.at(i).lastdelta > 30) {
						std::string seconds =
						   bformat(ngettext("%li second", "%li seconds", deltanow), deltanow);
						send_system_message_code(
						   "CLIENT_HUNG", d->settings.users.at(d->clients.at(i).usernum).name, seconds);
						d->clients.at(i).lastdelta = deltanow;
					}
				}
			}
		}
	}

	if (!d->waiting) {
		if (nrhung) {
			verb_log_info("[Host]: %i clients hung. Entering wait mode", nrhung);

			// Brake and wait
			d->waiting = true;
			broadcast_real_speed(0);

			SendPacket packet;
			packet.unsigned_8(NETCMD_WAIT);
			broadcast(packet);
		}
	} else {
		if (nrdelayed == 0) {
			d->waiting = false;
			broadcast_real_speed(d->networkspeed);
			if (!d->syncreport_pending) {
				request_sync_reports();
			}
		}
	}
}

void GameHost::broadcast_real_speed(uint32_t const speed) {
	assert(speed <= std::numeric_limits<uint16_t>::max());

	SendPacket packet;
	packet.unsigned_8(NETCMD_SETSPEED);
	packet.unsigned_16(speed);
	broadcast(packet);
}

/**
 * This is the algorithm that decides upon the effective network speed,
 * given the desired speed of all clients.
 *
 * This function is supposed to be the only code that ever changes
 * \ref GameHostImpl::networkspeed.
 *
 * The current implementation picks the median, or the average of
 * lower and upper median.
 * If only two players are playing (host + 1 client), there is a boundary
 * so neither the host, nor the players can abuse their setting to get to the
 * wished speed (e.g. without this boundary, the client might set his/her wished
 *               speed to 8x - even if the host sets his desired speed to PAUSE
 *               the median sped would be 4x).
 *
 * The immediate pausing (with the Pause key) is disabled completely in the
 * network games, as sudden pauses would be distracting to other players. A
 * hard interruption of the game can be achieved with the forced pause.
 */
void GameHost::update_network_speed() {
	uint32_t const oldnetworkspeed = d->networkspeed;

	// First check if a pause was forced by the host
	if (forced_pause_) {
		d->networkspeed = 0;
	}

	else {
		// No pause was forced - normal speed calculation
		std::vector<uint32_t> speeds;

		speeds.push_back(d->localdesiredspeed);
		for (const Client& client : d->clients) {
			if (client.playernum <= UserSettings::highest_playernum()) {
				speeds.push_back(client.desiredspeed);
			}
		}
		assert(!speeds.empty());

		std::sort(speeds.begin(), speeds.end());

		// Abuse prevention for 2 players
		if (speeds.size() == 2) {
			if (speeds[0] > speeds[1] + 1000) {
				speeds[0] = speeds[1] + 1000;
			}
			if (speeds[1] > speeds[0] + 1000) {
				speeds[1] = speeds[0] + 1000;
			}
		}

		d->networkspeed = (speeds.size() % 2) ?
                           speeds.at(speeds.size() / 2) :
                           (speeds.at(speeds.size() / 2) + speeds.at((speeds.size() / 2) - 1)) / 2;

		if (d->networkspeed > std::numeric_limits<uint16_t>::max()) {
			d->networkspeed = std::numeric_limits<uint16_t>::max();
		}
	}

	if (d->networkspeed != oldnetworkspeed && !d->waiting) {
		broadcast_real_speed(d->networkspeed);
	}
}

/**
 * Request sync reports from all clients at the next possible time.
 */
void GameHost::request_sync_reports() {
	assert(!d->syncreport_pending);

	d->syncreport_pending = true;
	d->syncreport_arrived = false;
	d->syncreport_time = d->committed_networktime + Duration(1);

	for (Client& client : d->clients) {
		client.syncreport_arrived = false;
	}

	verb_log_info("[Host]: Requesting sync reports for time %i", d->syncreport_time.get());
	d->game->report_sync_request();

	SendPacket packet;
	packet.unsigned_8(NETCMD_SYNCREQUEST);
	packet.unsigned_32(d->syncreport_time.get());
	broadcast(packet);

	d->game->enqueue_command(
	   new CmdNetCheckSync(d->syncreport_time, [this] { sync_report_callback(); }));

	committed_network_time(d->syncreport_time);
}

/**
 * Check whether all sync reports have arrived, and if so, compare.
 */
void GameHost::check_sync_reports() {
	assert(d->syncreport_pending);

	if (!d->syncreport_arrived) {
		return;
	}

	for (const Client& client : d->clients) {
		if (client.playernum != UserSettings::not_connected() && !client.syncreport_arrived) {
			return;
		}
	}

	d->syncreport_pending = false;
	verb_log_info("[Host]: comparing syncreports for time %i", d->syncreport_time.get());

	for (uint32_t i = 0; i < d->clients.size(); ++i) {
		Client& client = d->clients.at(i);
		if (client.playernum == UserSettings::not_connected()) {
			continue;
		}

		if (client.syncreport != d->syncreport) {
			log_err("[Host]: lost synchronization with client %u at time %i!\n"
			        "I have:     %s\n"
			        "Client has: %s\n",
			        i, d->syncreport_time.get(), d->syncreport.str().c_str(),
			        client.syncreport.str().c_str());

			d->game->save_syncstream(true);
			// Create syncstream excerpt and add faulting player number
			d->game->report_desync(i);

			SendPacket packet;
			packet.unsigned_8(NETCMD_INFO_DESYNC);
			broadcast(packet);

			disconnect_client(i, "CLIENT_DESYNCED");
			// Pause the game, so that host and client have time to handle the
			// desync.
			d->networkspeed = 0;
			broadcast_real_speed(d->networkspeed);
		}
	}
}

void GameHost::sync_report_callback() {
	assert(d->game->get_gametime() == d->syncreport_time);

	d->syncreport = d->game->get_sync_hash();
	d->syncreport_arrived = true;

	check_sync_reports();
}

void GameHost::handle_network() {

	if (d->promoter) {
		d->promoter->run();
	}

	// Check for new connections.
	Client peer;
	assert(d->net != nullptr);
	while (d->net->try_accept(&peer.sock_id)) {
		// Should only happen if the game has not been started yet
		assert(d->game == nullptr);
		peer.playernum = UserSettings::not_connected();
		peer.syncreport_arrived = false;
		peer.desiredspeed = 1000;
		peer.usernum = -1;  // == no user assigned for now.
		peer.hung_since = 0;
		peer.lastdelta = 0;
		d->clients.push_back(peer);
	}

	// if this is an internet game, handle the metaserver information
	if (internet_) {
		InternetGaming::ref().handle_metaserver_communication();
		// Maybe an important message was send on the metaserver,
		// that we should show in game as well.
		std::vector<ChatMessage> msgs;
		InternetGaming::ref().get_ingame_system_messages(msgs);
		for (const ChatMessage& msg : msgs) {
			send(msg);
		}
	}

	for (size_t i = 0; i < d->clients.size(); ++i) {
		if (!d->net->is_connected(d->clients.at(i).sock_id)) {
			disconnect_client(i, "CONNECTION_LOST", false);
		}
	}

	// Check if we hear anything from our clients
	for (size_t i = 0; i < d->clients.size(); ++i) {
		try {
			std::unique_ptr<RecvPacket> packet = d->net->try_receive(d->clients.at(i).sock_id);
			while (packet) {
				handle_packet(i, *packet);
				packet = d->net->try_receive(d->clients.at(i).sock_id);
			}
			// Thrown by handle_packet()
		} catch (const DisconnectException& e) {
			disconnect_client(i, e.what());
		} catch (const ProtocolException& e) {
			disconnect_client(i, "PROTOCOL_EXCEPTION", true, e.what());
		} catch (const std::exception& e) {
			disconnect_client(i, "MALFORMED_COMMANDS", true, e.what());
		}
	}

	// If a pause was forced or if the players all pause, send a ping regularly
	// to keep the sockets up and running
	if ((forced_pause_ || real_speed() == 0) && (time(nullptr) > (d->lastpauseping + 20))) {
		d->lastpauseping = time(nullptr);

		SendPacket send_packet;
		send_packet.unsigned_8(NETCMD_PING);
		broadcast(send_packet);
	}

	reaper();
}

void GameHost::handle_disconnect(uint32_t const client_num, RecvPacket& r) {
	uint8_t number = r.unsigned_8();
	std::string reason = r.string();
	if (number == 1) {
		disconnect_client(client_num, reason, false);
	} else {
		std::string arg = r.string();
		disconnect_client(client_num, reason, false, arg);
	}
}

void GameHost::handle_ping(Client& client) {
	verb_log_info("[Host]: Received ping from metaserver.");
	// Send PING back
	SendPacket packet;
	packet.unsigned_8(NETCMD_METASERVER_PING);
	d->net->send(client.sock_id, packet);

	// Remove metaserver from list of clients
	client.playernum = UserSettings::not_connected();
	d->net->close(client.sock_id);
	client.sock_id = 0;
}

/** Wait for NETCMD_HELLO and handle unexpected other commands */
void GameHost::handle_hello(uint32_t const client_num,
                            uint8_t const cmd,
                            Client& client,
                            RecvPacket& r) {
	// Now we wait for the client to say Hi in the right language,
	// unless the game has already started
	if (d->game) {
		throw DisconnectException("GAME_ALREADY_STARTED");
	}
	if (cmd != NETCMD_HELLO) {
		throw ProtocolException(cmd);
	}
	uint8_t version = r.unsigned_8();
	if (version != NETWORK_PROTOCOL_VERSION) {
		throw DisconnectException("DIFFERENT_PROTOCOL_VERS");
	}

	std::string clientname = r.string();
	client.build_id = r.string();

	welcome_client(client_num, clientname);
}

void GameHost::handle_changetribe(const Client& client, RecvPacket& r) {
	//  Do not be harsh about packets of this type arriving out of order -
	//  the client might just have had bad luck with the timing.
	if (!d->game) {
		uint8_t num = r.unsigned_8();
		if (num != client.playernum) {
			throw DisconnectException("NO_ACCESS_TO_PLAYER");
		}
		std::string tribe = r.string();
		bool random_tribe = r.unsigned_8();
		set_player_tribe(num, tribe, random_tribe);
	}
}

/** Handle changed sharing of clients by users */
void GameHost::handle_changeshared(const Client& client, RecvPacket& r) {
	//  Do not be harsh about packets of this type arriving out of order -
	//  the client might just have had bad luck with the timing.
	if (!d->game) {
		uint8_t num = r.unsigned_8();
		if (num != client.playernum) {
			throw DisconnectException("NO_ACCESS_TO_PLAYER");
		}
		set_player_shared(num, r.unsigned_8());
	}
}

void GameHost::handle_changeteam(const Client& client, RecvPacket& r) {
	if (!d->game) {
		uint8_t num = r.unsigned_8();
		if (num != client.playernum) {
			throw DisconnectException("NO_ACCESS_TO_PLAYER");
		}
		set_player_team(num, r.unsigned_8());
	}
}

void GameHost::handle_changecolor(const Client& client, RecvPacket& packet) {
	if (!d->game) {
		const uint8_t num = packet.unsigned_8();
		if (num != client.playernum) {
			throw DisconnectException("NO_ACCESS_TO_PLAYER");
		}
		const uint8_t r = packet.unsigned_8();
		const uint8_t g = packet.unsigned_8();
		const uint8_t b = packet.unsigned_8();
		set_player_color(num, RGBColor(r, g, b));
	}
}

void GameHost::handle_changeinit(const Client& client, RecvPacket& r) {
	if (!d->game) {
		// TODO(GunChleoc): For some nebulous reason, we don't receive the num that the client is
		// sending when a player changes slot. So, keeping the access to the client off for now.
		// Would be nice to have though.
		uint8_t num = r.unsigned_8();
		if (num != client.playernum) {
			throw DisconnectException("NO_ACCESS_TO_PLAYER");
		}
		set_player_init(num, r.unsigned_8());
	}
}

void GameHost::handle_changeposition(const Client& client, RecvPacket& r) {
	if (!d->game) {
		uint8_t const pos = r.unsigned_8();
		switch_to_player(client.usernum, pos);
	}
}

void GameHost::handle_nettime(uint32_t const client_num, RecvPacket& r) {
	if (!d->game) {
		throw DisconnectException("TIME_SENT_NOT_READY");
	}
	receive_client_time(client_num, Time(r.unsigned_32()));
}

void GameHost::handle_playercommmand(uint32_t const client_num, Client& client, RecvPacket& r) {
	if (!d->game) {
		throw DisconnectException("PLAYERCMD_WO_GAME");
	}
	Time time(r.unsigned_32());
	Widelands::PlayerCommand* plcmd = Widelands::PlayerCommand::deserialize(r);
	verb_log_info("[Host]: Client %u (%u) sent player command %u for %u, time = %u\n", client_num,
	              client.playernum, static_cast<unsigned int>(plcmd->id()), plcmd->sender(),
	              time.get());
	if (plcmd->sender() != client.playernum + 1) {
		throw DisconnectException("PLAYERCMD_FOR_OTHER");
	}
	do_send_player_command(plcmd);
}

void GameHost::handle_syncreport(uint32_t const client_num, Client& client, RecvPacket& r) {
	if (!d->game || !d->syncreport_pending || client.syncreport_arrived) {
		throw DisconnectException("UNEXPECTED_SYNC_REP");
	}
	Time time(r.unsigned_32());
	r.data(client.syncreport.data, 16);
	client.syncreport_arrived = true;
	receive_client_time(client_num, time);
	check_sync_reports();
}

void GameHost::handle_chat(Client& client, RecvPacket& r) {
	ChatMessage c(r.string());
	c.playern = d->settings.users.at(client.usernum).position;
	c.sender = d->settings.users.at(client.usernum).name;
	if (!c.msg.empty() && *c.msg.begin() == '@') {
		// Personal message
		std::string::size_type const space = c.msg.find(' ');
		if (space >= c.msg.size() - 1) {
			return;  // No Message after '@<User>'
		}
		c.recipient = c.msg.substr(1, space - 1);
		c.msg = c.msg.substr(space + 1);
	}
	send(c);
}

/** Take care of change of game speed PAUSE, 1x 2x 3x .... */
void GameHost::handle_speed(Client& client, RecvPacket& r) {
	client.desiredspeed = r.unsigned_16();
	update_network_speed();
}

/** a new file should be uploaded to all players */
void GameHost::handle_new_file(Client& client) {
	if (!file_) {  // Do we have a file for sending?
		throw DisconnectException("REQUEST_OF_N_E_FILE");
	}
	send_system_message_code(
	   "STARTED_SENDING_FILE", file_->filename, d->settings.users.at(client.usernum).name);
	// Send all parts to the interested client
	for (size_t part = 0; part < file_->parts.size(); ++part) {
		send_file_part(client.sock_id, part);
	}
	// Remember client as "currently receiving file"
	d->settings.users[client.usernum].ready = false;
	broadcast_setting_user(client.usernum);
}

/**
 * Handle a single received packet.
 *
 * The caller must catch exceptions and disconnect the client as appropriate.
 *
 * \param i the client number
 * \param r the received packet
 */
void GameHost::handle_packet(uint32_t const client_num, RecvPacket& r) {
	Client& client = d->clients.at(client_num);
	uint8_t const cmd = r.unsigned_8();

	if (cmd == NETCMD_DISCONNECT) {
		return handle_disconnect(client_num, r);
	}

	if (client.playernum == UserSettings::not_connected()) {
		if (cmd == NETCMD_METASERVER_PING) {
			return handle_ping(client);
		}
		// Now we wait for the client to say Hi in the right language,
		return handle_hello(client_num, cmd, client, r);
	}

	switch (cmd) {
	case NETCMD_PONG:
		verb_log_info("[Host]: Client %u: got pong", client_num);
		break;

	case NETCMD_SETTING_CHANGETRIBE:
		return handle_changetribe(client, r);
	case NETCMD_SETTING_CHANGESHARED:
		return handle_changeshared(client, r);
	case NETCMD_SETTING_CHANGETEAM:
		return handle_changeteam(client, r);
	case NETCMD_SETTING_CHANGECOLOR:
		return handle_changecolor(client, r);
	case NETCMD_SETTING_CHANGEINIT:
		return handle_changeinit(client, r);
	case NETCMD_SETTING_CHANGEPOSITION:
		return handle_changeposition(client, r);
	case NETCMD_TIME:
		return handle_nettime(client_num, r);
	case NETCMD_PLAYERCOMMAND:
		return handle_playercommmand(client_num, client, r);
	case NETCMD_SYNCREPORT:
		return handle_syncreport(client_num, client, r);
	case NETCMD_CHAT:
		return handle_chat(client, r);
	case NETCMD_SETSPEED:
		return handle_speed(client, r);
	case NETCMD_NEW_FILE_AVAILABLE:
		return handle_new_file(client);
	case NETCMD_FILE_PART:
		return handle_file_part(client, r);
	case NETCMD_SYSTEM_MESSAGE_CODE:
		return handle_system_message(r);

	case NETCMD_SETTING_MAP:
	case NETCMD_SETTING_PLAYER:
	case NETCMD_WIN_CONDITION:
	case NETCMD_PEACEFUL_MODE:
	case NETCMD_CUSTOM_STARTING_POSITIONS:
	case NETCMD_LAUNCH:
		if (!d->game) {  // not expected while game is in progress -> something is wrong here
			log_err("[Host]: Unexpected command %u while in game\n", cmd);
			throw DisconnectException(
			   "NO_ACCESS_TO_SERVER");  // TODO(k.halfmann): better use "UNEXPECTED_COMMAND" ?
		}
		break;

	default:
		throw ProtocolException(cmd);
	}
}

void GameHost::handle_system_message(RecvPacket& packet) {
	const std::string code = packet.string();
	const std::string arg1 = packet.string();
	const std::string arg2 = packet.string();
	const std::string arg3 = packet.string();
	if (code != "CHEAT") {
		log_err("[Host]: Received system command %s(%s,%s,%s) from client", code.c_str(),
		        arg1.c_str(), arg2.c_str(), arg3.c_str());
		throw DisconnectException("MALFORMED_COMMANDS");
	}
	send_system_message_code(code, arg1, arg2, arg3);
}

/** Handle uploading part of a file  */
void GameHost::handle_file_part(Client& client, RecvPacket& r) {
	if (!file_) {  // Do we have a file for sending
		throw DisconnectException("REQUEST_OF_N_E_FILE");
	}
	uint32_t part = r.unsigned_32();
	std::string md5sum = r.string();
	if (md5sum != file_->md5sum) {
		log_err("[Host]: File transfer checksum mismatch %s != %s\n", md5sum.c_str(),
		        file_->md5sum.c_str());
		return;  // Surely the file was changed, so we cancel here.
	}
	if (part >= file_->parts.size()) {
		log_warn(
		   "[Host]: Warning: Client reports to have received file part %u but we only have %" PRIuS
		   "\n",
		   part, file_->parts.size());
		return;
	}
	if (part == file_->parts.size() - 1) {
		send_system_message_code(
		   "COMPLETED_FILE_TRANSFER", file_->filename, d->settings.users.at(client.usernum).name);
		d->settings.users[client.usernum].ready = true;
		broadcast_setting_user(client.usernum);
	}
}

void GameHost::send_file_part(NetHostInterface::ConnectionId csock_id, uint32_t part) {
	assert(part < file_->parts.size());

	uint32_t left = file_->bytes - NETFILEPARTSIZE * part;
	uint32_t size = (left > NETFILEPARTSIZE) ? NETFILEPARTSIZE : left;

	// Send the part
	SendPacket packet;
	packet.unsigned_8(NETCMD_FILE_PART);
	packet.unsigned_32(part);
	packet.unsigned_32(size);
	packet.data(file_->parts[part].part, size);
	d->net->send(csock_id, packet, NetPriority::kFiletransfer);
}

void GameHost::disconnect_player_controller(uint8_t const number, const std::string& name) {
	log_warn("[Host]: disconnect_player_controller(%u, %s)\n", number, name.c_str());

	for (const UserSettings& setting : d->settings.users) {
		if (setting.position == number) {
			if (!d->game) {
				remove_player_name(number, name);
			}
			return;
		}
	}

	set_player_state(number, PlayerSettings::State::kOpen);
	// Don't replace player with AI, let host choose what to do
}

void GameHost::disconnect_client(uint32_t const client_number,
                                 const std::string& reason,
                                 bool const sendreason,
                                 const std::string& arg) {
	assert(client_number < d->clients.size());

	Client& client = d->clients.at(client_number);

	// If the client is linked to a player and it is the client that closes the connection
	// and the game has already started ...
	if (client.playernum != UserSettings::none() && reason != "SERVER_LEFT" &&
	    reason != "SERVER_CRASHED" && d->game != nullptr) {
		// And the client hasn't lost/won yet ...
		if (d->settings.users.at(client.usernum).result == Widelands::PlayerEndResult::kUndefined) {
			// If not shown yet, show a window and ask the host player what to do
			// with the tribe of the leaving client
			if (d->game->get_igbase()->show_game_client_disconnected()) {
				// Window has just been opened, pause game and create a save game
				if (!forced_pause()) {
					force_pause();
				}
				WLApplication::emergency_save(nullptr, *d->game, reason, 1, false);
			}
			// Client was active but is a winner of the game: Replace with normal AI
		} else if (d->settings.users.at(client.usernum).result == Widelands::PlayerEndResult::kWon) {
			replace_client_with_ai(client.playernum, AI::DefaultAI::normal_impl.name);
			// Client was active but has lost or gave up: Replace with empty AI
		} else {
			assert(d->settings.users.at(client.usernum).result == Widelands::PlayerEndResult::kLost ||
			       d->settings.users.at(client.usernum).result ==
			          Widelands::PlayerEndResult::kResigned);
			replace_client_with_ai(client.playernum, "empty");
		}
	}

	// If the client was completely connected before the disconnect, free the
	// user settings and send changes to the clients
	if (client.usernum >= 0) {
		uint8_t position = d->settings.users.at(client.usernum).position;
		d->settings.users.at(client.usernum).position = UserSettings::not_connected();
		client.playernum = UserSettings::not_connected();
		if (position <= UserSettings::highest_playernum()) {
			disconnect_player_controller(position, d->settings.users.at(client.usernum).name);
		}
		// Do NOT reset the clients name in the corresponding UserSettings, that way we keep the name
		// for the statistics.
		// d->settings.users.at(client.usernum).name = std::string();

		// Broadcast the user changes to everybody
		send_system_message_code(
		   "CLIENT_X_LEFT_GAME", d->settings.users.at(client.usernum).name, reason, arg);

		broadcast_setting_user(client.usernum);
	} else {
		send_system_message_code("UNKNOWN_LEFT_GAME", reason, arg);
	}
	log_warn("[Host]: disconnect_client(%u, %s, %s)\n", client_number, reason.c_str(), arg.c_str());

	if (client.sock_id > 0) {
		if (sendreason) {
			SendPacket packet;
			packet.unsigned_8(NETCMD_DISCONNECT);
			packet.unsigned_8(arg.empty() ? 1 : 2);
			packet.string(reason);
			if (!arg.empty()) {
				packet.string(arg);
			}
			d->net->send(client.sock_id, packet);
		}

		d->net->close(client.sock_id);
		client.sock_id = 0;
	}

	if (d->game) {
		check_hung_clients();
	}
}

/**
 * The grim reaper. This finally erases disconnected clients from the clients
 * array.
 *
 * Calls this when you're certain that nobody is holding any client indices or
 * iterators, since this function will invalidate them.
 */
void GameHost::reaper() {
	uint32_t index = 0;
	while (index < d->clients.size()) {
		if (d->clients.at(index).sock_id > 0) {
			++index;
		} else {
			d->clients.erase(d->clients.begin() + index);
		}
	}
}

void GameHost::report_result(uint8_t p_nr,
                             Widelands::PlayerEndResult result,
                             const std::string& info) {
	// Send to game
	Widelands::PlayerEndStatus pes;
	Widelands::Player* player = d->game->get_player(p_nr);
	assert(player);
	pes.player = player->player_number();
	pes.time = d->game->get_gametime();
	pes.result = result;
	pes.info = info;
	d->game->player_manager()->add_player_end_status(pes);

	// there might be more than one client that control this Widelands player
	// and maybe even none -> computer player
	for (UserSettings& user : d->settings.users) {
		if (user.position == p_nr - 1) {
			user.result = result;
			user.win_condition_string = info;
			if (result == Widelands::PlayerEndResult::kLost) {
				send_system_message_code("PLAYER_DEFEATED", user.name);
			}
		}
	}

	verb_log_info("GameHost::report_result(%d, %u, %s)", player->player_number(),
	              static_cast<uint8_t>(result), info.c_str());
}
