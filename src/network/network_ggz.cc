/*
 * Copyright (C) 2004-2009 by the Widelands Development Team
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

#include "network_ggz.h"

#if HAVE_GGZ

#include "log.h"
#include "i18n.h"
#include "warning.h"
#include "wexception.h"
#include "wlapplication.h"

#include <cstring>

static NetGGZ    * ggzobj    = 0;
static GGZMod    * mod       = 0;
static GGZServer * ggzserver = 0;


/// Constructor of NetGGZ.
/// It is private to avoid the creation of more than one ggz object.
NetGGZ::NetGGZ() :
	use_ggz       (false),
	m_fd          (-1),
	channelfd     (-1),
	gamefd        (-1),
	server_ip_addr(0),
	logged_in     (false),
	relogin       (false),
	tableseats    (1),
	userupdate    (false),
	tableupdate   (false),
	motd          ()
{}


/// \returns the _one_ ggzobject. There should be only this one object and it
/// _must_ be used in all cases.
NetGGZ & NetGGZ::ref() {
	if (!ggzobj)
		ggzobj = new NetGGZ();
	return *ggzobj;
}


/// sets the locale ggz core to "initialized"
void NetGGZ::init()
{
	use_ggz = true;
	log(">> GGZ: initialized\n");
}


/// \returns true, if ggz is used
bool NetGGZ::used()
{
	return use_ggz;
}


/// connects to the metaserver
///
/// \note The FD_SET macro from glibc uses old-style cast. We can not fix this
/// ourselves, so we temporarily turn the error into a warning. It is turned
/// back into an error after this function.
#pragma GCC diagnostic warning "-Wold-style-cast"
bool NetGGZ::connect()
{
	if (!used())
		return false;

	log("GGZ ## connect\n");
	mod = ggzmod_new(GGZMOD_GAME);

	// Set handler for ggzmod events:
	ggzmod_set_handler(mod, GGZMOD_EVENT_SERVER, &NetGGZ::ggzmod_server);
	ggzmod_set_handler(mod, GGZMOD_EVENT_ERROR, &NetGGZ::ggzmod_server);
	// not handled / not used events of the GGZMOD Server:
	// * GGZMOD_EVENT_STATE
	// * GGZMOD_EVENT_PLAYER
	// * GGZMOD_EVENT_SEAT
	// * GGZMOD_EVENT_SPECTATOR_SEAT
	// * GGZMOD_EVENT_CHAT
	// * GGZMOD_EVENT_STATS
	// * GGZMOD_EVENT_INFO
	// * GGZMOD_EVENT_RANKINGS

	if (ggzmod_connect(mod)) {
		log("GGZ ## connection failed\n");
		return false;
	}

	int32_t const fd = ggzmod_get_fd(mod);
	log("GGZ ## connection fd %i\n", fd);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 999 * 1000;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	while (ggzmod_get_state(mod) != GGZMOD_STATE_PLAYING) {
		select(fd + 1, &fdset, 0, 0, &timeout);
		ggzmod_dispatch(mod);
		//log("GGZ ## timeout!\n");
		if (usedcore())
			datacore();
	}

	return true;
}
#pragma GCC diagnostic error "-Wold-style-cast"


/// handles the events of the ggzmod server
void NetGGZ::ggzmod_server
	(GGZMod * const cbmod, GGZModEvent const e, void const * const cbdata)
{
	log("GGZ ## ggzmod_server\n");
	if (e == GGZMOD_EVENT_SERVER) {
		int32_t const fd = *static_cast<int32_t const *>(cbdata);
		ggzobj->m_fd = fd;
		log("GGZ ## got fd: %i\n", fd);
		ggzmod_set_state(cbmod, GGZMOD_STATE_PLAYING);
	} else if (e == GGZMOD_EVENT_ERROR) {
		const char * msg = static_cast<const char * >(cbdata);
		log("GGZ ## ERROR: %s\n", msg);
	} else
		log("GGZ ## HANDLE ERROR: %i\n", e);
}


// not used?
bool NetGGZ::host()
{
	int32_t spectator, seat;

	if (!used())
		return false;

	do {
		ggzmod_dispatch(mod);
		if (usedcore())
			datacore();
		ggzmod_get_player(mod, &spectator, &seat);
	} while (seat == -1);

	log("GGZ ## host? seat=%i\n", seat);
	return !seat;
}


/// \returns the ip of the server, if connected
char const * NetGGZ::ip()
{
	return server_ip_addr;
}


/// initializes the local ggz core
bool NetGGZ::initcore
	(char const * const metaserver, char const * const nick,
	 char const * const pwd, char const * const email,
	 bool newreg, bool anonymous)
{
	GGZOptions opt;

	if (usedcore())
		return false;

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
		(ggzserver, GGZ_CONNECTED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_NEGOTIATED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_LOGGED_IN, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_ENTERED, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CONNECT_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_NEGOTIATE_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_LOGIN_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_ENTER_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHANNEL_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHAT_FAIL, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_ROOM_LIST, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_TYPE_LIST, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_NET_ERROR, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_PROTOCOL_ERROR, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHANNEL_CONNECTED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook
		(ggzserver, GGZ_CHANNEL_READY, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook
		(ggzserver, GGZ_MOTD_LOADED, &NetGGZ::callback_server);
#if GGZCORE_VERSION_MINOR == 0
	ggzcore_server_set_hostinfo(ggzserver, metaserver, WL_METASERVER_PORT, 0);
#else
	ggzcore_server_set_hostinfo
		(ggzserver, metaserver, WL_METASERVER_PORT, GGZ_CONNECTION_CLEAR);
#endif

	// Login anonymously:
	if (anonymous)
		ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN_GUEST, nick, 0, 0);

	// Register a new account:
	else if (newreg)
		ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN_NEW, nick, pwd, email);

	// Normal login:
	else
		ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN, nick, pwd, 0);

	ggzcore_server_connect(ggzserver);

	log("GGZCORE ## start loop\n");
	while (ggzcore_login)
		datacore();
	log("GGZCORE ## end loop\n");

	username = nick;
	return logged_in;
}


/// shuts down the local ggz core, if active
void NetGGZ::deinitcore()
{
	if (!usedcore())
		return;
	formatedGGZChat(_("Closed the connection to the metaserver."), "", true);
	formatedGGZChat("*** *** ***", "", true);
	relogin = true;
	tablelist.clear();
	userlist.clear();
	if (ggzcore_server_is_at_table(ggzserver))
		ggzcore_room_leave_table(room, true);
	ggzcore_server_logout(ggzserver);
	ggzcore_server_disconnect(ggzserver);
	ggzcore_server_free(ggzserver);
	logged_in = false;
	ggzserver = 0;
	ggzcore_destroy();
	ggzcore_ready = false;
}


/// \returns true, if the local ggzserver core is initialized
bool NetGGZ::usedcore()
{
	return ggzserver;
}



/// checks if the widelands game server module sent new data or whether it
/// requests any data - this is only useful, when the player is in a table
///
/// \note The FD_SET macro from glibc uses old-style cast. We can not fix this
/// ourselves, so we temporarily turn the error into a warning. It is turned
/// back into an error after this function.
#pragma GCC diagnostic warning "-Wold-style-cast"
void NetGGZ::data()
{
	if (!used())
		return;

	int32_t op;
	char * ipstring;
	char * greeter;
	int32_t greeterversion;
	char ipaddress[17];
	int32_t fd = m_fd;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	{
		int32_t const ret = select(fd + 1, &fdset, 0, 0, &timeout);
		if (ret <= 0)
			return;
		log("GGZ ## select() returns: %i for fd %i\n", ret, fd);
	}

	{
		int32_t const ret = ggz_read_int(fd, &op);
		log("GGZ ## received opcode: %i (%i)\n", op, ret);
		if (ret < 0) {
			close(fd);
			ggzmod_disconnect(mod);
			ggzmod_free(mod);
			use_ggz = false;
			return;
		}
	}

	switch (op) {
	case op_greeting:
		ggz_read_string_alloc(fd, &greeter);
		ggz_read_int(fd, &greeterversion);
		log("GGZ ## server is: '%s' '%i'\n", greeter, greeterversion);
		ggz_free(greeter);
		break;
	case op_request_ip:
		log("GGZ ## ip request!\n");
		snprintf(ipaddress, sizeof(ipaddress), "%i.%i.%i.%i", 255, 255, 255, 255);
		ggz_write_int(fd, op_reply_ip);
		ggz_write_string(fd, ipaddress);
		break;
	case op_broadcast_ip:
		ggz_read_string_alloc(fd, &ipstring);
		log("GGZ ## ip broadcast: '%s'\n", ipstring);
		server_ip_addr = ggz_strdup(ipstring);
		ggz_free(ipstring);
		break;
	case op_unreachable:
		deinitcore();
		throw warning
			(_("Connection problem"), "%s",
			 _
			 	("Your Server was not reachable from the Internet.\n"
			 	 "Please try to solve the problem - Reading the notes\n"
			 	 "at http://wl.widelands.org/wiki/InternetGaming can\n"
			 	 "be advantageous."));
	default: log("GGZ ## opcode unknown!\n");
	}
}
#pragma GCC diagnostic error "-Wold-style-cast"


//\FIXME: mallformed updatedata, if the user is in a room and too many seats
//        are open (~ > 4).
/// checks for events on server, room and game
/// if data is pending the ggzcore_*_read_data function will call the fitting
/// callback function
void NetGGZ::datacore()
{
	if (!ggzserver)
		return;
	if (ggzcore_server_data_is_pending(ggzserver))
		ggzcore_server_read_data(ggzserver, ggzcore_server_get_fd(ggzserver));

	if (channelfd != -1)
		ggzcore_server_read_data
			(ggzserver, ggzcore_server_get_channel(ggzserver));

	if (gamefd != -1)
		ggzcore_game_read_data(ggzcore_server_get_cur_game(ggzserver));
}


/// callback function for all important server events
/// calls \ref event_server()
GGZHookReturn NetGGZ::callback_server
	(uint32_t const id, void const * const cbdata, void const *)
{
	log("GGZCORE ## callback: %i\n", id);
	ggzobj->event_server(id, cbdata);

	return GGZ_HOOK_OK;
}


/// callback function for all important room events
/// calls \ref event_room()
GGZHookReturn NetGGZ::callback_room
	(uint32_t const id, void const * const cbdata, void const *)
{
	log("GGZCORE/room ## callback: %i\n", id);
	ggzobj->event_room(id, cbdata);

	return GGZ_HOOK_OK;
}


/// callback function for all important game events
/// calls \ref event_game()
GGZHookReturn NetGGZ::callback_game
	(uint32_t const id, void const * const cbdata, void const *)
{
	log("GGZCORE/game ## callback: %i\n", id);
	ggzobj->event_game(id, cbdata);

	return GGZ_HOOK_OK;
}


/// handles all important server events
void NetGGZ::event_server(uint32_t const id, void const * const cbdata)
{
	switch (id) {
	case GGZ_CONNECTED:
		log("GGZCORE ## -- connected\n");
		break;
	case GGZ_NEGOTIATED:
		log("GGZCORE ## -- negotiated\n");
		ggzcore_server_login(ggzserver);
		break;
	case GGZ_LOGGED_IN:
		log("GGZCORE ## -- logged in\n");
		logged_in = true;
		ggzcore_server_list_gametypes(ggzserver, 0);
#if GGZCORE_VERSION_MINOR == 0
		ggzcore_server_list_rooms(ggzserver, -1, 1);
#else
		ggzcore_server_list_rooms(ggzserver, 1);
#endif
		break;
	case GGZ_ENTERED:
		log("GGZCORE ## -- entered room\n");

		//  Register callback functions for room events.
		//
		//  Not yet handled server events:
		//   * GGZ_PLAYER_LAG   (useful for us?)
		//   * GGZ_PLAYER_STATS (should be handled once Widelands knows winner,
		//                       loser and point handling)
		//   * GGZ_PLAYER_PERMS (useful as soon as login features and admin
		//                       features are implemented)
		room = ggzcore_server_get_cur_room(ggzserver);
#define ADD_EVENT_HOOK(event) \
   ggzcore_room_add_event_hook(room, event, NetGGZ::callback_room);
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
		ggzcore_room_list_tables(room, -1, 0);
#else
		ggzcore_room_list_tables(room);
#endif
		ggzcore_room_list_players(room);

		// now send some text about the room to the chat menu
		if (!relogin) {
			formatedGGZChat(_("Connected to the metaserver of:"), "", true);
			{
				std::string msg =
				"              </p>"
				"<p font-size=18 font-face=Widelands/Widelands font-weight=bold>";
				msg += ggzcore_room_get_name(room);
				formatedGGZChat(msg, "", true);
			}
			formatedGGZChat(ggzcore_room_get_desc(room), "", true);
			if (motd.motd.size()) {
				formatedGGZChat("*** *** ***", "", true);
				formatedGGZChat(_("Server MOTD:"), "", true);
				for (uint32_t i = 0; i < motd.motd.size(); ++i)
					formatedGGZChat(motd.formationstr + motd.motd[i], "", true);
				formatedGGZChat("*** *** ***", "", true);
			}
		} else {
			formatedGGZChat(_("Reconnected to the metaserver."), "", true);
			formatedGGZChat("*** *** ***", "", true);
		}
		formatedGGZChat
			(_
			 	("NOTE: The Internet gaming implementation is in very early "
			 	 "state."),
			 "",
			 true);
		formatedGGZChat(_("Please take a look at the notes at:"), "", true);
		formatedGGZChat("http://wl.widelands.org/wiki/InternetGaming", "", true);
		break;
	case GGZ_ROOM_LIST: {
		log("GGZCORE ## -- (room list)\n");
		int32_t const num = ggzcore_server_get_num_rooms(ggzserver);
		bool joined = false;
		for (int32_t i = 0; i < num; ++i) {
			room = ggzcore_server_get_nth_room(ggzserver, i);
			GGZGameType * const type = ggzcore_room_get_gametype(room);
			if (type) {
				if (!strcmp(ggzcore_gametype_get_name(type), "Widelands")) {
					ggzcore_server_join_room(ggzserver, room);
					joined = true;
					break;
				}
			}
		}
		if (!joined) {
			log("GGZCORE ## could not find room! :(\n");
		}
		break;
	}
	case GGZ_TYPE_LIST:
		log("GGZCORE ## -- (type list)\n");
		break;
	case GGZ_CHANNEL_CONNECTED:
		log("GGZCORE ## -- channel connected\n");
		channelfd = ggzcore_server_get_channel(ggzserver);
		break;
	case GGZ_CHANNEL_READY:
		log("GGZCORE ## -- channel ready\n");
		ggzcore_game_set_server_fd
			(ggzcore_server_get_cur_game(ggzserver), channelfd);
		channelfd = -1;
		init();
		break;
	case GGZ_CHAT_FAIL:
		{
			GGZErrorEventData const * ce =
				static_cast<GGZErrorEventData const *>(cbdata);
			log("GGZCORE ## -- chat error! (%s)\n", ce->message);
			std::string formated = ERRMSG;
			formated += ce->message;
			formatedGGZChat(formated, "", true);
			formatedGGZChat("*** *** ***", "", true);
		}
		break;
	case GGZ_LOGIN_FAIL:
	case GGZ_ENTER_FAIL:
#if GGZCORE_VERSION_MINOR > 0
	case GGZ_PROTOCOL_ERROR:
#endif
		{
			GGZErrorEventData const * eed =
				static_cast<GGZErrorEventData const *>(cbdata);
			log("GGZCORE ## -- error! (%s)\n", eed->message);
			std::string formated = ERRMSG;
			formated += eed->message;
			formatedGGZChat(formated, "", true);
			formatedGGZChat("*** *** ***", "", true);
			ggzcore_login = false;
		}
		break;
#if GGZCORE_VERSION_MINOR == 0
	case GGZ_PROTOCOL_ERROR:
#endif
	case GGZ_CHANNEL_FAIL:
	case GGZ_NEGOTIATE_FAIL:
	case GGZ_CONNECT_FAIL:
	case GGZ_NET_ERROR:
		{
			char const * msg =  static_cast<char const *>(cbdata);
			log("GGZCORE ## -- error! (%s)\n", msg);
			std::string formated = ERRMSG;
			formated += msg;
			formatedGGZChat(formated, "", true);
			formatedGGZChat("*** *** ***", "", true);
			ggzcore_login = false;
		}
		break;
	case GGZ_MOTD_LOADED:
		{
			log("GGZCORE ## -- motd loaded!\n");
			motd = MOTD(static_cast<GGZMotdEventData const * >(cbdata)->motd);
		}
		break;
	}
}


/// handles all important room events
void NetGGZ::event_room(uint32_t const id, void const * const cbdata)
{
	switch (id) {
		//  FIXME If a player is inside a table (launchgame menu / in game) the
		//  FIXME server seems to send malformed update data and so lets the
		//  FIXME clients freeze. At the moment that is the reason why we do not
		//  FIXME read updates while we are inside a table.
	case GGZ_TABLE_UPDATE:
	case GGZ_TABLE_LIST:
		log("GGZCORE/room ## -- table list\n");
		if (!room)
			room = ggzcore_server_get_cur_room(ggzserver);
		if (!room) {
			deinitcore();
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
		formatedGGZChat(msg, "", true);
		write_userlist();
		break;
	}
	case GGZ_ROOM_LEAVE: {
		log("GGZCORE/room ## -- user left\n");
		std::string msg =
			static_cast<GGZRoomChangeEventData const *>(cbdata)->player_name;
		msg += _(" left the metaserver.");
		formatedGGZChat(msg, "", true);
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
		recievedGGZChat(cbdata);
		break;
	}
}


/// handles all important game events
void NetGGZ::event_game(uint32_t const id, void const * const cbdata)
{
	switch (id) {
	case GGZ_GAME_PLAYING:
		log("GGZCORE/game ## -- playing\n");
		if (!room)
			room = ggzcore_server_get_cur_room(ggzserver);
		if (tableid == -1) {
			GGZTable    * const table    = ggzcore_table_new        ();
			GGZGameType * const gametype = ggzcore_room_get_gametype(room);
			ggzcore_table_init(table, gametype, servername.c_str(), tableseats);
			for (uint32_t i = 0; i < tableseats; ++i)
				ggzcore_table_set_seat(table, i, GGZ_SEAT_OPEN, 0);
			ggzcore_room_launch_table(room, table);
			ggzcore_table_free(table);
		} else {
			if (ggzcore_room_join_table(room, tableid, 0) < 0) {
				// Throw an errorcode - this can be removed once everything works
				// stable. ggzcore_room_join_table() is always returning -1 if an
				//  error occurred, so we need to check ourselves, what the problem
				// actually is.
				uint32_t errorcode = 0;
				if (!ggzcore_server_is_in_room(ggzserver))
					errorcode += 1;
				if (!(ggzcore_server_get_cur_room(ggzserver) == room))
					errorcode += 2;
				if (ggzcore_server_get_state(ggzserver) != GGZ_STATE_IN_ROOM) {
					errorcode += 4;
					errorcode += ggzcore_server_get_state(ggzserver) * 10;
				}
				if (!ggzcore_server_get_cur_game(ggzserver))
					errorcode += 100;
				if (!ggzcore_room_get_table_by_id(room, tableid))
					errorcode += 1000;
				deinitcore();
				throw wexception
					("unable to join the table - error: %u", errorcode);
			} else
				log ("GGZCORE/game ## -- joined the table\n");
		}
		break;
	case GGZ_GAME_LAUNCHED:
		log("GGZCORE/game ## --  launched\n");
		gamefd =
			ggzcore_game_get_control_fd(ggzcore_server_get_cur_game(ggzserver));
		init();
		connect();
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


/// \returns the tables in the room
std::vector<Net_Game_Info> const & NetGGZ::tables()
{
	return tablelist;
}

/// \returns the players in the room
std::vector<Net_Player>   const & NetGGZ::users()
{
	return userlist;
}


/// writes the list of tables after an table update arrived
void NetGGZ::write_tablelist()
{
	tablelist.clear();
	int32_t const num = ggzcore_room_get_num_tables(room);
	for (int32_t i = 0; i < num; ++i) {
		Net_Game_Info info;
		GGZTable * const table = ggzcore_room_get_nth_table(room, i);
		if (!table) {
			deinitcore();
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
			if (ggzcore_table_get_num_seats(table) > 8)
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
void NetGGZ::write_userlist()
{
	userlist.clear();
	if (!ggzserver)
		return;
	if (!room)
		room = ggzcore_server_get_cur_room(ggzserver);
	if (!room)
		return;

	int32_t const num = ggzcore_room_get_num_players(room);
	for (int32_t i = 0; i < num; ++i) {
		GGZPlayer * const player = ggzcore_room_get_nth_player(room, i);
		if (!player) {
			deinitcore();
			throw wexception("player can not be found!");
		}
		Net_Player user;
		user.name = ggzcore_player_get_name(player);
		GGZTable * tab = ggzcore_player_get_table(player);
		user.table = tab ? ggzcore_table_get_desc(tab) : "--";
		int buf[1];
		// TODO unfinished work down here!
		// TODO something in ggzd does not work as it should!
		/* int wins[1];
		int losses[1];
		int ties[1];
		int forfeits[1];*/
		if (ggzcore_player_get_rating(player, buf)) {
			snprintf(user.stats, sizeof(user.stats), "r %i", buf[1]);
			log(user.stats);
		/*} else if (ggzcore_player_get_highscore(player, buf)) {
			snprintf(user.stats, sizeof(user.stats), "hs %i", buf[1]);
			log(user.stats);
		} else if (ggzcore_player_get_ranking(player, buf)) {
			snprintf(user.stats, sizeof(user.stats), "ra %i", buf[1]);
			log(user.stats);
		} else if
			(ggzcore_player_get_record
			 (player, wins, losses, ties, forfeits))
		{
			snprintf
				(user.stats, sizeof(user.stats), "%i %i %i %i",
				 wins[1], losses[1], ties[1], forfeits[1]);
			log(user.stats);*/
		} else
			snprintf(user.stats, sizeof(user.stats), "%i", 0);
		user.type = ggzcore_player_get_type(player);
		userlist.push_back(user);
	}
	userupdate = true;
}


