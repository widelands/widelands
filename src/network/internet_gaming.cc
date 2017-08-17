/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#include <memory>
#include <random>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/warning.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "network/constants.h"
#include "network/internet_gaming_messages.h"

/// Private constructor by purpose: NEVER call directly. Always call InternetGaming::ref(), this
/// will ensure
/// that only one instance is running at time.
InternetGaming::InternetGaming()
   : net(nullptr),
     state_(OFFLINE),
     reg_(false),
     port_(INTERNET_GAMING_PORT),
     clientrights_(INTERNET_CLIENT_UNREGISTERED),
     gameips_(),
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
	pwd_ = "";
	reg_ = false;
	meta_ = INTERNET_GAMING_METASERVER;
	port_ = INTERNET_GAMING_PORT;
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
	if (!ig)
		ig = new InternetGaming();
	return *ig;
}

void InternetGaming::initialize_connection() {
	// First of all try to connect to the metaserver
	log("InternetGaming: Connecting to the metaserver.\n");
	NetAddress addr;
	net.reset();
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
                           const std::string& pwd,
                           bool reg,
                           const std::string& meta,
                           uint32_t port) {
	assert(state_ == OFFLINE);

	pwd_ = pwd;
	reg_ = reg;
	meta_ = meta;
	port_ = port;

	// If we are not connecting to a registered account, create a random value
	// to send as password. Used so the metaserver can match our IPv4 and IPv6 connections.
	// See internet_gaming_protocol.h for more information
	if (!reg_) {
		// Admittedly this is a pretty stupid generator. But it should be fine for us
		static const std::string random_chars = "0123456789ABCDEF";
		pwd_ = "";
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dist(0, random_chars.length() - 1);
		while (pwd_.length() < 8) {
			pwd_.push_back(random_chars[dist(gen)]);
		}
	}

	initialize_connection();

	// If we are here, a connection was established and we can send our login package through the
	// socket.
	log("InternetGaming: Sending login request.\n");
	SendPacket s;
	s.string(IGPCMD_LOGIN);
	s.string(boost::lexical_cast<std::string>(INTERNET_GAMING_PROTOCOL_VERSION));
	s.string(nick);
	s.string(build_id());
	s.string(bool2str(reg_));
	s.string(pwd_);
	net->send(s);

	// Now let's see, whether the metaserver is answering
	uint32_t const secs = time(nullptr);
	state_ = CONNECTING;
	while (INTERNET_GAMING_TIMEOUT > time(nullptr) - secs) {
		handle_metaserver_communication();
		// Check if we are a step further... if yes handle_packet has taken care about all the
		// paperwork, so we put our feet up and just return. ;)
		if (state_ != CONNECTING) {
			if (state_ == LOBBY) {
				format_and_add_chat(
				   "", "", true, _("For hosting a game, please take a look at the notes at:"));
				format_and_add_chat("", "", true, "http://wl.widelands.org/wiki/InternetGaming");

				// Try to establish a second connection to tell the metaserver about our IPv4 address
				create_second_connection();
				return true;
			} else if (error())
				return false;
		}
	}
	log("InternetGaming: No answer from metaserver!\n");
	logout("NO_ANSWER");
	return false;
}

