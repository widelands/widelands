/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/time_string.h"
#include "graphic/graphic.h"
#include "logic/map_objects/instances.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "wui/interactive_player.h"

using Widelands::Message;
using Widelands::MessageId;
using Widelands::MessageQueue;

inline InteractivePlayer & GameMessageMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}


GameMessageMenu::GameMessageMenu
	(InteractivePlayer & plr, UI::UniqueWindow::Registry & registry)
	:
	UI::UniqueWindow
		(&plr, "messages", &registry, 580, 375, _("Messages: Inbox")),
	message_body
		(this,
		 5, 154, 570, 216 - 5 - 34, // Subtracting height for message type icon
		 "", UI::Align_Left, 1),
	mode(Inbox)
{

	list = new UI::Table<uintptr_t>(this, 5, message_body.get_y() - 110, 570, 110);
	list->selected.connect(boost::bind(&GameMessageMenu::selected, this, _1));
	list->double_clicked.connect(boost::bind(&GameMessageMenu::double_clicked, this, _1));
	list->add_column (60, _("Select"), "", UI::Align_HCenter, true);
	list->add_column (60, _("Status"), "", UI::Align_HCenter);
	list->add_column(330, _("Title"));
	list->add_column(120, _("Time sent"));
	list->focus();

	// Buttons for message types
	m_geologistsbtn =
			new UI::Button
				(this, "filter_geologists_messages",
				 5, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_geologist.png"),
				 "",
				 true);
	m_geologistsbtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kGeologists));

	m_economybtn =
			new UI::Button
				(this, "filter_economy_messages",
				 2 * 5 + 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_build_flag.png"),
				 "",
				 true);
	m_economybtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kEconomy));

	m_seafaringbtn =
			new UI::Button
				(this, "filter_seafaring_messages",
				 3 * 5 + 2 * 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/start_expedition.png"),
				 "",
				 true);
	m_seafaringbtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kSeafaring));

	m_warfarebtn =
			new UI::Button
				(this, "filter_warfare_messages",
				 4 * 5 + 3 * 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/messages_warfare.png"),
				 "",
				 true);
	m_warfarebtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kWarfare));

	m_scenariobtn =
			new UI::Button
				(this, "filter_scenario_messages",
				 5 * 5 + 4 * 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_objectives.png"),
				 "",
				 true);
	m_scenariobtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::kScenario));

	m_message_filter = Widelands::Message::Type::kAllMessages;
	set_filter_messages_tooltips();
	// End: Buttons for message types

	UI::Button * clearselectionbtn =
		new UI::Button
			(this, "clear_selection",
			 5 * 5 + 6 * 34 + 17, 5, 34, 34,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_clear_selection.png"),
			 _("Clear selection"));
	clearselectionbtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::do_clear_selection, this));

	UI::Button * invertselectionbtn =
		new UI::Button
			(this, "invert_selection",
			 6 * 5 + 7 * 34 + 17, 5, 34, 34,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_selection_invert.png"),
			 _("Invert selection"));
	invertselectionbtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::do_invert_selection, this));

	m_archivebtn =
		new UI::Button
			(this, "archive_or_restore_selected_messages",
			 6 * 5 + 9 * 34 + 34, 5, 34, 34,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_archive.png"),
			 /** TRANSLATORS: %s is a tooltip, Del is the corresponding hotkey */
			 (boost::format(_("Del: %s"))
			  /** TRANSLATORS: Tooltip in the messages window */
			  % _("Archive selected messages")).str());
	m_archivebtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::archive_or_restore, this));

	m_togglemodebtn =
		new UI::Button
			(this, "toggle_between_inbox_or_archive",
			 7 * 5 + 10 * 34 + 34, 5, 34, 34,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_archived.png"),
			 _("Show Archive"));
	m_togglemodebtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::toggle_mode, this));

	m_centerviewbtn =
		new UI::Button
			(this, "center_main_mapview_on_location",
			 580 - 5 - 34, 5, 34, 34,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/menu_goto.png"),
			 /** TRANSLATORS: %s is a tooltip, G is the corresponding hotkey */
			 (boost::format(_("G: %s"))
			  /** TRANSLATORS: Tooltip in the messages window */
			  % _("Center main mapview on location")).str(),
			 false);
	m_centerviewbtn->sigclicked.connect(boost::bind(&GameMessageMenu::center_view, this));


	m_display_message_type_label =
		new UI::MultilineTextarea
			(this,
			 5, 375 - 5 - 34, 5 * 34, 40,
			 "<rt image=pics/message_new.png></rt>",
			 UI::Align::Align_BottomLeft, false);

	if (get_usedefaultpos())
		center_to_parent();

	list->set_column_compare
		(ColStatus, boost::bind(&GameMessageMenu::status_compare, this, _1, _2));
	list->set_sort_column(ColTimeSent);
	list->set_sort_descending(true);

	set_can_focus(true);
	focus();
}