/// Called by the client, to join an existing table (game) and to add all
/// hooks to get informed about all important events
void NetGGZ::join(char const * const tablename)
{
	if (!ggzcore_ready)
		return;

	if (!room)
		room = ggzcore_server_get_cur_room(ggzserver);
	ggzcore_room_get_gametype(room);

	tableid = -1;
	int32_t const num = ggzcore_room_get_num_tables(room);
	for (int32_t i = 0; i < num; ++i) {
		GGZTable * const table = ggzcore_room_get_nth_table(room, i);
		if (!table) {
			deinitcore();
			throw wexception("table can not be found!");
		}
		char const * desc = ggzcore_table_get_desc(table);
		if (!desc)
			desc = "Unnamed server";
		if (!strcmp(desc, tablename))
			tableid = ggzcore_table_get_id(table);
	}

	if (tableid == -1) {
		deinitcore();
		throw wexception
			("Selected table \"%s\" could not be found\n", tablename);
	}

	log("GGZCORE ## Joining Server \"%s\"\n", tablename);

	GGZGame * const game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, 0);

	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCHED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCH_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATE_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_PLAYING, &NetGGZ::callback_game);

	ggzcore_game_launch(game);
}


/// Called by the host, to launch a new table (game) and to add all
/// hooks to get informed about all important events
void NetGGZ::launch()
{
	if (!ggzcore_ready)
		return;

	log("GGZCORE ## launch table\n");

	tableid = -1;

	GGZGame * const game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, 0);

	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCHED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCH_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATE_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_PLAYING, &NetGGZ::callback_game);

	ggzcore_game_launch(game);
}