/// Relogin to metaserver after loosing connection
bool InternetGaming::relogin() {
	if (!error()) {
		throw wexception("InternetGaming::relogin: This only makes sense if there was an error.");
	}

	initialize_connection();

	// If we are here, a connection was established and we can send our login package through the
	// socket.
	log("InternetGaming: Sending relogin request.\n");
	SendPacket s;
	s.string(IGPCMD_RELOGIN);
	s.string(boost::lexical_cast<std::string>(INTERNET_GAMING_PROTOCOL_VERSION));
	s.string(clientname_);
	s.string(build_id());
	s.string(bool2str(reg_));
	s.string(pwd_);
	net->send(s);

	// Now let's see, whether the metaserver is answering
	uint32_t const secs = time(nullptr);
	state_ = CONNECTING;
	while (INTERNET_GAMING_TIMEOUT > time(nullptr) - secs) {
		handle_metaserver_communication();
		// Check if we are a step further... if yes handle_packet has taken care about all the
		// paperwork, so we put our feet up and just return. ;)
		if (state_ != CONNECTING) {
			if (state_ == LOBBY) {
				break;
			} else if (error())
				return false;
		}
	}

	if (INTERNET_GAMING_TIMEOUT <= time(nullptr) - secs) {
		log("InternetGaming: No answer from metaserver!\n");
		return false;
	}

	create_second_connection();

	// Client is reconnected, so let's try resend the timeouted command.
	if (waitcmd_ == IGPCMD_GAME_CONNECT)
		join_game(gamename_);
	else if (waitcmd_ == IGPCMD_GAME_OPEN) {
		state_ = IN_GAME;
		open_game();
	} else if (waitcmd_ == IGPCMD_GAME_START) {
		state_ = IN_GAME;
		set_game_playing();
	}

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
	log("InternetGaming: logout(%s)\n", msg.c_str());
	format_and_add_chat("", "", true, msg);

	reset();
}

/**
 * Handle situation when reading from socket failed.
 */
