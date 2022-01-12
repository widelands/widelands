/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#include "network/internet_gaming.h"

#include <algorithm>
#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "base/string.h"
#include "base/warning.h"
#include "build_info.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "network/internet_gaming_messages.h"
#include "network/internet_gaming_protocol.h"
#include "third_party/sha1/sha1.h"

/// Max length of formatted time string
const uint8_t kTimeFormatLength = 32;

/// Private constructor by purpose: NEVER call directly. Always call InternetGaming::ref(), this
/// will ensure
/// that only one instance is running at time.
InternetGaming::InternetGaming()
   : net(nullptr),
     state_(OFFLINE),
     reg_(false),
     port_(kInternetGamingPort),
     clientrights_(INTERNET_CLIENT_UNREGISTERED),
     clientupdateonmetaserver_(true),
     gameupdateonmetaserver_(true),
     clientupdate_(false),
     gameupdate_(false),
     time_offset_(0),
     waittimeout_(std::numeric_limits<int32_t>::max()),
     lastping_(time(nullptr)) {
	// Fill the list of possible messages from the server
	InternetGamingMessages::fill_map();

	// Set connection tracking variables to 0
	lastbrokensocket_[0] = 0;
	lastbrokensocket_[1] = 0;
}

/// resets all stored variables without the chat messages for a clean new login (not relogin)
void InternetGaming::reset() {
	net.reset();
	state_ = OFFLINE;
	authenticator_ = "";
	reg_ = false;
	meta_ = INTERNET_GAMING_METASERVER;
	port_ = kInternetGamingPort;
	clientname_ = "";
	clientrights_ = INTERNET_CLIENT_UNREGISTERED;
	gamename_ = "";
	gameips_ = std::make_pair(NetAddress(), NetAddress());
	clientupdateonmetaserver_ = true;
	gameupdateonmetaserver_ = true;
	clientupdate_ = false;
	gameupdate_ = false;
	time_offset_ = 0;
	waitcmd_ = "";
	waittimeout_ = std::numeric_limits<int32_t>::max();
	lastbrokensocket_[0] = 0;
	lastbrokensocket_[1] = 0;
	lastping_ = time(nullptr);

	clientlist_.clear();
	gamelist_.clear();
}

/// the one and only InternetGaming instance.
static InternetGaming* ig = nullptr;

/// \returns the one and only InternetGaming instance.
InternetGaming& InternetGaming::ref() {
	if (!ig) {
		ig = new InternetGaming();
	}
	return *ig;
}

void InternetGaming::initialize_connection() {
	// First of all try to connect to the metaserver
	verb_log_info("InternetGaming: Connecting to the metaserver.");
	NetAddress addr;
	if (NetAddress::resolve_to_v6(&addr, meta_, port_)) {
		net = NetClient::connect(addr);
	}
	if ((!net || !net->is_connected()) && NetAddress::resolve_to_v4(&addr, meta_, port_)) {
		net = NetClient::connect(addr);
	}
	if (!net || !net->is_connected()) {
		throw WLWarning(_("Could not establish connection to host"),
		                _("Widelands could not establish a connection to the given address.\n"
		                  "Either there was no metaserver running at the supposed port or\n"
		                  "your network setup is broken."));
	}

	// Of course not 100% true, but we just care about an answer at all, so we reset this tracker
	lastping_ = time(nullptr);
}

/// Login to metaserver
bool InternetGaming::login(const std::string& nick,
                           const std::string& authenticator,
                           bool registered,
                           const std::string& meta,
                           uint32_t port) {

	// Reset local state. Only resetting on logout() or error isn't enough since
	// the game might jump to the main menu from other places, too
	reset();

	clientname_ = nick;
	reg_ = registered;
	meta_ = meta;
	port_ = port;

	if (registered) {
		authenticator_ = authenticator;
	} else {
		authenticator_ = crypto::sha1(nick + authenticator);
	}

	assert(!authenticator_.empty());

	return do_login();
}

