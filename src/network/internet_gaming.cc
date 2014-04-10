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

#include "network/internet_gaming.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <libintl.h>

#include "compile_diagnostics.h"
#include "i18n.h"
#include "io/dedicated_log.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "network/internet_gaming_messages.h"
#include "warning.h"



/// Private constructor by purpose: NEVER call directly. Always call InternetGaming::ref(), this will ensure
/// that only one instance is running at time.
InternetGaming::InternetGaming() :
	m_sock                   (nullptr),
	m_sockset                (nullptr),
	m_state                  (OFFLINE),
	m_reg                    (false),
	m_port                   (INTERNET_GAMING_PORT),
	m_clientrights           (INTERNET_CLIENT_UNREGISTERED),
	m_maxclients             (1),
	m_gameip                 (""),
	clientupdateonmetaserver (true),
	gameupdateonmetaserver   (true),
	clientupdate             (false),
	gameupdate               (false),
	time_offset              (0),
	waittimeout              (std::numeric_limits<int32_t>::max()),
	lastping                 (time(nullptr))
{
	// Fill the list of possible messages from the server
	InternetGamingMessages::fill_map();

	// Set connection tracking variables to 0
	lastbrokensocket[0]      = 0;
	lastbrokensocket[1]      = 0;

}

/// resets all stored variables without the chat messages for a clean new login (not relogin)
void InternetGaming::reset() {
	m_sock                   = nullptr;
	m_sockset                = nullptr;
	m_state                  = OFFLINE;
	m_pwd                    = "";
	m_reg                    = false;
	m_meta                   = INTERNET_GAMING_METASERVER;
	m_port                   = INTERNET_GAMING_PORT;
	m_clientname             = "";
	m_clientrights           = INTERNET_CLIENT_UNREGISTERED;
	m_maxclients             = 1;
	m_gamename               = "";
	m_gameip                 = "";
	clientupdateonmetaserver = true;
	gameupdateonmetaserver   = true;
	clientupdate             = false;
	gameupdate               = false;
	time_offset              = 0;
	waitcmd                  = "";
	waittimeout              = std::numeric_limits<int32_t>::max();
	lastbrokensocket[0]      = 0;
	lastbrokensocket[1]      = 0;
	lastping                 = time(nullptr);

	clientlist.clear();
	gamelist.clear();
}


/// the one and only InternetGaming instance.
static InternetGaming * ig = nullptr;


/// \returns the one and only InternetGaming instance.
InternetGaming & InternetGaming::ref() {
	if (not ig)
		ig = new InternetGaming();
	return * ig;
}


void InternetGaming::initialiseConnection() {
	// First of all try to connect to the metaserver
	dedicatedlog("InternetGaming: Connecting to the metaserver.\n");
	IPaddress peer;
	if (hostent * const he = gethostbyname(m_meta.c_str())) {
		peer.host = (reinterpret_cast<in_addr *>(he->h_addr_list[0]))->s_addr;
GCC_DIAG_OFF("-Wold-style-cast")
		peer.port = htons(m_port);
GCC_DIAG_ON("-Wold-style-cast")
	} else
		throw warning
			(_("Connection problem"), "%s", _("Widelands could not connect to the metaserver."));

	SDLNet_ResolveHost (&peer, m_meta.c_str(), m_port);
	m_sock = SDLNet_TCP_Open(&peer);
	if (m_sock == nullptr)
		throw warning
			(_("Could not establish connection to host"),
			 _
			 	("Widelands could not establish a connection to the given address.\n"
			 	 "Either there was no metaserver running at the supposed port or\n"
			 	 "your network setup is broken."));

	m_sockset = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket (m_sockset, m_sock);

	// Of course not 100% true, but we just care about an answer at all, so we reset this tracker
	lastping = time(nullptr);
}