void InternetGaming::handle_failed_read() {
	set_error();
	const std::string& msg = InternetGamingMessages::get_message("CONNECTION_LOST");
	log("InternetGaming: Error: %s\n", msg.c_str());
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
void InternetGaming::handle_metaserver_communication() {
	if (error())
		return;
	try {
		while (net != nullptr) {
			// Check if the connection is still open
			if (!net->is_connected()) {
				handle_failed_read();
				return;
			}
			// Process all available packets
			RecvPacket packet;
			if (net->try_receive(&packet)) {
				handle_packet(packet);
			} else {
				// Nothing more to receive
				break;
			}
		}
	} catch (const std::exception& e) {
		logout((boost::format(_("Something went wrong: %s")) % e.what()).str());
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
			log("InternetGaming: reached a timeout for an awaited answer of the metaserver!\n");
			if (!relogin()) {
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
		if (!relogin()) {
			// Do not try to relogin again automatically.
			reset();
			set_error();
		}
	}
}

void InternetGaming::create_second_connection() {
	NetAddress addr;
	net->get_remote_address(&addr);
	if (!addr.is_ipv6()) {
		// Primary connection already is IPv4, abort
		return;
	}

	if (!NetAddress::resolve_to_v4(&addr, meta_, port_)) {
		// Could not get the IPv4 address of the metaserver? Strange :-/
		return;
	}

	std::unique_ptr<NetClient> tmpNet = NetClient::connect(addr);
	if (!tmpNet || !tmpNet->is_connected()) {
		// Connecting by IPv4 doesn't work? Well, nothing to do then
		return;
	}

	// Okay, we have a connection. Send the login message and terminate the connection
	SendPacket s;
	s.string(IGPCMD_TELL_IP);
	s.string(boost::lexical_cast<std::string>(INTERNET_GAMING_PROTOCOL_VERSION));
	s.string(clientname_);
	s.string(pwd_);
	tmpNet->send(s);

	// Close the connection
	tmpNet->close();
}

/// Handle one packet received from the metaserver.
void InternetGaming::handle_packet(RecvPacket& packet) {
	std::string cmd = packet.string();

	// First check if everything is fine or whether the metaserver broke up with the client.
	if (cmd == IGPCMD_DISCONNECT) {
		std::string reason = packet.string();
		format_and_add_chat("", "", true, InternetGamingMessages::get_message(reason));
		if (reason == "CLIENT_TIMEOUT") {
			// Try to relogin
			set_error();
			if (!relogin()) {
				// Do not try to relogin again automatically.
				reset();
				set_error();
			}
		}
		return;
	}

	// Are we already online?
	if (state_ == CONNECTING) {
		if (cmd == IGPCMD_LOGIN) {
			// Clients request to login was granted
			clientname_ = packet.string();
			clientrights_ = packet.string();
			state_ = LOBBY;
			log("InternetGaming: Client %s logged in.\n", clientname_.c_str());
			return;

		} else if (cmd == IGPCMD_RELOGIN) {
			// Clients request to relogin was granted
			state_ = LOBBY;
			log("InternetGaming: Client %s relogged in.\n", clientname_.c_str());
			format_and_add_chat("", "", true, _("Successfully reconnected to the metaserver!"));
			return;

		} else if (cmd == IGPCMD_ERROR) {
			std::string errortype = packet.string();
			if (errortype != "LOGIN" && errortype != "RELOGIN") {
				log("InternetGaming: Strange ERROR in connecting state: %s\n", errortype.c_str());
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
			throw WLWarning(
			   _("Unexpected packet"),
			   _("Expected a LOGIN, RELOGIN or REJECTED packet from server, but received command "
			     "%s. Maybe the metaserver is using a different protocol version ?"),
			   cmd.c_str());
		}
	}

	try {
		if (cmd == IGPCMD_LOGIN || cmd == IGPCMD_RELOGIN) {
			// Login specific commands but not in CONNECTING state...
			log("InternetGaming: Received %s cmd although client is not in CONNECTING state.\n",
			    cmd.c_str());
			std::string temp =
			   (boost::format(
			       _("WARNING: Received a %s command although we are not in CONNECTING state.")) %
			    cmd)
			      .str();
			format_and_add_chat("", "", true, temp);
		}

		else if (cmd == IGPCMD_TIME) {
			// Client received the server time
			time_offset_ = boost::lexical_cast<int>(packet.string()) - time(nullptr);
			log("InternetGaming: Server time offset is %d second(s).\n", time_offset_);
			std::string temp =
			   (boost::format(ngettext("Server time offset is %d second.",
			                           "Server time offset is %d seconds.", time_offset_)) %
			    time_offset_)
			      .str();
			format_and_add_chat("", "", true, temp);
		}

		else if (cmd == IGPCMD_PING) {
			// Client received a PING and should immediately PONG as requested
			SendPacket s;
			s.string(IGPCMD_PONG);
			net->send(s);

			lastping_ = time(nullptr);
		}

		else if (cmd == IGPCMD_CHAT) {
			// Client received a chat message
			std::string sender = packet.string();
			std::string message = packet.string();
			std::string type = packet.string();

			if (type != "public" && type != "private" && type != "system")
				throw WLWarning(
				   _("Invalid message type"), _("Invalid chat message type \"%s\"."), type.c_str());

			bool personal = type == "private";
			bool system = type == "system";

			format_and_add_chat(sender, personal ? clientname_ : "", system, message);
		}

		else if (cmd == IGPCMD_GAMES_UPDATE) {
			// Client received a note, that the list of games was changed
			log("InternetGaming: Game update on metaserver.\n");
			gameupdateonmetaserver_ = true;
		}

		else if (cmd == IGPCMD_GAMES) {
			// Client received the new list of games
			uint8_t number = boost::lexical_cast<int>(packet.string()) & 0xff;
			std::vector<InternetGame> old = gamelist_;
			gamelist_.clear();
			log("InternetGaming: Received a game list update with %u items.\n", number);
			for (uint8_t i = 0; i < number; ++i) {
				InternetGame* ing = new InternetGame();
				ing->name = packet.string();
				ing->build_id = packet.string();
				ing->connectable = str2bool(packet.string());
				gamelist_.push_back(*ing);

				bool found = false;
				for (InternetGame& old_game : old) {
					if (old_game.name == ing->name) {
						found = true;
						old_game.name = "";
						break;
					}
				}
				if (!found)
					format_and_add_chat(
					   "", "", true,
					   (boost::format(_("The game %s is now available")) % ing->name).str());

				delete ing;
				ing = nullptr;
			}

			for (InternetGame& old_game : old) {
				if (old_game.name.size()) {
					format_and_add_chat(
					   "", "", true,
					   (boost::format(_("The game %s has been closed")) % old_game.name).str());
				}
			}

			gameupdate_ = true;
		}

		else if (cmd == IGPCMD_CLIENTS_UPDATE) {
			// Client received a note, that the list of clients was changed
			log("InternetGaming: Client update on metaserver.\n");
			clientupdateonmetaserver_ = true;
		}

		else if (cmd == IGPCMD_CLIENTS) {
			// Client received the new list of clients
			uint8_t number = boost::lexical_cast<int>(packet.string()) & 0xff;
			std::vector<InternetClient> old = clientlist_;
			clientlist_.clear();
			log("InternetGaming: Received a client list update with %u items.\n", number);
			for (uint8_t i = 0; i < number; ++i) {
				InternetClient* inc = new InternetClient();
				inc->name = packet.string();
				inc->build_id = packet.string();
				inc->game = packet.string();
				inc->type = packet.string();
				inc->points = packet.string();
				clientlist_.push_back(*inc);

				bool found =
				   old.empty();  // do not show all clients, if this instance is the actual change
				for (InternetClient& client : old) {
					if (client.name == inc->name) {
						found = true;
						client.name = "";
						break;
					}
				}
				if (!found)
					format_and_add_chat(
					   "", "", true, (boost::format(_("%s joined the lobby")) % inc->name).str());

				delete inc;
				inc = nullptr;
			}

			for (InternetClient& client : old) {
				if (client.name.size()) {
					format_and_add_chat(
					   "", "", true, (boost::format(_("%s left the lobby")) % client.name).str());
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
		}

		else if (cmd == IGPCMD_GAME_CONNECT) {
			// Client received the ip for the game it wants to join
			assert(waitcmd_ == IGPCMD_GAME_CONNECT);
			waitcmd_ = "";
			// Save the received IP(s), so the client can connect to the game
			NetAddress::parse_ip(&gameips_.first, packet.string(), WIDELANDS_PORT);
			// If the next value is true, a secondary IP follows
			if (packet.string() == bool2str(true)) {
				NetAddress::parse_ip(&gameips_.second, packet.string(), WIDELANDS_PORT);
			}
			log("InternetGaming: Received ips of the game to join: %s %s.\n",
			    gameips_.first.ip.to_string().c_str(), gameips_.second.ip.to_string().c_str());
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
				if (reason == "NO_SUCH_USER")
					message = (boost::format("%s %s") % message %
					           (boost::format(InternetGamingMessages::get_message(reason)) %
					            packet.string().c_str()))
					             .str();
			}

			else if (subcmd == IGPCMD_GAME_OPEN) {
				// Something went wrong with the newly opened game
				message = InternetGamingMessages::get_message(reason);
				// we got our answer, so no need to wait anymore
				waitcmd_ = "";
			}
			message = (boost::format(_("ERROR: %s")) % message).str();

			// Finally send the error message as system chat to the client.
			format_and_add_chat("", "", true, message);
		}

		else
			// Inform the client about the unknown command
			format_and_add_chat(
			   "", "", true,
			   (boost::format(_("Received an unknown command from the metaserver: %s")) % cmd).str());

	} catch (WLWarning& e) {
		format_and_add_chat("", "", true, e.what());
	}
}

/// \returns Up to two NetAdress with ips of the game the client is on or wants to join
///          (or the client is hosting) or invalid addresses, if no ip available.
const std::pair<NetAddress, NetAddress>& InternetGaming::ips() {
	return gameips_;
}

/// called by a client to join the game \arg gamename
void InternetGaming::join_game(const std::string& gamename) {
	if (!logged_in())
		return;

	SendPacket s;
	s.string(IGPCMD_GAME_CONNECT);
	s.string(gamename);
	net->send(s);
	gamename_ = gamename;
	log("InternetGaming: Client tries to join a game with the name %s\n", gamename_.c_str());
	state_ = IN_GAME;

	// From now on we wait for a reply from the metaserver
	waitcmd_ = IGPCMD_GAME_CONNECT;
	waittimeout_ = time(nullptr) + INTERNET_GAMING_TIMEOUT;
}

/// called by a client to open a new game with name gamename_
void InternetGaming::open_game() {
	if (!logged_in())
		return;

	SendPacket s;
	s.string(IGPCMD_GAME_OPEN);
	s.string(gamename_);
	s.string("1024");  // Used to be maxclients, no longer used.
	net->send(s);
	log("InternetGaming: Client opened a game with the name %s.\n", gamename_.c_str());
	state_ = IN_GAME;

	// From now on we wait for a reply from the metaserver
	waitcmd_ = IGPCMD_GAME_OPEN;
	waittimeout_ = time(nullptr) + INTERNET_GAMING_TIMEOUT;
}

/// called by a client that is host of a game to inform the metaserver, that the game started
void InternetGaming::set_game_playing() {
	if (!logged_in())
		return;

	SendPacket s;
	s.string(IGPCMD_GAME_START);
	net->send(s);
	log("InternetGaming: Client announced the start of the game %s.\n", gamename_.c_str());

	// From now on we wait for a reply from the metaserver
	waitcmd_ = IGPCMD_GAME_START;
	waittimeout_ = time(nullptr) + INTERNET_GAMING_TIMEOUT;
}

/// called by a client to inform the metaserver, that it left the game and is back in the lobby.
/// If this is called by the hosting client, this further informs the metaserver, that the game was
/// closed.
void InternetGaming::set_game_done() {
	if (!logged_in())
		return;

	SendPacket s;
	s.string(IGPCMD_GAME_DISCONNECT);
	net->send(s);

	gameips_ = std::make_pair(NetAddress(), NetAddress());
	state_ = LOBBY;

	log("InternetGaming: Client announced the disconnect from the game %s.\n", gamename_.c_str());
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
	if (!logged_in()) {
		format_and_add_chat(
		   "", "", true, _("Message could not be sent: You are not connected to the metaserver!"));
		return;
	}

	SendPacket s;
	s.string(IGPCMD_CHAT);

	if (msg.size() && *msg.begin() == '@') {
		// Format a personal message
		std::string::size_type const space = msg.find(' ');
		if (space >= msg.size() - 1) {
			format_and_add_chat(
			   "", "", true,
			   _("Message could not be sent: Was this supposed to be a private message?"));
			return;
		}
		s.string(msg.substr(space + 1));     // message
		s.string(msg.substr(1, space - 1));  // recipient

		format_and_add_chat(clientname_, msg.substr(1, space - 1), false, msg.substr(space + 1));

	} else if (clientrights_ == INTERNET_CLIENT_SUPERUSER && msg.size() && *msg.begin() == '/') {
		// This is either a /me command, a super user command, or well... just a chat message
		// beginning
		// with a "/" - let's see...

		// Split up in "cmd" "arg"
		std::string cmd, arg;
		std::string temp = msg.substr(1);  // cut off '/'
		std::string::size_type const space = temp.find(' ');
		if (space > temp.size())
			// no argument
			goto normal;

		// get the cmd and the arg
		cmd = temp.substr(0, space);
		arg = temp.substr(space + 1);

		if (cmd == "motd") {
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
			net->send(s);
			return;
		} else if (cmd == "announcement") {
			// send the request to change the motd
			SendPacket m;
			m.string(IGPCMD_ANNOUNCEMENT);
			m.string(arg);
			net->send(s);
			return;
		} else
			// let everything else pass
			goto normal;

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
bool InternetGaming::str2bool(std::string str) {
	if ((str != "true") && (str != "false"))
		throw WLWarning(_("Conversion error"),
		                /** TRANSLATORS: Geeky message from the metaserver */
		                /** TRANSLATORS: This message is shown if %s isn't "true" or "false" */
		                _("Unable to determine truth value for \"%s\""), str.c_str());

	return str == "true";
}

/// \returns a string containing the boolean value \arg b to be send to metaserver
std::string InternetGaming::bool2str(bool b) {
	return b ? "true" : "false";
}

/// formates a chat message and adds it to the list of chat messages
void InternetGaming::format_and_add_chat(const std::string& from,
                                         const std::string& to,
                                         bool system,
                                         const std::string& msg) {
	ChatMessage c(msg);
	if (!system && from.empty()) {
		std::string unkown_string = (boost::format("<%s>") % _("unknown")).str();
		c.sender = unkown_string;
	} else {
		c.sender = from;
	}
	c.playern = system ? -1 : to.size() ? 3 : 7;
	c.recipient = to;

	receive(c);
	if (system && (state_ == IN_GAME)) {
		// Save system chat messages separately as well, so the nethost can import and show them in
		// game;
		c.msg = "METASERVER: " + msg;
		ingame_system_chat_.push_back(c);
	}
}