bool InternetGaming::do_login(bool should_relogin) {

	initialize_connection();

	// If we are here, a connection was established and we can send our login package through the
	// socket.
	verb_log_info("InternetGaming: Sending login request.");
	SendPacket s;
	s.string(IGPCMD_LOGIN);
	s.string(as_string(kInternetGamingProtocolVersion));
	s.string(clientname_);
	s.string(build_id());
	s.string(bool2str(reg_));
	s.string(reg_ ? "" : authenticator_);
	net->send(s);

	// Now let's see, whether the metaserver is answering
	uint32_t const secs = time(nullptr);
	state_ = CONNECTING;
	while (kInternetGamingTimeout > time(nullptr) - secs) {
		handle_metaserver_communication();
		// Check if we are a step further... if yes handle_packet has taken care about all the
		// paperwork, so we put our feet up and just return. ;)
		if (state_ != CONNECTING) {
			if (state_ == LOBBY) {
				if (!should_relogin) {
					format_and_add_chat(
					   "", "", true, _("Users marked with IRC will possibly not react to messages."));
				}

				return true;
			} else if (error()) {
				return false;
			}
		}
	}
	log_err("InternetGaming: No answer from metaserver!\n");
	logout("NO_ANSWER");
	return false;
}

/// Relogin to metaserver after loosing connection
bool InternetGaming::relogin() {
	if (!error()) {
		throw wexception("InternetGaming::relogin: This only makes sense if there was an error.");
	}

	if (!do_login(true)) {
		return false;
	}

	state_ = LOBBY;
	// Client is reconnected, so let's try resend the timeouted command.
	if (waitcmd_ == IGPCMD_GAME_CONNECT) {
		join_game(gamename_);
	} else if (waitcmd_ == IGPCMD_GAME_OPEN) {
		state_ = IN_GAME;
		open_game();
	} else if (waitcmd_ == IGPCMD_GAME_START) {
		state_ = IN_GAME;
		set_game_playing();
	}

	verb_log_info("InternetGaming: Reconnected to metaserver");
	format_and_add_chat("", "", true, _("Successfully reconnected to the metaserver!"));

	return true;
}

/// logout of the metaserver
/// \note \arg msgcode should be a message from the list of InternetGamingMessages
void InternetGaming::logout(const std::string& msgcode) {

	// Just in case the metaserver is listening on the socket - tell him we break up with him ;)
	if (net && net->is_connected()) {
		SendPacket s;
		s.string(IGPCMD_DISCONNECT);
		s.string(msgcode);
		net->send(s);
	}

	const std::string& msg = InternetGamingMessages::get_message(msgcode);
	verb_log_info("InternetGaming: logout(%s)", msg.c_str());
	format_and_add_chat("", "", true, msg);

	reset();
}

bool InternetGaming::check_password(const std::string& nick,
                                    const std::string& pwd,
                                    const std::string& metaserver,
                                    uint32_t port) {
	reset();

	meta_ = metaserver;
	port_ = port;
	initialize_connection();

	// Has to be set for the password challenge later on
	authenticator_ = pwd;

	verb_log_info("InternetGaming: Verifying password.");
	{
		SendPacket s;
		s.string(IGPCMD_CHECK_PWD);
		s.string(as_string(kInternetGamingProtocolVersion));
		s.string(nick);
		s.string(build_id());
		net->send(s);
	}

	// Now let's see, whether the metaserver is answering
	uint32_t const secs = time(nullptr);
	state_ = CONNECTING;
	while (kInternetGamingTimeout > time(nullptr) - secs) {
		handle_metaserver_communication(false);
		if (state_ != CONNECTING) {
			if (state_ == LOBBY) {
				SendPacket s;
				s.string(IGPCMD_DISCONNECT);
				s.string("CONNECTION_CLOSED");
				net->send(s);
				reset();
				return true;
			} else if (error()) {
				reset();
				return false;
			}
		}
	}
	log_err("InternetGaming: No answer from metaserver!\n");
	reset();
	return false;
}

/**
 * Handle situation when reading from socket failed.
 */
void InternetGaming::handle_failed_read() {
	set_error();
	const std::string& msg = InternetGamingMessages::get_message("CONNECTION_LOST");
	log_err("InternetGaming: Error: %s\n", msg.c_str());
	format_and_add_chat("", "", true, msg);

	// Check how much time passed since the socket broke the last time
	// Maybe something is completely wrong at the moment?
	// At least it seems to be, if the socket broke three times in the last 10 seconds...
	time_t now = time(nullptr);
	if ((now - lastbrokensocket_[1] < 10) && (now - lastbrokensocket_[0] < 10)) {
		reset();
		set_error();
		return;
	}
	lastbrokensocket_[1] = lastbrokensocket_[0];
	lastbrokensocket_[0] = now;

	// Try to relogin
	if (!relogin()) {
		// Do not try to relogin again automatically.
		reset();
		set_error();
	}
}

