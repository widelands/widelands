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

#include "log.h"

static NetGGZ *ggzobj = 0;
#ifdef HAVE_GGZ
static GGZMod *mod = 0;
static GGZServer *ggzserver = 0;
#endif

NetGGZ::NetGGZ()
{
	use_ggz = false;
	m_fd = -1;
	channelfd = -1;
	gamefd = -1;
	ip_address = NULL;
}

NetGGZ & NetGGZ::ref() {
	static NetGGZ ggzobj;
	return ggzobj;
}

void NetGGZ::init()
{
	use_ggz = true;
	log(">> GGZ: initialized\n");
}

bool NetGGZ::used()
{
	return use_ggz;
	//log(">> GGZ: is used\n");
}

bool NetGGZ::connect()
{
#ifdef HAVE_GGZ
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
#else
	return false;
#endif
}

#ifdef HAVE_GGZ
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
#endif

void NetGGZ::data()
{
#ifdef HAVE_GGZ
	int32_t op;
	char *ipstring;
	char *greeter;
	int32_t greeterversion;
	char ipaddress[17];
	int32_t fd;

	if (!used()) return;

	fd = m_fd;
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
		ip_address = ggz_strdup(ipstring);
		ggz_free(ipstring);
		break;
	default: log("GGZ ## opcode unknown!\n");
	}
#endif
}

bool NetGGZ::host()
{
#ifdef HAVE_GGZ
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
#else
	return false;
#endif
}

const char *NetGGZ::ip()
{
	return ip_address;
}

void NetGGZ::initcore(const char *hostname, const char *playername) {
#ifdef HAVE_GGZ
	GGZOptions opt;
	int32_t ret;

	if (usedcore()) return;

	log("GGZCORE ## initialization\n");
	ggzcore_login = true;
	ggzcore_ready = false;

	opt.flags = static_cast<GGZOptionFlags>(GGZ_OPT_EMBEDDED);
	ret = ggzcore_init(opt);

	ggzserver = ggzcore_server_new();
	ggzcore_server_add_event_hook(ggzserver, GGZ_CONNECTED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_NEGOTIATED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_LOGGED_IN, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_ENTERED, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_CONNECT_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_NEGOTIATE_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_LOGIN_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_ENTER_FAIL, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_CHANNEL_FAIL, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_ROOM_LIST, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_TYPE_LIST, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_NET_ERROR, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_PROTOCOL_ERROR, &NetGGZ::callback_server);

	ggzcore_server_add_event_hook(ggzserver, GGZ_CHANNEL_CONNECTED, &NetGGZ::callback_server);
	ggzcore_server_add_event_hook(ggzserver, GGZ_CHANNEL_READY, &NetGGZ::callback_server);

	ggzcore_server_set_hostinfo(ggzserver, hostname, 5688, 0);

#if GGZCORE_VERSION_MICRO < 11
	ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN_GUEST, playername, NULL);
#else
	ggzcore_server_set_logininfo(ggzserver, GGZ_LOGIN_GUEST, playername, NULL, NULL);
#endif

	ggzcore_server_connect(ggzserver);

	log("GGZCORE ## start loop\n");
	while (ggzcore_login)
		datacore();
	log("GGZCORE ## end loop\n");
#endif
}

void NetGGZ::deinitcore()
{
#ifdef HAVE_GGZ
	ggzcore_server_logout(ggzserver);
	ggzcore_server_disconnect(ggzserver);
	ggzcore_server_free(ggzserver);
	ggzserver = 0;
	ggzcore_destroy();
	ggzcore_ready = false;
#endif
}

bool NetGGZ::usedcore()
{
#ifdef HAVE_GGZ
   return (ggzserver != 0);
#else
   return 0; // To make gcc shut up
#endif
}

void NetGGZ::datacore()
{
#ifdef HAVE_GGZ
	GGZGame *game;

	if (!ggzserver) return;
	if (ggzcore_server_data_is_pending(ggzserver))
		ggzcore_server_read_data(ggzserver, ggzcore_server_get_fd(ggzserver));

	if (channelfd != -1)
		ggzcore_server_read_data(ggzserver, ggzcore_server_get_channel(ggzserver));

	if (gamefd != -1)
	{
		game = ggzcore_server_get_cur_game(ggzserver);
		ggzcore_game_read_data(game);
	}
#endif
}

#ifdef HAVE_GGZ
GGZHookReturn NetGGZ::callback_server(uint32_t id, const void *cbdata, const void *user)
{
	log("GGZCORE ## callback: %i\n", id);
	ggzobj->event_server(id, cbdata);

	return GGZ_HOOK_OK;
}
#endif

#ifdef HAVE_GGZ
GGZHookReturn NetGGZ::callback_room(uint32_t id, const void *cbdata, const void *user)
{
	log("GGZCORE/room ## callback: %i\n", id);
	ggzobj->event_room(id, cbdata);

	return GGZ_HOOK_OK;
}
#endif

#ifdef HAVE_GGZ
GGZHookReturn NetGGZ::callback_game(uint32_t id, const void *cbdata, const void *user)
{
	log("GGZCORE/game ## callback: %i\n", id);
	ggzobj->event_game(id, cbdata);

	return GGZ_HOOK_OK;
}
#endif

