/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "wui/game_message_menu.h"

#include "base/time_string.h"
#include "base/wexception.h"
#include "graphic/text_layout.h"
#include "logic/game_data_error.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"

using Widelands::Message;
using Widelands::MessageId;
using Widelands::MessageQueue;

inline InteractivePlayer& GameMessageMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

constexpr int kWindowWidth = 355;
constexpr int kWindowHeight = 375;
constexpr int kTableHeight = 125;
constexpr int kPadding = 5;
constexpr int kButtonSize = 34;
constexpr int kMessageBodyY = kButtonSize + 3 * kPadding + kTableHeight;

GameMessageMenu::GameMessageMenu(InteractivePlayer& plr, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&plr,
                      UI::WindowStyle::kWui,
                      "messages",
                      &registry,
                      kWindowWidth,
                      kWindowHeight,
                      _("Messages: Inbox")),
     message_body(this,
                  kPadding,
                  kMessageBodyY,
                  kWindowWidth - 2 * kPadding,
                  get_inner_h() - kMessageBodyY - 2 * kPadding - kButtonSize,
                  UI::PanelStyle::kWui,
                  "",
                  UI::Align::kLeft,
                  UI::MultilineTextarea::ScrollMode::kScrollNormalForced),
     mode(Mode::kInbox) {

	list = new UI::Table<uintptr_t>(this, kPadding, kButtonSize + 2 * kPadding,
	                                kWindowWidth - 2 * kPadding, kTableHeight, UI::PanelStyle::kWui,
	                                UI::TableRows::kMulti);
	list->add_column(kWindowWidth - 2 * kPadding - 60 - 60 - 75, _("Title"));
	list->add_column(60, pgettext("message", "Type"), "", UI::Align::kCenter);
	list->add_column(60, _("Status"), "", UI::Align::kCenter);
	/** TRANSLATORS: We have very little space here. You can also translate this as "Time" or "Time
	 * Sent" */
	/** TRANSLATORS: This is used in the game messages menu - please open an issue if you need more
	 * space. */
	list->add_column(75, pgettext("message", "Sent"), "", UI::Align::kRight);
	list->focus();

	// Buttons for message types
	geologistsbtn_ = new UI::Button(this, "filter_geologists_messages", kPadding, kPadding,
	                                kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
	                                g_image_cache->get("images/wui/fieldaction/menu_geologist.png"));

	economybtn_ = new UI::Button(this, "filter_economy_messages", 2 * kPadding + kButtonSize,
	                             kPadding, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
	                             g_image_cache->get("images/wui/stats/genstats_nrwares.png"));

	seafaringbtn_ =
	   new UI::Button(this, "filter_seafaring_messages", 3 * kPadding + 2 * kButtonSize, kPadding,
	                  kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
	                  g_image_cache->get("images/wui/buildings/start_expedition.png"));

	warfarebtn_ = new UI::Button(this, "filter_warfare_messages", 4 * kPadding + 3 * kButtonSize,
	                             kPadding, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
	                             g_image_cache->get("images/wui/messages/messages_warfare.png"));

	scenariobtn_ = new UI::Button(this, "filter_scenario_messages", 5 * kPadding + 4 * kButtonSize,
	                              kPadding, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiSecondary,
	                              g_image_cache->get("images/wui/menus/objectives.png"));

	message_filter_ = Widelands::Message::Type::kAllMessages;
	set_filter_messages_tooltips();
	// End: Buttons for message types

	archivebtn_ = new UI::Button(this, "archive_or_restore_selected_messages", kPadding,
	                             kWindowHeight - kPadding - kButtonSize, kButtonSize, kButtonSize,
	                             UI::ButtonStyle::kWuiPrimary,
	                             g_image_cache->get("images/wui/messages/message_archive.png"));
	update_archive_button_tooltip();

	togglemodebtn_ = new UI::Button(
	   this, "toggle_between_inbox_or_archive",
	   archivebtn_->get_x() + archivebtn_->get_w() + kPadding, archivebtn_->get_y(), kButtonSize,
	   kButtonSize, UI::ButtonStyle::kWuiPrimary,
	   g_image_cache->get("images/wui/messages/message_archived.png"), _("Show Archive"));

	centerviewbtn_ = new UI::Button(
	   this, "center_main_mapview_on_location", kWindowWidth - kPadding - kButtonSize,
	   archivebtn_->get_y(), kButtonSize, kButtonSize, UI::ButtonStyle::kWuiPrimary,
	   g_image_cache->get("images/wui/menus/goto.png"),
	   as_tooltip_text_with_hotkey(
	      /** TRANSLATORS: Tooltip in the messages window */
	      _("Center main mapview on location"),
	      shortcut_string_for(KeyboardShortcut::kInGameMessagesGoto), UI::PanelStyle::kWui));
	centerviewbtn_->set_enabled(false);

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	list->set_column_compare(
	   ColTitle, [this](uint32_t a, uint32_t b) { return compare_title(a, b); });
	list->set_column_compare(
	   ColStatus, [this](uint32_t a, uint32_t b) { return compare_status(a, b); });
	list->set_column_compare(ColType, [this](uint32_t a, uint32_t b) { return compare_type(a, b); });
	list->set_column_compare(
	   ColTimeSent, [this](uint32_t a, uint32_t b) { return compare_time_sent(a, b); });

	list->set_sort_column(ColTimeSent);
	list->layout();

	list->focus();

	geologistsbtn_->sigclicked.connect(
	   [this]() { filter_messages(Widelands::Message::Type::kGeologists); });
	economybtn_->sigclicked.connect(
	   [this]() { filter_messages(Widelands::Message::Type::kEconomy); });
	seafaringbtn_->sigclicked.connect(
	   [this]() { filter_messages(Widelands::Message::Type::kSeafaring); });
	warfarebtn_->sigclicked.connect(
	   [this]() { filter_messages(Widelands::Message::Type::kWarfare); });
	scenariobtn_->sigclicked.connect(
	   [this]() { filter_messages(Widelands::Message::Type::kScenario); });
	list->selected.connect([this](uint32_t a) { selected(a); });
	list->double_clicked.connect([this](uint32_t a) { double_clicked(a); });
	archivebtn_->sigclicked.connect([this]() { archive_or_restore(); });
	togglemodebtn_->sigclicked.connect([this]() { toggle_mode(); });
	centerviewbtn_->sigclicked.connect([this]() { center_view(); });

	initialization_complete();
}

