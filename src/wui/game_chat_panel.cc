/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#include "wui/game_chat_panel.h"

#include <SDL_keycode.h>
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
   : UI::Panel(parent, style, x, y, w, h),
     chat_(chat),
     vbox_(this, style, 0, 0, UI::Box::Vertical),
     chatbox(&vbox_,
             0,
             0,
             0,
             0,
             style,
             "",
             UI::Align::kLeft,
             UI::MultilineTextarea::ScrollMode::kScrollLog),
     hbox_(&vbox_, style, 0, 0, UI::Box::Horizontal),
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
	vbox_.add(&hbox_, UI::Box::Resizing::kFullSize);

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
		// Other direction: If the input changed, update the dropdown
		// Also react to doubled space for autocompletition
		editbox.changed.connect([this]() { key_changed(); });
		// Figure out whether the local player has teammates
		has_team_ = chat_.participants_->needs_teamchat();
		// Fill the dropdown menu with usernames
		prepare_recipients();
		// In the dropdown, select the entry that was used last time the menu was open
		// If the recipient no longer exists, this will fail and "@all" will still be selected
		recipient_dropdown_.select(chat_.last_recipient_);
		// Insert "@playername " into the edit field if the dropdown currently has a selection
		set_recipient();
		update_signal_connection_ = chat_.participants_->participants_updated.subscribe([this]() {
			// When the participants change, create new contents for dropdown
			has_team_ = chat_.participants_->needs_teamchat();
			prepare_recipients();
			select_recipient();
		});
		hbox_.add(&recipient_dropdown_, UI::Box::Resizing::kAlign);
		hbox_.add_space(4);
	}

	hbox_.add(&editbox, UI::Box::Resizing::kFillSpace);

	chat_message_subscriber_ =
	   Notifications::subscribe<ChatMessage>([this](const ChatMessage&) { recalculate(true); });
	recalculate(false);
	layout();
}

void GameChatPanel::layout() {
	vbox_.set_size(get_inner_w(), get_inner_h());
}

/**
 * If Ctrl+Space is pressed, try to autocomplete the player name
 */
bool GameChatPanel::handle_key(const bool down, const SDL_Keysym code) {

	if (chat_.participants_ == nullptr) {
		// Nothing to do here
		return false;
	}

	if (!down || code.sym != SDLK_SPACE || !(SDL_GetModState() & KMOD_CTRL)) {
		return false;
	}

	// User is pressing Ctrl+Space, try autocomplete

	if (editbox.text().empty()) {
		return false;
	}

	// Try autocomplete. If it worked, we handled the key press
	return try_autocomplete();
}

/**
 * If doubled space is entered pressed, try to autocomplete the player name.
 * Also keep dropdown selection in sync with entered "@name" text
 */
void GameChatPanel::key_changed() {
	if (chat_.participants_ == nullptr) {
		// Nothing to do here
		return;
	}
	// Check if last entered two characters are space
	std::string str = editbox.text();
	const size_t cursor_pos = editbox.caret_pos();
	if (cursor_pos < 2 || str[cursor_pos - 1] != ' ' || str[cursor_pos - 2] != ' ') {
		// Update state of dropdown, we might have no valid recipient now
		select_recipient();
		return;
	}
	assert(!editbox.text().empty());
	// Go back two chars since try_autocomplete() assumes that the cursor is in/at the name part
	editbox.set_caret_pos(cursor_pos - 2);
	if (!try_autocomplete()) {
		// Set the cursor back
		editbox.set_caret_pos(cursor_pos);
		select_recipient();
	} else {
		// It worked and we replaced something. Remove the two spaces
		str = editbox.text();
		const size_t end_of_replacement = editbox.caret_pos();
		str.erase(end_of_replacement, 2);
		editbox.set_text(str);
	}
}

/**
 * Try autocompletition of player names around the cursor position.
 * E.g., if input is "pl" complete to "playername " if the start of the name is unique
 */
