/*
 * Copyright (C) 2004-2006, 2008-2009, 2012-2013 by the Widelands Development Team
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

#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>
#endif

#include "build_info.h"
#include "chat.h"
#include "network/internet_gaming_protocol.h"
#include "network/network.h"
#include "network/network_lan_promotion.h"


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
		(const std::string & nick, const std::string & pwd, bool registered,
		 const std::string & metaserver, uint32_t port);
	bool relogin();
	void logout(const std::string & msgcode = "CONNECTION_CLOSED");

	/// \returns whether the client is logged in
	bool logged_in() {return (m_state == LOBBY) || (m_state == CONNECTING) || (m_state == IN_GAME);}
	bool error()     {return (m_state == COMMUNICATION_ERROR);}
	void setError()  {m_state = COMMUNICATION_ERROR; gameupdate = true; clientupdate = true;}

	void handle_metaserver_communication();

	// Game specific functions
	const std::string & ip();
	void join_game(const std::string & gamename);
	void open_game();
	void set_game_playing();
	void set_game_done();


	// Informative functions for lobby
	bool updateForGames();
	const std::vector<INet_Game>   & games();
	bool updateForClients();
	const std::vector<INet_Client> & clients();

	/// \returns the maximum allowed number of clients in a game (players + spectators)
	uint32_t max_clients() {return INTERNET_GAMING_MAX_CLIENTS_PER_GAME;}

	/// sets the maximum number of players that may be in the game
	void set_local_maxclients(uint32_t mp) {m_maxclients = mp;}

	/// sets the name of the local server as shown in the games list
	void set_local_servername(const std::string & name) {m_gamename = name;}


	/// \returns the name of the local server
	std::string & get_local_servername() {return m_gamename;}

	/// \returns the name of the local client
	std::string & get_local_clientname() {return m_clientname;}

	/// \returns the rights of the local client
	std::string & get_local_clientrights() {return m_clientrights;}


	/// ChatProvider: sends a message via the metaserver.
	void send(const std::string &) override;

	/// ChatProvider: adds the message to the message list and calls parent.
	void receive(const ChatMessage & msg) {messages.push_back(msg); ChatProvider::send(msg);}

	/// ChatProvider: returns the list of chatmessages.
	const std::vector<ChatMessage> & getMessages() const override {return messages;}

	/// Silence the internet lobby chat if we are in game as we do not see the messages anyways
	bool sound_off() override {return m_state == IN_GAME;}

	/// writes the ingame_system_chat messages to \arg msg and resets it afterwards
	void getIngameSystemMessages(std::vector<ChatMessage> & msg) {
		msg = ingame_system_chat;
		ingame_system_chat.clear();
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
	std::string              waitcmd;
	int32_t                  waittimeout;

	/// Connection tracking specific variables
	time_t                   lastbrokensocket[2];
	time_t                   lastping;

};

#endif