void NetGGZ::event_server(uint32_t id, const void *cbdata) {
#ifdef HAVE_GGZ
	GGZRoom *room;
	GGZGameType *type;
	GGZGame *game;
	int32_t num, i;
	int32_t joined;

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
		room = ggzcore_server_get_cur_room(ggzserver);
		ggzcore_room_add_event_hook(room, GGZ_TABLE_LIST, &NetGGZ::callback_room);
		ggzcore_room_list_tables(room, -1, 0);
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
#if GGZCORE_VERSION_MICRO < 14
					ggzcore_server_join_room(ggzserver, i);
#else
					ggzcore_server_join_room(ggzserver, room);
#endif
					joined = 1;
					break;
				}
			}
		}
		if (!joined)
		{
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
		break;
	case GGZ_CONNECT_FAIL:
	case GGZ_NEGOTIATE_FAIL:
	case GGZ_LOGIN_FAIL:
	case GGZ_ENTER_FAIL:
	case GGZ_CHANNEL_FAIL:
	case GGZ_NET_ERROR:
	case GGZ_PROTOCOL_ERROR:
		log("GGZCORE ## -- error! (%s) :(\n", static_cast<const char *>(cbdata));
		ggzcore_login = false;
		break;
	}
#endif
}

void NetGGZ::event_room(uint32_t id, const void *cbdata) {
#ifdef HAVE_GGZ
	GGZRoom *room;
	int32_t i, num;
	GGZTable *table;
	const char *desc;

	switch (id) {
	case GGZ_TABLE_LIST:
		log("GGZCORE/room ## -- table list\n");
		room = ggzcore_server_get_cur_room(ggzserver);
		num = ggzcore_room_get_num_tables(room);
		for (i = 0; i < num; ++i) {
			table = ggzcore_room_get_nth_table(room, i);
			desc = ggzcore_table_get_desc(table);
			log("GGZCORE/room ## table: %s\n", desc);
			if (!desc) desc = "(unknown map)";
			tablelist.push_back(desc);
		}
		ggzcore_login = false;
		ggzcore_ready = true;
		break;
	}
#endif
}

void NetGGZ::event_game(uint32_t id, const void *cbdata) {
#ifdef HAVE_GGZ
	GGZRoom *room;
	GGZGame *game;
	GGZTable *table;
	GGZGameType *gametype;

	switch (id) {
	case GGZ_GAME_PLAYING:
		log("GGZCORE/game ## -- playing\n");
		room = ggzcore_server_get_cur_room(ggzserver);
		if (tableid == -1)
		{
			table = ggzcore_table_new();
			gametype = ggzcore_room_get_gametype(room);
			ggzcore_table_init
				(table,
				 gametype,
				 "test",
				 ggzcore_gametype_get_max_players(gametype));
			for
				(int32_t i = 1;
				 i < ggzcore_gametype_get_max_players(gametype);
				 ++i)
				ggzcore_table_set_seat(table, i, GGZ_SEAT_OPEN, NULL);
			ggzcore_room_launch_table(room, table);
			ggzcore_table_free(table);
		}
		else ggzcore_room_join_table(room, tableid, 0);
		break;
	case GGZ_GAME_LAUNCHED:
		log("GGZCORE/game ## -- launched\n");
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
#endif
}

std::list<std::string> NetGGZ::tables()
{
	return tablelist;
}

void NetGGZ::join(const char *tablename) {
#ifdef HAVE_GGZ
	GGZRoom *room;
	GGZGameType *type;
	GGZGame *game;
	int32_t i, num;
	GGZTable *table;
	const char *desc;

	if (!ggzcore_ready) return;

	log("GGZCORE ## join table %s\n", tablename);

	room = ggzcore_server_get_cur_room(ggzserver);
	type = ggzcore_room_get_gametype(room);

	tableid = -1;
	num = ggzcore_room_get_num_tables(room);
	for (i = 0; i < num; ++i) {
		table = ggzcore_room_get_nth_table(room, i);
		desc = ggzcore_table_get_desc(table);
		if (!desc) desc = "(unknown map)";
		if (!strcmp(desc, tablename)) tableid = i;
	}

	log("GGZCORE ## that is table id %i\n", tableid);
	if (tableid == -1) return;

	game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, NULL);

	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCHED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCH_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_NEGOTIATED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_NEGOTIATE_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_PLAYING, &NetGGZ::callback_game);

	ggzcore_game_launch(game);
#endif
}

void NetGGZ::launch()
{
#ifdef HAVE_GGZ
	GGZGame *game;

	if (!ggzcore_ready) return;

	log("GGZCORE ## launch table\n");

	tableid = -1;

	game = ggzcore_game_new();
	ggzcore_game_init(game, ggzserver, NULL);

	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCHED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_LAUNCH_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_NEGOTIATED, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_NEGOTIATE_FAIL, &NetGGZ::callback_game);
	ggzcore_game_add_event_hook(game, GGZ_GAME_PLAYING, &NetGGZ::callback_game);

	ggzcore_game_launch(game);
#endif
}
