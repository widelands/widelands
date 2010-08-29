/*
* Copyright (C) 2004-2010 by the Widelands Development Team
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

#include "ggz_ggzcore.h"
#include "network_ggz.h"
#include "log.h"
#include "constants.h"
#include "wexception.h"

#include <stdint.h>
#include <string>
#include <vector>

#include <limits>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif

static GGZServer * ggzserver = 0;

ggz_ggzcore * ggzcoreobj = 0;

ggz_ggzcore::ggz_ggzcore() :
	ggzcore_login (false),
	ggzcore_ready (false),
	m_logged_in   (false),
	relogin       (false),
	m_tableid     (0),
	m_channelfd   (-1),
	m_gamefd      (-1),
	m_server_fd   (-1),
	m_room        (NULL),
	m_tablestate   (ggzcoretablestate_notinroom),
	m_state       (ggzcorestate_disconnected),
	m_servername  ("WLDefault"),
	m_tableseats  (1)
{}


ggz_ggzcore & ggz_ggzcore::ref() {
	if (!ggzcoreobj)
		ggzcoreobj = new ggz_ggzcore();
	return *ggzcoreobj;
}

bool ggz_ggzcore::init
	(char const * const metaserver, char const * const nick,
	 char const * const pwd, bool registered)
{
	log("GGZCORE ## init()\n");

	// do not init if already done
	if (m_state != ggzcorestate_disconnected or ggzserver){
		log("GGZCORE ## WARNING: already connected\n");
		return false;
	}
	GGZOptions opt;

	log("GGZCORE ## initialization\n");
	ggzcore_login = true;
	ggzcore_ready = false;

	opt.flags = static_cast<GGZOptionFlags>(GGZ_OPT_EMBEDDED);
	ggzcore_init(opt);

	//  Register callback functions for server events.
	//
	//  Not yet handled server events:
	//   * GGZ_SERVER_PLAYERS_CHANGED (instead only room players are handled)
	//   * GGZ_LOGOUT                 (useful for us?)
	//   * GGZ_SERVER_ROOMS_CHANGED   (should not happen on our own server.
	//                                 There should only be the widelands room.)
	//   * GGZ_STATE_CHANGE           (a.t.m. we explicitly check the state if
	//                                 we need it, but perhaps it could be a
	//                                 good idea to generally monitor it for
	//                                 debug reasons.)
	ggzserver = ggzcore_server_new();
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CONNECTED, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_NEGOTIATED, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_LOGGED_IN, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_ENTERED, &ggz_ggzcore::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CONNECT_FAIL, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_NEGOTIATE_FAIL, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_LOGIN_FAIL, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_ENTER_FAIL, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHANNEL_FAIL, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHAT_FAIL, &ggz_ggzcore::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_ROOM_LIST, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_TYPE_LIST, &ggz_ggzcore::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_NET_ERROR, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_PROTOCOL_ERROR, &ggz_ggzcore::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHANNEL_CONNECTED, &ggz_ggzcore::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHANNEL_READY, &ggz_ggzcore::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_MOTD_LOADED, &ggz_ggzcore::callback_server);
	
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_LOGOUT, &ggz_ggzcore::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_STATE_CHANGE, &ggz_ggzcore::callback_server);

	#if GGZCORE_VERSION_MINOR == 0
	ggzcore_server_set_hostinfo(ggzserver, metaserver, WL_METASERVER_PORT, 0);
	#else
	ggzcore_server_set_hostinfo
		(ggzserver, metaserver, WL_METASERVER_PORT, GGZ_CONNECTION_CLEAR);
	#endif

	// Login to registered account:
	if (registered)
		ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN, nick, pwd, 0);
	// Login anonymously:
	else
		ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN_GUEST, nick, 0, 0);

	ggzcore_server_connect(ggzserver);

	return true;
}

//\FIXME: mallformed updatedata, if the user is in a room and too many seats
//        are open (~ > 4).
/// checks for events on server, room and game
/// if data is pending the ggzcore_*_read_data function will call the fitting
/// callback function
void ggz_ggzcore::process()
{
	if (!ggzserver) {
		//log("ggz_ggzcore::process(): not ggzserver\n");
		return;
	}

	if (ggzcore_server_data_is_pending(ggzserver)) {
		//log("ggz_ggzcore::process(): server_data_is_pending\n");
		ggzcore_server_read_data(ggzserver, ggzcore_server_get_fd(ggzserver));
	}

	if (m_channelfd != -1)
		ggzcore_server_read_data
			(ggzserver, ggzcore_server_get_channel(ggzserver));
					
	if (m_gamefd != -1)
		ggzcore_game_read_data(ggzcore_server_get_cur_game(ggzserver));
}

/// Called by the client, to join an existing table (game) and to add all
/// hooks to get informed about all important events
void ggz_ggzcore::join(const char* tablename)
{
	log("GGZCORE ## join()\n");

	if (!ggzcore_ready) {
		log("GGZCORE ## not ready!\n");
		return;
	}
	
	if (!m_room)
		m_room = ggzcore_server_get_cur_room(ggzserver);
	ggzcore_room_get_gametype(m_room);
	
	m_tableid = -1;
	int32_t const num = ggzcore_room_get_num_tables(m_room);
	for (int32_t i = 0; i < num; ++i) {
		GGZTable * const table = ggzcore_room_get_nth_table(m_room, i);
		if (!table) {
			deinit();
			throw wexception("table can not be found!");
		}
				char const * desc = ggzcore_table_get_desc(table);
				if (!desc)
					desc = "Unnamed server";
				if (!strcmp(desc, tablename))
					m_tableid = ggzcore_table_get_id(table);
	}
	
	if (m_tableid == -1) {
		deinit();
		throw wexception
			("Selected table \"%s\" could not be found\n", tablename);
	}

	log("GGZCORE ## Joining Server \"%s\"\n", tablename);

	GGZGame * const game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, 0);

	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCHED, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCH_FAIL, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATED, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATE_FAIL, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_PLAYING, &ggz_ggzcore::callback_game);

	ggzcore_game_launch(game);
}


/// Called by the host, to launch a new table (game) and to add all
/// hooks to get informed about all important events
void ggz_ggzcore::launch(uint32_t tableseats, std::string tablename)
{
	log("GGZCORE ## launch table\n");

	if (!ggzcore_ready) {
		log("GGZCORE ## not ready!\n");
		return;
	}

	m_tableid = -1;
	m_servername = tablename;
	m_tableseats = tableseats;

	GGZGame * const game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, 0);

	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCHED, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCH_FAIL, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATED, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATE_FAIL, &ggz_ggzcore::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_PLAYING, &ggz_ggzcore::callback_game);

	ggzcore_game_launch(game);
}

/// callback function for all important server events
/// calls \ref event_server()
GGZHookReturn ggz_ggzcore::callback_server
	(uint32_t const id, void const * const cbdata, void const *)
{
	log("GGZCORE ## callback: %i\n", id);
	ggzcoreobj->event_server(id, cbdata);

	return GGZ_HOOK_OK;
}


/// callback function for all important room events
/// calls \ref event_room()
GGZHookReturn ggz_ggzcore::callback_room
	(uint32_t const id, void const * const cbdata, void const *)
{
	log("GGZCORE/room ## callback: %i\n", id);
	ggzcoreobj->event_room(id, cbdata);

	return GGZ_HOOK_OK;
}


/// callback function for all important game events
/// calls \ref event_game()
GGZHookReturn ggz_ggzcore::callback_game
	(uint32_t const id, void const * const cbdata, void const *)
{
	log("GGZCORE/game ## callback: %i\n", id);
	ggzcoreobj->event_game(id, cbdata);

	return GGZ_HOOK_OK;
}

/// handles all important server events
void ggz_ggzcore::event_server(uint32_t const id, void const * const cbdata)
{
	MOTD & motd = NetGGZ::ref().motd;
	switch (id) {
	case GGZ_CONNECTED:
		log("GGZCORE/event_server ## -- connected\n");
		break;
	case GGZ_NEGOTIATED:
		log("GGZCORE/event_server ## -- negotiated\n");
		ggzcore_server_login(ggzserver);
		break;
	case GGZ_LOGGED_IN:
		log("GGZCORE/event_server ## -- logged in\n");
		m_logged_in = true;
		ggzcore_server_list_gametypes(ggzserver, 0);
#if GGZCORE_VERSION_MINOR == 0
		ggzcore_server_list_rooms(ggzserver, -1, 1);
#else
		ggzcore_server_list_rooms(ggzserver, 1);
#endif
		break;
	case GGZ_ENTERED:
		log("GGZCORE/event_server ## -- entered room\n");

		//  Register callback functions for room events.
		//
		//  Not yet handled server events:
		//   * GGZ_PLAYER_LAG   (useful for us?)
		//   * GGZ_PLAYER_STATS (should be handled once Widelands knows winner,
		//                       loser and point handling)
		//   * GGZ_PLAYER_PERMS (useful as soon as login features and admin
		//                       features are implemented)
		m_room = ggzcore_server_get_cur_room(ggzserver);
#define ADD_EVENT_HOOK(event) \
   ggzcore_room_add_event_hook(m_room, event, ggz_ggzcore::callback_room);
		ADD_EVENT_HOOK(GGZ_TABLE_LIST);
		ADD_EVENT_HOOK(GGZ_TABLE_UPDATE);
		ADD_EVENT_HOOK(GGZ_TABLE_LAUNCHED);
		ADD_EVENT_HOOK(GGZ_TABLE_LAUNCH_FAIL);
		ADD_EVENT_HOOK(GGZ_TABLE_JOINED);
		ADD_EVENT_HOOK(GGZ_TABLE_JOIN_FAIL);
		ADD_EVENT_HOOK(GGZ_TABLE_LEFT);
		ADD_EVENT_HOOK(GGZ_PLAYER_LIST);
		ADD_EVENT_HOOK(GGZ_ROOM_ENTER);
		ADD_EVENT_HOOK(GGZ_ROOM_LEAVE);
		ADD_EVENT_HOOK(GGZ_CHAT_EVENT);

		// Request list of tables and players
#if GGZCORE_VERSION_MINOR == 0
		ggzcore_room_list_tables(m_room, -1, 0);
#else
		ggzcore_room_list_tables(m_room);
#endif
		ggzcore_room_list_players(m_room);

		
		
		// now send some text about the room to the chat menu
		if (!relogin) {
			NetGGZ::ref().formatedGGZChat(_("Connected to the metaserver of:"), "", true);
			{
				std::string msg =
				"              </p>"
				"<p font-size=18 font-face=Widelands/Widelands font-weight=bold>";
				msg += ggzcore_room_get_name(m_room);
				NetGGZ::ref().formatedGGZChat(msg, "", true);
			}
			NetGGZ::ref().formatedGGZChat(ggzcore_room_get_desc(m_room), "", true);
			if (motd.motd.size()) {
				NetGGZ::ref().formatedGGZChat("*** *** ***", "", true);
				NetGGZ::ref().formatedGGZChat(_("Server MOTD:"), "", true);
				for (uint32_t i = 0; i < motd.motd.size(); ++i)
					NetGGZ::ref().formatedGGZChat(motd.formationstr + motd.motd[i], "", true);
				NetGGZ::ref().formatedGGZChat("*** *** ***", "", true);
			}
		} else {
			NetGGZ::ref().formatedGGZChat(_("Reconnected to the metaserver."), "", true);
			NetGGZ::ref().formatedGGZChat("*** *** ***", "", true);
		}
		NetGGZ::ref().formatedGGZChat
			(_
			 	("NOTE: The Internet gaming implementation is in very early "
			 	 "state."),
			 "",
			 true);
		NetGGZ::ref().formatedGGZChat(_("Please take a look at the notes at:"), "", true);
		NetGGZ::ref().formatedGGZChat("http://wl.widelands.org/wiki/InternetGaming", "", true);
		break;
	case GGZ_ROOM_LIST: {
		log("GGZCORE/event_server ## -- (room list)\n");
		int32_t const num = ggzcore_server_get_num_rooms(ggzserver);
		bool joined = false;
		for (int32_t i = 0; i < num; ++i) {
			m_room = ggzcore_server_get_nth_room(ggzserver, i);
			GGZGameType * const type = ggzcore_room_get_gametype(m_room);
			if (type) {
				if (!strcmp(ggzcore_gametype_get_name(type), "Widelands")) {
					ggzcore_server_join_room(ggzserver, m_room);
					joined = true;
					break;
				}
			}
		}
		if (!joined) {
			log("GGZCORE/event_server ## could not find room! :(\n");
		}
		break;
	}
	case GGZ_TYPE_LIST:
		log("GGZCORE/event_server ## -- (type list)\n");
		break;
	case GGZ_CHANNEL_CONNECTED:
		log("GGZCORE/event_server ## -- channel connected\n");
		m_channelfd = ggzcore_server_get_channel(ggzserver);
		break;
	case GGZ_CHANNEL_READY:
		log("GGZCORE/event_server ## -- channel ready\n");
		ggzcore_game_set_server_fd
			(ggzcore_server_get_cur_game(ggzserver), m_channelfd);
		m_channelfd = -1;
		break;
	case GGZ_CHAT_FAIL:
		{
			GGZErrorEventData const * ce =
				static_cast<GGZErrorEventData const *>(cbdata);
			log("GGZCORE/event_server ## -- chat error! (%s)\n", ce->message);
			std::string formated = ERRMSG;
			formated += ce->message;
			NetGGZ::ref().formatedGGZChat(formated, "", true);
			NetGGZ::ref().formatedGGZChat("*** *** ***", "", true);
		}
		break;
	case GGZ_LOGIN_FAIL:
	case GGZ_ENTER_FAIL:
	case GGZ_PROTOCOL_ERROR:
	case GGZ_CHANNEL_FAIL:
	case GGZ_NEGOTIATE_FAIL:
	case GGZ_CONNECT_FAIL:
	case GGZ_NET_ERROR:
		{
			GGZErrorEventData const * eed =
			static_cast<GGZErrorEventData const *>(cbdata);
				log("GGZCORE/event_server ## -- error! (%s)\n", eed->message);
			std::string formated = ERRMSG;
			formated += eed->message;
			NetGGZ::ref().formatedGGZChat(formated, "", true);
			NetGGZ::ref().formatedGGZChat("*** *** ***", "", true);
			ggzcore_login = false;
		}
		break;
	case GGZ_MOTD_LOADED:
		{
			log("GGZCORE/event_server ## -- motd loaded!\n");
			motd = MOTD(static_cast<GGZMotdEventData const * >(cbdata)->motd);
		}
		break;
	case GGZ_STATE_CHANGE:
		// The state of ggzcore changed
		// cbdata is NULL
		log("GGZCORE/event_server ## -- state changed to: %d\n",
			 ggzcore_server_get_state(ggzserver));
		break;
	case GGZ_LOGOUT:
		// cbdata is NULL
		log("GGZCORE/event_server ## -- logged out!\n");
		m_state = ggzcorestate_disconnected;
		m_tablestate = ggzcoretablestate_notinroom;
		m_logged_in = false;
		break;
	default:
		log("GGZCORE/event_server ## WARNING: unknown/unhandled event!\n");
		break;
	}
}


/// handles all important room events
void ggz_ggzcore::event_room(uint32_t const id, void const * const cbdata)
{
	switch (id) {
		//  FIXME If a player is inside a table (launchgame menu / in game) the
		//  FIXME server seems to send malformed update data and so lets the
		//  FIXME clients freeze. At the moment that is the reason why we do not
		//  FIXME read updates while we are inside a table.
	case GGZ_TABLE_UPDATE:
	case GGZ_TABLE_LIST:
		log("GGZCORE/room ## -- table list\n");
		if (!m_room)
			m_room = ggzcore_server_get_cur_room(ggzserver);
		if (!m_room) {
			deinit();
			throw wexception("room was not found!");
		}
		write_tablelist();
		ggzcore_login = false;
		ggzcore_ready = true;
		break;
	case GGZ_TABLE_LAUNCHED:
		log("GGZCORE/room ## -- table launched\n");
		// nothing to be done here - just for debugging
		// cbdata is NULL
		break;
	case GGZ_TABLE_LAUNCH_FAIL: {
		// nothing useful done yet - just debug output
		GGZErrorEventData const & eed =
			*static_cast<GGZErrorEventData const *>(cbdata);
		log("GGZCORE/room ## -- table launch failed! (%s)\n", eed.message);
		break;
	}
	case GGZ_TABLE_JOINED:
		log("GGZCORE/room ## -- table joined\n");
		// nothing to be done here - just for debugging
		// cbdata is the table index (int*) of the table we joined
		break;
	case GGZ_TABLE_JOIN_FAIL: {
		// nothing useful done yet - just debug output
		char const * const msg =  static_cast<char const *>(cbdata);
		log("GGZCORE ## -- error! (%s)\n", msg);
		break;
	}
	case GGZ_ROOM_ENTER: {
		log("GGZCORE/room ## -- user joined\n");
		std::string msg =
			static_cast<GGZRoomChangeEventData const *>(cbdata)->player_name;
		msg += _(" joined the metaserver.");
		NetGGZ::ref().formatedGGZChat(msg, "", true);
		write_userlist();
		break;
	}
	case GGZ_ROOM_LEAVE: {
		log("GGZCORE/room ## -- user left\n");
		std::string msg =
			static_cast<GGZRoomChangeEventData const *>(cbdata)->player_name;
		msg += _(" left the metaserver.");
		NetGGZ::ref().formatedGGZChat(msg, "", true);
		write_userlist();
		break;
	}
	case GGZ_PLAYER_LIST:
	case GGZ_PLAYER_COUNT: // cbdata is the GGZRoom* where players were counted
		log("GGZCORE/room ## -- user list\n");
		write_userlist();
		break;
	case GGZ_CHAT_EVENT:
		log("GGZCORE/room ## -- chat message\n");
		NetGGZ::ref().recievedGGZChat(cbdata);
		break;
	}
}


/// handles all important game events
void ggz_ggzcore::event_game(uint32_t const id, void const * const cbdata)
{
	switch (id) {
	case GGZ_GAME_PLAYING:
		log("GGZCORE/game ## -- playing\n");
		if (!m_room)
			m_room = ggzcore_server_get_cur_room(ggzserver);
		if (m_tableid == -1) {
			GGZTable    * const table    = ggzcore_table_new        ();
			GGZGameType * const gametype = ggzcore_room_get_gametype(m_room);
			log("GGZCORE/game ## ggzcore_table_init(name: %s, seats: %d)\n", m_servername.c_str(), m_tableseats);
			ggzcore_table_init(table, gametype, m_servername.c_str(), m_tableseats);
			for (uint32_t i = 0; i < m_tableseats; ++i)
				ggzcore_table_set_seat(table, i, GGZ_SEAT_OPEN, 0);
			ggzcore_room_launch_table(m_room, table);
			ggzcore_table_free(table);
		} else {
			if (ggzcore_room_join_table(m_room, m_tableid, 0) < 0) {
				// Throw an errorcode - this can be removed once everything works
				// stable. ggzcore_room_join_table() is always returning -1 if an
				//  error occurred, so we need to check ourselves, what the problem
				// actually is.
				uint32_t errorcode = 0;
				if (!ggzcore_server_is_in_room(ggzserver))
					errorcode += 1;
				if (!(ggzcore_server_get_cur_room(ggzserver) == m_room))
					errorcode += 2;
				if (ggzcore_server_get_state(ggzserver) != GGZ_STATE_IN_ROOM) {
					errorcode += 4;
					errorcode += ggzcore_server_get_state(ggzserver) * 10;
				}
				if (!ggzcore_server_get_cur_game(ggzserver))
					errorcode += 100;
				if (!ggzcore_room_get_table_by_id(m_room, m_tableid))
					errorcode += 1000;
				deinit();
				throw wexception
					("unable to join the table - error: %u", errorcode);
			} else
				log ("GGZCORE/game ## -- joined the table\n");
		}
		break;
	case GGZ_GAME_LAUNCHED:
		log("GGZCORE/game ## --  launched\n");
		m_gamefd =
			ggzcore_game_get_control_fd(ggzcore_server_get_cur_game(ggzserver));
		m_tablestate = ggzcoretablestate_launched;
		break;
	case GGZ_GAME_NEGOTIATED:
		log("GGZCORE/game ## -- negotiated\n");
		ggzcore_server_create_channel(ggzserver);
		break;
	case GGZ_GAME_LAUNCH_FAIL:
	case GGZ_GAME_NEGOTIATE_FAIL:
		log
			("GGZCORE/game ## -- error! (%s) :(\n",
			 static_cast<const char *>(cbdata));
		break;
	}
}

/// shuts down the local ggz core, if active
void ggz_ggzcore::deinit()
{
	if (!ggzcoreobj)
		return;
	NetGGZ::ref().formatedGGZChat(_("Closed the connection to the metaserver."), "", true);
	NetGGZ::ref().formatedGGZChat("*** *** ***", "", true);
	relogin = true;
	tablelist.clear();
	userlist.clear();
	if (ggzcore_server_is_at_table(ggzserver))
		ggzcore_room_leave_table(m_room, true);
	ggzcore_server_logout(ggzserver);
	ggzcore_server_disconnect(ggzserver);
	ggzcore_server_free(ggzserver);
	m_logged_in = false;
	ggzserver = 0;
	ggzcore_destroy();
	ggzcore_ready = false;
}

/// writes the list of tables after an table update arrived
void ggz_ggzcore::write_tablelist()
{
	tablelist.clear();
	int32_t const num = ggzcore_room_get_num_tables(m_room);
	for (int32_t i = 0; i < num; ++i) {
		Net_Game_Info info;
		GGZTable * const table = ggzcore_room_get_nth_table(m_room, i);
		if (!table) {
			deinit();
			throw wexception("table can not be found!");
		}
		strncpy
			(info.hostname,
			 ggzcore_table_get_desc(table),
			 sizeof(info.hostname));
		GGZTableState const state = ggzcore_table_get_state(table);
		if (state == GGZ_TABLE_WAITING) {
			// To avoid freezes for users with build15 when trying to connect to
			// a table with seats > 8 - could surely happen once the seats problem
			// is fixed.
			//  FIXME it's even > 7 due to a ggz 0.14.1 bug
			//if (ggzcore_table_get_num_seats(table) > 8)
			if (ggzcore_table_get_num_seats(table) > 7)
				info.state = LAN_GAME_CLOSED;
			else if (ggzcore_table_get_seat_count(table, GGZ_SEAT_OPEN) > 0)
				info.state = LAN_GAME_OPEN;
			else
				info.state = LAN_GAME_CLOSED;
		}
		else if (state == GGZ_TABLE_PLAYING)
			info.state = LAN_GAME_CLOSED;
		else
			continue;
		tablelist.push_back(info);
	}
	tableupdate   = true;

	// If a table was changed at least one user changed to that table as well
	write_userlist();
}


/// writes the list of online users after an user update arrived
void ggz_ggzcore::write_userlist()
{
	userlist.clear();
	if (!ggzserver) {
		log("ggz_ggzcore::write_userlist(): not ggzserver\n");
		return;
	}
	if (!m_room)
		m_room = ggzcore_server_get_cur_room(ggzserver);
	if (!m_room)
	{
		log("ggz_ggzcore::write_userlist(): Not in a room!\n");
		return;
	}

	int32_t const num = ggzcore_room_get_num_players(m_room);
	log("ggz_ggzcore::write_userlist(): number of users: %d\n", num);
	for (int32_t i = 0; i < num; ++i) {
		GGZPlayer * const player = ggzcore_room_get_nth_player(m_room, i);
		if (!player) {
			deinit();
			throw wexception("player can not be found!");
		}
		Net_Player user;
		user.name = ggzcore_player_get_name(player);
		GGZTable * tab = ggzcore_player_get_table(player);
		user.table = tab ? ggzcore_table_get_desc(tab) : "--";

		int buf, wins, losses, ties, forfeits;
		if
			(ggzcore_player_get_record
			 (player, &wins, &losses, &ties, &forfeits)
			 and
			 ggzcore_player_get_rating(player, &buf))
		{
			snprintf
				(user.stats, sizeof(user.stats), "%i, %i, %i",
				 buf - 1500, wins, losses);
			log
				("GGZ: Stats (\"%s\"): score: %i, wins: %i, losses: %i - \"%s\"\n",
				 user.name.c_str(), buf, wins, losses, user.stats);
		} else if (ggzcore_player_get_rating(player, &buf)) {
			snprintf(user.stats, sizeof(user.stats), "%i", buf - 1500);
			//log(user.stats);
		} else
			snprintf(user.stats, sizeof(user.stats), "-");
		user.type = ggzcore_player_get_type(player);
		userlist.push_back(user);
	}
	log("ggz_ggzcore::write_userlist(): userlist.size(): %d\n", userlist.size());
	userupdate = true;
}

void ggz_ggzcore::send_message(const char* to, const char* msg)
{
	int sent;
	assert(m_room);
	log("GGZCORE/room ## send chat message: \"%s\"\n", msg);
	if (to)
		sent = ggzcore_room_chat(m_room, GGZ_CHAT_PERSONAL, to, msg);
	else
		sent = ggzcore_room_chat(m_room, GGZ_CHAT_NORMAL, "", msg);
	if (sent < 0)
		log("GGZCORE/room/chat ## error sending chat message!\n");
}

bool ggz_ggzcore::is_in_room()
{
	return m_logged_in && ggzserver && ggzcore_server_is_in_room(ggzserver);
}

bool ggz_ggzcore::data_pending()
{
	/*
	if (m_channelfd != -1 or m_gamefd != -1)
	{
		fd_set read_fd_set;
		int result;
		struct timeval tv;
		FD_ZERO(&read_fd_set);
		FD_SET(m_server_fd, &read_fd_set);
		tv.tv_sec = tv.tv_usec = 0;
		if (
			 select
				 (((m_channelfd >  m_gamefd)?m_channelfd:m_gamefd) + 1,
				  &read_fd_set, NULL, NULL, &tv)
				  > 0)
			return true;
	}
	*/
	return ggzserver && ggzcore_server_data_is_pending(ggzserver);
}

