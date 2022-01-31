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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_NETWORK_INTERNET_GAMING_H
#define WL_NETWORK_INTERNET_GAMING_H

#include <memory>

#include "chat/chat.h"
#include "network/netclient.h"
#include "network/network.h"

/// A simple network client struct
struct InternetClient {
	std::string name;
	std::string build_id;
	std::string game;
	std::string type;
};

/// A simple network game struct
struct InternetGame {
	std::string name;
	std::string build_id;
	std::string connectable;
};

/**
 * The InternetGaming struct.
 */
struct InternetGaming : public ChatProvider {

	/// The only instance of InternetGaming -> the constructor is private by purpose!
	static InternetGaming& ref();

	void reset();

	// Login and logout

	void initialize_connection();

	/**
	 * Try to login on the metaserver.
	 * @param nick The preferred username. Another username might be chosen by the metaserver if
	 *             the requested one is already in use.
	 * @param authenticator If \c registered is \c true, this is the password. Otherwise, it is some
	 * unique
	 *             id the server can use to identify the user.
	 * @param metaserver The hostname of the metaserver.
	 * @param port The port number of the metaserver.
	 * @return Whether the login was successful.
	 */
	bool login(const std::string& nick,
	           const std::string& authenticator,
	           bool registered,
	           const std::string& metaserver,
	           uint32_t port);
	bool relogin();
	void logout(const std::string& msgcode = "CONNECTION_CLOSED");

	/**
	 * Connects to the metaserver and checks the password without logging in.
	 *
	 * Note that the user might be logged in with another username and as unregistered
	 * if the user account is already in use by another client.
	 * @warning Resets the current connection.
	 * @param nick The username.
	 * @param pwd The password.
	 * @param metaserver The hostname of the metaserver.
	 * @param port The port number of the metaserver.
	 * @return Whether the password was valid.
	 */
	bool check_password(const std::string& nick,
	                    const std::string& pwd,
	                    const std::string& metaserver,
	                    uint32_t port);

	/// \returns whether the client is logged in
	bool logged_in() {
		return (state_ == LOBBY) || (state_ == CONNECTING) || (state_ == IN_GAME);
	}
	bool error() {
		return (state_ == COMMUNICATION_ERROR);
	}
	void set_error() {
		state_ = COMMUNICATION_ERROR;
		gameupdate_ = true;
		clientupdate_ = true;
	}

	void handle_metaserver_communication(bool relogin_on_error = true);

	// Game specific functions
	/**
	 * Returns a pair containing up to two NetAddress'es of the game host to connect to.
	 * Contains two addresses when the host supports IPv4 and IPv6, one address when the host
	 * only supports one of the protocols, no addresses when no join-request was sent to
	 * the metaserver. "No address" means a default constructed address.
	 * Also returns the IPs of the relay server when trying to host a game.
	 * Use NetAddress::is_valid() to check whether a NetAddress has been default constructed.
	 * @return The addresses.
	 */
	const std::pair<NetAddress, NetAddress>& ips();

	/**
	 * Blocks for some time until either the ips() method is able to return the IPs of the relay
	 * or an error occurred or the timeout is met.
	 * @return \c True iff ips() can return something.
	 */
	bool wait_for_ips();

	/**
	 * Returns the password required to connect to the relay server as host.
	 */
	const std::string relay_password();

	void join_game(const std::string& gamename);
	void open_game();
	void set_game_playing();
	void set_game_done();

	// Informative functions for lobby
	bool update_for_games();
	const std::vector<InternetGame>* games();
	bool update_for_clients();
	const std::vector<InternetClient>* clients();

	/// sets the name of the local server as shown in the games list
	void set_local_servername(const std::string& name) {
		gamename_ = name;
	}

	/// \returns the name of the local server
	std::string& get_local_servername() {
		return gamename_;
	}

	/// \returns the name of the local client
	std::string& get_local_clientname() {
		return clientname_;
	}

	/// \returns the rights of the local client
	std::string& get_local_clientrights() {
		return clientrights_;
	}

	/// ChatProvider: sends a message via the metaserver.
	void send(const std::string&) override;

	/// ChatProvider: adds the message to the message list and calls parent.
	void receive(const ChatMessage& msg) {
		messages_.push_back(msg);
		Notifications::publish(msg);
	}

	/// ChatProvider: returns the list of chatmessages.
	const std::vector<ChatMessage>& get_messages() const override {
		return messages_;
	}

	/// Silence the internet lobby chat if we are in game as we do not see the messages anyways
	bool sound_off() override {
		return state_ == IN_GAME;
	}

	/// writes the ingame_system_chat_ messages to \arg msg and resets it afterwards
	void get_ingame_system_messages(std::vector<ChatMessage>& msg) {
		msg = ingame_system_chat_;
		ingame_system_chat_.clear();
	}

	bool has_been_set() const override {
		return true;
	}

	void format_and_add_chat(const std::string& from,
	                         const std::string& to,
	                         bool system,
	                         const std::string& msg);

	bool valid_username(const std::string&);

private:
	InternetGaming();

	/**
	 * Temporarily creates a second connection to the metaserver.
	 * If the primary connection is an IPv6 connection, we also try
	 * an IPv4 connection to tell the metaserver our IP.
	 * This way, when we host a game later on, the metaserver
	 * knows how to reach us for both protocol versions.
	 * The established connection does a login, then the connection is
	 * immediately closed.
	 *
	 * If the primary connection already is IPv4, this method does nothing.
	 * Since we first try to connect with IPv6, another try is futile.
	 */
	void create_second_connection();

	void handle_packet(RecvPacket& packet, bool relogin_on_error = true);
	void handle_failed_read();

	// conversion functions
	bool str2bool(const std::string&);
	std::string bool2str(bool);

	void format_time(char* time_str, uint8_t length);

	/**
	 * Does the real work of the login.
	 * \param relogin Whether this is a relogin. Only difference is that
	 *                on first login a greeting is shown.
	 */
	bool do_login(bool relogin = false);

	/// The connection to the metaserver
	std::unique_ptr<NetClient> net;

	/// Current state of this class
	enum { OFFLINE, CONNECTING, LOBBY, IN_GAME, COMMUNICATION_ERROR } state_;

	/// data saved for possible relogin
	std::string authenticator_;
	bool reg_;

	/// Password for connecting as host to a game on the relay server
	std::string relay_password_;

	std::string meta_;
	uint16_t port_;

	/// local clients name and rights
	std::string clientname_;
	std::string clientrights_;

	/// information of the clients game
	std::string gamename_;
	/// The IPv4/v6 addresses of the game host we are / will be connected to.
	/// See InternetGaming::ips().
	std::pair<NetAddress, NetAddress> gameips_;

	/// Metaserver information
	bool clientupdateonmetaserver_;
	bool gameupdateonmetaserver_;
	bool clientupdate_;
	bool gameupdate_;
	std::vector<InternetClient> clientlist_;
	std::vector<InternetGame> gamelist_;
	int32_t time_offset_;

	/// ChatProvider: chat messages
	std::vector<ChatMessage> messages_;
	std::vector<ChatMessage> ingame_system_chat_;

	/// An important response of the metaserver, the client is waiting for.
	std::string waitcmd_;
	int32_t waittimeout_;

	/// Connection tracking specific variables
	time_t lastbrokensocket_[2];  /// last times when socket last broke in s.
	time_t lastping_;
};

#endif  // end of include guard: WL_NETWORK_INTERNET_GAMING_H
