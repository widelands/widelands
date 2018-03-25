/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include <boost/format.hpp>

#include "graphic/color.h"
#include "graphic/playercolor.h"
#include "graphic/text_layout.h"
#include "logic/player.h"

namespace {

// Returns the hexcolor for the 'player'.
std::string color(const int16_t playern) {
	if ((playern >= 0) && playern < kMaxPlayers) {
		return kPlayerColors[playern].hex_value();
	}
	return g_gr->styles().font_style(StyleManager::FontStyle::kChatServer).color.hex_value();
}

std::string as_playercolor(const int16_t playern, const std::string& text) {
	const std::string colored_text = (boost::format("<font color=%s>%s</font>") % color(playern) % text).str();
	return g_gr->styles().font_style(StyleManager::FontStyle::kChatPlayername).as_font_tag(colored_text);
}

}  // namespace

// Returns a richtext string that can be displayed to the user.
std::string format_as_richtext(const ChatMessage& chat_message) {
	const std::string sanitized = sanitize_message(chat_message);
	const std::string sender_escaped = richtext_escape(chat_message.sender);
	const std::string recipient_escaped = richtext_escape(chat_message.recipient);

	std::string message = "";

	if (chat_message.recipient.size() && chat_message.sender.size()) {
		// Personal message handling
		if (sanitized.compare(0, 3, "/me")) {
			message = as_playercolor(chat_message.playern,
						(boost::format("%s @%s:") % sender_escaped % recipient_escaped).str());
			message = g_gr->styles().font_style(StyleManager::FontStyle::kChatWhisper).as_font_tag(
						(boost::format("%s %s") % message % sanitized).str());
		} else {
			message = message = as_playercolor(chat_message.playern, (
						(boost::format("@%s: -&gt; %s") % recipient_escaped % sender_escaped).str()));
			message = g_gr->styles().font_style(StyleManager::FontStyle::kChatWhisper).as_font_tag(
						(boost::format("%s %s") % message % sanitized.substr(3)).str());
		}
	} else {
		// Normal messages handling
		if (!sanitized.compare(0, 3, "/me")) {
			message = message = as_playercolor(chat_message.playern, (
						(boost::format("-&gt; %s") % (chat_message.sender.empty() ? "***" : sender_escaped)).str()));
			message = g_gr->styles().font_style(StyleManager::FontStyle::kChatMessage).as_font_tag(
						(boost::format("%s %s") % message % sanitized.substr(3)).str());
		} else if (chat_message.sender.size()) {
			const std::string sender_formatted = as_playercolor(chat_message.playern, (boost::format("%s:") % sender_escaped).str());
			message =
			   (boost::format("%s %s") %
			    sender_formatted % g_gr->styles().font_style(StyleManager::FontStyle::kChatMessage).as_font_tag(sanitized))
			      .str();
		} else {
			message = g_gr->styles().font_style(StyleManager::FontStyle::kChatServer).as_font_tag("*** " + sanitized);
		}
	}

	// Time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M]", localtime(&chat_message.time));

	return (boost::format("<p>%s %s</p>") % g_gr->styles().font_style(StyleManager::FontStyle::kChatTimestamp).as_font_tag(ts) % message).str();
}

std::string sanitize_message(const ChatMessage& chat_message) {
	// Escape richtext characters
	// The goal of this code is two-fold:
	//  1. Assuming an honest game host, we want to prevent the ability of
	//     clients to use richtext.
	//  2. Assuming a malicious host or meta server, we want to reduce the
	//     likelihood that a bug in the richtext renderer can be exploited,
	//     by restricting the set of allowed richtext commands.
	//     Most notably, images are not allowed in richtext at all.
	//
	// Note that we do want host and meta server to send some richtext code,
	// as the ability to send formatted commands is nice for the usability
	// of meta server so we're treading a bit of a fine line here.

	if (chat_message.playern >= 0) {
		return richtext_escape(chat_message.msg);
	}

	std::string sanitized;
	for (std::string::size_type pos = 0; pos < chat_message.msg.size(); ++pos) {
		if (chat_message.msg[pos] == '<') {
			static const std::string good1 = "</p><p";
			static const std::string good2 = "<br>";
			if (!chat_message.msg.compare(pos, good1.size(), good1)) {
				// TODO(MiroslavR): The logic here seems flawed.
				std::string::size_type nextclose = chat_message.msg.find('>', pos + good1.size());
				if (nextclose != std::string::npos &&
				    (nextclose == pos + good1.size() || chat_message.msg[pos + good1.size()] == ' ')) {
					sanitized += good1;
					pos += good1.size() - 1;
					continue;
				}
			} else if (!chat_message.msg.compare(pos, good2.size(), good2)) {
				sanitized += good2;
				pos += good2.size() - 1;
				continue;
			}

			sanitized += "&lt;";
		} else {
			sanitized += chat_message.msg[pos];
		}
	}
	return sanitized;
}