/// handles all communication between the metaserver and the client
void InternetGaming::handle_metaserver_communication(bool relogin_on_error) {
	if (error()) {
		return;
	}
	try {
		while (net != nullptr) {
			// Check if the connection is still open
			if (!net->is_connected()) {
				handle_failed_read();
				return;
			}
			// Process all available packets
			std::unique_ptr<RecvPacket> packet = net->try_receive();
			if (packet) {
				handle_packet(*packet, relogin_on_error);
			} else {
				// Nothing more to receive
				break;
			}
		}
	} catch (const std::exception& e) {
		logout(format(_("Something went wrong: %s"), e.what()));
		set_error();
	}

	if (state_ == LOBBY) {
		// client is in the lobby and therefore we want realtime information updates
		if (clientupdateonmetaserver_) {
			SendPacket s;
			s.string(IGPCMD_CLIENTS);
			net->send(s);

			clientupdateonmetaserver_ = false;
		}

		if (gameupdateonmetaserver_) {
			SendPacket s;
			s.string(IGPCMD_GAMES);
			net->send(s);

			gameupdateonmetaserver_ = false;
		}
	}

	if (!waitcmd_.empty()) {
		// Check if timeout is reached
		time_t now = time(nullptr);
		if (now > waittimeout_) {
			set_error();
			waittimeout_ = std::numeric_limits<int32_t>::max();
			log_err("InternetGaming: reached a timeout for an awaited answer of the metaserver!\n");
			if (relogin_on_error && !relogin()) {
				// Do not try to relogin again automatically.
				reset();
				set_error();
			}
		}
	}

	// Check connection to the metaserver
	// Was a ping received in the last 4 minutes?
	if (time(nullptr) - lastping_ > 240) {
		// Try to relogin
		set_error();
		if (relogin_on_error && !relogin()) {
			// Do not try to relogin again automatically.
			reset();
			set_error();
		}
	}
}

