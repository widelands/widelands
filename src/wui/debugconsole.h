/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_DEBUGCONSOLE_H
#define WL_WUI_DEBUGCONSOLE_H

#include <functional>
#include <string>
#include <vector>

struct ChatProvider;

/**
 * The debug console is a tool to allow ad-hoc commands that inspect
 * and modify the current game state in a way that is not supposed
 * to be possible in a normal game.
 *
 * The point is (besides cheating) to make it easier to debug complex
 * aspects of the game, such as the behaviour of AI.
 */
namespace DebugConsole {

/**
 * Derive your class from Handler to register console commands.
 *
 * Call \ref addCommand in your constructor to register console commands.
 * All registered commands are automatically unregistered when the destructor
 * runs.
 */
struct Handler {
	/**
	 * Type for command callback functions.
	 * The passed array contains a list of all (space-separated) components
	 * of the command, including the command name itself.
	 */
	using HandlerFn = std::function<void(const std::vector<std::string>&)>;

	Handler() = default;
	virtual ~Handler();

protected:
	void addCommand(const std::string&, const HandlerFn&);
	void setDefaultCommand(const HandlerFn&);

private:
	std::vector<std::string> commands_;
};

/**
 * Returns the \ref ChatProvider that acts as textual interface to
 * the debug console. Sending a message to this ChatProvider executes
 * the given command string, and the log of messages contains a log
 * of console messages.
 */
ChatProvider* get_chat_provider();

/**
 * Write some output on the console.
 *
 * \p text the text to be written, without a newline at the end.
 * The newline is appended automatically.
 *
 * \note This is not the same as sending a message via
 * \ref get_chat_provider(). Sending a message via \ref get_chat_provider()
 * executes a command, while this function writes to the console.
 */
void write(const std::string& text);
}  // namespace DebugConsole

#endif  // end of include guard: WL_WUI_DEBUGCONSOLE_H
