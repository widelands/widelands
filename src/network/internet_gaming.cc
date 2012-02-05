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


#include "internet_gaming.h"

#include "i18n.h"
#include "internet_gaming_messages.h"
#include "log.h"
#include "warning.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>



/// Private constructor by purpose: NEVER call directly. Always call InternetGaming::ref(), this will ensure
/// that only one instance is running at time.
InternetGaming::InternetGaming() :
	m_sock                   (0),
	m_sockset                (0),
	m_state                  (OFFLINE),
	m_clientrights           (0),
	m_maxclients             (1),
	m_gameip                 (""),
	clientupdateonmetaserver (false),
	gameupdateonmetaserver   (false),
	clientupdate             (false),
	gameupdate               (false),
	time_offset              (0)
{
	// Fill the list of possible messages from the server
	InternetGamingMessages::fill_map();
}


/// the one and only InternetGaming instance.
static InternetGaming * ig = 0;


/// \returns the one and only InternetGaming instance.
InternetGaming & InternetGaming::ref() {
	if (not ig)
		ig = new InternetGaming();
	return * ig;
}



/// Login to metaserver
bool InternetGaming::login
	(std::string const & nick, std::string const & pwd, bool reg, std::string const & meta, uint32_t port)
{
	assert(m_state == OFFLINE);

	// First of all try to connect to the metaserver
	dedicatedlog("InternetGaming: Connecting to metaserver.\n");
	IPaddress peer;
	if (hostent * const he = gethostbyname(meta.c_str())) {
		peer.host = (reinterpret_cast<in_addr *>(he->h_addr_list[0]))->s_addr;
		peer.port = htons(port);
	} else
		throw warning
			(_("Connection problem"), "%s", _("Widelands has not been able to connect to the metaserver."));

	SDLNet_ResolveHost (&peer, meta.c_str(), port);
	m_sock = SDLNet_TCP_Open(&peer);
	if (m_sock == 0)
		throw warning
			(_("Could not establish connection to host"),
			 _
			 	("Widelands could not establish a connection to the given address.\n"
			 	 "Either there was no metaserver running at the supposed port or\n"
			 	 "your network setup is broken."));

	m_sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (m_sockset, m_sock);


	// If we are here, a connection was established and we can send our login package through the socket.
	dedicatedlog("InternetGaming: Sending log in request.\n");
	SendPacket s;
	s.String(IGPCMD_LOGIN);
	s.String(boost::lexical_cast<std::string>(INTERNET_GAMING_PROTOCOL_VERSION));
	s.String(nick);
	s.String(build_id());
	s.String(bool2str(reg));
	if (reg)
		s.String(pwd);
	s.send(m_sock);

	// Now let's see, whether the metaserver is answering
	uint32_t const secs = time(0);
	m_state = CONNECTING;
	while (INTERNET_GAMING_TIMEOUT > time(0) - secs) {
		handle_metaserver_communication();
		// Check if we are a step further... if yes handle_packet has taken care about all the
		// paperwork, so we put our feet up and just return. ;)
		if (m_state != CONNECTING) {
			if (m_state == LOBBY) {
				formatAndAddChat("", "", true, _("For hosting a game, please take a look at the notes at:"));
				formatAndAddChat("", "", true, "http://wl.widelands.org/wiki/InternetGaming");
				return true;
			} else if (m_state == OFFLINE)
				return false;
		}
	}
	dedicatedlog("InternetGaming: No answer from metaserver!\n");
	logout("NO_ANSWER");
	return false;
}



/// logout of the metaserver
/// \note \arg msgcode should be a message from the list of InternetGamingMessages
void InternetGaming::logout(std::string const & msgcode) {

	// Just in case the metaserver is listening on the socket - tell him we break up with him ;)
	SendPacket s;
	s.String(IGPCMD_DISCONNECT);
	s.String(msgcode);
	s.send(m_sock);

	const std::string & msg = InternetGamingMessages::get_message(msgcode);
	dedicatedlog("InternetGaming: %s!\n", msg.c_str());
	formatAndAddChat("", "", true, msg);

	m_sock    = 0;
	m_sockset = 0;
	m_state   = OFFLINE;
}



