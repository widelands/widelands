/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#ifndef WL_WUI_GAMECHATPANEL_H
#define WL_WUI_GAMECHATPANEL_H

#include <memory>

#include "chat/chat.h"
#include "ui_basic/box.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"

struct ChatProvider;

/**
 * Provides a panel that contains chat message scrollbar and a chat message
 * entry field.
 */
struct GameChatPanel : public UI::Panel {
	GameChatPanel(UI::Panel*, int32_t x, int32_t y, uint32_t w, uint32_t h, ChatProvider&);

	// Signal is called when a message has been sent by the user.
	boost::signals2::signal<void()> sent;

	// Signal is called when the user has aborted entering a message.
	boost::signals2::signal<void()> aborted;

	const std::string& get_edit_text() const {
		return editbox.text();
	}
	void set_edit_text(const std::string& text) {
		editbox.set_text(text);
	}

	void focus_edit();

private:
	void layout() override;
	void recalculate();
	void key_enter();
	void key_escape();

	ChatProvider& chat_;
	UI::Box box_;
	UI::MultilineTextarea chatbox;
	UI::EditBox editbox;
	uint32_t chat_message_counter;
	std::unique_ptr<Notifications::Subscriber<ChatMessage>> chat_message_subscriber_;
};

#endif  // end of include guard: WL_WUI_GAMECHATPANEL_H
