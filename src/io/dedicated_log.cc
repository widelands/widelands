/*
 * Copyright (C) 2012-2013 by the Widelands Development Team
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

#include "io/dedicated_log.h"

#include <boost/format.hpp>

#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"

/// The dedicated server logger
static DedicatedLog * logger;

extern std::ostream & wout;

void dedicatedlog(const char * const fmt, ...) {
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	// Here comes the difference to widelands standard log() ;)
	DedicatedLog::get()->dlog(buffer);

	wout << buffer;
	wout.flush();
}

/// protected constructor
DedicatedLog::DedicatedLog()
:
m_chat_file_path(""),
m_info_file_path(""),
m_log_file_path(""),
d_name(""),
d_motd(""),
d_start(""),
d_logins(0),
d_logouts(0),
d_chatmessages(0),
root(new RealFSImpl("/"))
{
	char ts[42];
	time_t currenttime = time(nullptr);
	strftime(ts, sizeof(ts), "%a %Y/%m/%d, %H:%M:%S", localtime(&currenttime));
	d_start = ts;
	d_ip = (boost::format("\\<%s\\>") % _("unknown")).str();
}


/// \returns the dedicated server logger, if it is not yet initialized, this is done before.
DedicatedLog * DedicatedLog::get() {
	if (logger == nullptr)
		logger = new DedicatedLog();
	return logger;
}


/**
 * chat(ChatMessage & c)
 *
 * Writes the ChatMessage \arg c to the chat log, if initialized.
 */
void DedicatedLog::chat(ChatMessage & c) {
	if (!m_info_file_path.empty()) {
		++d_chatmessages;
		info_update();
	}

	if (m_chat_file_path.empty())
		return;

	std::string temp("<tr>");
	temp += "<td class=\"time\">";
	char ts[32];
	strftime(ts, sizeof(ts), "[%Y-%m-%d, %H:%M]", localtime(&c.time));
	temp += (boost::format("%s</td><td class=\"player%i\">") % ts % c.playern).str();
	temp += c.sender.empty() ? "SYSTEM" : c.sender;
	temp += "</td><td class=\"recipient\"> ->" + c.recipient + "</td><td class=\"message\">";
	temp += c.msg + "</td></tr>\n";
	m_chat.Printf("%s", temp.c_str());
	m_chat.WriteAppend(*root, m_chat_file_path.c_str());
}

/// Add's a spacer to the chat log
void DedicatedLog::chatAddSpacer() {
	if (m_chat_file_path.empty())
		return;

	m_chat.Printf("<tr><td class=\"space\"></td><td class=\"space\"></td>");
	m_chat.Printf("<td class=\"space\"></td><td class=\"space\"></td></tr>\n");
	m_chat.WriteAppend(*root, m_chat_file_path.c_str());
}


/// Sets the basic server informations
void DedicatedLog::set_server_data(std::string name, std::string motd) {
	if (!d_name.empty())
		return;
	d_name = name;
	d_motd = motd;
	info_update();
}


/// Sets the servers ip informations
void DedicatedLog::set_server_ip(std::string ip) {
	if (d_ip != ip) {
		d_ip = ip;
		info_update();
	}
}


/// Saves the data of a newly started game
void DedicatedLog::game_start(std::vector<std::string> clients, std::string mapname) {
	GameStatistic * new_game = new GameStatistic;
	new_game->mapname = mapname;
	new_game->times.push_back(time(nullptr));
	new_game->clients = clients;
	d_games.push_back(*new_game);
	delete new_game;
	new_game = nullptr;
	info_update();
}


/// Saves the winners of the last started game
void DedicatedLog::game_end(std::vector<std::string> winners) {
	assert(!d_games.empty());
	d_games.back().winners = winners;
	d_games.back().times.push_back(time(nullptr));
	info_update();
}


/// Updates the server information file with current data
void DedicatedLog::info_update() {
	if (m_info_file_path.empty())
		return;

	std::string temp("<table class=\"infohead\">\n");
	// Basic information
	temp += "<tr><td class=\"infoname\">Servername</td><td class=\"info\">"  + d_name  + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Server IP</td><td class=\"info\">"   + d_ip    + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Server MOTD</td><td class=\"info\">" + d_motd  + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Started on</td><td class=\"info\">"  + d_start + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Logins</td><td class=\"info\">";
	temp += (boost::format("%u") % d_logins).str() + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Logouts</td><td class=\"info\">";
	temp += (boost::format("%u") % d_logouts).str() + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Chat messages</td><td class=\"info\">";
	temp += (boost::format("%u") % d_chatmessages).str() + "</td></tr>\n";
	temp += "<tr><td class=\"infoname\">Games started</td><td class=\"info\">";
	temp += (boost::format("%u") % d_games.size()).str() + "</td></tr>\n";
	if (!d_games.empty()) {
		// Games information
		temp += "</table><br><table class=\"infogames\">\n";
		temp += "<tr><th>start/end of game</th><th>map name</th><th>client(s)</th><th>winner(s)</th></tr>\n";

		for (uint16_t i = 0; i < d_games.size(); ++i) {
			assert(!d_games.at(i).clients.empty() && !d_games.at(i).times.empty());
			// Start and (if already ended) end time
			char ts[42];
			strftime(ts, sizeof(ts), "S: %Y/%m/%d, %H:%M:%S", localtime(&d_games.at(i).times.at(0)));
			temp += (boost::format("<tr><td>%s") % ts).str();
			if (d_games.at(i).times.size() > 1) {
				strftime(ts, sizeof(ts), "E: %Y/%m/%d, %H:%M:%S", localtime(&d_games.at(i).times.at(1)));
				temp += (boost::format("<br>%s") % ts).str();
			}
			// Map name
			temp += (boost::format("</td><td>%s</td><td>") % d_games.at(i).mapname).str();
			// Players
			for (uint16_t j = 0; j < d_games.at(i).clients.size(); ++j) {
				if (j > 0)
					temp += ", ";
				temp += d_games.at(i).clients.at(j);
			}
			temp += "</td><td>";
			// Winners
			for (uint16_t j = 0; j < d_games.at(i).winners.size(); ++j) {
				if (j > 0)
					temp += ", ";
				temp += d_games.at(i).winners.at(j);
			}
			temp += "</td></tr>\n";
		}
	}
	temp += "</table>\n";
	m_chat.Printf("%s", temp.c_str());
	m_chat.Write(*root, m_info_file_path.c_str());
}