/// handles all communication between the metaserver and the client
void InternetGaming::handle_metaserver_communication() {
	try {
		while (m_sock != 0 && SDLNet_CheckSockets(m_sockset, 0) > 0) {
			// Perform only one read operation, then process all packets
			// from this read. This ensures that we process DISCONNECT
			// packets that are followed immediately by connection close.
			if (!m_deserializer.read(m_sock)) {
				logout("CONNECTION_LOST");
				return;
			}

			// Process all the packets from the last read
			while (m_sock && m_deserializer.avail()) {
				RecvPacket packet(m_deserializer);
				handle_packet(packet);
			}
		}
	} catch (std::exception const & e) {
		// TODO  wouldn't it be better to throw warning or throw wexception ?
		std::string reason = _("Something went wrong: ");
		reason += e.what();
		logout(reason);
	}

	if (m_state == LOBBY) {
		// client is in the lobby and therefore we want realtime information updates
		if (clientupdateonmetaserver) {
			SendPacket s;
			s.String(IGPCMD_CLIENTS);
			s.send(m_sock);

			clientupdateonmetaserver = false;
		}

		if (gameupdateonmetaserver) {
			SendPacket s;
			s.String(IGPCMD_GAMES);
			s.send(m_sock);

			gameupdateonmetaserver = false;
		}
	}

	if (waitlist.size() > 0) {
		// TODO Check if a timeout is reached
#warning needs implementation
/*
		time_t now = time(0);
		for (uint8_t i = 0; i < waitlist.size(); ++i) {
			if (now > waitlist.at(i).timeout) {
			}
		}
		*/
	}
}