/**
 * When comparing messages by title, order is alphabetical.
 * If both are identical, sort by time sent.
 */
bool GameMessageMenu::compare_title(uint32_t a, uint32_t b) {
	const MessageQueue& mq = iplayer().player().messages();
	const Message* msga = mq[MessageId((*list)[a])];
	const Message* msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		if (msga->title() == msgb->title()) {
			return compare_time_sent(a, b);
		}
		return msga->title() < msgb->title();
	}
	return false;  // shouldn't happen
}

/**
 * When comparing messages by status, new messages come before others.
 * If both are identical, sort by time sent.
 */
bool GameMessageMenu::compare_status(uint32_t a, uint32_t b) {
	const MessageQueue& mq = iplayer().player().messages();
	const Message* msga = mq[MessageId((*list)[a])];
	const Message* msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		if (msga->status() == msgb->status()) {
			return compare_time_sent(a, b);
		}
		return msga->status() == Message::Status::kNew && msgb->status() != Message::Status::kNew;
	}
	return false;  // shouldn't happen
}

/**
 * When comparing messages by type, order is the same as in the enum class.
 * If both are identical, sort by time sent.
 */
bool GameMessageMenu::compare_type(uint32_t a, uint32_t b) {
	const MessageQueue& mq = iplayer().player().messages();
	const Message* msga = mq[MessageId((*list)[a])];
	const Message* msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		const Widelands::Message::Type cat_a = msga->message_type_category();
		const Widelands::Message::Type cat_b = msgb->message_type_category();
		if (cat_a == cat_b) {
			return compare_time_sent(a, b);
		}
		return static_cast<int>(cat_a) < static_cast<int>(cat_b);
	}
	return false;  // shouldn't happen
}

/**
 * When comparing messages by time sent, older messages come before others.
 */