/// Login to metaserver
bool InternetGaming::login
	(const std::string & nick, const std::string & pwd, bool reg, const std::string & meta, uint32_t port)
{
	assert(m_state == OFFLINE);

	m_pwd  = pwd;
	m_reg  = reg;
	m_meta = meta;
	m_port = port;

	initialiseConnection();

	// If we are here, a connection was established and we can send our login package through the socket.
	dedicatedlog("InternetGaming: Sending login request.\n");
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
	uint32_t const secs = time(nullptr);
	m_state = CONNECTING;
	while (INTERNET_GAMING_TIMEOUT > time(nullptr) - secs) {
		handle_metaserver_communication();
		// Check if we are a step further... if yes handle_packet has taken care about all the
		// paperwork, so we put our feet up and just return. ;)
		if (m_state != CONNECTING) {
			if (m_state == LOBBY) {
				formatAndAddChat("", "", true, _("For hosting a game, please take a look at the notes at:"));
				formatAndAddChat("", "", true, "http://wl.widelands.org/wiki/InternetGaming");
				return true;
			} else if (error())
				return false;
		}
	}
	dedicatedlog("InternetGaming: No answer from metaserver!\n");
	logout("NO_ANSWER");
	return false;
}



/// Relogin to metaserver after loosing connection
bool InternetGaming::relogin()
{
	assert(error());

	initialiseConnection();

	// If we are here, a connection was established and we can send our login package through the socket.
	dedicatedlog("InternetGaming: Sending relogin request.\n");
	SendPacket s;
	s.String(IGPCMD_RELOGIN);
	s.String(boost::lexical_cast<std::string>(INTERNET_GAMING_PROTOCOL_VERSION));
	s.String(m_clientname);
	s.String(build_id());
	s.String(bool2str(m_reg));
	if (m_reg)
		s.String(m_pwd);
	s.send(m_sock);

	// Now let's see, whether the metaserver is answering
	uint32_t const secs = time(nullptr);
	m_state = CONNECTING;
	while (INTERNET_GAMING_TIMEOUT > time(nullptr) - secs) {
		handle_metaserver_communication();
		// Check if we are a step further... if yes handle_packet has taken care about all the
		// paperwork, so we put our feet up and just return. ;)
		if (m_state != CONNECTING) {
			if (m_state == LOBBY) {
				break;
			} else if (error())
				return false;
		}
	}

	if (INTERNET_GAMING_TIMEOUT <= time(nullptr) - secs) {
		dedicatedlog("InternetGaming: No answer from metaserver!\n");
		return false;
	}

	// Client is reconnected, so let's try resend the timeouted command.
	if (waitcmd == IGPCMD_GAME_CONNECT)
		join_game(m_gamename);
	else if (waitcmd == IGPCMD_GAME_OPEN) {
		m_state = IN_GAME;
		open_game();
	} else if (waitcmd == IGPCMD_GAME_START) {
		m_state = IN_GAME;
		set_game_playing();
	}

	return true;
}



/// logout of the metaserver
/// \note \arg msgcode should be a message from the list of InternetGamingMessages
void InternetGaming::logout(const std::string & msgcode) {

	// Just in case the metaserver is listening on the socket - tell him we break up with him ;)
	SendPacket s;
	s.String(IGPCMD_DISCONNECT);
	s.String(msgcode);
	s.send(m_sock);

	const std::string & msg = InternetGamingMessages::get_message(msgcode);
	dedicatedlog("InternetGaming: logout(%s)\n", msg.c_str());
	formatAndAddChat("", "", true, msg);

	reset();
}



