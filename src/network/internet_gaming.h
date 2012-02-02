/*
 * Copyright (C) 2004-2006, 2008-2009, 2012 by the Widelands Development Team
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

#ifndef INTERNET_GAMING_H
#define INTERNET_GAMING_H

#include "build_info.h"
#include "chat.h"
#include "internet_gaming_protocol.h"
#include "network_lan_promotion.h"

#include <stdint.h>
#include <string>
#include <vector>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif

// TODO remove once ggz is removed
#define WL_METASERVER "widelands.org"
#define WL_METASERVER_PORT 5688


/// A simply network player struct
struct Net_Client {
	std::string   game;
	std::string   name;
	std::string   build_id;
	uint32_t      type;
	char          stats[16];
};


/**
 * The InternetGaming struct.
 */
struct InternetGaming : public ChatProvider {
	/// The only instance of InternetGaming -> the constructor is private by purpose!
	static InternetGaming & ref();

	// Login and logout
	//virtual void login() = 0;
	virtual void init() = 0;
	virtual bool connect() = 0;

	virtual bool used() = 0;
	virtual bool host() = 0;
	virtual void data() = 0;
	virtual char const * ip() = 0;

	virtual bool updateForGames() = 0;
	virtual bool updateForClients() = 0;

	std::vector<Net_Game_Info> const & games();
	std::vector<Net_Client>    const & clients();

	enum Protocol
	{
		op_greeting = 1,
		op_request_ip = 2, // request the IP of the host
		op_reply_ip = 3, // tell the server, that following package is our IP
		op_broadcast_ip = 4,
		op_state_playing = 5, // tell the server that the game was started
		op_state_done = 6, // tell the server that the game ended
		op_game_statistics = 7, // send game statistics
		op_unreachable = 99 // the metaserver says we are unreachable
	};

	virtual bool initcore(const char *, const char *, const char *, bool) = 0;
	virtual void deinitcore() = 0;
	virtual bool usedcore() = 0;
	virtual void datacore() = 0;
	virtual void launch  () = 0;
	virtual void send_game_playing() = 0;
	virtual void send_game_done() = 0;
	virtual void join(char const * gamename) = 0;

	// functions for clients server setup
	virtual uint32_t max_players() = 0;
	/// sets the maximum number of players that may be in the game
	void set_local_maxplayers(uint32_t mp) {
		maxclients = mp;
	}
	/// sets the name of the local server as shown in the games list
	void set_local_servername(std::string const & name) {
		servername  = name.empty() ? "WL-Default" : name;
		servername += " (";
		servername += build_id();
		servername += ')';
	}
	/// \returns the name of the local server
	std::string & get_local_servername() {return servername;}
	std::string & get_local_clientname() {return clientname;}

	// ChatProvider: sends a message via the metaserver.
	virtual void send(std::string const &) = 0;

	// ChatProvider: adds the message to the message list and calls parent.
	void receive(ChatMessage const & msg) {
		messages.push_back(msg);
		ChatProvider::send(msg);
	}

	// ChatProvider: returns the list of chatmessages.
	std::vector<ChatMessage> const & getMessages() const {
		return messages;
	}

protected:
	InternetGaming();

	// client informations
	std::string clientname;

	// informations of clients game
	std::string servername;
	uint32_t maxclients;

	// Server informations
	std::vector<Net_Game_Info> gamelist;
	std::vector<Net_Client>    clientlist;

	// The chat messages
	std::vector<ChatMessage> messages;

};

#endif