bool GameMessageMenu::compare_time_sent(uint32_t a, uint32_t b) {
	const MessageQueue& mq = iplayer().player().messages();
	const Message* msga = mq[MessageId((*list)[a])];
	const Message* msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		return msga->sent() > msgb->sent();
	}
	return false;  // shouldn't happen
}

bool GameMessageMenu::should_be_hidden(const Widelands::Message& message) {
	// Wrong box
	return ((mode == Mode::kArchive) != (message.status() == Message::Status::kArchived)) ||
	       // Filtered out
	       (message_filter_ != Message::Type::kAllMessages &&
	        message.message_type_category() != message_filter_);
}

static char const* const status_picture_filename[] = {"images/wui/messages/message_new.png",
                                                      "images/wui/messages/message_read.png",
                                                      "images/wui/messages/message_archived.png"};

void GameMessageMenu::show_new_message(MessageId const id, const Widelands::Message& message) {
	// Do not disturb the user while multiselecting.
	if (list->selections().size() > 1) {
		return;
	}

	assert(iplayer().player().messages()[id] == &message);
	Message::Status const status = message.status();
	if ((mode == Mode::kArchive) != (status == Message::Status::kArchived)) {
		toggle_mode();
	}

	if (auto* entry = list->find(id.value())) {
		const uintptr_t e = reinterpret_cast<uintptr_t>(entry->entry());
		for (uint32_t i = 0; i < list->size(); ++i) {
			if (e == (*list)[i]) {
				list->clear_selections();
				list->scroll_to_item(i);
				list->select(i);
				return;
			}
		}
		NEVER_HERE();
	}

	UI::Table<uintptr_t>::EntryRecord& te = list->add(id.value());
	update_record(te, message);
	list->sort();
	list->clear_selections();
	list->select(0);
	list->scroll_to_top();
}

void GameMessageMenu::think() {
	const MessageQueue& mq = iplayer().player().messages();
	size_t no_selections = list->selections().size();
	size_t list_size = list->size();

	// Update messages in the list and remove messages
	// that should no longer be shown
	uint32_t removed = 0;
	const auto& sel = list->selections();
	const uint32_t max_index = (sel.empty() ? 0 : *sel.rbegin());
	for (uint32_t j = list->size(); j; --j) {
		MessageId id_((*list)[j - 1]);
		if (Message const* const message = mq[id_]) {
			if (should_be_hidden(*message)) {
				removed++;
				list->remove(j - 1);
			} else {
				update_record(list->get_record(j - 1), *message);
			}
		} else {
			removed++;
			list->remove(j - 1);
		}
	}
	if (removed > 0) {
		// If something was removed, select entry below lowest removed entry
		uint32_t index = 0;
		if (removed <= max_index) {
			index = std::min(max_index - removed + 1, list->size() - 1);
		}
		list->select(index);
		list->scroll_to_item(index);
	}

	// Add new messages to the list
	for (const auto& temp_message : mq) {
		MessageId const id = temp_message.first;
		const Message& message = *temp_message.second;
		if (!should_be_hidden(message) && !list->find(id.value())) {
			UI::Table<uintptr_t>::EntryRecord& er = list->add(id.value());
			update_record(er, message);
			list->sort();
		}
	}

	if (!list->empty()) {
		if (!list->has_selection()) {
			list->select(0);
		}
	} else {
		centerviewbtn_->set_enabled(false);
		message_body.set_text(std::string());
	}

	if (list_size != list->size() || no_selections != list->selections().size()) {
		update_archive_button_tooltip();
	}
}

void GameMessageMenu::update_record(UI::Table<uintptr_t>::EntryRecord& er,
                                    const Widelands::Message& message) {
	er.set_picture(ColType, g_image_cache->get(display_message_type_icon(message)));
	er.set_picture(
	   ColStatus, g_image_cache->get(status_picture_filename[static_cast<int>(message.status())]));
	er.set_picture(ColTitle, message.icon(), message.title());

	er.set_string(ColTimeSent, gametimestring(message.sent().get()));
}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	const Widelands::Player& player = iplayer().player();
	const MessageQueue& mq = player.messages();
	if (t != UI::Table<uintptr_t>::no_selection_index()) {
		MessageId const id = MessageId((*list)[t]);
		if (Message const* const message = mq[id]) {
			//  Maybe the message was removed since think?
			if (message->status() == Message::Status::kNew) {
				Widelands::Game& game = iplayer().game();
				game.send_player_command(new Widelands::CmdMessageSetStatusRead(
				   game.get_gametime(), player.player_number(), id));
			}
			centerviewbtn_->set_enabled(bool(message->position()));
			message_body.set_text(as_message(message->heading(), message->body()));
			update_archive_button_tooltip();
			return;
		}
	}
	centerviewbtn_->set_enabled(false);
	message_body.set_text(std::string());
}

