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

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "network/participantlist.h"
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
		vbox_(this, 0, 0, UI::Box::Vertical),
		chatbox(&vbox_,
			0,
			0,
			0,
			0,
			style,
			"",
			UI::Align::kLeft,
			UI::MultilineTextarea::ScrollMode::kScrollLog),
		hbox_(&vbox_, 0, 0, UI::Box::Horizontal),
		recipient_dropdown_(&hbox_,
			"chat_recipient_dropdown",
			0,
			h - 25,
			25,
			16,
			25,
			_("Recipient"),
			UI::DropdownType::kPictorial,
			UI::PanelStyle::kFsMenu,
			UI::ButtonStyle::kFsMenuSecondary),
		editbox(&hbox_, 28, 0, w - 28, style),
		chat_message_counter(0),
		chat_sound(SoundHandler::register_fx(SoundType::kChat, "sound/lobby_chat")),
		has_team_(false) {

	vbox_.add(&chatbox, UI::Box::Resizing::kExpandBoth);
	vbox_.add_space(4);
	vbox_.add(&hbox_, UI::Box::Resizing::kFullSize);//, UI::Box::Resizing::kFullSize);

	editbox.ok.connect([this]() { key_enter(); });
	editbox.cancel.connect([this]() { key_escape(); });
	editbox.activate_history(true);

	set_handle_mouse(true);
	set_can_focus(true);

	if (chat_.participants_ == nullptr) {
		// No access to participant list. Hide the dropdown
		recipient_dropdown_.set_visible(false);
		// Increase the size of the edit box to fill the empty space
		editbox.set_pos(Vector2i(editbox.get_x() - 28, editbox.get_y()));
		editbox.set_size(editbox.get_w() + 28, editbox.get_h());
		editbox.set_text(chat_.last_recipient_);
	} else {
		// When an entry has been selected, update the "@playername " in the edit field
		recipient_dropdown_.selected.connect([this]() { set_recipient(); });
		// Fill the dropdown menu with usernames
		prepare_recipients();
		// Insert "@playername " into the edit field if the dropdown currently has a selection
		set_recipient();
		update_signal_connection = chat_.participants_->participants_updated.connect([this]() {
				// When the participants change, create new contents for dropdown
				prepare_recipients();
			});
		hbox_.add(&recipient_dropdown_, UI::Box::Resizing::kAlign);
		hbox_.add_space(4);

	hbox_.add(&editbox, UI::Box::Resizing::kFillSpace);

	chat_message_subscriber_ =
	   Notifications::subscribe<ChatMessage>([this](const ChatMessage&) { recalculate(true); });
	recalculate(true);
	layout();
}

void GameChatPanel::layout() {
	vbox_.set_size(get_inner_w(), get_inner_h());
}

GameChatPanel::~GameChatPanel() {
	if (chat_.participants_ != nullptr) {
		update_signal_connection.disconnect();
	}
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
 * Set the recipient in the input box to whatever is selected in the dropdown
 */
void GameChatPanel::set_recipient() {
	assert(chat_.participants_ != nullptr);
	assert(recipient_dropdown_.has_selection());

	// Replace the old recipient, if any

	const std::string& recipient = recipient_dropdown_.get_selected();
	std::string str = editbox.text();

	// We have a recipient already
	if (str[0] == '@') {
		size_t pos_first_space = str.find(' ');
		if (pos_first_space == std::string::npos) {
			// Its only the recipient in the input field (no space separating the message).
			// Replace it completely.
			// If we want to sent to @all, recipient is empty so we basically clear the input
			str = recipient;
		} else {
			// There is some message, so replace the old with the new (possibly empty) recipient
			// The separating space is already in recipient (see prepare_recipients())
			str.replace(0, pos_first_space + 1, recipient);
		}
	} else {
		// No recipient yet, prepend it
		str = recipient + str;
	}

	// Set the updated string
	editbox.set_text(str);
	// Something has been selected. Re-focus the input box
	editbox.focus();
}

/**
 * Prepare the entries for chat recipients in the dropdown box
 */
void GameChatPanel::prepare_recipients() {
	assert(chat_.participants_ != nullptr);

	recipient_dropdown_.clear();
	recipient_dropdown_.add(_("All"), "",
		g_image_cache->get("images/wui/menus/toggle_minimap.png"));
	// Select the "All" entry by default. Do *not* use the add() parameter for selecting it since
	// it calls the listener for selected()
	recipient_dropdown_.select("");
		recipient_dropdown_.add(_("Team"), "@team ",
			g_image_cache->get("images/wui/buildings/menu_list_workers.png"));

	// Iterate over all human players (except ourselves) and add their names
	const int16_t n_humans = chat_.participants_->get_participant_counts()[0];
	const std::string& local_name = chat_.participants_->get_local_playername();

	for (int16_t i = 0; i < n_humans; ++i) {
		assert(chat_.participants_->get_participant_type(i) != ParticipantList::ParticipantType::kAI);
		const std::string& name = chat_.participants_->get_participant_name(i);
		if (name == local_name) {
			continue;
		}

		if (chat_.participants_->get_participant_type(i)
			== ParticipantList::ParticipantType::kSpectator) {
			recipient_dropdown_.add(name, "@" + name + " ",
				g_image_cache->get("images/wui/fieldaction/menu_tab_watch.png"));
		} else {
			recipient_dropdown_.add(name, "@" + name + " ",
				playercolor_image(chat_.participants_->get_participant_color(i),
					"images/players/genstats_player.png"));
		}
	}
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