/// Handle one packet received from the metaserver.
void InternetGaming::handle_packet(RecvPacket& packet, bool relogin_on_error) {
	std::string cmd = packet.string();

	// First check if everything is fine or whether the metaserver broke up with the client.
	if (cmd == IGPCMD_DISCONNECT) {
		std::string reason = packet.string();
		format_and_add_chat("", "", true, InternetGamingMessages::get_message(reason));
		if (reason == "CLIENT_TIMEOUT") {
			// Try to relogin
			set_error();
			if (relogin_on_error && !relogin()) {
				// Do not try to relogin again automatically.
				reset();
				set_error();
			}
		}
		return;
	} else if (cmd == IGPCMD_PING) {
		// Client received a PING and should immediately PONG as requested
		SendPacket s;
		s.string(IGPCMD_PONG);
		net->send(s);

		lastping_ = time(nullptr);
		return;
	}

	// Are we already online?
	if (state_ == CONNECTING) {
		if (cmd == IGPCMD_PWD_CHALLENGE) {
			const std::string nonce = packet.string();
			SendPacket s;
			s.string(IGPCMD_PWD_CHALLENGE);
			s.string(crypto::sha1(nonce + authenticator_));
			net->send(s);
			return;

		} else if (cmd == IGPCMD_LOGIN) {
			// Clients request to login was granted
			format_and_add_chat("", "", true, _("Welcome to the Widelands Metaserver!"));
			const std::string assigned_name = packet.string();
			if (clientname_ != assigned_name) {
				format_and_add_chat("", "", true,
				                    format(_("You have been logged in as ‘%s’ since your "
				                             "requested name is already in use or reserved."),
				                           assigned_name));
			}
			clientname_ = assigned_name;
			clientrights_ = packet.string();
			if (reg_ && clientrights_ == INTERNET_CLIENT_UNREGISTERED) {
				// Permission downgrade: We logged in with less rights than we wanted to.
				// Happens when we are already logged in with another client.
				reg_ = false;
				authenticator_ = crypto::sha1(clientname_ + authenticator_);
			}
			format_and_add_chat("", "", true, _("Our forums can be found at:"));
			format_and_add_chat("", "", true, "https://www.widelands.org/forum/");
			format_and_add_chat("", "", true, _("For reporting bugs, visit:"));
			format_and_add_chat("", "", true, "https://www.widelands.org/wiki/ReportingBugs/");
			state_ = LOBBY;
			// Append UTC time to login message to ease linking between
			// client output and metaserver logs.
			char time_str[kTimeFormatLength];
			format_time(time_str, kTimeFormatLength);
			verb_log_info(
			   "InternetGaming: Client %s logged in at UTC %s", clientname_.c_str(), time_str);
			return;

		} else if (cmd == IGPCMD_PWD_OK) {
			char time_str[kTimeFormatLength];
			format_time(time_str, kTimeFormatLength);
			verb_log_info("InternetGaming: Password check successful at UTC %s", time_str);
			state_ = LOBBY;
			return;

		} else if (cmd == IGPCMD_ERROR) {
			std::string errortype = packet.string();
			if (errortype != IGPCMD_LOGIN && errortype != IGPCMD_PWD_CHALLENGE) {
				log_err("InternetGaming: Strange ERROR in connecting state: %s\n", errortype.c_str());
				throw WLWarning(
				   _("Mixed up"), _("The metaserver sent a strange ERROR during connection"));
			}
			// Clients login request got rejected
			logout(packet.string());
			set_error();
			return;

		} else {
			logout();
			set_error();
			log_err(
			   "InternetGaming: Expected a LOGIN, PWD_CHALLENGE or ERROR packet from server, but "
			   "received command %s. Maybe the metaserver is using a different protocol version?\n",
			   cmd.c_str());
			throw WLWarning(
			   _("Unexpected packet"),
			   _("Received an unexpected network packet from the metaserver. The metaserver could be "
			     "using a different protocol version. If the error persists, try updating your "
			     "game."));
		}
	}
	try {
		if (cmd == IGPCMD_LOGIN) {
			// Login specific commands but not in CONNECTING state...
			log_err("InternetGaming: Received %s cmd although client is not in CONNECTING state.\n",
			        cmd.c_str());
			std::string temp = format(
			   _("WARNING: Received a %s command although we are not in CONNECTING state."), cmd);
			format_and_add_chat("", "", true, temp);
		}

		else if (cmd == IGPCMD_TIME) {
			// Client received the server time
			time_offset_ = stoi(packet.string()) - time(nullptr);
			verb_log_info("InternetGaming: Server time offset is %d second(s).", time_offset_);
			std::string temp = format(ngettext("Server time offset is %d second.",
			                                   "Server time offset is %d seconds.", time_offset_),
			                          time_offset_);
			format_and_add_chat("", "", true, temp);
		}

		else if (cmd == IGPCMD_CHAT) {
			// Client received a chat message
			std::string sender = packet.string();
			std::string message = packet.string();
			std::string type = packet.string();

			if (type != "public" && type != "private" && type != "system") {
				throw WLWarning(
				   _("Invalid message type"), _("Invalid chat message type \"%s\"."), type.c_str());
			}

			bool personal = type == "private";
			bool system = type == "system";

			format_and_add_chat(sender, personal ? clientname_ : "", system, message);
		}

		else if (cmd == IGPCMD_GAMES_UPDATE) {
			// Client received a note, that the list of games was changed
			verb_log_info("InternetGaming: Game update on metaserver.");
			gameupdateonmetaserver_ = true;
		}

		else if (cmd == IGPCMD_GAMES) {
			// Client received the new list of games
			uint8_t number = stoi(packet.string()) & 0xff;
			std::vector<InternetGame> old = gamelist_;
			gamelist_.clear();
			verb_log_info("InternetGaming: Received a game list update with %u items.", number);
			for (uint8_t i = 0; i < number; ++i) {
				InternetGame* ing = new InternetGame();
				ing->name = packet.string();
				ing->build_id = packet.string();
				ing->connectable = packet.string();
				gamelist_.push_back(*ing);

				bool found = false;
				for (InternetGame& old_game : old) {
					if (old_game.name == ing->name) {
						found = true;
						old_game.name = "";
						break;
					}
				}
				if (!found && ing->connectable != INTERNET_GAME_RUNNING &&
				    (ing->build_id == build_id() || (ing->build_id.compare(0, 6, "build-") != 0 &&
				                                     build_id().compare(0, 6, "build-") != 0))) {
					format_and_add_chat(
					   "", "", true, format(_("The game %s is now available"), ing->name));
				}

				delete ing;
				ing = nullptr;
			}

			for (InternetGame& old_game : old) {
				if (!old_game.name.empty()) {
					format_and_add_chat(
					   "", "", true, format(_("The game %s has been closed"), old_game.name));
				}
			}

			gameupdate_ = true;
		}

		else if (cmd == IGPCMD_CLIENTS_UPDATE) {
			// Client received a note, that the list of clients was changed
			verb_log_info("InternetGaming: Client update on metaserver.");
			clientupdateonmetaserver_ = true;
		}

		else if (cmd == IGPCMD_CLIENTS) {
			// Client received the new list of clients
			uint8_t number = stoi(packet.string()) & 0xff;
			std::vector<InternetClient> old = clientlist_;
			// Push admins/registred/IRC users to a temporary list and add them back later
			clientlist_.clear();
			verb_log_info("InternetGaming: Received a client list update with %u items.", number);
			InternetClient inc;
			for (uint8_t i = 0; i < number; ++i) {
				inc.name = packet.string();
				inc.build_id = packet.string();
				inc.game = packet.string();
				inc.type = packet.string();

				clientlist_.push_back(inc);

				bool found =
				   old.empty();  // do not show all clients, if this instance is the actual change
				for (InternetClient& client : old) {
					if (client.name == inc.name && client.type == inc.type) {
						found = true;
						client.name = "";
						break;
					}
				}
				if (!found) {
					format_and_add_chat("", "", true, format(_("%s joined the lobby"), inc.name));
				}
			}

			std::sort(clientlist_.begin(), clientlist_.end(),
			          [](const InternetClient& left, const InternetClient& right) {
				          return (left.name < right.name);
			          });

			for (InternetClient& client : old) {
				if (!client.name.empty()) {
					format_and_add_chat("", "", true, format(_("%s left the lobby"), client.name));
				}
			}
			clientupdate_ = true;
		}

		else if (cmd == IGPCMD_GAME_OPEN) {
			// Client received the acknowledgment, that the game was opened
			// We can't use an assert here since this message might arrive after the game already
			// started
			if (waitcmd_ == IGPCMD_GAME_OPEN) {
				waitcmd_ = "";
			}
			// Get the challenge
			std::string challenge = packet.string();
			relay_password_ = crypto::sha1(challenge + authenticator_);
			// Save the received IP(s), so the client can connect to the game
			NetAddress::parse_ip(&gameips_.first, packet.string(), kInternetRelayPort);
			// If the next value is true, a secondary IP follows
			if (packet.string() == bool2str(true)) {
				NetAddress::parse_ip(&gameips_.second, packet.string(), kInternetRelayPort);
			}
			verb_log_info("InternetGaming: Received ips of the relay to host: %s %s.",
			              gameips_.first.ip.to_string().c_str(),
			              gameips_.second.ip.to_string().c_str());
			state_ = IN_GAME;
		}

		else if (cmd == IGPCMD_GAME_CONNECT) {
			// Client received the ip for the game it wants to join
			assert(waitcmd_ == IGPCMD_GAME_CONNECT);
			waitcmd_ = "";
			// Save the received IP(s), so the client can connect to the game
			NetAddress::parse_ip(&gameips_.first, packet.string(), kInternetRelayPort);
			// If the next value is true, a secondary IP follows
			if (packet.string() == bool2str(true)) {
				NetAddress::parse_ip(&gameips_.second, packet.string(), kInternetRelayPort);
			}
			verb_log_info("InternetGaming: Received ips of the game to join: %s %s.",
			              gameips_.first.ip.to_string().c_str(),
			              gameips_.second.ip.to_string().c_str());
		}

		else if (cmd == IGPCMD_GAME_START) {
			// Client received the acknowledgment, that the game was started
			assert(waitcmd_ == IGPCMD_GAME_START);
			waitcmd_ = "";
		}

		else if (cmd == IGPCMD_ERROR) {
			// Client received an ERROR message - seems something went wrong
			std::string subcmd(packet.string());
			std::string reason(packet.string());
			std::string message;

			if (subcmd == IGPCMD_CHAT) {
				// Something went wrong with the chat message the user sent.
				message += _("Chat message could not be sent.");
				if (reason == "NO_SUCH_USER") {
					message = format("%s %s", message, InternetGamingMessages::get_message(reason));
				}
			}

			else if (subcmd == IGPCMD_CMD) {
				// Something went wrong with the command
				message += _("Command could not be executed.");
				message = format("%s %s", message, InternetGamingMessages::get_message(reason));
			}

			else if (subcmd == IGPCMD_GAME_OPEN) {
				// Something went wrong with the newly opened game
				message = InternetGamingMessages::get_message(reason);
				// we got our answer, so no need to wait anymore
				waitcmd_ = "";
			}

			else if (subcmd == IGPCMD_GAME_CONNECT && reason == "NO_SUCH_GAME") {
				log_warn("InternetGaming: The game no longer exists, maybe it has just been closed\n");
				message = InternetGamingMessages::get_message(reason);
				assert(waitcmd_ == IGPCMD_GAME_CONNECT);
				waitcmd_ = "";
			}
			if (!message.empty()) {
				message = format(_("ERROR: %s"), message);
			} else {
				message =
				   format(_("An unexpected error message has been received about command %1%: %2%"),
				          subcmd, reason);
			}

			// Finally send the error message as system chat to the client.
			format_and_add_chat("", "", true, message);
		}

		else {
			// Inform the client about the unknown command
			format_and_add_chat(
			   "", "", true, format(_("Received an unknown command from the metaserver: %s"), cmd));
		}

	} catch (WLWarning& e) {
		format_and_add_chat("", "", true, e.what());
	}
}

