/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#include "wui/chat_msg_layout.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "chat/chat.h"
#include "graphic/color.h"
#include "graphic/text_layout.h"
#include "logic/player.h"

namespace {

// Returns the hexcolor for the 'player'.
std::string color(const int16_t playern) {
	if ((playern >= 0) && playern < kMaxPlayers) {
		const RGBColor& clr = kPlayerColors[playern];
		char buf[sizeof("ffffff")];
		snprintf(buf, sizeof(buf), "%.2x%.2x%.2x", clr.r, clr.g, clr.b);
		return buf;
	}
	return "999999";
}

std::string sanitize_message(const std::string& given_text) {
	std::string result = richtext_escape(given_text);
	// Preserve br tag
	boost::replace_all(result, "&lt;br&gt;", "<br>");
	return result;
}

}  // namespace

// Returns a richtext string that can be displayed to the user.
std::string format_as_richtext(const ChatMessage& chat_message) {
	const std::string& font_face = "serif";
	std::string message = "<p><font color=33ff33 size=9>";

	const std::string sanitized = sanitize_message(chat_message.msg);

	// time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M] ", localtime(&chat_message.time));
	message += ts;

	message = (boost::format("%s</font><font size=14 face=%s color=%s") % message % font_face %
	           color(chat_message.playern))
	             .str();

	const std::string sender_escaped = richtext_escape(chat_message.sender);
	const std::string recipient_escaped = richtext_escape(chat_message.recipient);

	if (chat_message.recipient.size() && chat_message.sender.size()) {
		// Personal message handling
		if (sanitized.compare(0, 3, "/me")) {
			message = (boost::format(
			              "%s bold=1>%s @ %s:</font><font size=14 face=%s shadow=1 color=eeeeee> %s") %
			           message % sender_escaped % recipient_escaped % font_face % sanitized)
			             .str();

		} else {
			message =
			   (boost::format(
			       "%s>@%s &gt; </font><font size=14 face=%s color=%s italic=1 shadow=1> %s%s") %
			    message % recipient_escaped % font_face % color(chat_message.playern) %
			    sender_escaped % sanitized.substr(3))
			      .str();
		}
	} else {
		// Normal messages handling
		if (!sanitized.compare(0, 3, "/me")) {
			message += " italic=1>-&gt; ";
			if (chat_message.sender.size())
				message += sender_escaped;
			else
				message += "***";
			message += sanitized.substr(3);
		} else if (chat_message.sender.size()) {
			message =
			   (boost::format("%s bold=1>%s:</font><font size=14 face=%s shadow=1 color=eeeeee> %s") %
			    message % sender_escaped % font_face % sanitized)
			      .str();
		} else {
			message += " bold=1>*** ";
			message += sanitized;
		}
	}

	// return the formated message
	return message + "</font><br></p>";
}