/// Handle one packet received from the metaserver.
void InternetGaming::handle_packet(RecvPacket & packet)
{
	std::string cmd = packet.String();

	// First check if everything is fine or whether the metaserver broke up with the client.
	if (cmd == IGPCMD_DISCONNECT) {
		std::string reason = packet.String();
		logout(reason);
		return;
	}

	// Are we already online?
	if (m_state == CONNECTING) {
		if (cmd == IGPCMD_LOGIN) {
			// Clients request to login was granted
			m_clientname   = packet.String();
			formatAndAddChat("", "", true, packet.String()); // welcome message
			m_clientrights = boost::lexical_cast<uint8_t>(packet.String());
			m_state        = LOBBY;
			dedicatedlog("InternetGaming: Client %s logged in.\n", m_clientname.c_str());
			return;

		} else if (cmd == IGPCMD_RELOGIN) {
			// Clients request to relogin was granted
			m_state = LOBBY;
			dedicatedlog("InternetGaming: Client %s relogged in.\n", m_clientname.c_str());
			return;

		} else if (cmd == IGPCMD_REJECTED) {
			// Clients login request got rejected
			logout(packet.String());
			return;

		} else {
			logout();
			throw warning
				(_
				 	("Expected a LOGIN, RELOGIN or REJECTED packet from server, but received command "
				 	 "%s. Maybe the metaserver is using a different protocol version ?"),
				 cmd.c_str());
		}
	}

	try {
		if (cmd == IGPCMD_LOGIN || cmd == IGPCMD_RELOGIN || cmd == IGPCMD_REJECTED) {
			// Login specific commands but not in CONNECTING state...
			dedicatedlog
				("InternetGaming: Received %s cmd although client is not in CONNECTING state.\n", cmd.c_str());
			std::string temp =
				(boost::format
					(_("WARNING: Received a %s command although we are not in CONNECTING state.")) % cmd)
				.str();
			formatAndAddChat("", "", true, temp);
		}

		if (cmd == IGPCMD_TIME) {
			// Client received the server time
			time_offset = boost::lexical_cast<int32_t>(packet.String());
			dedicatedlog("InternetGaming: Server time offset is %i seconds.\n", time_offset);
			std::string temp = (boost::format(_("Server time offset is %i seconds.")) % time_offset).str();
			formatAndAddChat("", "", true, temp);
		}

		if (cmd == IGPCMD_PING) {
			// Client received a PING and should immediately PONG as requested
			SendPacket s;
			s.String(IGPCMD_PONG);
			s.send(m_sock);
		}

		if (cmd == IGPCMD_CHAT) {
			// Client received a chat message
			std::string sender   = packet.String();
			std::string message  = packet.String();
			bool        personal = str2bool(packet.String());
			bool        system   = str2bool(packet.String());

			formatAndAddChat(sender, personal ? m_clientname : "", system, message);
		}

		if (cmd == IGPCMD_GAMES_UPDATE) {
			// Cliente received a note, that the list of games was changed
			gameupdateonmetaserver = true;
		}

		if (cmd == IGPCMD_GAMES) {
			// Cliente received the new list of games
			uint8_t number = boost::lexical_cast<uint8_t>(packet.String());
			gamelist.clear();
			for (uint8_t i = 0; i < number; ++i) {
				INet_Game * ing  = new INet_Game();
				ing->name        = packet.String();
				ing->build_id    = packet.String();
				ing->connectable = str2bool(packet.String());
				gamelist.push_back(*ing);
			}
			gameupdate = true;
		}

		if (cmd == IGPCMD_CLIENTS_UPDATE) {
			// Client received a note, that the list of clients was changed
			clientupdateonmetaserver = true;
		}

		if (cmd == IGPCMD_CLIENTS) {
			// Cliente received the new list of clients
			uint8_t number = boost::lexical_cast<uint8_t>(packet.String());
			clientlist.clear();
			for (uint8_t i = 0; i < number; ++i) {
				INet_Client * inc  = new INet_Client();
				inc->name        = packet.String();
				inc->build_id    = packet.String();
				inc->game        = packet.String();
				inc->type        = boost::lexical_cast<uint8_t>(packet.String());
				inc->points      = packet.String();
				clientlist.push_back(*inc);
			}
			clientupdate = true;
		}

		if (cmd == IGPCMD_GAME_OPEN) {
			// Client received the acknowledgment, that the game was opened
	#warning implement timeout
		}

		if (cmd == IGPCMD_GAME_CONNECT) {
			// Client received the ip for the game it wants to join
			m_gameip = packet.String();
		}

		if (cmd == IGPCMD_GAME_START) {
			// Client received the acknowledgment, that the game was started
	#warning implement timeout
		}

		if (cmd == IGPCMD_GAME_NOT_CONNECTABLE) {
			// Client received the message, that the game is not connectable
	#warning add system chat to client as soon as the nethost can handle imports of system chats to the game
		}
	} catch (warning & e) {
		formatAndAddChat("", "", true, e.what());
	}

}



/// \returns the ip of the game the client is on or wants to join (or the client is hosting)
///          or 0, if no ip available.
const std::string & InternetGaming::ip() {
	return m_gameip;
}



/// called by a client to join the game \arg gamename
void InternetGaming::join_game(const std::string & gamename) {
	SendPacket s;
	s.String(IGPCMD_GAME_CONNECT);
	s.String(gamename);
	s.send(m_sock);
	m_gamename = gamename;
	dedicatedlog("InternetGaming: Client tries to join a game with the name %s\n", m_gamename.c_str());
	m_state = IN_GAME;

/*
	// From now on we wait for a reply from the metaserver
	WaitForReply * wait = new WaitForReply;
	wait->cmd     = IGPCMD_GAME_CONNECT;
	wait->timeout = time(0) + INTERNET_GAMING_TIMEOUT;
	waitlist.push_back(*wait);
	*/
}



/// called by a client to open a new game with name m_gamename
void InternetGaming::open_game() {
	SendPacket s;
	s.String(IGPCMD_GAME_OPEN);
	s.String(m_gamename);
	s.String(boost::lexical_cast<std::string>(m_maxclients));
	s.send(m_sock);
	dedicatedlog("InternetGaming: Client opened a game with the name %s.\n", m_gamename.c_str());
	m_state = IN_GAME;

/*
	// From now on we wait for a reply from the metaserver
	WaitForReply * wait = new WaitForReply;
	wait->cmd     = IGPCMD_GAME_OPEN;
	wait->timeout = time(0) + INTERNET_GAMING_TIMEOUT;
	waitlist.push_back(*wait);
	*/
}



