/*
 * Copyright (C) 2004-2008 by the Widelands Development Team
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
#include "wexception.h"
#include "wlapplication.h"

#include <cstring>

static NetGGZ *ggzobj = 0;
static GGZMod *mod = 0;
static GGZServer *ggzserver = 0;

NetGGZ::NetGGZ()
{
	use_ggz = false;
	m_fd = -1;
	channelfd = -1;
	gamefd = -1;
	server_ip_addr = NULL;
	userupdate = false;
	tableupdate = false;
}

NetGGZ & NetGGZ::ref() {
	if (!ggzobj)
		ggzobj = new NetGGZ();
	return *ggzobj;
}

void NetGGZ::init()
{
	use_ggz = true;
	log(">> GGZ: initialized\n");
}

bool NetGGZ::used()
{
	return use_ggz;
}

bool NetGGZ::connect()
{
	int32_t ret;

	if (!used()) return false;

	log("GGZ ## connect\n");
	mod = ggzmod_new(GGZMOD_GAME);
	ggzmod_set_handler(mod, GGZMOD_EVENT_SERVER, &NetGGZ::ggzmod_server);
	ret = ggzmod_connect(mod);
	if (ret)
	{
		log("GGZ ## connection failed\n");
		return false;
	}

	int32_t fd = ggzmod_get_fd(mod);
	log("GGZ ## connection fd %i\n", fd);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 999 * 1000;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	while (ggzmod_get_state(mod) != GGZMOD_STATE_PLAYING)
	{
		select(fd + 1, &fdset, NULL, NULL, &timeout);
		ggzmod_dispatch(mod);
		//log("GGZ ## timeout!\n");
		if (usedcore())
			datacore();
	}

	return true;
}


void NetGGZ::ggzmod_server(GGZMod *cbmod, GGZModEvent e, const void *cbdata)
{
	log("GGZ ## ggzmod_server\n");
	if (e == GGZMOD_EVENT_SERVER)
	{
		int32_t fd = *static_cast<const int32_t *>(cbdata);
		ggzobj->m_fd = fd;
		log("GGZ ## got fd: %i\n", fd);
		ggzmod_set_state(cbmod, GGZMOD_STATE_PLAYING);
	}
}


/// checks if new data arrived via network and handles the data
void NetGGZ::data()
{
	datacore();

	if (!used())
		return;

	int32_t op;
	char *ipstring;
	char *greeter;
	int32_t greeterversion;
	char ipaddress[17];
	int32_t fd = m_fd;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	int32_t ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
	if (ret <= 0) return;
	log("GGZ ## select() returns: %i for fd %i\n", ret, fd);

	ret = ggz_read_int(fd, &op);
	log("GGZ ## received opcode: %i (%i)\n", op, ret);
	if (ret < 0)
	{
		close(fd);
		ggzmod_disconnect(mod);
		ggzmod_free(mod);
		use_ggz = false;
		return;
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
	default: log("GGZ ## opcode unknown!\n");
	}
}

bool NetGGZ::host()
{
	int32_t spectator, seat;

	if (!used()) return false;

	do
	{
		ggzmod_dispatch(mod);
		if (usedcore())
			datacore();
		ggzmod_get_player(mod, &spectator, &seat);
	}
	while (seat == -1);

	log("GGZ ## host? seat=%i\n", seat);
	if (!seat) return true;
	return false;
}


/// returns the ip of the server, if connected
const char *NetGGZ::ip()
{
	return server_ip_addr;
}


/// initializes the local ggz core
void NetGGZ::initcore(const char *metaserver, const char *playername) {
	GGZOptions opt;
	int32_t ret;

	if (usedcore())
		return;

	log("GGZCORE ## initialization\n");
	ggzcore_login = true;
	ggzcore_ready = false;

	opt.flags = static_cast<GGZOptionFlags>(GGZ_OPT_EMBEDDED);
	ret = ggzcore_init(opt);

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

	ggzcore_server_set_hostinfo(ggzserver, metaserver, WL_METASERVER_PORT, 0);

	ggzcore_server_set_logininfo
		(ggzserver, GGZ_LOGIN_GUEST, playername, NULL, NULL);

	ggzcore_server_connect(ggzserver);

	log("GGZCORE ## start loop\n");
	while (ggzcore_login)
		datacore();
	log("GGZCORE ## end loop\n");
}


/// shuts down the local ggz core, if active
void NetGGZ::deinitcore()
{
	if (!usedcore())
		return;
	formatedGGZChat(_("Dropping connection to the metaserver."), "", true);
	formatedGGZChat("", "", true);
	if (ggzcore_server_is_at_table(ggzserver))
		ggzcore_room_leave_table(room, true);
	ggzcore_server_logout(ggzserver);
	ggzcore_server_disconnect(ggzserver);
	ggzcore_server_free(ggzserver);
	ggzserver = 0;
	ggzcore_destroy();
	ggzcore_ready = false;
}


/// \returns true, if the local ggzserver core is initialized
bool NetGGZ::usedcore()
{
	return (ggzserver != 0);
}

void NetGGZ::datacore()
{
	GGZGame *game;

	if (!ggzserver) return;
	if (ggzcore_server_data_is_pending(ggzserver))
		ggzcore_server_read_data(ggzserver, ggzcore_server_get_fd(ggzserver));

	if (channelfd != -1)
		ggzcore_server_read_data
			(ggzserver, ggzcore_server_get_channel(ggzserver));

	if (gamefd != -1)
	{
		game = ggzcore_server_get_cur_game(ggzserver);
		ggzcore_game_read_data(game);
	}
}


GGZHookReturn NetGGZ::callback_server
	(uint32_t id, const void *cbdata, const void *) //void * user)
{
	log("GGZCORE ## callback: %i\n", id);
	ggzobj->event_server(id, cbdata);

	return GGZ_HOOK_OK;
}


GGZHookReturn NetGGZ::callback_room
	(uint32_t id, const void *cbdata, const void *) //void * user)
{
	log("GGZCORE/room ## callback: %i\n", id);
	ggzobj->event_room(id, cbdata);

	return GGZ_HOOK_OK;
}


GGZHookReturn NetGGZ::callback_game
	(uint32_t id, const void *cbdata, const void *) //void * user)
{
	log("GGZCORE/game ## callback: %i\n", id);
	ggzobj->event_game(id, cbdata);

	return GGZ_HOOK_OK;
}


void NetGGZ::event_server(uint32_t id, const void *cbdata) {
	GGZGameType *type;
	GGZGame *game;
	int32_t num, i;
	int32_t joined;
	std::string msg;

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
		ggzcore_server_list_gametypes(ggzserver, 0);
		ggzcore_server_list_rooms(ggzserver, -1, 1);
		break;
	case GGZ_ENTERED:
		log("GGZCORE ## -- entered\n");
		// Add hooks for all interesting data for the room
		room = ggzcore_server_get_cur_room(ggzserver);
		ggzcore_room_add_event_hook(room, GGZ_TABLE_LIST, &NetGGZ::callback_room);
		ggzcore_room_add_event_hook
			(room, GGZ_TABLE_UPDATE, &NetGGZ::callback_room);
		ggzcore_room_add_event_hook
			(room, GGZ_PLAYER_LIST, &NetGGZ::callback_room);
		ggzcore_room_add_event_hook
			(room, GGZ_PLAYER_COUNT, &NetGGZ::callback_room);
		ggzcore_room_add_event_hook(room, GGZ_ROOM_ENTER, &NetGGZ::callback_room);
		ggzcore_room_add_event_hook(room, GGZ_ROOM_LEAVE, &NetGGZ::callback_room);
		ggzcore_room_add_event_hook(room, GGZ_CHAT_EVENT, &NetGGZ::callback_room);

		// Request list of tables and players
		ggzcore_room_list_tables(room, -1, 0);
		ggzcore_room_list_players(room);

		// now send some text about the room to the chat menu
		msg = _("Connected to the metaserver of:");
		formatedGGZChat(msg, "", true);
		formatedGGZChat("", "", true);
		msg = "                 </p>";
		msg += "<p font-size=14 font-face=Widelands/Widelands font-weight=bold>";
		msg += ggzcore_room_get_name(room);
		formatedGGZChat(msg, "", true);
		msg = ggzcore_room_get_desc(room);
		formatedGGZChat(msg, "", true);
		formatedGGZChat("", "", true);
		break;
	case GGZ_ROOM_LIST:
		log("GGZCORE ## -- (room list)\n");
		num = ggzcore_server_get_num_rooms(ggzserver);
		joined = 0;
		for (i = 0; i < num; ++i) {
			room = ggzcore_server_get_nth_room(ggzserver, i);
			type = ggzcore_room_get_gametype(room);
			if (type)
			{
				if (!strcmp(ggzcore_gametype_get_name(type), "Widelands"))
				{
					ggzcore_server_join_room(ggzserver, room);
					joined = 1;
					break;
				}
			}
		}
		if (!joined) {
			log("GGZCORE ## couldn't find room! :(\n");
		}
		break;
	case GGZ_TYPE_LIST:
		log("GGZCORE ## -- (type list)\n");
		break;
	case GGZ_CHANNEL_CONNECTED:
		log("GGZCORE ## -- channel connected\n");
		channelfd = ggzcore_server_get_channel(ggzserver);
		break;
	case GGZ_CHANNEL_READY:
		log("GGZCORE ## -- channel ready\n");
		game = ggzcore_server_get_cur_game(ggzserver);
		ggzcore_game_set_server_fd(game, channelfd);
		channelfd = -1;
		init();
		break;
	case GGZ_CONNECT_FAIL:
	case GGZ_NEGOTIATE_FAIL:
	case GGZ_LOGIN_FAIL:
	case GGZ_ENTER_FAIL:
	case GGZ_CHANNEL_FAIL:
	case GGZ_NET_ERROR:
	case GGZ_PROTOCOL_ERROR:
		msg = static_cast<const char *>(cbdata);
		log("GGZCORE ## -- error! (%s)\n", msg.c_str());
		formatedGGZChat(ERRMSG + msg, "", true);
		formatedGGZChat("", "", true);
		ggzcore_login = false;
		break;
	}
}

void NetGGZ::event_room(uint32_t id, const void *cbdata) {
	int32_t i, num;
	GGZTable *table;

	switch (id) {
		case GGZ_TABLE_UPDATE:
		case GGZ_TABLE_LIST:
			log("GGZCORE/room ## -- table list\n");
			if (!room)
				room = ggzcore_server_get_cur_room(ggzserver);
			if (!room) {
				deinitcore();
				throw wexception("room was not found!");
			}
			tablelist.clear();
			num = ggzcore_room_get_num_tables(room);
			for (i = 0; i < num; ++i) {
				Net_Game_Info info;
				table = ggzcore_room_get_nth_table(room, i);
				if (!table) {
					deinitcore();
					throw wexception("table can not be found!");
				}
				strncpy
					(info.hostname,
					 ggzcore_table_get_desc(table),
					 sizeof(info.hostname));
				GGZTableState state = ggzcore_table_get_state(table);
				if (state == GGZ_TABLE_WAITING)
					info.state = LAN_GAME_OPEN;
				else if (state == GGZ_TABLE_PLAYING)
					info.state = LAN_GAME_CLOSED;
				else
					continue;
				tablelist.push_back(info);
			}
			tableupdate = true;
			ggzcore_login = false;
			ggzcore_ready = true;
			break;
		case GGZ_ROOM_ENTER:
		case GGZ_ROOM_LEAVE:
		case GGZ_PLAYER_LIST:
		case GGZ_PLAYER_COUNT:
			log("GGZCORE/room ## -- user list\n");
			write_userlist();
			userupdate = true;
			break;
		case GGZ_CHAT_EVENT:
			log("GGZCORE/room ## -- chat message\n");
			const GGZChatEventData * msg =
				static_cast<const GGZChatEventData *>(cbdata);
			formatedGGZChat(msg->message, msg->sender);
			break;
	}
}

void NetGGZ::event_game(uint32_t id, const void *cbdata) {
	GGZGame *game;
	GGZTable *table;
	GGZGameType *gametype;

	switch (id) {
	case GGZ_GAME_PLAYING:
		log("GGZCORE/game ## -- playing\n");
		if (!room)
			room = ggzcore_server_get_cur_room(ggzserver);
		if (tableid == -1) {
			table = ggzcore_table_new();
			gametype = ggzcore_room_get_gametype(room);
			ggzcore_table_init
				(table,
				 gametype,
				 servername.c_str(), 8);
				 // FIXME: problem in ggz - for some reasons only 8 seats are
				 // currently available. I already posted this problem to the
				 // ggz mailinglist. -- nasenbaer
				 // ggzcore_gametype_get_max_players(gametype));
			for
				(int32_t i = 1;
				 i < 8; // ggzcore_gametype_get_max_players(gametype); // FIXME
				 ++i)
				ggzcore_table_set_seat(table, i, GGZ_SEAT_OPEN, NULL);
			ggzcore_room_launch_table(room, table);
			ggzcore_table_free(table);
		} else {
			if (ggzcore_room_join_table(room, tableid, 0) < 0) {
				// Throw an errorcode - this can be removed once everything works
				// stable. ggzcore_room_join_table() is always returning -1 if an
				// error occured, so we need to check ourselfes, what the problem
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
				throw wexception("unable to join the table - error: %u", errorcode);
			} else
				log ("GGZCORE/game ## -- joined the table\n");
		}
		break;
	case GGZ_GAME_LAUNCHED:
		log("GGZCORE/game ## --  launched\n");
		game = ggzcore_server_get_cur_game(ggzserver);
		gamefd = ggzcore_game_get_control_fd(game);
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

void NetGGZ::write_userlist() {
	userlist.clear();
	if (!ggzserver)
		return;
	if (!room)
		room = ggzcore_server_get_cur_room(ggzserver);
	if (!room)
		return;

	int32_t i, num;
	GGZPlayer * player;
	num = ggzcore_room_get_num_players(room);
	for (i = 0; i < num; ++i) {
		player = ggzcore_room_get_nth_player(room, i);
		if (!player) {
			deinitcore();
			throw wexception("player can not be found!");
		}
		Net_Player user;
		user.name = ggzcore_player_get_name(player);
		GGZTable * tab = ggzcore_player_get_table(player);
		if (!tab)
			user.table = "--";
		else
			user.table = ggzcore_table_get_desc(tab);
		userlist.push_back(user);
	}
}


void NetGGZ::join(const char *tablename) {
	GGZGameType *type;
	GGZGame *game;
	int32_t i, num;
	GGZTable *table;
	const char *desc;

	if (!ggzcore_ready) return;

	if (!room)
		room = ggzcore_server_get_cur_room(ggzserver);
	type = ggzcore_room_get_gametype(room);

	tableid = -1;
	num = ggzcore_room_get_num_tables(room);
	for (i = 0; i < num; ++i) {
		table = ggzcore_room_get_nth_table(room, i);
		if (!table) {
			deinitcore();
			throw wexception("table can not be found!");
		}
		desc = ggzcore_table_get_desc(table);
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

	game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, NULL);

	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCHED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_LAUNCH_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook
		(game, GGZ_GAME_NEGOTIATE_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_PLAYING, &NetGGZ::callback_game);

	ggzcore_game_launch(game);

	request_server_ip();
}

void NetGGZ::launch()
{
	GGZGame *game;

	if (!ggzcore_ready) return;

	log("GGZCORE ## launch table\n");

	tableid = -1;

	game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, NULL);

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

void NetGGZ::request_server_ip()
{
	if (used())
		ggz_write_int(gamefd, op_request_ip);
}

/// Sends a chat message via ggz room chat
void NetGGZ::send(std::string const & msg)
{
	if (!usedcore())
		return;
	int16_t sent = ggzcore_room_chat(room, GGZ_CHAT_NORMAL, "", msg.c_str());
	if (sent < 0)
		log("GGZCORE/room/chat ## error sending message!\n");
}


/// Send a formated message to the chat menu
void NetGGZ::formatedGGZChat
	(std::string const & msg, std::string const & sender, bool system)
{
	ChatMessage c;
	c.time = WLApplication::get()->get_time();
	if (!system && sender.empty()) {
		c.sender = "<unknown>";
	} else {
		c.sender = sender;
	}
	if (system)
		c.playern = -1;
	else
		c.playern = 7;
	c.msg = msg;

	receive(c);
}

#endif
