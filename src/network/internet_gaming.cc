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
#include "warning.h"

#include <boost/lexical_cast.hpp>


/// Private constructor by purpose: NEVER call directly. Always call InternetGaming::ref(), this will ensure
/// that only one instance is running at time.
InternetGaming::InternetGaming() :
	m_sock        (0),
	m_sockset     (0),
	m_state       (OFFLINE),
	m_maxclients  (1),
	clientupdate  (false),
	gameupdate    (false)
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
	SendPacket s;
	s.String(IGPCMD_LOGIN);
	s.String(boost::lexical_cast<std::string>(INTERNET_GAMING_PROTOCOL_VERSION));
	s.String(nick);
	s.String(build_id());
	s.String(reg ? "true" : "false");
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
			if (m_state == LOBBY)
				return true;
			else if (m_state == OFFLINE)
				return false;
		}
	}
#warning max number of retries should be used here before we give up;
	logout("NO_ANSWERS");
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

#warning send the logout message from msgcode to the chat

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
		std::string reason = _("Something went wrong: ");
		reason += e.what();
#warning this should be an InternetGamingMessages code - the error should instead be posted in a pop up.
		logout(reason);
	}
}

/**
 * Handle one packet received from the metaserver.
 */
void InternetGaming::handle_packet(RecvPacket & packet)
{
	std::string cmd = packet.String();

	// First check if everything is fine or whether the metaserver broke up with us.
	if (cmd == IGPCMD_DISCONNECT) {
		std::string reason = packet.String();
		logout(reason);
		return;
	}

	// Are we already online?
	if (m_state == OFFLINE) {
		if (cmd == IGPCMD_LOGIN) {
			m_clientname = packet.String();
#warning show welcome message and save and handle client rights
			packet.String(); // welcome message
			packet.String(); // client rights
			m_state = LOBBY;
			return;
		} else if (cmd == IGPCMD_RELOGIN) {
			m_state = LOBBY;
			return;
		} else if (cmd == IGPCMD_REJECTED) {
			logout(packet.String());
			return;
		} else {
#warning fix me!!!
			logout();
			/*throw DisconnectException
				(_
				 	("Expected a LOGIN, RELOGIN or REJECTED packet from server, but received command "
				 	 "%s. Maybe the metaserver is using a different protocol version ?"),
				 cmd.c_str());*/
		}
	}

}


/// \returns the ip of the game the client is on or wants to join (or the client is hosting)
///          or 0, if no ip available.
char const * InternetGaming::ip() {return 0;}


/// called by a client to join the game \arg gamename
void InternetGaming::join_game(const std::string & gamename) {}


/// called by a client to open a new game with name \arg gamename
void InternetGaming::open_game() {}


/// called by a client that is host of a game to inform the metaserver, that the game started
void InternetGaming::set_game_playing() {}


/// called by a client that is host of a game to inform the metaserver, that the game was ended.
void InternetGaming::set_game_done() {}


/// \returns whether the local gamelist was updated
bool InternetGaming::updateForGames() {
	bool temp = gameupdate;
	gameupdate = false;
	return temp;
}

/// \returns the tables in the room
std::vector<Net_Game_Info> const & InternetGaming::games() {
	return gamelist;
}

/// \returns whether the local clientlist was updated
bool InternetGaming::updateForClients() {
	bool temp = clientupdate;
	clientupdate = false;
	return temp;
}

/// \returns the players in the room
std::vector<Net_Client>    const & InternetGaming::clients() {
	return clientlist;
}


/// ChatProvider: sends a message via the metaserver.
void InternetGaming::send(std::string const & msg) {}