/// Appends the String \arg msg to the log file
void DedicatedLog::dlog(std::string msg) {
	if (m_log_file_path.empty())
		return;

	std::string temp("<tr><td class=\"time\">");
	char ts[32];
	time_t * t = new time_t(time(nullptr));
	strftime(ts, sizeof(ts), "[%Y-%m-%d, %H:%M]", localtime(t));
	delete t;
	temp += ts;
	temp += "</td><td class=\"log\">";
	temp += msg;
	temp += "</td></tr>\n";
	m_chat.Printf("%s", temp.c_str());
	m_chat.WriteAppend(*root, m_log_file_path.c_str());
}


/**
 * set_chat_file_path(std::string path)
 *
 * Post initialization - this function can be called more than once, but will only handle the input data
 * as long as the chat file path is not yet set up correctly.
 * The function takes care:
 *    - Whether the file at \arg path is writeable - \returns false if not.
 *    - About file cleanup - all following data will be attached, therefore the original file will be removed
 *    - About the initial formating like table headers, etc.
 *
 * \returns false, if path is not writeable, in all other cases true
 */
bool DedicatedLog::set_chat_file_path(std::string path) {
	if (!m_chat_file_path.empty() || path.empty())
		return true;

	if (!check_file_writeable(path))
		return false;

	// Everything's fine, set the path
	m_chat_file_path = path;

	// Initialize the chat file
	m_chat.Printf("<tr><th>Time</th><th>Sender</th><th>Recipient</th><th>Message</th></tr>");
	m_chat.Write(*root, m_chat_file_path.c_str()); // Not WriteAppend, to make sure the file is cleared
	return true;
}


/**
 * set_info_file_path(std::string path)
 *
 * Post initialization - this function can be called more than once, but will only handle the input data
 * as long as the info file path is not yet set up correctly.
 * The function takes care:
 *    - Whether the file at \arg path is writeable - \returns false if not.
 *    - About file cleanup - all following data will be attached, therefore the original file will be removed
 *    - About the initial formating like table headers, etc.
 *
 * \returns false, if path is not writeable, in all other cases true
 */
bool DedicatedLog::set_info_file_path(std::string path) {
	if (!m_info_file_path.empty() || path.empty())
		return true;

	if (!check_file_writeable(path))
		return false;

	// Everything's fine, set the path and write info for the first time;
	m_info_file_path = path;
	info_update();
	return true;
}


/**
 * set_log_file_path(std::string path)
 *
 * Post initialization - this function can be called more than once, but will only handle the input data
 * as long as the log file path is not yet set up correctly.
 * The function takes care:
 *    - Whether the file at \arg path is writeable - \returns false if not.
 *    - About file cleanup - all following data will be attached, therefore the original file will be removed
 *    - About the initial formating like table headers, etc.
 *
 * \returns false, if path is not writeable, in all other cases true
 */
bool DedicatedLog::set_log_file_path (std::string path) {
	if (!m_log_file_path.empty() || path.empty())
		return true;

	if (!check_file_writeable(path))
		return false;

	// Everything's fine, set the path
	m_log_file_path = path;

	// Initialize the log file
	m_chat.Printf("<tr><th></th><th>Widelands dedicated server log:</th></tr>\n");
	m_chat.Write(*root, m_log_file_path.c_str()); // Not WriteAppend, to make sure the file is cleared
	return true;
}


/**
 * check_file_writeable(std::string & path)
 *
 * Checks if a file is writeable to \arg path and if yes and a file of that name is already existing
 * moves the original file to path + "~".
 *
 * \returns false, if path is not writeable or if path is a directory or if the directory the file should be
 *          written to does not exist, in all other cases true.
 */
bool DedicatedLog::check_file_writeable(std::string & path) {
	bool existing = root->FileExists(path);
	if (existing && root->IsDirectory(path))
		return false;
	if (root->FileIsWriteable(path)) {
		if (existing) {
			std::string rnpath(path + '~');
			if (root->FileIsWriteable(rnpath))
				root->Rename(path, rnpath);
			else
				log("Note: original file %s could not be backuped\n", path.c_str());
		}
		return true;
	}
	return false;
}
