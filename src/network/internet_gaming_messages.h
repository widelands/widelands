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

#ifndef INTERNET_GAMING_MESSAGES_H
#define INTERNET_GAMING_MESSAGES_H

#include "i18n.h"

#include <string>
#include <map>

struct InternetGamingMessages {
public:
	static const std::string & get_message(const std::string & code) {
		if(messages.find(code) != messages.end())
			return messages[code];
		// if no message for code was found, just return code
		return code;
	}

	static void fill_map() {
		messages["WRONG_PASSWORD"]       = _("The send password was incorrect!");
		messages["ALREADY_LOGGED_IN"]    = _("You are already logged in!");
		messages["DEFICIENT_PERMISSION"] =
			_
			 ("You got disconnected, as you send a superuser command without superuser permission."
			  "This incident will be logged and reported to the administrator.");
		messages["RESTARTING"]           = _("You got disconnected, as the metaserver is currently restarting");
	}

private:
	static std::map <std::string, std::string> messages;
}
