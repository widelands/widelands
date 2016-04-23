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

#include "wui/chat_msg_layout.h"

/**
 * Create a game chat panel
 */
GameChatPanel::GameChatPanel
	(UI::Panel * parent,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 ChatProvider & chat)
	:
	UI::Panel(parent, x, y, w, h),
	chat_   (chat),
	chatbox  (this, 0, 0, w, h - 25, "", UI::Align::kLeft,
				 UI::MultilineTextarea::ScrollMode::kScrollLogForced),
	editbox  (this, 0, h - 20, w, 20, 4),
	chat_message_counter(std::numeric_limits<uint32_t>::max())
{
	editbox.ok.connect(boost::bind(&GameChatPanel::key_enter, this));
	editbox.cancel.connect(boost::bind(&GameChatPanel::key_escape, this));
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
	const std::vector<ChatMessage> msgs = chat_.get_messages();

	std::string str = "<rt>";
	for (uint32_t i = 0; i < msgs.size(); ++i) {
		str += format_as_old_richtext(msgs[i]);
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
		if (!msgs.back().sender.empty() && !chat_.sound_off())
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
void GameChatPanel::focus_edit()
{
	editbox.focus();
}

void GameChatPanel::key_enter()
{
	const std::string & str = editbox.text();

	if (str.size())
		chat_.send(str);

	editbox.set_text("");
	sent();
}

void GameChatPanel::key_escape()
{
	editbox.set_text("");
	aborted();
}
