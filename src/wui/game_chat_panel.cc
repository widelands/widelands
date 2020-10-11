/*
 * Copyright (C) 2008-2020 by the Widelands Development Team
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

#include <SDL_mouse.h>

#include "sound/sound_handler.h"
#include "ui_basic/mouse_constants.h"
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
     box_(this, 0, 0, UI::Box::Vertical),
     chatbox(&box_,
             0,
             0,
             0,
             0,
             style,
             "",
             UI::Align::kLeft,
             UI::MultilineTextarea::ScrollMode::kScrollLog),
     editbox(this, 0, 0, w, style),
     chat_message_counter(0),
     chat_sound(SoundHandler::register_fx(SoundType::kChat, "sound/lobby_chat")) {

	box_.add(&chatbox, UI::Box::Resizing::kExpandBoth);
	box_.add_space(4);
	box_.add(&editbox, UI::Box::Resizing::kFullSize);

	editbox.ok.connect([this]() { key_enter(); });
	editbox.cancel.connect([this]() { key_escape(); });
	editbox.activate_history(true);

	set_handle_mouse(true);
	set_can_focus(true);

	chat_message_subscriber_ =
	   Notifications::subscribe<ChatMessage>([this](const ChatMessage&) { recalculate(true); });
	recalculate(true);
	layout();
}

void GameChatPanel::layout() {
	box_.set_size(get_inner_w(), get_inner_h());
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

	if (str.size()) {
		chat_.send(str);
	}

	editbox.set_text("");
	sent();
}

void GameChatPanel::key_escape() {
	if (editbox.text().empty()) {
		unfocus_edit();
	}
	editbox.set_text("");
	aborted();
}

/**
 * The mouse was clicked on this chatbox
 */
bool GameChatPanel::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus_edit();
		clicked();
		return true;
	}

	return false;
}
void GameChatPanel::draw(RenderTarget& dst) {
	dst.brighten_rect(Recti(chatbox.get_x(), chatbox.get_y(), chatbox.get_w(), chatbox.get_h()),
	                  -MOUSE_OVER_BRIGHT_FACTOR);
}
