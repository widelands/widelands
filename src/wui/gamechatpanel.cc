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
	chat_message_counter(0)
{
	chatbox.set_scrollmode(UI::Multiline_Textarea::ScrollLog);
	editbox.ok.connect(boost::bind(&GameChatPanel::keyEnter, this));
	editbox.cancel.connect(boost::bind(&GameChatPanel::keyEscape, this));
	editbox.setAlign(UI::Align_Left);
	editbox.activate_history(true);

	set_handle_mouse(true);
	set_can_focus(true);

	connect(m_chat);
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
		// FIXME use toPrintable() when old renderer is kicked out
		str += msgs[i].toOldRichText();
		str += '\n';
	}
	str += "</rt>";

	chatbox.set_text(str);

	// If there are new messages, play a sound
	if (msgs.size() > chat_message_counter)
	{
		// computer generated ones are ignored
		// Note: if many messages arrive simultaneously,
		// the latest is a system message and some others
		// are not, then this act wrong!
		if (!msgs.back().sender.empty() && !m_chat.sound_off())
			// The latest message is not a system message
			play_new_chat_message();
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

void GameChatPanel::receive(const ChatMessage &)
{
	recalculate();
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