/**
 * a message was double clicked
 */
void GameMessageMenu::double_clicked(uint32_t const /* t */) {
	if (centerviewbtn_->enabled()) {
		center_view();
	}
}

/**
 * Handle message menu hotkeys.
 */
bool GameMessageMenu::handle_key(bool down, SDL_Keysym code) {
	// Special ESCAPE handling
	// When ESCAPE is pressed down is false
	if (code.sym == SDLK_ESCAPE) {
		return UI::Window::handle_key(true, code);
	}

	if (down) {
		if (matches_shortcut(KeyboardShortcut::kCommonDeleteItem, code)) {
			archive_or_restore();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesGoto, code)) {
			if (centerviewbtn_->enabled()) {
				center_view();
			}
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesFilterAll, code)) {
			filter_messages(Widelands::Message::Type::kAllMessages);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesFilterGeologists, code)) {
			filter_messages(Widelands::Message::Type::kGeologists);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesFilterEconomy, code)) {
			filter_messages(Widelands::Message::Type::kEconomy);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesFilterSeafaring, code)) {
			filter_messages(Widelands::Message::Type::kSeafaring);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesFilterWarfare, code)) {
			filter_messages(Widelands::Message::Type::kWarfare);
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kInGameMessagesFilterScenario, code)) {
			filter_messages(Widelands::Message::Type::kScenario);
			return true;
		}
	}

	return UI::Panel::handle_key(down, code);
}

void GameMessageMenu::archive_or_restore() {
	if (!list->has_selection()) {
		return;
	}
	Widelands::Game& game = iplayer().game();
	const Widelands::PlayerNumber plnum = iplayer().player().player_number();

	std::set<uint32_t> selections = list->selections();
	for (const uint32_t index : selections) {
		const uintptr_t selected_record = UI::Table<uintptr_t>::get(list->get_record(index));
		switch (mode) {
		case Mode::kInbox:
			// Archive highlighted message
			game.send_player_command(new Widelands::CmdMessageSetStatusArchived(
			   game.get_gametime(), plnum, MessageId(selected_record)));
			break;
		case Mode::kArchive:
			// Restore highlighted message
			game.send_player_command(new Widelands::CmdMessageSetStatusRead(
			   game.get_gametime(), plnum, MessageId(selected_record)));
			break;
		}
	}
}

void GameMessageMenu::center_view() {
	size_t const selection = list->selection_index();
	assert(selection < list->size());
	if (Message const* const message =
	       iplayer().player().messages()[MessageId((*list)[selection])]) {
		assert(message->position());
		iplayer().map_view()->scroll_to_field(message->position(), MapView::Transition::Smooth);
	}
}

/**
 * Show only messages of a certain type
 * @param msgtype the types of messages to show
 */
