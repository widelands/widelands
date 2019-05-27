/*
 * Copyright (C) 2008-2019 by the Widelands Development Team
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

#include "wui/game_chat_panel.h"

#include <limits>
#include <string>

#include "sound/sound_handler.h"
#include "wui/chat_msg_layout.h"

/**
 * Create a game chat panel
 */
GameChatPanel::GameChatPanel(UI::Panel* parent,
                             int32_t const x,
                             int32_t const y,
                             uint32_t const w,
                             uint32_t const h,
                             ChatProvider& chat,
                             UI::PanelStyle style)
   : UI::Panel(parent, x, y, w, h),
     chat_(chat),
     chatbox(this,
             0,
             0,
             w,
             h - 30,
             style,
             "",
             UI::Align::kLeft,
             UI::MultilineTextarea::ScrollMode::kScrollLogForced),
     editbox(this, 0, h - 25, w, style),
     chat_message_counter(0),
     chat_sound(SoundHandler::register_fx(SoundType::kChat, "sound/lobby_chat")) {

	editbox.ok.connect(boost::bind(&GameChatPanel::key_enter, this));
	editbox.cancel.connect(boost::bind(&GameChatPanel::key_escape, this));
	editbox.activate_history(true);

	set_handle_mouse(true);
	set_can_focus(true);

	chat_message_subscriber_ =
	   Notifications::subscribe<ChatMessage>([this](const ChatMessage&) { recalculate(true); });
	recalculate();
}

/**
 * Updates the chat message area.
 */
void GameChatPanel::recalculate(bool has_new_message) {
	const std::vector<ChatMessage> msgs = chat_.get_messages();

	const size_t msgs_size = msgs.size();
	std::string str = "<rt>";
	for (uint32_t i = 0; i < msgs_size; ++i) {
		str += format_as_richtext(msgs[i]);
	}
	str += "</rt>";

	chatbox.set_text(str);

	// Play a sound if there is a new non-system message
	if (!chat_.sound_off() && has_new_message) {
		for (size_t i = chat_message_counter; i < msgs_size; ++i) {
			if (!msgs[i].sender.empty()) {
				// Got a message that is no system message. Beep
				g_sh->play_fx(SoundType::kChat, chat_sound);
				break;
			}
		}
	}
	chat_message_counter = msgs_size;
}

/**
 * Put the focus on the message input panel.
 */
void GameChatPanel::focus_edit() {
	editbox.set_can_focus(true);
	editbox.focus();
}

/**
 * Remove the focus from the message input panel.
 */
void GameChatPanel::unfocus_edit() {
	editbox.set_can_focus(false);
}

void GameChatPanel::key_enter() {
	const std::string& str = editbox.text();

	if (str.size())
		chat_.send(str);

	editbox.set_text("");
	sent();
}

void GameChatPanel::key_escape() {
	editbox.set_text("");
	aborted();
}

/**
 * The mouse was clicked on this chatbox
 */
bool GameChatPanel::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus_edit();
		return true;
	}

	return false;
}
