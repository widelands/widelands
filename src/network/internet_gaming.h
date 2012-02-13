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
#include "network.h"
#include "network_lan_promotion.h"

#include <stdint.h>
#include <string>
#include <vector>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif


/// A simple network client struct
struct INet_Client {
	std::string   name;
	std::string   build_id;
	std::string   game;
	std::string   type;
	std::string   points;
};

/// A simple network game struct
struct INet_Game {
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
	void initialiseConnection();
	bool login
		(std::string const & nick, std::string const & pwd, bool registered,
		 std::string const & metaserver, uint32_t port);
	bool relogin();
	void logout(std::string const & msgcode = "CONNECTION_CLOSED");

	/// \returns whether the client is logged in
	bool logged_in() {return (m_state == LOBBY) || (m_state == CONNECTING) || (m_state == IN_GAME);}
	bool error()     {return (m_state == COMMUNICATION_ERROR);}
	void setError()  {m_state = COMMUNICATION_ERROR; gameupdate = true; clientupdate = true;}

	void handle_metaserver_communication();

	// Game specific functions
	const std::string & ip();
	void join_game(std::string const & gamename);
	void open_game();
	void set_game_playing();
	void set_game_done();


	// Informative functions for lobby
	bool updateForGames();
	std::vector<INet_Game>   const & games();
	bool updateForClients();
	std::vector<INet_Client> const & clients();

	/// \returns the maximum allowed number of clients in a game (players + spectators)
	uint32_t max_clients() {return INTERNET_GAMING_MAX_CLIENTS_PER_GAME;}

	/// sets the maximum number of players that may be in the game
	void set_local_maxclients(uint32_t mp) {m_maxclients = mp;}

	/// sets the name of the local server as shown in the games list
	void set_local_servername(std::string const & name) {m_gamename = name;}


	/// \returns the name of the local server
	std::string & get_local_servername() {return m_gamename;}

	/// \returns the name of the local client
	std::string & get_local_clientname() {return m_clientname;}

	/// \returns the rights of the local client
	std::string & get_local_clientrights() {return m_clientrights;}


	// ChatProvider: sends a message via the metaserver.
	void send(std::string const &);

	/// ChatProvider: adds the message to the message list and calls parent.
	void receive(ChatMessage const & msg) {messages.push_back(msg); ChatProvider::send(msg);}

	/// ChatProvider: returns the list of chatmessages.
	std::vector<ChatMessage> const & getMessages() const {return messages;}

	/// \returns and resets the ingame_system_chat messages
	std::vector<ChatMessage> const & getIngameSystemMessages() {
		std::vector<ChatMessage> const * temp = new std::vector<ChatMessage>(ingame_system_chat);
		ingame_system_chat.clear();
		return *temp;
	}

private:
	InternetGaming();

	void handle_packet(RecvPacket & packet);

	// conversion functions
	bool str2bool(std::string);
	std::string bool2str(bool);

	void formatAndAddChat(std::string from, std::string to, bool system, std::string msg);


	/// The socket that connects us to the host
	TCPsocket        m_sock;

	/// Socket set used for selection
	SDLNet_SocketSet m_sockset;

	/// Deserializer acts as a buffer for packets (reassembly/splitting up)
	Deserializer     m_deserializer;

	/// Current state of this class
	enum {
		OFFLINE,
		CONNECTING,
		LOBBY,
		IN_GAME,
		COMMUNICATION_ERROR
	}                m_state;

	/// data saved for possible relogin
	std::string      m_pwd;
	bool             m_reg;
	std::string      m_meta;
	uint32_t         m_port;

	/// local clients name and rights
	std::string      m_clientname;
	std::string      m_clientrights;

	/// informations of the clients game
	std::string      m_gamename;
	uint32_t         m_maxclients;
	std::string      m_gameip;

	/// Metaserver informations
	bool                     clientupdateonmetaserver;
	bool                     gameupdateonmetaserver;
	bool                     clientupdate;
	bool                     gameupdate;
	std::vector<INet_Client> clientlist;
	std::vector<INet_Game>   gamelist;
	int32_t                  time_offset;

	/// ChatProvider: chat messages
	std::vector<ChatMessage> messages;
	std::vector<ChatMessage> ingame_system_chat;

	/// An important response of the metaserver, the client is waiting for.
	std::string               waitcmd;
	int32_t                   waittimeout;

};

#endif
