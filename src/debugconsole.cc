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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
	Handler::HandlerFn default_handler;

	Console()
	{
		addCommand("help", boost::bind(&Console::cmdHelp, this, _1));
		addCommand("ls", boost::bind(&Console::cmdLs, this, _1));
		default_handler = boost::bind(&Console::cmdErr, this, _1);
	}

	~Console()
	{
	}

	void cmdHelp(const std::vector<std::string> &)
	{
		write("Use 'ls' to list all available commands.");
	}

	void cmdLs(const std::vector<std::string> &)
	{
		container_iterate_const(CommandMap, commands, i)
			write(i.current->first);
	}

	void cmdErr(const std::vector<std::string> & args) {
		write("Unknown command: " + args[0]);
	}

	void send(const std::string & msg) override
	{
		std::vector<std::string> arg;
		std::string::size_type pos = 0;

		write("# " + msg);

		while ((pos = msg.find_first_not_of(' ', pos)) != std::string::npos) {
			std::string::size_type const end = msg.find(' ', pos);
			arg.push_back(msg.substr(pos, end - pos));
			pos = end;
		}

		if (arg.empty())
			return;

		CommandMap::const_iterator it = commands.find(arg[0]);
		if (it == commands.end()) {
			default_handler(arg);
			return;
		}

		it->second(arg);
	}

	const std::vector<ChatMessage> & getMessages() const override
	{
		return messages;
	}

	void write(const std::string & msg)
	{
		ChatMessage cm;

		cm.time = time(nullptr);
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

void write(const std::string & text)
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

void Handler::addCommand(const std::string & cmd, const HandlerFn & fun)
{
	g_console.commands[cmd] = fun;
	m_commands.push_back(cmd);
}

void Handler::setDefaultCommand(const HandlerFn & fun)
{
	g_console.default_handler = fun;
}

}