bool GameChatPanel::try_autocomplete() {

	// Extract the name to complete
	// rfind starts at the given pos and goes forward until it finds a space
	std::string str = editbox.text();
	size_t namepart_start = str.rfind(' ', (editbox.caret_pos() > 1 ? editbox.caret_pos() - 1 : 0));
	if (namepart_start == std::string::npos) {
		// Not found, meaning the input only contains the name
		namepart_start = 0;
	} else {
		// Found something. Cut off the space
		++namepart_start;
	}
	if (namepart_start == 0 && str[0] == '@') {
		// If we are at the beginning of the line and the first char is @,
		// assume it is the whisper-sign and ignore the @
		++namepart_start;
	}
	size_t namepart_end = str.find(' ', editbox.caret_pos());

	if (namepart_end <= namepart_start || namepart_start == str.size()) {
		// Nothing to complete
		// Maybe pressed it on an empty field or a single space or at the end of the input
		return false;
	}

	// Extract part, also remove trailing spaces
	const std::string namepart = str.substr(namepart_start, namepart_end - namepart_start);

	// Helper function: Count the number of equal chars ignoring case
	static const auto count_equal_chars = [](const std::string& a, const std::string& b) {
		const size_t len = std::min(a.size(), b.size());
		for (size_t i = 0; i < len; ++i) {
			if (std::tolower(a[i]) != std::tolower(b[i])) {
				return i;
			}
		}
		return len;
	};

	// Helper function: Compare the given names and extract a common prefix (if existing)
	// Note: Do *not* use the capture clause. Since the lambda is static, it will still refer
	// to the old variables which might no longer be valid
	static const auto compare_names = [](const std::string& my_namepart, std::string& my_candidate,
	                                     const std::string& my_name) {
		size_t n_equal_chars = count_equal_chars(my_namepart, my_name);
		if (n_equal_chars == my_namepart.size()) {
			// We have a candidate!
			// Check if we already have a candidate. If not, use this one
			if (my_candidate.empty()) {
				// Append a space so the user can continue typing after the completition
				my_candidate = my_name + ' ';
			} else {
				// We already have one. Create an new candidate that is the combination of the two
				n_equal_chars = count_equal_chars(my_candidate, my_name);
				// No space appended here since the name is not complete yet
				my_candidate = my_candidate.substr(0, n_equal_chars);
			}
		}
	};

	// Iterate over all possible completitions (i.e., usernames).
	// For each, check whether they start with $namepart. If it does, store as candidate.
	// If there already is a candidate, create a new candidate from the part that is
	// the same for both candidates. (Note that the merged candidate might have wrong case,
	// but that is fixed on the next completition)
	const int16_t n_humans = chat_.participants_->get_participant_counts().humans;
	const std::string& local_name = chat_.participants_->get_local_playername();
	std::string candidate;
	for (int16_t i = 0; i < n_humans; ++i) {
		assert(chat_.participants_->get_participant_type(i) != ParticipantList::ParticipantType::kAI);
		const std::string& name = chat_.participants_->get_participant_name(i);
		if (namepart_start == 1 && str[0] == '@' && name == local_name) {
			// Don't autocomplete to our own username when searching for a recipient
			// Still do the autocomplete when in the middle of the message
			continue;
		}
		compare_names(namepart, candidate, name);
	}
	// Also offer to complete to "@team" but only when at the beginning of the input
	if (has_team_ && namepart_start == 1 && str[0] == '@') {
		compare_names(namepart, candidate, "team");
	}

	// If we have no candidate, abort
	if (candidate.empty()) {
		return false;
	}

	// We have a candidate, set it in the input box

	if (namepart == candidate || (candidate.back() == ' ' && namepart + " " == candidate)) {
		// Nothing would change, so abort
		return false;
	}

	str.replace(namepart_start, namepart_end - namepart_start, candidate);
	editbox.set_text(str);
	// Move the cursor behind the completed name
	editbox.set_caret_pos(namepart_start + candidate.size());
	// Try to select the matching dropdown state again
	select_recipient();
	return true;
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
	if (!str.empty()) {
		if (chat_.participants_ != nullptr) {
			const size_t pos_first_space = str.find(' ');

			std::string recipient;
			// Reset message to only the recipient
			if (str[0] == '@') {
				// When pos+1 is greater than the string length, the whole string is returned
				recipient = str.substr(0, pos_first_space + 1);
			}

			// Make sure we have a chat message to send and it is more than just the recipient
			// Either it has no recipient or there is text behind the recipient
			if (str[0] != '@' || pos_first_space < str.size() - 1) {
				chat_.send(str);
				chat_.last_recipient_ = recipient;
			}

			editbox.set_text(recipient);
			// Set selection of dropdown to entered recipient, if possible
			recipient_dropdown_.select(recipient);
			if (recipient_dropdown_.get_selected() != recipient) {
				// Seems the user is writing to someone unknown
				recipient_dropdown_.set_errored(_("Unknown Recipient"));
			}
		} else {
			// We don't have access to the player list, so just send the message
			chat_.send(str);
			editbox.set_text("");
		}
	}
	// Trigger signal that a message was sent
	sent();
}

void GameChatPanel::key_escape() {
	if (editbox.text().empty()) {
		unfocus_edit();
	}
	editbox.set_text("");
	// Re-set the current selection to clean up a possible error state
	if (chat_.participants_ != nullptr) {
		recipient_dropdown_.select(chat_.last_recipient_);
		set_recipient();
	}
	// Trigger the signal that writing was aborted
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
	recipient_dropdown_.add(_("All"), "", g_image_cache->get("images/wui/menus/toggle_minimap.png"));
	// Select the "All" entry by default. Do *not* use the add() parameter for selecting it since
	// it calls the listener for selected()
	recipient_dropdown_.select("");
	if (has_team_) {
		recipient_dropdown_.add(
		   _("Team"), "@team ", g_image_cache->get("images/wui/buildings/menu_list_workers.png"));
	}

	// Iterate over all human players (except ourselves) and add their names
	const int16_t n_humans = chat_.participants_->get_participant_counts().humans;
	const std::string& local_name = chat_.participants_->get_local_playername();

	for (int16_t i = 0; i < n_humans; ++i) {
		assert(chat_.participants_->get_participant_type(i) != ParticipantList::ParticipantType::kAI);
		const std::string& name = chat_.participants_->get_participant_name(i);
		if (name == local_name) {
			continue;
		}

		if (chat_.participants_->get_participant_type(i) ==
		    ParticipantList::ParticipantType::kSpectator) {
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
 * Tries to select the recipient entered in the input field in the dropdown
 * Sets errored-state otherwise. Returns whether recipient could be selected
 */
bool GameChatPanel::select_recipient() {
	// Get the current recipient
	std::string recipient;
	const std::string& text = editbox.text();
	if (!text.empty() && text[0] == '@') {
		// Get the recipient string including the first space
		// If there is no space, return the whole string
		const size_t pos_space = text.find(' ');
		if (pos_space != std::string::npos) {
			recipient = text.substr(0, pos_space + 1);
		} else {
			// Append a space since that increases the chance
			// to get a match in the dropdown
			recipient = text + ' ';
		}
	}
	// Try to re-set the recipient
	recipient_dropdown_.select(recipient);
	if (recipient != recipient_dropdown_.get_selected()) {
		recipient_dropdown_.set_errored(_("Unknown Recipient"));
		return false;
	}
	return true;
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
