/*
 * Copyright (C) 2004-2016 by the Widelands Development Team
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

#ifndef WL_NETWORK_INTERNET_GAMING_H
#define WL_NETWORK_INTERNET_GAMING_H

#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>
#endif

#include "build_info.h"
#include "chat/chat.h"
#include "network/internet_gaming_protocol.h"
#include "network/network.h"
#include "network/network_lan_promotion.h"


/// A simple network client struct
struct InternetClient {
	std::string   name;
	std::string   build_id;
	std::string   game;
	std::string   type;
	std::string   points; // Currently unused
};

/// A simple network game struct
struct InternetGame {
	std::string   name;
	std::string   build_id;
	bool          connectable;
};


/**
 * The InternetGaming struct.
 */
struct InternetGaming : public ChatProvider {
	/// The only instance of InternetGaming -> the constructor is private by purpose!
	static InternetGaming & ref();

	void reset();

	// Login and logout
	void initialize_connection();
	bool login
		(const std::string & nick, const std::string & pwd, bool registered,
		 const std::string & metaserver, uint32_t port);
	bool relogin();
	void logout(const std::string & msgcode = "CONNECTION_CLOSED");

	/// \returns whether the client is logged in
	bool logged_in() {return (state_ == LOBBY) || (state_ == CONNECTING) || (state_ == IN_GAME);}
	bool error()     {return (state_ == COMMUNICATION_ERROR);}
	void set_error()  {state_ = COMMUNICATION_ERROR; gameupdate_ = true; clientupdate_ = true;}

	void handle_metaserver_communication();

	// Game specific functions
	const std::string & ip();
	void join_game(const std::string & gamename);
	void open_game();
	void set_game_playing();
	void set_game_done();


	// Informative functions for lobby
	bool update_for_games();
	const std::vector<InternetGame>* games();
	bool update_for_clients();
	const std::vector<InternetClient>* clients();

	/// sets the name of the local server as shown in the games list
	void set_local_servername(const std::string & name) {gamename_ = name;}

	/// \returns the name of the local server
	std::string & get_local_servername() {return gamename_;}

	/// \returns the name of the local client
	std::string & get_local_clientname() {return clientname_;}

	/// \returns the rights of the local client
	std::string & get_local_clientrights() {return clientrights_;}


	/// ChatProvider: sends a message via the metaserver.
	void send(const std::string &) override;

	/// ChatProvider: adds the message to the message list and calls parent.
	void receive(const ChatMessage & msg) {
		messages_.push_back(msg);
		Notifications::publish(msg);
	}

	/// ChatProvider: returns the list of chatmessages.
	const std::vector<ChatMessage> & get_messages() const override {return messages_;}

	/// Silence the internet lobby chat if we are in game as we do not see the messages anyways
	bool sound_off() override {return state_ == IN_GAME;}

	/// writes the ingame_system_chat_ messages to \arg msg and resets it afterwards
	void get_ingame_system_messages(std::vector<ChatMessage> & msg) {
		msg = ingame_system_chat_;
		ingame_system_chat_.clear();
	}

	bool has_been_set() const override {return true;}

private:
	InternetGaming();

	void handle_packet(RecvPacket & packet);
	void handle_failed_read();

	// conversion functions
	bool str2bool(std::string);
	std::string bool2str(bool);

	void format_and_add_chat(std::string from, std::string to, bool system, std::string msg);


	/// The socket that connects us to the host
	TCPsocket        sock_;

	/// Socket set used for selection
	SDLNet_SocketSet sockset_;

	/// Deserializer acts as a buffer for packets (reassembly/splitting up)
	Deserializer     deserializer_;

	/// Current state of this class
	enum {
		OFFLINE,
		CONNECTING,
		LOBBY,
		IN_GAME,
		COMMUNICATION_ERROR
	}                state_;

	/// data saved for possible relogin
	std::string      pwd_;
	bool             reg_;
	std::string      meta_;
	uint32_t         port_;

	/// local clients name and rights
	std::string      clientname_;
	std::string      clientrights_;

	/// informations of the clients game
	std::string      gamename_;
	std::string      gameip_;

	/// Metaserver informations
	bool                        clientupdateonmetaserver_;
	bool                        gameupdateonmetaserver_;
	bool                        clientupdate_;
	bool                        gameupdate_;
	std::vector<InternetClient> clientlist_;
	std::vector<InternetGame>   gamelist_;
	int32_t                     time_offset_;

	/// ChatProvider: chat messages
	std::vector<ChatMessage> messages_;
	std::vector<ChatMessage> ingame_system_chat_;

	/// An important response of the metaserver, the client is waiting for.
	std::string              waitcmd_;
	int32_t                  waittimeout_;

	/// Connection tracking specific variables
	time_t                   lastbrokensocket_[2]; /// last times when socket last broke in s.
	time_t                   lastping_;

};

#endif  // end of include guard: WL_NETWORK_INTERNET_GAMING_H