/**
 * When comparing messages by status, new messages come before others.
 */
bool GameMessageMenu::status_compare(uint32_t a, uint32_t b)
{
	MessageQueue & mq = iplayer().player().messages();
	const Message * msga = mq[MessageId((*list)[a])];
	const Message * msgb = mq[MessageId((*list)[b])];

	if (msga && msgb) {
		return msga->status() == Message::Status::kNew && msgb->status() != Message::Status::kNew;
	}
	return false; // shouldn't happen
}

static char const * const status_picture_filename[] = {
	"pics/message_new.png",
	"pics/message_read.png",
	"pics/message_archived.png"
};

void GameMessageMenu::show_new_message
	(MessageId const id, const Widelands::Message & message)
{
	assert(iplayer().player().messages()[id] == &message);
	assert(!list->find(id.value()));
	Message::Status const status = message.status();
	if ((mode == Archive) != (status == Message::Status::kArchived))
		toggle_mode();
	UI::Table<uintptr_t>::EntryRecord & te = list->add(id.value(), true);
	update_record(te, message);
}

void GameMessageMenu::think()
{
	MessageQueue & mq = iplayer().player().messages();

	// Update messages in the list and remove messages
	// that should no longer be shown
	for (uint32_t j = list->size(); j; --j) {
		MessageId m_id((*list)[j - 1]);
		if (Message const * const message = mq[m_id]) {
			if ((mode == Archive) != (message->status() == Message::Status::kArchived)) {
				list->remove(j - 1);
			} else {
				update_record(list->get_record(j - 1), *message);
			}
		} else {
			list->remove(j - 1);
		}
	}

	// Add new messages to the list
	for (const std::pair<MessageId, Message *>& temp_message : mq) {
		MessageId      const id      =  temp_message.first;
		const Message &       message = *temp_message.second;
		Message::Status const status  = message.status();
		if ((mode == Archive) != (status == Message::Status::kArchived))
			continue;
		if (!list->find(id.value())) {
			UI::Table<uintptr_t>::EntryRecord & er = list->add(id.value());
			update_record(er, message);
			list->sort();
		}
	}

	// Filter message type
	if (m_message_filter != Message::Type::kAllMessages) {
		set_display_message_type_label(m_message_filter);
		for (uint32_t j = list->size(); j; --j) {
			MessageId m_id((*list)[j - 1]);
			if (Message const * const message = mq[m_id]) {
				if (message->message_type_category() != m_message_filter) {
					list->remove(j - 1);
				}
			}
		}
	}

	if (list->size()) {
		if (!list->has_selection())
			list->select(0);
			// TODO(unknown): Workaround for bug #691928: There should
			// be a solution without this extra update().
			list->update();
	} else {
		m_centerviewbtn->set_enabled(false);
		message_body.set_text(std::string());
		set_display_message_type_label(Widelands::Message::Type::kNoMessages);
	}
}

void GameMessageMenu::update_record
	(UI::Table<uintptr_t>::EntryRecord & er,
	 const Widelands::Message & message)
{
	er.set_picture
		(ColStatus,
		 g_gr->images().get(status_picture_filename[static_cast<int>(message.status())]));
	er.set_string(ColTitle, message.title());

	const uint32_t time = message.sent();
	er.set_string(ColTimeSent, gamestring_with_leading_zeros(time));
}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	Widelands::Player & player = iplayer().player  ();
	MessageQueue      & mq     = player   .messages();
	if (t != UI::Table<uintptr_t>::no_selection_index()) {
		MessageId const id = MessageId((*list)[t]);
		if (Message const * const message = mq[id]) {
			//  Maybe the message was removed since think?
			if (message->status() == Message::Status::kNew) {
				Widelands::Game & game = iplayer().game();
				game.send_player_command
					(*new Widelands::CmdMessageSetStatusRead
					 	(game.get_gametime(), player.player_number(), id));
			}
			m_centerviewbtn->set_enabled(message->position());
			message_body.set_text(message->body    ());
			set_display_message_type_label(message->message_type_category());
			return;
		}
	}
	m_centerviewbtn->set_enabled(false);
	message_body.set_text(std::string());
}

/**
 * a message was double clicked
 */
void GameMessageMenu::double_clicked(uint32_t const /* t */) {
	if (m_centerviewbtn->enabled()) center_view();
}

/**
 * Handle message menu hotkeys.
 */