/// \returns Up to two NetAdress with ips of the game the client is on or wants to join
///          (or the client is hosting) or invalid addresses, if no ip available.
const std::pair<NetAddress, NetAddress>& InternetGaming::ips() {
	return gameips_;
}

bool InternetGaming::wait_for_ips() {
	// Wait until the metaserver provided us with an IP address
	uint32_t const secs = time(nullptr);
	const bool is_waiting_for_connect = (waitcmd_ == IGPCMD_GAME_CONNECT);
	while (!gameips_.first.is_valid()) {
		if (error()) {
			return false;
		}
		if (is_waiting_for_connect && waitcmd_.empty()) {
			// Was trying to join a game but failed.
			// It probably means that the game is no longer available
			return false;
		}
		handle_metaserver_communication();
		// give some time for the answer + for a relogin, if a problem occurs.
		if ((kInternetGamingTimeout * 5 / 3) < time(nullptr) - secs) {
			return false;
		}
	}
	return true;
}

const std::string InternetGaming::relay_password() {
	return relay_password_;
}

/// called by a client to join the game \arg gamename
void InternetGaming::join_game(const std::string& gamename) {
	if (!logged_in()) {
		return;
	}

	// Reset the game ips, we should receive new ones shortly
	gameips_ = std::make_pair(NetAddress(), NetAddress());

	SendPacket s;
	s.string(IGPCMD_GAME_CONNECT);
	s.string(gamename);
	net->send(s);
	gamename_ = gamename;
	verb_log_info("InternetGaming: Client tries to join a game with the name %s", gamename_.c_str());
	state_ = IN_GAME;

	// From now on we wait for a reply from the metaserver
	waitcmd_ = IGPCMD_GAME_CONNECT;
	waittimeout_ = time(nullptr) + kInternetGamingTimeout;
}