void GameMessageMenu::filter_messages(Widelands::Message::Type const msgtype) {
	list->clear_selections();
	switch (msgtype) {
	case Widelands::Message::Type::kGeologists:
		toggle_filter_messages_button(*geologistsbtn_, msgtype);
		break;
	case Widelands::Message::Type::kEconomy:
		toggle_filter_messages_button(*economybtn_, msgtype);
		break;
	case Widelands::Message::Type::kSeafaring:
		toggle_filter_messages_button(*seafaringbtn_, msgtype);
		break;
	case Widelands::Message::Type::kWarfare:
		toggle_filter_messages_button(*warfarebtn_, msgtype);
		break;
	case Widelands::Message::Type::kScenario:
		toggle_filter_messages_button(*scenariobtn_, msgtype);
		break;

	case Widelands::Message::Type::kNoMessages:
	case Widelands::Message::Type::kAllMessages:
	case Widelands::Message::Type::kGameLogic:
	case Widelands::Message::Type::kEconomySiteOccupied:
	case Widelands::Message::Type::kWarfareSiteDefeated:
	case Widelands::Message::Type::kWarfareSiteLost:
	case Widelands::Message::Type::kWarfareUnderAttack:
	case Widelands::Message::Type::kTradeOfferReceived:
		set_filter_messages_tooltips();
		message_filter_ = Widelands::Message::Type::kAllMessages;
		geologistsbtn_->set_perm_pressed(false);
		economybtn_->set_perm_pressed(false);
		seafaringbtn_->set_perm_pressed(false);
		warfarebtn_->set_perm_pressed(false);
		scenariobtn_->set_perm_pressed(false);
		break;
	}
	think();
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::toggle_filter_messages_button(UI::Button& button,
                                                    Widelands::Message::Type msgtype) {
	set_filter_messages_tooltips();
	if (button.style() == UI::Button::VisualState::kPermpressed) {
		button.set_perm_pressed(false);
		message_filter_ = Widelands::Message::Type::kAllMessages;
	} else {
		geologistsbtn_->set_perm_pressed(false);
		economybtn_->set_perm_pressed(false);
		seafaringbtn_->set_perm_pressed(false);
		warfarebtn_->set_perm_pressed(false);
		scenariobtn_->set_perm_pressed(false);
		button.set_perm_pressed(true);
		message_filter_ = msgtype;

		/** TRANSLATORS: %1% is a tooltip, %2% is the corresponding hotkey */
		button.set_tooltip(as_tooltip_text_with_hotkey(
		   /** TRANSLATORS: Tooltip in the messages window */
		   _("Show all messages"), shortcut_string_for(KeyboardShortcut::kInGameMessagesFilterAll),
		   UI::PanelStyle::kWui));
	}
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::set_filter_messages_tooltips() {
	geologistsbtn_->set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the messages window */
	   _("Show geologists’ messages only"),
	   shortcut_string_for(KeyboardShortcut::kInGameMessagesFilterGeologists),
	   UI::PanelStyle::kWui));
	economybtn_->set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the messages window */
	   _("Show economy messages only"),
	   shortcut_string_for(KeyboardShortcut::kInGameMessagesFilterEconomy), UI::PanelStyle::kWui));
	seafaringbtn_->set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the messages window */
	   _("Show seafaring messages only"),
	   shortcut_string_for(KeyboardShortcut::kInGameMessagesFilterSeafaring), UI::PanelStyle::kWui));
	warfarebtn_->set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the messages window */
	   _("Show warfare messages only"),
	   shortcut_string_for(KeyboardShortcut::kInGameMessagesFilterWarfare), UI::PanelStyle::kWui));
	scenariobtn_->set_tooltip(as_tooltip_text_with_hotkey(
	   /** TRANSLATORS: Tooltip in the messages window */
	   _("Show scenario messages only"),
	   shortcut_string_for(KeyboardShortcut::kInGameMessagesFilterScenario), UI::PanelStyle::kWui));
}

/**
 * Get the filename for a message category's icon
 */
std::string GameMessageMenu::display_message_type_icon(const Widelands::Message& message) {
	switch (message.message_type_category()) {
	case Widelands::Message::Type::kGeologists:
		return "images/wui/fieldaction/menu_geologist.png";
	case Widelands::Message::Type::kEconomy:
		return "images/wui/stats/genstats_nrwares.png";
	case Widelands::Message::Type::kSeafaring:
		return "images/wui/buildings/start_expedition.png";
	case Widelands::Message::Type::kWarfare:
		return "images/wui/messages/messages_warfare.png";
	case Widelands::Message::Type::kScenario:
		return "images/wui/menus/objectives.png";
	case Widelands::Message::Type::kGameLogic:
		return "images/ui_basic/menu_help.png";
	case Widelands::Message::Type::kNoMessages:
	case Widelands::Message::Type::kAllMessages:
	case Widelands::Message::Type::kEconomySiteOccupied:
	case Widelands::Message::Type::kWarfareSiteDefeated:
	case Widelands::Message::Type::kWarfareSiteLost:
	case Widelands::Message::Type::kWarfareUnderAttack:
	case Widelands::Message::Type::kTradeOfferReceived:
		return "images/wui/messages/message_new.png";
	}
	NEVER_HERE();
}

