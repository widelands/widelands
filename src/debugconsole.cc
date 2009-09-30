/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

	void cmdHelp(std::vector<std::string> const &)
	{
		write("Use 'ls' to list all available commands.");
	}

	void cmdLs(std::vector<std::string> const &)
	{
		container_iterate_const(CommandMap, commands, i)
			write(i.current->first);
	}

	void send(std::string const & msg)
	{
		std::vector<std::string> arg;
		size_t pos = 0;

		write("# " + msg);

		while ((pos = msg.find_first_not_of(' ', pos)) != std::string::npos) {
			size_t end = msg.find_first_of(' ', pos);
			arg.push_back(msg.substr(pos, end - pos));
			pos = end;
		}

		if (arg.empty())
			return;

		CommandMap::const_iterator it = commands.find(arg[0]);
		if (it == commands.end()) {
			write("Unknown command: " + arg[0]);
			return;
		}

		it->second(arg);
	}

	std::vector<ChatMessage> const & getMessages() const
	{
		return messages;
	}

	void write(std::string const & msg)
	{
		ChatMessage cm;

		cm.time = time(0);
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

void write(std::string const & text)
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
	if (this != &g_console)
		container_iterate_const(std::vector<std::string>, m_commands, i)
			g_console.commands.erase(*i.current);
}

void Handler::addCommand(std::string const & cmd, HandlerFn const & fun)
{
	g_console.commands[cmd] = fun;
	m_commands.push_back(cmd);
}

}