/// \returns the maximum number of seats in a widelands table (game)
uint32_t NetGGZ::max_players()
{
	if (!ggzserver)
		return 1;
	if (!ggzcore_server_is_in_room(ggzserver))
		return 1;
	GGZGameType * const gametype = ggzcore_room_get_gametype(room);
	//  FIXME problem in ggz - for some reasons only 8 seats are currently
	//  FIXME available. I already posted this problem to the ggz
	//  FIXME mailinglist. -- nasenbaer
	//return gametype ? ggzcore_gametype_get_max_players(gametype) : 1;
	return gametype ? 8 : 1;
}


/// Tells the metaserver that the game started
void NetGGZ::send_game_playing()
{
	if (used()) {
		if (ggz_write_int(m_fd, op_state_playing) < 0)
			log("ERROR: Game state could not be send!\n");
	} else
		log("ERROR: GGZ not used!\n");
}


/// Tells the metaserver that the game is done
void NetGGZ::send_game_done()
{
	if (used()) {
		if (ggz_write_int(m_fd, op_state_done) < 0)
			log("ERROR: Game state could not be send!\n");
	} else
		log("ERROR: GGZ not used!\n");
}


/// Sends a chat message via ggz room chat
void NetGGZ::send(std::string const & msg)
{
	if (!usedcore())
		return;
	int16_t sent;
	if (msg.size() && *msg.begin() == '@') {
		// Format a personal message
		std::string::size_type const space = msg.find(' ');
			if (space >= msg.size() - 1)
			return;
		std::string const to = msg.substr(1, space - 1);
		std::string const pm = msg.substr(space + 1);
		sent = ggzcore_room_chat(room, GGZ_CHAT_PERSONAL, to.c_str(), pm.c_str());
		// Add the pm to own message list
		formatedGGZChat(pm, username, false, to);
	} else
		sent = ggzcore_room_chat(room, GGZ_CHAT_NORMAL, "", msg.c_str());
	if (sent < 0)
		log("GGZCORE/room/chat ## error sending message!\n");
}


/// Called when a chatmessage was received.
void NetGGZ::recievedGGZChat(void const * const cbdata)
{
	GGZChatEventData const * const msg =
		static_cast<GGZChatEventData const *>(cbdata);
	formatedGGZChat
		(msg->message,
		 msg->sender,
		 msg->type == GGZ_CHAT_ANNOUNCE,
		 msg->type == GGZ_CHAT_PERSONAL ? username : std::string());
}


/// Send a formated message to the chat menu
void NetGGZ::formatedGGZChat
	(std::string const & msg, std::string const & sender,
	 bool system, std::string recipient)
{
	ChatMessage c;
	c.time = time(0);
	c.sender = !system && sender.empty() ? "<unknown>" : sender;
	c.playern = system ? -1 : recipient.size() ? 3 : 7;
	c.msg = msg;
	c.recipient = recipient;

	receive(c);
}

#endif
