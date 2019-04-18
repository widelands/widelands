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

#include "graphic/color.h"
#include "graphic/playercolor.h"
#include "graphic/text_layout.h"
#include "logic/player.h"

namespace {

// Returns a player name font tag with player color.
std::string as_playercolor(const int16_t playern, const std::string& text) {
	const RGBColor& playercolor = ((playern >= 0) && playern < kMaxPlayers) ?
				kPlayerColors[playern] :
				g_gr->styles().font_style(UI::FontStyle::kChatServer).color();
	return g_gr->styles().font_style(UI::FontStyle::kChatPlayername).as_font_tag(g_gr->styles().color_tag(text, playercolor));
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
	// Time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M] ", localtime(&chat_message.time));

	std::string message = g_gr->styles().font_style(UI::FontStyle::kChatTimestamp).as_font_tag(ts);

	const std::string sanitized = sanitize_message(chat_message.msg);
	const std::string sender_escaped = richtext_escape(chat_message.sender);
	const std::string recipient_escaped = richtext_escape(chat_message.recipient);

	if (chat_message.recipient.size() && chat_message.sender.size()) {
		// Personal message handling
		if (sanitized.compare(0, 3, "/me")) {
			message += as_playercolor(chat_message.playern,
						(boost::format("%s @%s:") % sender_escaped % recipient_escaped).str());
			message += g_gr->styles().font_style(UI::FontStyle::kChatWhisper).as_font_tag(
						(boost::format("%s %s") % message % sanitized).str());
		} else {
			message += message = as_playercolor(chat_message.playern, (
						(boost::format("@%s: -&gt; %s") % recipient_escaped % sender_escaped).str()));
			message += g_gr->styles().font_style(UI::FontStyle::kChatWhisper).as_font_tag(
						(boost::format("%s %s") % message % sanitized.substr(3)).str());
		}
	} else {
		// Normal messages handling
		if (!sanitized.compare(0, 3, "/me")) {
			message += message = as_playercolor(chat_message.playern, (
						(boost::format("-&gt; %s") % (chat_message.sender.empty() ? "***" : sender_escaped)).str()));
			message += g_gr->styles().font_style(UI::FontStyle::kChatMessage).as_font_tag(
						(boost::format("%s %s") % message % sanitized.substr(3)).str());
		} else if (chat_message.sender.size()) {
			const std::string sender_formatted = as_playercolor(chat_message.playern, (boost::format("%s:") % sender_escaped).str());
			message +=
			   (boost::format("%s %s") %
			    sender_formatted % g_gr->styles().font_style(UI::FontStyle::kChatMessage).as_font_tag(sanitized))
			      .str();
		} else {
			message += g_gr->styles().font_style(UI::FontStyle::kChatServer).as_font_tag("*** " + sanitized);
		}
	}

	return (boost::format("<p>%s</p>") % message).str();
}
