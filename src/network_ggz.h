/*
 * Copyright (C) 2004-2006 by the Widelands Development Team
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

#ifndef __NETWORK_GGZ_H__
#define __NETWORK_GGZ_H__

#include <list>
#include <stdint.h>
#include <string>

#ifdef USE_GGZ
#define HAVE_GGZ 1
#endif

#ifdef HAVE_GGZ
#include <ggzmod.h>
#include <ggzcore.h>
#endif

#ifdef __WIN32__
#include <winsock2.h>
#include <io.h>
#endif

struct NetGGZ {
	static NetGGZ* ref();

	void init();
	bool connect();

	bool used();
	bool host();
	void data();
	const char *ip();

	std::list<std::string> tables();

	enum Protocol
	{
		op_greeting = 1,
		op_request_ip = 2,
		op_reply_ip = 3,
		op_broadcast_ip = 4
	};

	void   initcore(const char * hostname, const char * playername);
	void deinitcore();
	bool usedcore();
	void datacore();
	void launch    ();
	void join(const char *tablename);

private:
	NetGGZ();
#ifdef HAVE_GGZ
	static void ggzmod_server(GGZMod *cbmod, GGZModEvent e, const void *cbdata);
	static GGZHookReturn callback_server(uint32_t id, const void *cbdata, const void *user);
	static GGZHookReturn callback_room(uint32_t id, const void *cbdata, const void *user);
	static GGZHookReturn callback_game(uint32_t id, const void *cbdata, const void *user);
#endif
	void event_server(uint32_t id, const void *cbdata);
	void event_room(uint32_t id, const void *cbdata);
	void event_game(uint32_t id, const void *cbdata);

	bool use_ggz;
	int32_t m_fd;
	int32_t channelfd;
	int32_t gamefd;
	int32_t tableid;
	char *ip_address;
	bool ggzcore_login;
	bool ggzcore_ready;

	std::list<std::string> tablelist;
};

#endif
