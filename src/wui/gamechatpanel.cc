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

#include "gamechatpanel.h"


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
	editbox  (this, 0, h - 20, w,  20)
{
	chatbox.set_scrollmode(UI::Multiline_Textarea::ScrollLog);
	editbox.ok.set(this, &GameChatPanel::keyEnter);
	editbox.cancel.set(this, &GameChatPanel::keyEscape);
	editbox.setAlign(UI::Align_Left);

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
		str += msgs[i].toPrintable();
		str += '\n';
	}
	str += "</rt>";

	chatbox.set_text(str);
}

/**
 * Put the focus on the message input panel.
 */
void GameChatPanel::focusEdit()
{
	editbox.focus();
}

void GameChatPanel::receive(ChatMessage const &)
{
	recalculate();
}

void GameChatPanel::keyEnter()
{
	std::string const & str = editbox.text();

	if (str.size())
		m_chat.send(str);

	editbox.setText("");
	sent.call();
}

void GameChatPanel::keyEscape()
{
	editbox.setText("");
	aborted.call();
}
