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

#include "chat.h"


std::string ChatMessage::toPrintable() const
{
	std::string message = "<p font-size=14 font-face=FreeSerif font-color=#";
	message += color();
	if ((msg.size() > 3) & (msg.substr(0, 3) == "/me")) {
		message += " font-style=italic>-> " + (sender.size() ? sender : "***") +
			msg.substr(3, msg.length() - 1);
	} else if (sender.size())
		message += " font-decoration=underline>" + sender +
			":</p><p font-size=14 font-face=FreeSerif> " + msg;
	else
		message += " font-weight=bold>*** " + msg;
	return message + "<br></p>";
}

std::string ChatMessage::color() const
{
	switch (playern) {
		case 0: return "0000ff";
		case 1: return "ff0000";
		case 2: return "ffff00";
		case 3: return "00ff00";
		case 4: return "333333";
		case 5: return "c37a00";
		case 6: return "a300c3";
		case 7: return "ffffff";
		default: return "999999";
	}
}
