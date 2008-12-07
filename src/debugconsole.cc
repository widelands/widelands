/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "debugconsole.h"

#include <map>
#include <boost/bind.hpp>

#include "chat.h"
#include "log.h"
#include "wlapplication.h"

namespace DebugConsole {

struct Console : public ChatProvider, public Handler {
	typedef std::map<std::string, Handler::HandlerFn> CommandMap;

	std::vector<ChatMessage> messages;
	CommandMap commands;

	Console()
	{
		addCommand("help", boost::bind(&Console::cmdHelp, this, _1));
		addCommand("ls", boost::bind(&Console::cmdLs, this, _1));
	}

	~Console()
	{
	}

	void cmdHelp(const std::vector<std::string>&)
	{
		write("Use 'ls' to list all available commands.");
	}

	void cmdLs(const std::vector<std::string>&)
	{
		for
			(CommandMap::const_iterator it = commands.begin();
			 it != commands.end();
			 ++it)
		{
			write(it->first);
		}
	}

	void send(const std::string& msg)
	{
		std::vector<std::string> arg;
		size_t pos = 0;

		write("# " + msg);

		while ((pos = msg.find_first_not_of(' ', pos)) != std::string::npos) {
			size_t end = msg.find_first_of(' ', pos);
			arg.push_back(msg.substr(pos, end-pos));
			pos = end;
		}

		if (!arg.size())
			return;

		CommandMap::const_iterator it = commands.find(arg[0]);
		if (it == commands.end()) {
			write("Unknown command: " + arg[0]);
			return;
		}

		it->second(arg);
	}

	const std::vector<ChatMessage>& getMessages() const
	{
		return messages;
	}

	void write(const std::string& msg)
	{
		ChatMessage cm;

		cm.time = WLApplication::get()->get_time();
		cm.msg = msg;
		messages.push_back(cm);

		log("*** %s\n", msg.c_str());

		// Arbitrary choice of backlog size
		if (messages.size() > 1000)
			messages.erase(messages.begin(), messages.begin() + 100);

		ChatProvider::send(cm); // Notify listeners, i.e. the UI
	}
};

Console g_console;

ChatProvider * getChatProvider()
{
	return &g_console;
}

void write(const std::string& text)
{
	g_console.write(text);
}


Handler::Handler()
{
}

Handler::~Handler()
{
	// This check is an evil hack to account for the singleton-nature
	// of the Console
	if (this != &g_console) {
		for
			(std::vector<std::string>::const_iterator it = m_commands.begin();
			 it != m_commands.end();
			 ++it)
			g_console.commands.erase(*it);
	}
}

void Handler::addCommand(const std::string& cmd, const HandlerFn& fun)
{
	g_console.commands[cmd] = fun;
	m_commands.push_back(cmd);
}

} // namespace DebugConsole
