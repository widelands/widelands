/*
 * Copyright (C) 2008-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_GAME_CHAT_PANEL_H
#define WL_WUI_GAME_CHAT_PANEL_H

#include <memory>

#include "chat/chat.h"
#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textinput.h"

/**
 * Provides a panel that contains chat message scrollbar and a chat message
 * entry field.
 */
struct GameChatPanel : public UI::Panel {
	GameChatPanel(UI::Panel*,
	              ChatColorForPlayer fn,
	              int32_t x,
	              int32_t y,
	              uint32_t w,
	              uint32_t h,
	              ChatProvider&,
	              UI::PanelStyle style);

	// Signal is called when a message has been sent by the user.
	Notifications::Signal<> sent;

	// Signal is called when the user has aborted entering a message.
	Notifications::Signal<> aborted;

	const std::string& get_edit_text() const {
		return editbox.get_text();
	}
	void set_edit_text(const std::string& text) {
		editbox.set_text(text);
	}

	bool handle_key(bool down, SDL_Keysym code) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	void focus_edit();
	void unfocus_edit();

private:
	void layout() override;
	void recalculate(bool has_new_message = false);
	void key_enter();
	void key_escape();
	void draw(RenderTarget& dst) override;
	void set_recipient();
	void prepare_recipients();
	bool select_recipient();
	void key_changed();
	bool try_autocomplete();

	ChatColorForPlayer color_functor_;
	ChatProvider& chat_;
	UI::Box vbox_;
	UI::MultilineTextarea chatbox;
	UI::Box hbox_;
	UI::Dropdown<std::string> recipient_dropdown_;
	UI::EditBox editbox;
	size_t chat_message_counter{0U};
	FxId chat_sound;
	bool has_team_{false};
	std::unique_ptr<Notifications::Subscriber<ChatMessage>> chat_message_subscriber_;
	std::unique_ptr<Notifications::Signal<>::SignalSubscriber> update_signal_connection_;
};

#endif  // end of include guard: WL_WUI_GAME_CHAT_PANEL_H