bool GameMessageMenu::handle_key(bool down, SDL_Keysym code)
{
	if (down) {
		switch (code.sym) {
		// Don't forget to change the tooltips if any of these get reassigned
		case SDLK_g:
			if (m_centerviewbtn->enabled())
				center_view();
			return true;
		case SDLK_0:
			filter_messages(Widelands::Message::Type::kAllMessages);
			return true;
		case SDLK_1:
			filter_messages(Widelands::Message::Type::kGeologists);
			return true;
		case SDLK_2:
			filter_messages(Widelands::Message::Type::kEconomy);
			return true;
		case SDLK_3:
			filter_messages(Widelands::Message::Type::kSeafaring);
			return true;
		case SDLK_4:
			filter_messages(Widelands::Message::Type::kWarfare);
			return true;
		case SDLK_5:
			filter_messages(Widelands::Message::Type::kScenario);
			return true;
		case SDLK_DELETE:
			archive_or_restore();
			return true;
		case SDL_SCANCODE_KP_PERIOD:
		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM)
				break;
			/* no break */
		default:
			break; // not handled
		}
	}

	return list->handle_key(down, code);
}

void GameMessageMenu::archive_or_restore()
{
	Widelands::Game         &       game     = iplayer().game();
	uint32_t                  const gametime = game.get_gametime();
	Widelands::Player       &       player   = iplayer().player();
	Widelands::PlayerNumber  const plnum    = player.player_number();
	bool work_done = false;

	switch (mode) {
	case Inbox:
		//archive selected messages
		for (size_t i = 0; i < list->size(); ++i)
			if (list->get_record(i).is_checked(ColSelect))
			{
				work_done = true;
				game.send_player_command
					(*new Widelands::CmdMessageSetStatusArchived
					 	(gametime, plnum, MessageId((*list)[i])));
			}

		//archive highlighted message, if nothing was selected
		if (!work_done) {
			if (!list->has_selection()) return;

			game.send_player_command
				(*new Widelands::CmdMessageSetStatusArchived
					(gametime, plnum, MessageId(list->get_selected())));
		}
		break;
	case Archive:
		//restore selected messages
		for (size_t i = 0; i < list->size(); ++i)
			if (list->get_record(i).is_checked(ColSelect))
			{
				work_done = true;
				game.send_player_command
					(*new Widelands::CmdMessageSetStatusRead
					 	(gametime, plnum, MessageId((*list)[i])));
			}

		//restore highlighted message, if nothing was selected
		if (!work_done) {
			if (!list->has_selection()) return;

			game.send_player_command
				(*new Widelands::CmdMessageSetStatusRead
					(gametime, plnum, MessageId(list->get_selected())));
		}
		break;
	default:
		assert(false); // there is nothing but Archive and Inbox
	}
}

void GameMessageMenu::center_view()
{
	size_t const selection = list->selection_index();
	assert(selection < list->size());
	if
		(Message const * const message =
		 	iplayer().player().messages()[MessageId((*list)[selection])])
	{
		assert(message->position());
		iplayer().move_view_to(message->position());
	}
}

/**
 * Show only messages of a certain type
 * @param msgtype the types of messages to show
 */
void GameMessageMenu::filter_messages(Widelands::Message::Type const msgtype) {
	switch (msgtype) {
		case Widelands::Message::Type::kGeologists:
			toggle_filter_messages_button(*m_geologistsbtn, msgtype);
			break;
		case Widelands::Message::Type::kEconomy:
			toggle_filter_messages_button(*m_economybtn, msgtype);
			break;
		case Widelands::Message::Type::kSeafaring:
			toggle_filter_messages_button(*m_seafaringbtn, msgtype);
			break;
		case Widelands::Message::Type::kWarfare:
			toggle_filter_messages_button(*m_warfarebtn, msgtype);
			break;
		case Widelands::Message::Type::kScenario:
			toggle_filter_messages_button(*m_scenariobtn, msgtype);
			break;
		default:
			set_filter_messages_tooltips();
			m_message_filter = Widelands::Message::Type::kAllMessages;
			m_geologistsbtn->set_perm_pressed(false);
			m_economybtn->set_perm_pressed(false);
			m_seafaringbtn->set_perm_pressed(false);
			m_warfarebtn->set_perm_pressed(false);
			m_scenariobtn->set_perm_pressed(false);
	}
	think();
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::toggle_filter_messages_button(UI::Button & button, Widelands::Message::Type msgtype) {
	set_filter_messages_tooltips();
	if (button.get_perm_pressed()) {
		button.set_perm_pressed(false);
		m_message_filter = Widelands::Message::Type::kAllMessages;
	} else {
		m_geologistsbtn->set_perm_pressed(false);
		m_economybtn->set_perm_pressed(false);
		m_seafaringbtn->set_perm_pressed(false);
		m_warfarebtn->set_perm_pressed(false);
		m_scenariobtn->set_perm_pressed(false);
		button.set_perm_pressed(true);
		m_message_filter = msgtype;
		/** TRANSLATORS: %1% is a tooltip, %2% is the corresponding hotkey */
		button.set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
								  /** TRANSLATORS: Tooltip in the messages window */
								  % _("Show all messages")
								  % "0").str());
	}
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::set_filter_messages_tooltips() {
	m_geologistsbtn->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
											/** TRANSLATORS: Tooltip in the messages window */
											% _("Show geologists' messages only")
											% "1").str());
	m_economybtn->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										/** TRANSLATORS: Tooltip in the messages window */
										% _("Show economy messages only")
										% "2").str());
	m_seafaringbtn->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										  /** TRANSLATORS: Tooltip in the messages window */
										  % _("Show seafaring messages only")
										  % "3").str());
	m_warfarebtn->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										/** TRANSLATORS: Tooltip in the messages window */
										% _("Show warfare messages only")
										% "4").str());
	m_scenariobtn->set_tooltip((boost::format(_("%1% (Hotkey: %2%)"))
										 /** TRANSLATORS: Tooltip in the messages window */
										 % _("Show scenario messages only")
										 % "5").str());
}