/// handles all communication between the metaserver and the client
void InternetGaming::handle_metaserver_communication() {
	if (error())
		return;
	try {
		while (m_sock != nullptr && SDLNet_CheckSockets(m_sockset, 0) > 0) {
			// Perform only one read operation, then process all packets
			// from this read. This ensures that we process DISCONNECT
			// packets that are followed immediately by connection close.
			if (!m_deserializer.read(m_sock)) {
				setError();
				const std::string & msg = InternetGamingMessages::get_message("CONNECTION_LOST");
				dedicatedlog("InternetGaming: Error: %s\n", msg.c_str());
				formatAndAddChat("", "", true, msg);

				// Check how much time passed since the socket broke the last time
				// Maybe something is completely wrong at the moment?
				// At least it seems to be, if the socket broke three times in the last 10 seconds...
				time_t now = time(nullptr);
				if ((now - lastbrokensocket[1] < 10) && (now - lastbrokensocket[0] < 10)) {
					reset();
					setError();
					return;
				}
				lastbrokensocket[1] = lastbrokensocket[0];
				lastbrokensocket[0] = now;

				// Try to relogin
				if (!relogin()) {
					// Do not try to relogin again automatically.
					reset();
					setError();
				}
				return;
			}

			// Process all the packets from the last read
			while (m_sock && m_deserializer.avail()) {
				RecvPacket packet(m_deserializer);
				handle_packet(packet);
			}
		}
	} catch (const std::exception & e) {
		std::string reason = _("Something went wrong: ");
		reason += e.what();
		logout(reason);
		setError();
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

	if (waitcmd.size() > 0) {
		// Check if timeout is reached
		time_t now = time(nullptr);
		if (now > waittimeout) {
			setError();
			waittimeout = std::numeric_limits<int32_t>::max();
			dedicatedlog("InternetGaming: reached a timeout for an awaited answer of the metaserver!\n");
			if (!relogin()) {
				// Do not try to relogin again automatically.
				reset();
				setError();
			}
		}
	}

	// Check connection to the metaserver
	// Was a ping received in the last 4 minutes?
	if (time(nullptr) - lastping > 240)  {
		// Try to relogin
		setError();
		if (!relogin()) {
			// Do not try to relogin again automatically.
			reset();
			setError();
		}
	}
}



/// Handle one packet received from the metaserver.
void InternetGaming::handle_packet(RecvPacket & packet)
{
	std::string cmd = packet.String();

	// First check if everything is fine or whether the metaserver broke up with the client.
	if (cmd == IGPCMD_DISCONNECT) {
		std::string reason = packet.String();
		formatAndAddChat("", "", true, InternetGamingMessages::get_message(reason));
		if (reason == "CLIENT_TIMEOUT") {
			// Try to relogin
			setError();
			if (!relogin()) {
				// Do not try to relogin again automatically.
				reset();
				setError();
			}
		}
		return;
	}

	// Are we already online?
	if (m_state == CONNECTING) {
		if (cmd == IGPCMD_LOGIN) {
			// Clients request to login was granted
			m_clientname   = packet.String();
			m_clientrights = packet.String();
			m_state        = LOBBY;
			dedicatedlog("InternetGaming: Client %s logged in.\n", m_clientname.c_str());
			return;

		} else if (cmd == IGPCMD_RELOGIN) {
			// Clients request to relogin was granted
			m_state = LOBBY;
			dedicatedlog("InternetGaming: Client %s relogged in.\n", m_clientname.c_str());
			formatAndAddChat("", "", true, _("Successfully reconnected to the metaserver!"));
			return;

		} else if (cmd == IGPCMD_ERROR) {
			std::string errortype = packet.String();
			if (errortype != "LOGIN" && errortype != "RELOGIN") {
				dedicatedlog("InternetGaming: Strange ERROR in connecting state: %s\n", errortype.c_str());
				throw warning(_("Mixed up"), _("The metaserver sent a strange ERROR during connection"));
			}
			// Clients login request got rejected
			logout(packet.String());
			setError();
			return;

		} else {
			logout();
			setError();
			throw warning
				(_("Unexpected packet"),
				 _
				 	("Expected a LOGIN, RELOGIN or REJECTED packet from server, but received command "
				 	 "%s. Maybe the metaserver is using a different protocol version ?"),
				 cmd.c_str());
		}
	}

	try {
		if (cmd == IGPCMD_LOGIN || cmd == IGPCMD_RELOGIN) {
			// Login specific commands but not in CONNECTING state...
			dedicatedlog
				("InternetGaming: Received %s cmd although client is not in CONNECTING state.\n", cmd.c_str());
			std::string temp =
				(boost::format
					(_("WARNING: Received a %s command although we are not in CONNECTING state.")) % cmd)
				.str();
			formatAndAddChat("", "", true, temp);
		}

		else if (cmd == IGPCMD_TIME) {
			// Client received the server time
			time_offset = boost::lexical_cast<int>(packet.String()) - time(nullptr);
			dedicatedlog
				(ngettext
					("InternetGaming: Server time offset is %u second.",
				 	 "InternetGaming: Server time offset is %u seconds.", time_offset),
				 time_offset);
			std::string temp =
				(boost::format
					(ngettext("Server time offset is %u second.",
			        	 "Server time offset is %u seconds.", time_offset))
				 % time_offset)
				 .str();
			formatAndAddChat("", "", true, temp);
		}

		else if (cmd == IGPCMD_PING) {
			// Client received a PING and should immediately PONG as requested
			SendPacket s;
			s.String(IGPCMD_PONG);
			s.send(m_sock);

			lastping = time(nullptr);
		}

		else if (cmd == IGPCMD_CHAT) {
			// Client received a chat message
			std::string sender   = packet.String();
			std::string message  = packet.String();
			std::string type     = packet.String();

			if (type != "public" && type != "private" && type != "system")
				throw warning(_("Invalid message type"), _("Invalid chat message type \"%s\"."), type.c_str());

			bool        personal = type == "private";
			bool        system   = type == "system";

			formatAndAddChat(sender, personal ? m_clientname : "", system, message);
		}

		else if (cmd == IGPCMD_GAMES_UPDATE) {
			// Client received a note, that the list of games was changed
			dedicatedlog("InternetGaming: Game update on metaserver.\n");
			gameupdateonmetaserver = true;
		}

		else if (cmd == IGPCMD_GAMES) {
			// Client received the new list of games
			uint8_t number = boost::lexical_cast<int>(packet.String()) & 0xff;
			std::vector<INet_Game> old = gamelist;
			gamelist.clear();
			dedicatedlog("InternetGaming: Received a game list update with %u items.\n", number);
			for (uint8_t i = 0; i < number; ++i) {
				INet_Game * ing  = new INet_Game();
				ing->name        = packet.String();
				ing->build_id    = packet.String();
				ing->connectable = str2bool(packet.String());
				gamelist.push_back(*ing);

				bool found = false;
				for (std::vector<INet_Game>::size_type j = 0; j < old.size(); ++j)
					if (old[j].name == ing->name) {
						found = true;
						old[j].name = "";
						break;
					}
				if (!found)
					formatAndAddChat
						("", "", true, (boost::format(_("The game %s is now available")) % ing->name).str());

				delete ing;
				ing = nullptr;
			}

			for (std::vector<INet_Game>::size_type i = 0; i < old.size(); ++i)
				if (old[i].name.size())
					formatAndAddChat
						("", "", true, (boost::format(_("The game %s has been closed")) % old[i].name).str());

			gameupdate = true;
		}

		else if (cmd == IGPCMD_CLIENTS_UPDATE) {
			// Client received a note, that the list of clients was changed
			dedicatedlog("InternetGaming: Client update on metaserver.\n");
			clientupdateonmetaserver = true;
		}

		else if (cmd == IGPCMD_CLIENTS) {
			// Client received the new list of clients
			uint8_t number = boost::lexical_cast<int>(packet.String()) & 0xff;
			std::vector<INet_Client> old = clientlist;
			clientlist.clear();
			dedicatedlog("InternetGaming: Received a client list update with %u items.\n", number);
			for (uint8_t i = 0; i < number; ++i) {
				INet_Client * inc  = new INet_Client();
				inc->name        = packet.String();
				inc->build_id    = packet.String();
				inc->game        = packet.String();
				inc->type        = packet.String();
				inc->points      = packet.String();
				clientlist.push_back(*inc);

				bool found = old.empty(); // do not show all clients, if this instance is the actual change
				for (std::vector<INet_Client>::size_type j = 0; j < old.size(); ++j)
					if (old[j].name == inc->name) {
						found = true;
						old[j].name = "";
						break;
					}
				if (!found)
					formatAndAddChat
						("", "", true, (boost::format(_("%s joined the lobby")) % inc->name).str());

				delete inc;
				inc = nullptr;
			}

			for (std::vector<INet_Client>::size_type i = 0; i < old.size(); ++i)
				if (old[i].name.size())
					formatAndAddChat
						("", "", true, (boost::format(_("%s left the lobby")) % old[i].name).str());

			clientupdate = true;
		}

		else if (cmd == IGPCMD_GAME_OPEN) {
			// Client received the acknowledgment, that the game was opened
			assert (waitcmd == IGPCMD_GAME_OPEN);
			waitcmd = "";
		}

		else if (cmd == IGPCMD_GAME_CONNECT) {
			// Client received the ip for the game it wants to join
			assert (waitcmd == IGPCMD_GAME_CONNECT);
			waitcmd = "";
			// save the received ip, so the client cann connect to the game
			m_gameip = packet.String();
			log("InternetGaming: Received ip of the game to join: %s.\n", m_gameip.c_str());
		}

		else if (cmd == IGPCMD_GAME_START) {
			// Client received the acknowledgment, that the game was started
			assert (waitcmd == IGPCMD_GAME_START);
			waitcmd = "";
		}

		else if (cmd == IGPCMD_ERROR) {
			// Client received an ERROR message - seems something went wrong
			std::string subcmd    (packet.String());
			std::string reason (packet.String());
			std::string message = "";

			if (subcmd == IGPCMD_CHAT) {
				// Something went wrong with the chat message the user sent.
				message += _("Chat message could not be sent. ");
				if (reason == "NO_SUCH_USER")
					message +=
						(boost::format
							(InternetGamingMessages::get_message(reason)) % packet.String().c_str())
						.str();
			}

			else if (subcmd == IGPCMD_GAME_OPEN) {
				// Something went wrong with the newly opened game
				message = InternetGamingMessages::get_message(reason);
				// we got our answer, so no need to wait anymore
				waitcmd = "";
			}
			message = (boost::format(_("ERROR: %s")) % message).str();

			// Finally send the error message as system chat to the client.
			formatAndAddChat("", "", true, message);
		}

		else
			// Inform the client about the unknown command
			formatAndAddChat(
				"", "", true,
				(boost::format(_("Received an unknown command from the metaserver: %s")) % cmd).str()
			);

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
	if (!logged_in())
		return;

	SendPacket s;
	s.String(IGPCMD_GAME_CONNECT);
	s.String(gamename);
	s.send(m_sock);
	m_gamename = gamename;
	dedicatedlog("InternetGaming: Client tries to join a game with the name %s\n", m_gamename.c_str());
	m_state = IN_GAME;


	// From now on we wait for a reply from the metaserver
	waitcmd     = IGPCMD_GAME_CONNECT;
	waittimeout = time(nullptr) + INTERNET_GAMING_TIMEOUT;
}



/// called by a client to open a new game with name m_gamename
void InternetGaming::open_game() {
	if (!logged_in())
		return;

	SendPacket s;
	s.String(IGPCMD_GAME_OPEN);
	s.String(m_gamename);
	s.String(boost::lexical_cast<std::string>(m_maxclients));
	s.send(m_sock);
	dedicatedlog("InternetGaming: Client opened a game with the name %s.\n", m_gamename.c_str());
	m_state = IN_GAME;

	// From now on we wait for a reply from the metaserver
	waitcmd     = IGPCMD_GAME_OPEN;
	waittimeout = time(nullptr) + INTERNET_GAMING_TIMEOUT;
}



/// called by a client that is host of a game to inform the metaserver, that the game started
void InternetGaming::set_game_playing() {
	if (!logged_in())
		return;

	SendPacket s;
	s.String(IGPCMD_GAME_START);
	s.send(m_sock);
	dedicatedlog("InternetGaming: Client announced the start of the game %s.\n", m_gamename.c_str());

	// From now on we wait for a reply from the metaserver
	waitcmd     = IGPCMD_GAME_START;
	waittimeout = time(nullptr) + INTERNET_GAMING_TIMEOUT;
}



/// called by a client to inform the metaserver, that it left the game and is back in the lobby.
/// If this is called by the hosting client, this further informs the metaserver, that the game was closed.
void InternetGaming::set_game_done() {
	if (!logged_in())
		return;

	SendPacket s;
	s.String(IGPCMD_GAME_DISCONNECT);
	s.send(m_sock);

	m_gameip  = "";
	m_state   = LOBBY;

	dedicatedlog("InternetGaming: Client announced the disconnect from the game %s.\n", m_gamename.c_str());
}



/// \returns whether the local gamelist was updated
/// \note this function resets gameupdate. So if you call it, please really handle the output.
bool InternetGaming::updateForGames() {
	bool temp = gameupdate;
	gameupdate = false;
	return temp;
}



/// \returns the tables in the room, if no error occured
const std::vector<INet_Game> & InternetGaming::games() {
	return error() ? * (new std::vector<INet_Game>()) : gamelist;
}



/// \returns whether the local clientlist was updated
/// \note this function resets clientupdate. So if you call it, please really handle the output.
bool InternetGaming::updateForClients() {
	bool temp = clientupdate;
	clientupdate = false;
	return temp;
}



/// \returns the players in the room, if no error occured
const std::vector<INet_Client> & InternetGaming::clients() {
	return error() ? * (new std::vector<INet_Client>()) : clientlist;
}



/// ChatProvider: sends a message via the metaserver.
void InternetGaming::send(const std::string & msg) {
	if (!logged_in()) {
		formatAndAddChat
			("", "", true, _("Message could not be sent: You are not connected to the metaserver!"));
		return;
	}

	SendPacket s;
	s.String(IGPCMD_CHAT);

	if (msg.size() && *msg.begin() == '@') {
		// Format a personal message
		std::string::size_type const space = msg.find(' ');
		if (space >= msg.size() - 1) {
			formatAndAddChat
				("", "", true, _("Message could not be sent: Was this supposed to be a private message?"));
			return;
		}
		s.String(msg.substr(space + 1));    // message
		s.String(msg.substr(1, space - 1)); // recipient

		formatAndAddChat(m_clientname, msg.substr(1, space - 1), false, msg.substr(space + 1));

	} else if (m_clientrights == INTERNET_CLIENT_SUPERUSER && msg.size() && *msg.begin() == '/') {
		// This is either a /me command, a super user command, or well... just a chat message beginning
		// with a "/" - let's see...

		// Split up in "cmd" "arg"
		std::string cmd, arg;
		std::string temp = msg.substr(1); // cut off '/'
		std::string::size_type const space = temp.find(' ');
		if (space > temp.size())
			// no argument
			goto normal;

		// get the cmd and the arg
		cmd = temp.substr(0, space);
		arg = temp.substr(space + 1);

		if (cmd == "motd") {
			SendPacket m;
			m.String(IGPCMD_MOTD);
			// Check whether motd is attached or should be loaded from a file
			if (arg.size() > 1 && arg.at(0) == '%') {
				// Seems we should load the motd from a file
				temp = arg.substr(1); // cut of the "%"
				if (g_fs->FileExists(temp) && !g_fs->IsDirectory(temp)) {
					// Read in the file
					Widelands::FileRead fr;
					fr.Open(*g_fs, temp.c_str());
					if (!fr.EndOfFile()) {
						arg = fr.ReadLine();
						while (!fr.EndOfFile()) {
							arg += fr.ReadLine();
						}
					}
				}
			}
			// send the request to change the motd
			m.String(arg);
			m.send(m_sock);
			return;
		} else if (cmd == "announcement") {
			// send the request to change the motd
			SendPacket m;
			m.String(IGPCMD_ANNOUNCEMENT);
			m.String(arg);
			m.send(m_sock);
			return;
		} else
			// let everything else pass
			goto normal;

	} else {
		normal:
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
	if ((str != "true") && (str != "false"))
		throw warning
			(_("Conversion error"),
			_("Conversion from std::string to bool failed. String was \"%s\""), str.c_str());

	return str == "true";
}

/// \returns a string containing the boolean value \arg b to be send to metaserver
std::string InternetGaming::bool2str(bool b) {
	return b ? "true" : "false";
}


/// formates a chat message and adds it to the list of chat messages
void InternetGaming::formatAndAddChat(std::string from, std::string to, bool system, std::string msg) {
	ChatMessage c;
	if (!system && from.empty()) {
		std::string unkown_string =
			(boost::format("\\<%s\\>") % _("unknown")).str();
		c.sender = unkown_string;
	} else {
		c.sender = from;
	}
	c.time      = time(nullptr);
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