void GameMessageMenu::toggle_mode() {
	list->clear();
	switch (mode) {
	case Mode::kInbox:
		mode = Mode::kArchive;
		set_title(_("Messages: Archive"));
		archivebtn_->set_pic(g_image_cache->get("images/wui/messages/message_restore.png"));
		togglemodebtn_->set_pic(g_image_cache->get("images/wui/messages/message_new.png"));
		togglemodebtn_->set_tooltip(_("Show Inbox"));
		break;
	case Mode::kArchive:
		mode = Mode::kInbox;
		set_title(_("Messages: Inbox"));
		archivebtn_->set_pic(g_image_cache->get("images/wui/messages/message_archive.png"));
		togglemodebtn_->set_pic(g_image_cache->get("images/wui/messages/message_archived.png"));
		togglemodebtn_->set_tooltip(_("Show Archive"));
		break;
	}
	update_archive_button_tooltip();
}

void GameMessageMenu::update_archive_button_tooltip() {
	if (list->empty() || !list->has_selection()) {
		archivebtn_->set_tooltip("");
		archivebtn_->set_enabled(false);
		return;
	}
	archivebtn_->set_enabled(true);
	std::string button_tooltip;
	size_t no_selections = list->selections().size();
	switch (mode) {
	case Mode::kArchive:
		if (no_selections > 1) {
			button_tooltip =
			   /** TRANSLATORS: Tooltip in the messages window. There is a separate string for 1
			    * message.
			    * DO NOT omit the placeholder in your translation.
			    */
			   (boost::format(ngettext("Restore the selected %d message",
			                           "Restore the selected %d messages", no_selections)) %
			    no_selections)
			      .str();
		} else {
			/** TRANSLATORS: Tooltip in the messages window */
			button_tooltip = _("Restore selected message");
		}
		break;
	case Mode::kInbox:
		if (no_selections > 1) {
			button_tooltip =
			   /** TRANSLATORS: Tooltip in the messages window. There is a separate string for 1
			    * message.
			    * DO NOT omit the placeholder in your translation.
			    */
			   (boost::format(ngettext("Archive the selected %d message",
			                           "Archive the selected %d messages", no_selections)) %
			    no_selections)
			      .str();
		} else {
			/** TRANSLATORS: Tooltip in the messages window */
			button_tooltip = _("Archive selected message");
		}
		break;
	}
	archivebtn_->set_tooltip(as_tooltip_text_with_hotkey(
	   button_tooltip, shortcut_string_for(KeyboardShortcut::kCommonDeleteItem),
	   UI::PanelStyle::kWui));
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& GameMessageMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r = dynamic_cast<InteractivePlayer&>(ib).message_menu_;
			r.create();
			assert(r.window);
			GameMessageMenu& m = dynamic_cast<GameMessageMenu&>(*r.window);
			m.think();  // Fills the list
			if ((m.mode == Mode::kInbox) ^ (fr.unsigned_8() == 1)) {
				m.toggle_mode();
			}
			m.filter_messages(static_cast<Widelands::Message::Type>(fr.unsigned_8()));
			size_t nr_sel = fr.unsigned_32();
			if (nr_sel) {
				for (; nr_sel; --nr_sel) {
					m.list->multiselect(fr.unsigned_32(), true);
				}
				m.list->multiselect(fr.unsigned_32(), true);
			}
			return m;
		} else {
			throw Widelands::UnhandledVersionError(
			   "Messages Menu", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("messages menu: %s", e.what());
	}
}
void GameMessageMenu::save(FileWrite& fw, Widelands::MapObjectSaver&) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(mode == Mode::kInbox ? 1 : 0);
	fw.unsigned_8(static_cast<uint8_t>(message_filter_));
	const size_t nr_sel = list->selections().size();
	fw.unsigned_32(nr_sel);
	if (nr_sel) {
		for (const uint32_t& s : list->selections()) {
			fw.unsigned_32(s);
		}
		fw.unsigned_32(list->selection_index());
	}
}
