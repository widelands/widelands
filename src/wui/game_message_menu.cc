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

#include "base/deprecated.h"
#include "base/time_string.h"
#include "graphic/graphic.h"
#include "logic/instances.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "wui/interactive_player.h"

using Widelands::Message;
using Widelands::Message_Id;
using Widelands::MessageQueue;

inline Interactive_Player & GameMessageMenu::iplayer() const {
	return ref_cast<Interactive_Player, UI::Panel>(*get_parent());
}


GameMessageMenu::GameMessageMenu
	(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
	:
	UI::UniqueWindow
		(&plr, "messages", &registry, 580, 375, _("Messages: Inbox")),
	message_body
		(this,
		 5, 154, 570, 216,
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
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::geologists));

	m_economybtn =
			new UI::Button
				(this, "filter_economy_messages",
				 2 * 5 + 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_build_flag.png"),
				 "",
				 true);
	m_economybtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::economy));

	m_seafaringbtn =
			new UI::Button
				(this, "filter_seafaring_messages",
				 3 * 5 + 2 * 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/start_expedition.png"),
				 "",
				 true);
	m_seafaringbtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::seafaring));

	m_warfarebtn =
			new UI::Button
				(this, "filter_warfare_messages",
				 4 * 5 + 3 * 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/messages_warfare.png"),
				 "",
				 true);
	m_warfarebtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::warfare));

	m_scenariobtn =
			new UI::Button
				(this, "filter_scenario_messages",
				 5 * 5 + 4 * 34, 5, 34, 34,
				 g_gr->images().get("pics/but0.png"),
				 g_gr->images().get("pics/menu_objectives.png"),
				 "",
				 true);
	m_scenariobtn->sigclicked.connect
			(boost::bind(&GameMessageMenu::filter_messages, this, Widelands::Message::Type::scenario));

	m_message_filter = Widelands::Message::Type::allMessages;
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
		new UI::Multiline_Textarea
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
	const Message * msga = mq[Message_Id((*list)[a])];
	const Message * msgb = mq[Message_Id((*list)[b])];

	if (msga && msgb) {
		return msga->status() == Message::New && msgb->status() != Message::New;
	}
	return false; // shouldn't happen
}

static char const * const status_picture_filename[] = {
	"pics/message_new.png",
	"pics/message_read.png",
	"pics/message_archived.png"
};

void GameMessageMenu::show_new_message
	(Message_Id const id, const Widelands::Message & message)
{
	assert(iplayer().player().messages()[id] == &message);
	assert(!list->find(id.value()));
	Message::Status const status = message.status();
	if ((mode == Archive) != (status == Message::Archived))
		toggle_mode();
	UI::Table<uintptr_t>::Entry_Record & te = list->add(id.value(), true);
	update_record(te, message);
}