/// called by a client that is host of a game to inform the metaserver, that the game started
void InternetGaming::set_game_playing() {
	SendPacket s;
	s.String(IGPCMD_GAME_START);
	s.send(m_sock);
	dedicatedlog("InternetGaming: Client announced the start of the game %s.\n", m_gamename.c_str());

/*
	// From now on we wait for a reply from the metaserver
	WaitForReply * wait = new WaitForReply;
	wait->cmd     = IGPCMD_GAME_START;
	wait->timeout = time(0) + INTERNET_GAMING_TIMEOUT;
	waitlist.push_back(*wait);
	*/
}



/// called by a client that is host of a game to inform the metaserver, that the game was ended.
void InternetGaming::set_game_done() {
	SendPacket s;
	s.String(IGPCMD_GAME_DISCONNECT);
	s.send(m_sock);
	m_gameip  = "";
	dedicatedlog("InternetGaming: Client announced the end of the game %s.\n", m_gamename.c_str());
}



/// \returns whether the local gamelist was updated
/// \note this function resets gameupdate. So if you call it, please really handle the output.
bool InternetGaming::updateForGames() {
	bool temp = gameupdate;
	gameupdate = false;
	return temp;
}



/// \returns the tables in the room
std::vector<INet_Game> const & InternetGaming::games() {
	return gamelist;
}



/// \returns whether the local clientlist was updated
/// \note this function resets clientupdate. So if you call it, please really handle the output.
bool InternetGaming::updateForClients() {
	bool temp = clientupdate;
	clientupdate = false;
	return temp;
}



/// \returns the players in the room
std::vector<INet_Client> const & InternetGaming::clients() {
	return clientlist;
}



/// ChatProvider: sends a message via the metaserver.
void InternetGaming::send(std::string const & msg) {
	if (!logged_in()) {
		formatAndAddChat
			("", "", true, _("Message could not be send: You are not connected to the metaserver!"));
		return;
	}

	SendPacket s;
	s.String(IGPCMD_CHAT);

	if (msg.size() && *msg.begin() == '@') {
		// Format a personal message
		std::string::size_type const space = msg.find(' ');
		if (space >= msg.size() - 1) {
			formatAndAddChat
				("", "", true, _("Message could not be send: Was this supposed to be a private message?"));
			return;
		}
		s.String(msg.substr(space + 1));    // message
		s.String(msg.substr(1, space - 1)); // recipient

		formatAndAddChat(m_clientname, msg.substr(1, space - 1), false, msg.substr(space + 1));
	} else {
		s.String(msg);
		s.String("");
	}

	s.send(m_sock);
}



/**
 * \returns the boolean value of a string received from the metaserver.
 * If conversion fails, it throws a \ref warning
 */
bool InternetGaming::str2bool(std::string str) {
	if (str == "true" || str == "false")
		throw warning("Conversion from std::string to bool failed. String was \"%s\"", str.c_str());
	return str == "true";
}

/// \returns a string containing the boolean value \arg b to be send to metaserver
std::string InternetGaming::bool2str(bool b) {
	return b ? "true" : "false";
}


/// formates a chat message and adds it to the list of chat messages
void InternetGaming::formatAndAddChat(std::string from, std::string to, bool system, std::string msg) {
	ChatMessage c;
	c.time      = time(0);
	c.sender    = !system && from.empty() ? _("<unknown>") : from;
	c.playern   = system ? -1 : to.size() ? 3 : 7;
	c.msg       = msg;
	c.recipient = to;

	receive(c);
	if (system && (m_state == IN_GAME)) {
		// Save system chat messages seperately as well, so the nethost can import and show them in game;
		c.msg = "METASERVER: " + msg;
		ingame_system_chat.push_back(c);
	}
}