/// called by a client to open a new game with name gamename_
void InternetGaming::open_game() {
	if (!logged_in()) {
		return;
	}

	// Reset the game ips, we should receive new ones shortly
	gameips_ = std::make_pair(NetAddress(), NetAddress());

	SendPacket s;
	s.string(IGPCMD_GAME_OPEN);
	s.string(gamename_);
	net->send(s);
	verb_log_info("InternetGaming: Client opened a game with the name %s.", gamename_.c_str());

	// From now on we wait for a reply from the metaserver
	waitcmd_ = IGPCMD_GAME_OPEN;
	waittimeout_ = time(nullptr) + kInternetGamingTimeout;
}

/// called by a client that is host of a game to inform the metaserver, that the game started
void InternetGaming::set_game_playing() {
	if (!logged_in()) {
		return;
	}

	SendPacket s;
	s.string(IGPCMD_GAME_START);
	net->send(s);
	verb_log_info("InternetGaming: Client announced the start of the game %s.", gamename_.c_str());

	// From now on we wait for a reply from the metaserver
	waitcmd_ = IGPCMD_GAME_START;
	waittimeout_ = time(nullptr) + kInternetGamingTimeout;
}

/// called by a client to inform the metaserver, that it left the game and is back in the lobby.
/// If this is called by the hosting client, this further informs the metaserver, that the game was
/// closed.
void InternetGaming::set_game_done() {
	if (!logged_in()) {
		return;
	}

	SendPacket s;
	s.string(IGPCMD_GAME_DISCONNECT);
	net->send(s);

	gameips_ = std::make_pair(NetAddress(), NetAddress());
	state_ = LOBBY;

	verb_log_info(
	   "InternetGaming: Client announced the disconnect from the game %s.", gamename_.c_str());
}