/**
 * Update image and tooltip for message category label
 */
void GameMessageMenu::set_display_message_type_label(Widelands::Message::Type msgtype) {
	std::string message_type_tooltip = "";
	std::string message_type_image = "";

	switch (msgtype) {
		case Widelands::Message::Type::kGeologists:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Geologists");
			message_type_image =  "<rt image=pics/menu_geologist.png></rt>";
			break;
		case Widelands::Message::Type::kEconomy:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Economy");
			message_type_image =  "<rt image=pics/menu_build_flag.png></rt>";
			break;
		case Widelands::Message::Type::kSeafaring:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Seafaring");
			message_type_image =  "<rt image=pics/start_expedition.png></rt>";
			break;
		case Widelands::Message::Type::kWarfare:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Warfare");
			message_type_image =  "<rt image=pics/messages_warfare.png></rt>";
			break;
		case Widelands::Message::Type::kScenario:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Scenario");
			message_type_image =  "<rt image=pics/menu_objectives.png></rt>";
			break;
		case Widelands::Message::Type::kNoMessages:
			/** TRANSLATORS: This show up instead of a message's type when there are no messages found */
			message_type_tooltip =  _("No message found");
			break;
		default:
			/** TRANSLATORS: This is the default message type */
			message_type_tooltip = _("General");
			message_type_image =  "<rt image=pics/message_new.png></rt>";
	}

	m_display_message_type_label->set_tooltip(
				 /** TRANSLATORS: %s is a message's type */
				 (boost::format(_("Type of this message: %s"))
				  /** TRANSLATORS: Tooltip in the messages window */
				  % message_type_tooltip).str());
	m_display_message_type_label->set_text(message_type_image);
}


/**
 * Clear the current selection of messages.
 */
void GameMessageMenu::do_clear_selection()
{
	for (size_t i = 0; i < list->size(); ++i)
		list->get_record(i).set_checked
			(ColSelect, false);
}

void GameMessageMenu::do_invert_selection()
{
	for (size_t i = 0; i < list->size(); ++i)
		list->get_record(i).toggle(ColSelect);
}

void GameMessageMenu::toggle_mode()
{
	list->clear();
	switch (mode) {
	case Inbox:
		mode = Archive;
		set_title(_("Messages: Archive"));
		m_archivebtn->set_pic(g_gr->images().get("pics/message_restore.png"));
		/** TRANSLATORS: %s is a tooltip, Del is the corresponding hotkey */
		m_archivebtn->set_tooltip((boost::format(_("Del: %s"))
											/** TRANSLATORS: Tooltip in the messages window */
											% _("Restore selected messages")).str());
		m_togglemodebtn->set_pic(g_gr->images().get("pics/message_new.png"));
		m_togglemodebtn->set_tooltip(_("Show Inbox"));
		break;
	case Archive:
		mode = Inbox;
		set_title(_("Messages: Inbox"));
		m_archivebtn->set_pic(g_gr->images().get("pics/message_archive.png"));
		/** TRANSLATORS: %s is a tooltip, Del is the corresponding hotkey */
		m_archivebtn->set_tooltip((boost::format(_("Del: %s"))
											/** TRANSLATORS: Tooltip in the messages window */
											% _("Archive selected messages")).str());
		m_togglemodebtn->set_pic(g_gr->images().get("pics/message_archived.png"));
		m_togglemodebtn->set_tooltip(_("Show Archive"));
		break;
	default:
		assert(false); // there is nothing but Archive and Inbox
	}
}
