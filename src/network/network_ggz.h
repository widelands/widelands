/*
 * Copyright (C) 2004-2006, 2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef NETWORK_GGZ_H
#define NETWORK_GGZ_H

#ifdef USE_GGZ
#define HAVE_GGZ 1

#define WL_METASERVER "widelands.org"
#define WL_METASERVER_PORT 5688

#define ERRMSG "</p><p font-size=14 font-color=#ff6633 font-weight=bold>ERROR: "

#include "build_info.h"
#include "chat.h"
#include "network_lan_promotion.h"

#include <ggzmod.h>
#include <ggzcore.h>
#include <stdint.h>
#include <string>
#include <vector>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif

/// A simply network player struct
struct Net_Player {
	std::string table;
	std::string name;
};

/// A MOTD struct for easier output to the chat panel
struct MOTD {
	std::string formationstr;
	std::vector<std::string> motd;

	MOTD() {}
	MOTD(std::string msg) {
		// if msg is empty -> return
		if (msg.size() < 1)
			return;

		// first char is always \n - so we remove it
		msg = msg.substr(1, msg.size() - 1);
		size_t j = msg.find_first_of("\n");

		// split the message parts to have good looking texts
		for (int32_t i = 0; msg.size(); ++i) {
			if (j == std::string::npos) {
				motd.push_back(msg);
				break;
			}
			if (i == 0 && msg.substr(0, 1) == "<")
				formationstr = msg.substr(0, j);
			else
				motd.push_back(msg.substr(0, j));
			msg = msg.substr(j + 1, msg.size() - j);
			j = msg.find_first_of("\n");
		}
	}
};

/**
 * The GGZ implementation
 *
 * It is handling all communication between the Widelands client/server and the
 * metaserver, including metaserver lobbychat.
 */
struct NetGGZ : public ChatProvider {
	static NetGGZ & ref();

	void init();
	bool connect();

	bool used();
	bool host();
	void data();
	const char *ip();

	bool updateForTables() {
		bool temp = tableupdate;
		tableupdate = false;
		return temp;
	}
	bool updateForUsers() {
		bool temp = userupdate;
		userupdate = false;
		return temp;
	}
	std::vector<Net_Game_Info> const & tables();
	std::vector<Net_Player>    const & users();

	enum Protocol
	{
		op_greeting = 1,
		op_request_ip = 2, // request the IP of the host
		op_reply_ip = 3, // tell the server, that following package is our IP
		op_broadcast_ip = 4,
		op_state_playing = 5, // tell the server that the game was stated
		op_state_done = 6 // tell the server that the gam ended
	};

	bool initcore(const char * metaserver, const char * playername);
	void deinitcore();
	bool usedcore();
	void datacore();
	void launch  ();
	void send_game_playing();
	void send_game_done();
	void join(char const * tablename);

	// functions for local server setup
	uint32_t max_players();
	/// sets the maximum number of players that may be in the game
	void set_local_maxplayers(uint32_t mp) {
		tableseats = mp;
	}
	/// sets the servername shown in the games list
	void set_local_servername(std::string const & name) {
		servername  = name.empty() ? "WL-Default" : name;
		servername += " (";
		servername += build_id();
		servername += ')';
	}

	// ChatProvider: sends a message via GGZnetwork.
	void send(std::string const &);

	// ChatProvider: adds the message to the message list and calls parent.
	void receive(ChatMessage const & msg) {
		messages.push_back(msg);
		ChatProvider::send(msg);
	}

	// ChatProvider: returns the list of chatmessages.
	std::vector<ChatMessage> const & getMessages() const {
		return messages;
	}

	/// Called when a message is received via GGZnetwork.
	void recievedGGZChat(void const * cbdata);

	// Adds a GGZchatmessage in selected format to the list of chatmessages.
	void formatedGGZChat
		(std::string const &, std::string const &,
		 bool system = false, std::string recipient = std::string());

private:
	NetGGZ();
	static void ggzmod_server(GGZMod *, GGZModEvent, void const * cbdata);
	static GGZHookReturn
		callback_server(uint32_t id, void const * cbdata, void const * user);
	static GGZHookReturn
		callback_room(uint32_t id, void const * cbdata, void const * user);
	static GGZHookReturn
		callback_game(uint32_t id, void const * cbdata, void const * user);
	void event_server(uint32_t id, void const * cbdata);
	void event_room(uint32_t id, void const * cbdata);
	void event_game(uint32_t id, void const * cbdata);

	void write_tablelist();
	void write_userlist();

	bool use_ggz;
	int32_t m_fd;
	int32_t channelfd;
	int32_t gamefd;
	int32_t tableid;
	char *server_ip_addr;
	bool ggzcore_login;
	bool ggzcore_ready;
	bool relogin;
	GGZRoom *room;

	std::string username;
	std::string servername;
	uint32_t tableseats;

	bool userupdate;
	bool tableupdate;
	std::vector<Net_Game_Info> tablelist;
	std::vector<Net_Player>    userlist;
	MOTD motd;

	// The chat messages
	std::vector<ChatMessage> messages;
};

#endif

#endif