/// \returns whether the local gamelist was updated
/// \note this function resets gameupdate_. So if you call it, please really handle the output.
bool InternetGaming::update_for_games() {
	bool temp = gameupdate_;
	gameupdate_ = false;
	return temp;
}

/// \returns the tables in the room, if no error occured, or nullptr in case of error
const std::vector<InternetGame>* InternetGaming::games() {
	return error() ? nullptr : &gamelist_;
}

/// \returns whether the local clientlist_ was updated
/// \note this function resets clientupdate_. So if you call it, please really handle the output.
bool InternetGaming::update_for_clients() {
	bool temp = clientupdate_;
	clientupdate_ = false;
	return temp;
}

/// \returns the players in the room, if no error occured, or nullptr in case of error
const std::vector<InternetClient>* InternetGaming::clients() {
	return error() ? nullptr : &clientlist_;
}

/// ChatProvider: sends a message via the metaserver.
void InternetGaming::send(const std::string& msg) {
	// TODO(Notabilis): Messages can get lost when we are temporarily disconnected from the
	// metaserver, even when we reconnect again. "Answered" messages like IGPCMD_GAME_CONNECT
	// are resent but chat messages are not. Resend them after some time when we did not receive
	// the matching IGPCMD_CHAT command from the server? For global/public messages we could wait
	// for the returned IGPCMD_CHAT from the metaserver, similar to other commands.
	// What about private messages? Maybe modify the metaserver to send them back, too?
	if (!logged_in()) {
		format_and_add_chat(
		   "", "", true, _("Message could not be sent: You are not connected to the metaserver!"));
		return;
	}

	std::string trimmed = msg;
	trim(trimmed);
	if (trimmed.empty()) {
		// Message is empty or only space characters. We don't want it either way
		return;
	}

	SendPacket s;
	s.string(IGPCMD_CHAT);

	if (*msg.begin() == '@') {
		// Format a personal message
		std::string::size_type const space = msg.find(' ');
		if (space >= msg.size() - 1) {
			format_and_add_chat(
			   "", "", true,
			   _("Message could not be sent: Was this supposed to be a private message?"));
			return;
		}
		trimmed = msg.substr(space + 1);
		trim(trimmed);
		if (trimmed.empty()) {
			format_and_add_chat(
			   "", "", true,
			   _("Message could not be sent: Was this supposed to be a private message?"));
			return;
		}

		s.string(trimmed);                   // message
		s.string(msg.substr(1, space - 1));  // recipient

		format_and_add_chat(clientname_, msg.substr(1, space - 1), false, msg.substr(space + 1));

	} else if (clientrights_ == INTERNET_CLIENT_SUPERUSER && *msg.begin() == '/') {
		// This is either a /me command, a super user command, or well... just a chat message
		// beginning
		// with a "/" - let's see...

		if (msg == "/help") {
			format_and_add_chat("", "", true, _("Supported admin commands:"));
			format_and_add_chat("", "", true, _("/motd <msg> - Set a permanent greeting message"));
			format_and_add_chat("", "", true, _("/announce <msg> - Send a one time system message"));
			format_and_add_chat(
			   "", "", true,
			   _("/warn <user> <msg> - Send a private system message to the given user"));
			format_and_add_chat(
			   "", "", true,
			   _("/kick <user|game> - Remove the given user or game from the metaserver"));
			format_and_add_chat(
			   "", "", true, _("/ban <user> - Ban a user for 24 hours from the metaserver"));
			return;
		}

		// Split up in "cmd" "arg"
		std::string cmd, arg;
		std::string temp = msg.substr(1);  // cut off '/'
		std::string::size_type const space = temp.find(' ');
		if (space > temp.size()) {
			// no argument
			goto normal;
		}

		// get the cmd and the arg
		cmd = temp.substr(0, space);
		arg = temp.substr(space + 1);
		trim(arg);

		if (!arg.empty() && cmd == "motd") {
			SendPacket m;
			m.string(IGPCMD_MOTD);
			// Check whether motd is attached or should be loaded from a file
			if (arg.size() > 1 && arg.at(0) == '%') {
				// Seems we should load the motd from a file
				temp = arg.substr(1);  // cut of the "%"
				if (g_fs->file_exists(temp) && !g_fs->is_directory(temp)) {
					// Read in the file
					FileRead fr;
					fr.open(*g_fs, temp);
					if (!fr.end_of_file()) {
						arg = fr.read_line();
						while (!fr.end_of_file()) {
							arg += fr.read_line();
						}
					}
				}
			}
			// send the request to change the motd
			m.string(arg);
			net->send(m);
			return;
		} else if (!arg.empty() && cmd == "announce") {
			// send the request to make an announcement
			SendPacket m;
			m.string(IGPCMD_ANNOUNCEMENT);
			m.string(arg);
			net->send(m);
			return;
		} else if (!arg.empty() && (cmd == "warn" || cmd == "kick" || cmd == "ban")) {
			// warn a user by sending a private system message or
			// kick a user (for 5 minutes) or a game from the metaserver or
			// ban a user for 24 hours
			SendPacket m;
			m.string(IGPCMD_CMD);
			m.string(cmd);
			m.string(arg);
			net->send(m);
			return;
		} else {
			// let everything else pass
			goto normal;
		}
	} else {
	normal:
		s.string(msg);
		s.string("");
	}

	net->send(s);
}

