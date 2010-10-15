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

#ifndef GGZ_GGZCORE_H
#define GGZ_GGZCORE_H

#ifdef USE_GGZ

#include <ggzcore.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <sys/stat.h>

struct Net_Player;
struct Net_Game_Info;

/**
 * Does the ggzcore part of ggz. Ggzcore is the part of ggz client which handles
 * the login to the metaserver and initiaties the game.
 * @note ggzcore do not have to be run in widelands. It is possible that ggzcore
 *   runs in another application (like kggz, ggz-gtk). Running ggzcore embedded
 *   in widelands should be optional and instead using a running ggzcore handler
 *   should be supported
 * @todo implement connecting to metaserver through an external
 *   ggzcore programm.
 */
class ggz_ggzcore
{
public:
	ggz_ggzcore();
	~ggz_ggzcore();

	/**
	 * Initialize the ggzcore object. This prepares for a login into
	 * the metaserver
	 * @param metaserver The address of the metaserver
	 * @param registered If true log into a registered account. Else play as
	 *                   a guest.
	 */
	bool init
		(char const * const metaserver, char const * const nick,
		 char const * const pwd, bool registered);

	/**
	 * This starts the connection to the metaserver. must be called after
	 * @ref init()
	 */
	bool connect();

	/** 
	 * Deinitialize the ggzcore object. This disconnects from the metaserver.
	 */
	void deinit();


	/**
	 * Join an existing table (game)
	 */
	void join(char const * tablename);
	
	/**
	 * Create a new table (game)
	 */
	void launch(uint32_t tableseats, std::string tablename);

	int get_max_players();

	bool updateForTables() {
		bool temp = tableupdate;
		tableupdate = false;
		return temp;
	}

	bool updateForUsers() {
		bool temp = userupdate;
		userupdate = false;
		return temp;
	}

	std::vector<Net_Game_Info> tablelist;
	std::vector<Net_Player>    userlist;

	void process();

	void send_message(const char *, const char *);

	bool is_in_room();
	bool is_in_table();
	bool logged_in() {
		return m_logged_in;
	}

	bool data_pending();

	enum ggzcore_error {
		ggzcoreerror_no_error,
		ggzcoreerror_login_failed,
		ggzcoreerror_unreachable,
		ggzcoreerror_unknown
	};
	
	enum ggzcore_state {
		ggzcorestate_disconnected,
		ggzcorestate_connecting,
		ggzcorestate_connected,
		ggzcorestate_error_disconnecting
	};
	enum ggzcore_tablestate {
		ggzcoretablestate_notinroom,
		ggzcoretablestate_launching,
		ggzcoretablestate_launched,
		ggzcoretablestate_playing,
		ggzcoretablestate_joined
	};

	enum ggzcore_tablestate get_tablestate(){
		return m_tablestate;
	}

	enum ggzcore_state get_state(){
		return m_state;
	}

	bool is_connecting() {
		return ggzcore_login and not ggzcore_ready and not m_logged_in;
	}

	inline void set_fds (fd_set & set) {
		if (m_server_fd >= 0 and m_server_fd < FD_SETSIZE)
			FD_SET(m_server_fd, &set);
		if (m_channelfd >= 0 and m_channelfd < FD_SETSIZE)
			FD_SET(m_channelfd, &set);
		if (m_gamefd >= 0 and m_gamefd < FD_SETSIZE)
			FD_SET(m_gamefd, &set);
	}

private:
	ggz_ggzcore(const ggz_ggzcore &) {}
	ggz_ggzcore & operator=(const ggz_ggzcore &) {}

	static GGZHookReturn
		callback_server(uint32_t id, void const * cbdata, void const * user);
	static GGZHookReturn
		callback_room(uint32_t id, void const * cbdata, void const * user);
	static GGZHookReturn
		callback_game(uint32_t id, void const * cbdata, void const * user);

	void event_server(uint32_t const id, void const * const cbdata);
	void event_game(uint32_t const id, void const * const cbdata);
	void event_room(uint32_t const id, void const * const cbdata);

	void write_userlist();
	void write_tablelist();

	bool ggzcore_login;
	bool ggzcore_ready;
	bool m_logged_in;
	bool relogin;

	int32_t m_tableid;

	int32_t m_channelfd;
	int32_t m_gamefd;
	/// filedescriptor to communicate with metaserver
	int32_t m_server_fd;

	GGZRoom * m_room;
	
	std::string m_servername;
	uint32_t m_tableseats;

	enum ggzcore_tablestate m_tablestate;
	enum ggzcore_state m_state;
	enum ggzcore_error m_error;

	/// \ref userlist was updated \ref updateForUsers
	bool userupdate;
	/// \ref tablelist was updated \ref updateForTables
	bool tableupdate;

	GGZServer * ggzserver;
	static ggz_ggzcore * ggzcoreobj;
};

#endif // USE_GGZ
#endif // GGZ_GGZCORE_H
