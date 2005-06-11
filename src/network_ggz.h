/*
 * Copyright (C) 2004-2005 by the Widelands Development Team
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
#include <string>
#include "wexception.h"
#include "types.h"

#ifdef USE_GGZ
#define HAVE_GGZ 1
#endif

#ifdef HAVE_GGZ
#include <ggzmod.h>
#include <ggzcore.h>
#endif

class NetGGZ {
    public:
	NetGGZ();
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

 	void initcore(const char *hostname);
 	void deinitcore();
	bool usedcore();
	void datacore();
 	void launch();
	void join(const char *tablename);

    private:
#ifdef HAVE_GGZ
	static void ggzmod_server(GGZMod *mod, GGZModEvent e, const void *data);
	static GGZHookReturn callback_server(unsigned int id, const void *data, const void *user);
	static GGZHookReturn callback_room(unsigned int id, const void *data, const void *user);
	static GGZHookReturn callback_game(unsigned int id, const void *data, const void *user);
#endif
	void event_server(unsigned int id, const void *data);
	void event_room(unsigned int id, const void *data);
	void event_game(unsigned int id, const void *data);

	bool use_ggz;
	int fd;
	int channelfd;
	int gamefd;
	int tableid;
	char *ip_address;
	bool ggzcore_login;

	std::list<std::string> tablelist;
};

#endif

