/*
 * Copyright (C) 2008, 2011, 2013 by the Widelands Development Team
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

#include "wui/gamechatpanel.h"

#include <limits>
#include <string>

namespace  {

std::string to_old_richtext(const ChatMessage& chat_message) {
	std::string message = "<p font-color=#33ff33 font-size=9>";

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
	// of meta server and dedicated servers, so we're treading a bit of a
	// fine line here.
	std::string sanitized;
	for (std::string::size_type pos = 0; pos < chat_message.msg.size(); ++pos) {
		if (chat_message.msg[pos] == '<') {
			if (chat_message.playern < 0) {
				static const std::string good1 = "</p><p";
				static const std::string good2 = "<br>";
				if (!chat_message.msg.compare(pos, good1.size(), good1)) {
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
			}

			sanitized += "&lt;";
		} else {
			sanitized += chat_message.msg[pos];
		}
	}

	// time calculation
	char ts[13];
	strftime(ts, sizeof(ts), "[%H:%M] </p>", localtime(&chat_message.time));
	message += ts;

	message += "<p font-size=14 font-face=DejaVuSerif font-color=#";
	message += chat_message.color();

	if (chat_message.recipient.size() && chat_message.sender.size()) {
		// Personal message handling
		if (sanitized.compare(0, 3, "/me")) {
			message += " font-decoration=underline>";
			message += chat_message.sender;
			message += " @ ";
			message += chat_message.recipient;
			message += ":</p><p font-size=14 font-face=DejaVuSerif> ";
			message += sanitized;
		} else {
			message += ">@";
			message += chat_message.recipient;
			message += " >> </p><p font-size=14";
			message += " font-face=DejaVuSerif font-color=#";
			message += chat_message.color();
			message += " font-style=italic> ";
			message += chat_message.sender;
			message += sanitized.substr(3);
		}
	} else {
		// Normal messages handling
		if (not sanitized.compare(0, 3, "/me")) {
			message += " font-style=italic>-> ";
			if (chat_message.sender.size())
				message += chat_message.sender;
			else
				message += "***";
			message += sanitized.substr(3);
		} else if (chat_message.sender.size()) {
			message += " font-decoration=underline>";
			message += chat_message.sender;
			message += ":</p><p font-size=14 font-face=DejaVuSerif> ";
			message += sanitized;
		} else {
			message += " font-weight=bold>*** ";
			message += sanitized;
		}
	}

	// return the formated message
	return message + "<br></p>";
}

}  // namespace

/**
 * Create a game chat panel
 */
GameChatPanel::GameChatPanel
	(UI::Panel * parent,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 ChatProvider & chat)
	:
	UI::Panel(parent, x, y, w, h),
	m_chat   (chat),
	chatbox  (this, 0, 0, w, h - 25, "", UI::Align_Left, 1),
	editbox  (this, 0, h - 20, w,  20),
	chat_message_counter(std::numeric_limits<uint32_t>::max())
{
	chatbox.set_scrollmode(UI::Multiline_Textarea::ScrollLog);
	editbox.ok.connect(boost::bind(&GameChatPanel::keyEnter, this));
	editbox.cancel.connect(boost::bind(&GameChatPanel::keyEscape, this));
	editbox.setAlign(UI::Align_Left);
	editbox.activate_history(true);

	set_handle_mouse(true);
	set_can_focus(true);

	chat_message_subscriber_ =
	   Notifications::subscribe<ChatMessage>([this](const ChatMessage&) {recalculate();});
	recalculate();
}

/**
 * Updates the chat message area.
 */
void GameChatPanel::recalculate()
{
	const std::vector<ChatMessage> msgs = m_chat.getMessages();

	std::string str = "<rt>";
	for (uint32_t i = 0; i < msgs.size(); ++i) {
		// FIXME use the method from chatoverlay.cc when old renderer is kicked out
		str += to_old_richtext(msgs[i]);
		str += '\n';
	}
	str += "</rt>";

	chatbox.set_text(str);

	// If there are new messages, play a sound
	if (0 < msgs.size() && msgs.size() != chat_message_counter)
	{
		// computer generated ones are ignored
		// Note: if many messages arrive simultaneously,
		// the latest is a system message and some others
		// are not, then this act wrong!
		if (!msgs.back().sender.empty() && !m_chat.sound_off())
		{
			// The latest message is not a system message
			if (std::string::npos == msgs.back().sender.find("(IRC)") && chat_message_counter < msgs.size())
			{
				// The latest message was not relayed from IRC.
				// The above built-in string constant should match
				// that of the IRC bridge.
				play_new_chat_message();
			}
		}
		chat_message_counter = msgs . size();

	}
}

/**
 * Put the focus on the message input panel.
 */
void GameChatPanel::focusEdit()
{
	editbox.focus();
}

void GameChatPanel::keyEnter()
{
	const std::string & str = editbox.text();

	if (str.size())
		m_chat.send(str);

	editbox.setText("");
	sent();
}

void GameChatPanel::keyEscape()
{
	editbox.setText("");
	aborted();
}