void GameMessageMenu::think()
{
	MessageQueue & mq = iplayer().player().messages();

	// Update messages in the list and remove messages
	// that should no longer be shown
	for (uint32_t j = list->size(); j; --j) {
		Message_Id m_id((*list)[j - 1]);
		if (Message const * const message = mq[m_id]) {
			if ((mode == Archive) != (message->status() == Message::Archived)) {
				list->remove(j - 1);
			} else {
				update_record(list->get_record(j - 1), *message);
			}
		} else {
			list->remove(j - 1);
		}
	}

	// Add new messages to the list
	for (const std::pair<Message_Id, Message *>& temp_message : mq) {
		Message_Id      const id      =  temp_message.first;
		const Message &       message = *temp_message.second;
		Message::Status const status  = message.status();
		if ((mode == Archive) != (status == Message::Archived))
			continue;
		if (!list->find(id.value())) {
			UI::Table<uintptr_t>::Entry_Record & er = list->add(id.value());
			update_record(er, message);
			list->sort();
		}
	}

	// Filter message type
	if (m_message_filter != Message::Type::allMessages) {
		set_display_message_type_label(m_message_filter);
		for (uint32_t j = list->size(); j; --j) {
			Message_Id m_id((*list)[j - 1]);
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
		set_display_message_type_label(Widelands::Message::Type::noMessages);
	}
}

void GameMessageMenu::update_record
	(UI::Table<uintptr_t>::Entry_Record & er,
	 const Widelands::Message & message)
{
	er.set_picture
		(ColStatus,
		 g_gr->images().get(status_picture_filename[message.status()]));
	er.set_string(ColTitle, message.title());

	const uint32_t time = message.sent();
	er.set_string(ColTimeSent, gametimestring(time));
}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	Widelands::Player & player = iplayer().player  ();
	MessageQueue      & mq     = player   .messages();
	if (t != UI::Table<uintptr_t>::no_selection_index()) {
		Message_Id const id = Message_Id((*list)[t]);
		if (Message const * const message = mq[id]) {
			//  Maybe the message was removed since think?
			if (message->status() == Message::New) {
				Widelands::Game & game = iplayer().game();
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusRead
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
bool GameMessageMenu::handle_key(bool down, SDL_keysym code)
{
	if (down) {
		switch (code.sym) {
		// Don't forget to change the tooltips if any of these get reassigned
		case SDLK_g:
			if (m_centerviewbtn->enabled())
				center_view();
			return true;
		case SDLK_0:
			filter_messages(Widelands::Message::Type::allMessages);
			return true;
		case SDLK_1:
			filter_messages(Widelands::Message::Type::geologists);
			return true;
		case SDLK_2:
			filter_messages(Widelands::Message::Type::economy);
			return true;
		case SDLK_3:
			filter_messages(Widelands::Message::Type::seafaring);
			return true;
		case SDLK_4:
			filter_messages(Widelands::Message::Type::warfare);
			return true;
		case SDLK_5:
			filter_messages(Widelands::Message::Type::scenario);
			return true;
		case SDLK_KP_PERIOD:

			if (code.mod & KMOD_NUM)
				break;
			/* no break */
		case SDLK_DELETE:
			archive_or_restore();
			return true;

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
	Widelands::Player_Number  const plnum    = player.player_number();
	bool work_done = false;

	switch (mode) {
	case Inbox:
		//archive selected messages
		for (size_t i = 0; i < list->size(); ++i)
			if (list->get_record(i).is_checked(ColSelect))
			{
				work_done = true;
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusArchived
					 	(gametime, plnum, Message_Id((*list)[i])));
			}

		//archive highlighted message, if nothing was selected
		if (!work_done) {
			if (!list->has_selection()) return;

			game.send_player_command
				(*new Widelands::Cmd_MessageSetStatusArchived
					(gametime, plnum, Message_Id(list->get_selected())));
		}
		break;
	case Archive:
		//restore selected messages
		for (size_t i = 0; i < list->size(); ++i)
			if (list->get_record(i).is_checked(ColSelect))
			{
				work_done = true;
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusRead
					 	(gametime, plnum, Message_Id((*list)[i])));
			}

		//restore highlighted message, if nothing was selected
		if (!work_done) {
			if (!list->has_selection()) return;

			game.send_player_command
				(*new Widelands::Cmd_MessageSetStatusRead
					(gametime, plnum, Message_Id(list->get_selected())));
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
		 	iplayer().player().messages()[Message_Id((*list)[selection])])
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
		case Widelands::Message::Type::geologists:
			toggle_filter_messages_button(*m_geologistsbtn, msgtype);
			break;
		case Widelands::Message::Type::economy:
			toggle_filter_messages_button(*m_economybtn, msgtype);
			break;
		case Widelands::Message::Type::seafaring:
			toggle_filter_messages_button(*m_seafaringbtn, msgtype);
			break;
		case Widelands::Message::Type::warfare:
			toggle_filter_messages_button(*m_warfarebtn, msgtype);
			break;
		case Widelands::Message::Type::scenario:
			toggle_filter_messages_button(*m_scenariobtn, msgtype);
			break;
		default:
			set_filter_messages_tooltips();
			m_message_filter = Widelands::Message::Type::allMessages;
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
		m_message_filter = Widelands::Message::Type::allMessages;
	} else {
		m_geologistsbtn->set_perm_pressed(false);
		m_economybtn->set_perm_pressed(false);
		m_seafaringbtn->set_perm_pressed(false);
		m_warfarebtn->set_perm_pressed(false);
		m_scenariobtn->set_perm_pressed(false);
		button.set_perm_pressed(true);
		m_message_filter = msgtype;
		/** TRANSLATORS: %s is a tooltip, 0 is the corresponding hotkey */
		button.set_tooltip((boost::format(_("0: %s"))
								  /** TRANSLATORS: Tooltip in the messages window */
								  % _("Show all messages")).str());
	}
}

/**
 * Helper for filter_messages
 */
void GameMessageMenu::set_filter_messages_tooltips() {
	/** TRANSLATORS: %s is a tooltip, 1 is the corresponding hotkey */
	m_geologistsbtn->set_tooltip((boost::format(_("1: %s"))
											/** TRANSLATORS: Tooltip in the messages window */
											% _("Show geologists' messages only")).str());
	/** TRANSLATORS: %s is a tooltip, 2 is the corresponding hotkey */
	m_economybtn->set_tooltip((boost::format(_("2: %s"))
										/** TRANSLATORS: Tooltip in the messages window */
										% _("Show economy messages only")).str());
	/** TRANSLATORS: %s is a tooltip, 3 is the corresponding hotkey */
	m_seafaringbtn->set_tooltip((boost::format(_("3: %s)"))
										  /** TRANSLATORS: Tooltip in the messages window */
										  % _("Show seafaring messages only")).str());
	/** TRANSLATORS: %s is a tooltip, 4 is the corresponding hotkey */
	m_warfarebtn->set_tooltip((boost::format(_("4: %s"))
										/** TRANSLATORS: Tooltip in the messages window */
										% _("Show warfare messages only")).str());
	/** TRANSLATORS: %s is a tooltip, 5 is the corresponding hotkey */
	m_scenariobtn->set_tooltip((boost::format(_("5: %s"))
										 /** TRANSLATORS: Tooltip in the messages window */
										 % _("Show scenario messages only")).str());
}

/**
 * Update image and tooltip for message category label
 */
void GameMessageMenu::set_display_message_type_label(Widelands::Message::Type msgtype) {
	std::string message_type_tooltip = "";
	std::string message_type_image = "";

	switch (msgtype) {
		case Widelands::Message::Type::geologists:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Geologists");
			message_type_image =  "<rt image=pics/menu_geologist.png></rt>";
			break;
		case Widelands::Message::Type::economy:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Economy");
			message_type_image =  "<rt image=pics/menu_build_flag.png></rt>";
			break;
		case Widelands::Message::Type::seafaring:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Seafaring");
			message_type_image =  "<rt image=pics/start_expedition.png></rt>";
			break;
		case Widelands::Message::Type::warfare:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Warfare");
			message_type_image =  "<rt image=pics/messages_warfare.png></rt>";
			break;
		case Widelands::Message::Type::scenario:
			/** TRANSLATORS: This is a message's type */
			message_type_tooltip =  _("Scenario");
			message_type_image =  "<rt image=pics/menu_objectives.png></rt>";
			break;
		case Widelands::Message::Type::noMessages:
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
