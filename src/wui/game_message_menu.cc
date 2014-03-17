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

#include "container_iterate.h"
#include "graphic/graphic.h"
#include "logic/instances.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "timestring.h"
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
		 5, 150, 570, 220,
		 "", UI::Align_Left, 1),
	mode(Inbox)
{
	list = new UI::Table<uintptr_t>(this, 5, 35, 570, 110);
	list->selected.connect(boost::bind(&GameMessageMenu::selected, this, _1));
	list->double_clicked.connect(boost::bind(&GameMessageMenu::double_clicked, this, _1));
	list->add_column (60, _("Select"), "", UI::Align_HCenter, true);
	list->add_column (60, _("Status"), "", UI::Align_HCenter);
	list->add_column(330, _("Title"));
	list->add_column(120, _("Time sent"));

	UI::Button * clearselectionbtn =
		new UI::Button
			(this, "clear_selection",
			 5, 5, 140, 25,
			 g_gr->images().get("pics/but0.png"),
			 _("Clear"), _("Clear selection"));
	clearselectionbtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::do_clear_selection, this));

	UI::Button * invertselectionbtn =
		new UI::Button
			(this, "invert_selection",
			 150, 5, 140, 25,
			 g_gr->images().get("pics/but0.png"),
			 _("Invert"), _("Invert selection"));
	invertselectionbtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::do_invert_selection, this));

	m_archivebtn =
		new UI::Button
			(this, "archive_or_restore_selected_messages",
			 295, 5, 25, 25,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/message_archive.png"),
			 _("Archive selected messages"));
	m_archivebtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::archive_or_restore, this));

	m_togglemodebtn =
		new UI::Button
			(this, "toggle_between_inbox_or_archive",
			 325, 5, 190, 25,
			 g_gr->images().get("pics/but2.png"),
			 _("Show Archive"));
	m_togglemodebtn->sigclicked.connect
		(boost::bind(&GameMessageMenu::toggle_mode, this));

	m_centerviewbtn =
		new UI::Button
			(this, "center_main_mapview_on_location",
			 550, 5, 25, 25,
			 g_gr->images().get("pics/but2.png"),
			 g_gr->images().get("pics/menu_goto.png"),
			 _("center main mapview on location"),
			 false);
	m_centerviewbtn->sigclicked.connect(boost::bind(&GameMessageMenu::center_view, this));

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
	assert(not list->find(id.value()));
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
	container_iterate_const(MessageQueue, mq, i) {
		Message_Id      const id      =  i.current->first;
		const Message &       message = *i.current->second;
		Message::Status const status  = message.status();
		if ((mode == Archive) != (status == Message::Archived))
			continue;
		if (!list->find(id.value())) {
			UI::Table<uintptr_t>::Entry_Record & er = list->add(id.value());
			update_record(er, message);
			list->sort();
		}
	}

	if (list->size()) {
		if (not list->has_selection())
			list->select(0);
			// FIXME Workaround for bug #691928: There should
			// FIXME be a solution without this extra update().
			list->update();
	} else {
		m_centerviewbtn->set_enabled(false);
		message_body.set_text(std::string());
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
		case SDLK_g:
			if (m_centerviewbtn->enabled())
				center_view();
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
		for
			(wl_index_range<uint8_t> i(0, list->size());
			 i;
			 ++i)
			if (list->get_record(i.current).is_checked(ColSelect))
			{
				work_done = true;
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusArchived
					 	(gametime, plnum, Message_Id((*list)[i.current])));
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
		for
			(wl_index_range<uint8_t> i(0, list->size());
			 i;
			 ++i)
			if (list->get_record(i.current).is_checked(ColSelect))
			{
				work_done = true;
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusRead
					 	(gametime, plnum, Message_Id((*list)[i.current])));
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
 * Clear the current selection of messages.
 */
void GameMessageMenu::do_clear_selection()
{
	for (wl_index_range<uint8_t> i(0, list->size()); i; ++i)
		list->get_record(i.current).set_checked
			(ColSelect, false);
}

void GameMessageMenu::do_invert_selection()
{
	for (wl_index_range<uint8_t> i(0, list->size()); i; ++i)
		list->get_record(i.current).toggle(ColSelect);
}

void GameMessageMenu::toggle_mode()
{
	list->clear();
	switch (mode) {
	case Inbox:
		mode = Archive;
		set_title(_("Messages: Archive"));
		m_archivebtn->set_pic(g_gr->images().get("pics/message_restore.png"));
		m_archivebtn->set_tooltip(_("Restore selected messages"));
		m_togglemodebtn->set_title(_("Show Inbox"));
		break;
	case Archive:
		mode = Inbox;
		set_title(_("Messages: Inbox"));
		m_archivebtn->set_pic(g_gr->images().get("pics/message_archive.png"));
		m_archivebtn->set_tooltip(_("Archive selected messages"));
		m_togglemodebtn->set_title(_("Show Archive"));
		break;
	default:
		assert(false); // there is nothing but Archive and Inbox
	}
}