/**
 * \returns the boolean value of a string received from the metaserver.
 * If conversion fails, it throws a \ref warning
 */
bool InternetGaming::str2bool(const std::string& str) {
	if ((str != "true") && (str != "false")) {
		throw WLWarning(_("Conversion error"),
		                /** TRANSLATORS: Geeky message from the metaserver */
		                /** TRANSLATORS: This message is shown if %s isn't "true" or "false" */
		                _("Unable to determine truth value for \"%s\""), str.c_str());
	}
	return str == "true";
}

/// \returns a string containing the boolean value \arg b to be send to metaserver
std::string InternetGaming::bool2str(bool b) {
	return b ? "true" : "false";
}

/// Formats the current time as string in \arg time_str of max \arg length
void InternetGaming::format_time(char* time_str, uint8_t length) {
	const time_t now = time(nullptr);
	// Time format: Www Mmm dd yyyy hh:mm:ss
	strftime(time_str, length, "%a %b %d %Y %H:%M:%S", gmtime(&now));
}

/// formates a chat message and adds it to the list of chat messages
void InternetGaming::format_and_add_chat(const std::string& from,
                                         const std::string& to,
                                         bool system,
                                         const std::string& msg) {
	ChatMessage c(msg);
	if (!system && from.empty()) {
		std::string unkown_string = format("<%s>", pgettext("chat_sender", "Unknown"));
		c.sender = unkown_string;
	} else {
		c.sender = from;
	}
	c.playern = system ? -1 : to.empty() ? 7 : 3;
	c.recipient = to;

	receive(c);
	if (system && (state_ == IN_GAME)) {
		// Save system chat messages separately as well, so the nethost can import and show them in
		// game;
		c.msg = "METASERVER: " + msg;
		ingame_system_chat_.push_back(c);
	}
}

/**
 * Check for vaild username characters and make sure it's not "team".
 */
bool InternetGaming::valid_username(const std::string& username) {
	if (username.empty() ||
	    username.find_first_not_of("abcdefghijklmnopqrstuvwxyz"
	                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890@.+-_") <= username.size()) {
		return false;
	}
	// Check whether the username is not "team" without regarding upper/lower case
	return to_lower(username) != "team";
}
