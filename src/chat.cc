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

#include "chat.h"
#include "logic/editor_game_base.h"

using namespace Widelands;

std::string ChatMessage::toPrintable() const
{
	std::string message = "<p font-color=#33ff33 font-size=9>";

	// time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M] </p>", localtime(&time));
	message += ts;

	message += "<p font-size=14 font-face=FreeSerif font-color=#";
	message += color();

	if (!recipient.empty() && !sender.empty()) {
	// Personal message handling
		if ((msg.size() > 3) & (msg.substr(0, 3) == "/me")) {
			message += ">@" + recipient + " >> </p><p font-size=14";
			message += " font-face=FreeSerif font-color=#" + color();
			message += " font-style=italic> " + sender;
			message += msg.substr(3, msg.length() - 3);
		} else {
			message += " font-decoration=underline>" + sender + " @ " + recipient;
			message += ":</p><p font-size=14 font-face=FreeSerif> " + msg;
		}
	} else {
	// Normal messages handling
		if ((msg.size() > 3) & (msg.substr(0, 3) == "/me")) {
			message += " font-style=italic>-> " + (sender.size() ? sender : "***");
			message += msg.substr(3, msg.length() - 3);
		} else if (sender.size()) {
			message += " font-decoration=underline>" + sender;
			message += ":</p><p font-size=14 font-face=FreeSerif> " + msg;
		} else
			message += " font-weight=bold>*** " + msg;
	}

	// return the formated message
	return message + "<br></p>";
}

std::string ChatMessage::toPlainString() const
{
	return sender + ": " + msg;
}

std::string ChatMessage::color() const
{
	if ((playern >= 0) && playern < MAX_PLAYERS) {
		const uint8_t * cols = g_playercolors[playern];
		char buf[sizeof("ffffff")];
		snprintf(buf, sizeof(buf), "%.2x%.2x%.2x", cols[9], cols[10], cols[11]);
		return buf;
	}
	return "999999";
}
