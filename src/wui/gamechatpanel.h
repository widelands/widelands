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

#ifndef GAMECHATPANEL_H
#define GAMECHATPANEL_H

#include "chat.h"

#include "ui_editbox.h"
#include "ui_multilinetextarea.h"


struct ChatProvider;

/**
 * Provides a panel that contains chat message scrollbar and a chat message
 * entry field.
 */
struct GameChatPanel : public UI::Panel,
		public Widelands::NoteReceiver<ChatMessage>
{
	GameChatPanel
		(UI::Panel    *,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 ChatProvider &);

	/**
	 * Signal is called when a message has been sent by the user.
	 */
	UI::Signal sent;

	/**
	 * Signal is called when the user has aborted entering a message.
	 */
	UI::Signal aborted;

	void focusEdit();

	void receive(ChatMessage const &);

private:
	void recalculate();
	void keyEnter();
	void keyEscape();

	ChatProvider & m_chat;
	UI::Multiline_Textarea chatbox;
	UI::EditBox editbox;
};

#endif
