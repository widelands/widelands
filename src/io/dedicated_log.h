/*
 * Copyright (C) 2012 by the Widelands Development Team
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

#ifndef DEDICATED_LOG_H
#define DEDICATED_LOG_H

#include "chat.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filewrite.h"
#include "log.h"

void dedicatedlog(const char *, ...) PRINTF_FORMAT(1, 2);

/// This struct writes some statistics and chat data to commandline defined files
struct DedicatedLog {
	// Always call DedicatedLog via get() to have only one instance
	static DedicatedLog * get();

	// chat logging functions
	void   chatAddSpacer();
	void   chat(ChatMessage & c);

	// info logging functions
	bool   write_info_active() {return !m_info_file_path.empty();}
	void   set_server_data(std::string name, std::string motd);
	void   set_server_ip  (std::string ip);
	void   client_login()  {++d_logins;  info_update();}  // simple counter
	void   client_logout() {++d_logouts; info_update();} // simple counter
	void   game_start(std::vector<std::string> clients, std::string mapname);
	void   game_end  (std::vector<std::string> winners);
	void   info_update(); // updates the info file

	// normal log logging function
	void   dlog(std::string);

	// Post initialization - can be called more than once, but only the first call will be handled
	// The functions do not only set up the output path, but as well take care about file cleanup and
	// generate the initial formating
	bool   set_chat_file_path(std::string path);
	bool   set_info_file_path(std::string path);
	bool   set_log_file_path (std::string path);

protected:
	/// Constructor is protected by purpose - only one instance of DedicatedLog is allowed
	/// call DedicatedLog::get() instead.
	DedicatedLog();

private:
	bool   check_file_writeable(std::string & path);

	std::string m_chat_file_path;
	std::string m_info_file_path;
	std::string m_log_file_path;

	FileWrite m_chat;
	FileWrite m_info;
	FileWrite m_path;

	// statics data
	struct GameStatistic {
		std::string              mapname;
		std::vector<std::string> clients;
		std::vector<std::string> winners;
		std::vector<time_t>      times;
	};
	std::string d_name, d_ip, d_motd, d_start;
	uint32_t    d_logins, d_logouts, d_chatmessages;
	std::vector<GameStatistic> d_games;


	RealFSImpl * root;
};

#endif
