/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "game_message_menu.h"

#include <boost/bind.hpp>

#include "interactive_player.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"

#include "container_iterate.h"

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
		(&plr, "messages", &registry, 370, 375, _("Message Menu: Inbox")),
	list                                (*this),
	message_body
		(this, 
		 5, 150, 360, 220,
		 "", UI::Align_Left, 1),
	clear_selection                     (*this),
	invert_selection                    (*this),
	archive_or_restore_selected_messages(*this),
	toggle_between_inbox_and_archive    (*this),
	center_main_mapview_on_location     (*this),
	mode                                (Inbox)
{
	if (get_usedefaultpos())
		center_to_parent();

	list.set_column_compare(List::Status, boost::bind(&GameMessageMenu::status_compare, this, _1, _2));

	set_can_focus(true);
	focus();
}

/**
 * When comparing messages by status, new messages come before others.
 */
bool GameMessageMenu::status_compare(uint32_t a, uint32_t b)
{
	MessageQueue & mq = iplayer().player().messages();
	const Message * msga = mq[Message_Id(list[a])];
	const Message * msgb = mq[Message_Id(list[b])];

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
	(Message_Id const id, Widelands::Message const & message)
{
	assert(iplayer().player().messages()[id] == &message);
	assert(not list.find(id.value()));
	Message::Status const status = message.status();
	if ((mode == Archive) != (status == Message::Archived))
		toggle_between_inbox_and_archive.clicked();
	List::Entry_Record & te = list.add(id.value(), true);
	update_record(te, message);
}

void GameMessageMenu::think()
{
	MessageQueue & mq = iplayer().player().messages();

	// Update messages in the list and remove messages that should no longer be shown
	for(uint32_t j = list.size(); j; --j) {
		if (Message const * const message = mq[Message_Id(list[j-1])]) {
			if ((mode == Archive) != (message->status() == Message::Archived)) {
				list.remove(j-1);
			} else {
				update_record(list.get_record(j-1), *message);
			}
		} else {
			list.remove(j-1);
		}
	}

	// Add new messages to the list
	container_iterate_const(MessageQueue, mq, i) {
		Message_Id      const id      =  i.current->first;
		Message const &       message = *i.current->second;
		Message::Status const status  = message.status();
		if ((mode == Archive) != (status == Message::Archived))
			continue;
		if (!list.find(id.value())) {
			List::Entry_Record & er = list.add(id.value());
			update_record(er, message);
		}
	}

	if (list.size()) {
		if (not list.has_selection())
			list.select(0);
	} else {
		center_main_mapview_on_location.set_enabled(false);
		message_body.set_text(std::string());
	}
}

void GameMessageMenu::update_record(GameMessageMenu::List::Entry_Record& er, const Widelands::Message& message)
{
	er.set_picture
		(List::Status,
		 g_gr->get_picture(PicMod_UI, status_picture_filename[message.status()]));
	er.set_string(List::Title, message.title());

	uint32_t time = message.sent();
	char timestring[] = "000:00:00.000";
	timestring[12] +=  time        % 10;
	timestring[11] += (time /= 10) % 10;
	timestring[10] += (time /= 10) % 10;
	timestring [8] += (time /= 10) % 10;
	timestring [7] += (time /= 10) %  6;
	timestring [5] += (time /=  6) % 10;
	timestring [4] += (time /= 10) %  6;
	timestring [2] += (time /=  6) % 10;
	timestring [1] += (time /= 10) % 10;
	timestring [0] +=  time /= 10;
	er.set_string(List::Time_Sent, time < 10 ? timestring : "-------------");
}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	Widelands::Player & player = iplayer().player  ();
	MessageQueue      & mq     = player   .messages();
	if (t != List::no_selection_index()) {
		Message_Id const id = Message_Id(list[t]);
		if (Message const * const message = mq[id]) {
			//  Maybe the message was removed since think?
			if (message->status() == Message::New) {
				Widelands::Game & game = iplayer().game();
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusRead
					 	(game.get_gametime(), player.player_number(), id));
			}
			center_main_mapview_on_location.set_enabled(message->position());
			message_body                   .set_text   (message->body    ());
			return;
		}
	}
	center_main_mapview_on_location.set_enabled(false);
	message_body                   .set_text   (std::string());
}

/**
 * Handle message menu hotkeys.
 */
bool GameMessageMenu::handle_key(bool down, SDL_keysym code)
{
	if (down) {
		switch(code.sym) {
		case SDLK_g:
			if (center_main_mapview_on_location.enabled())
				center_main_mapview_on_location.clicked();
			return true;

		case SDLK_DELETE:
			do_delete();
			return true;

		default:
			break; // not handled
		}
	}

	return UI::Panel::handle_key(down, code);
}

/**
 * Delete the currently selected message, if any.
 */
void GameMessageMenu::do_delete()
{
	if (mode == Archive)
		return;

	if (!list.has_selection())
		return;

	Widelands::Game & game = iplayer().game();
	game.send_player_command
		(*new Widelands::Cmd_MessageSetStatusArchived
			(game.get_gametime(), iplayer().player_number(), Message_Id(list.get_selected())));
}

void GameMessageMenu::Archive_Or_Restore_Selected_Messages::clicked() {
	GameMessageMenu & menu =
		ref_cast<GameMessageMenu, UI::Panel>(*get_parent());
	Widelands::Game         &       game     = menu.iplayer().game         ();
	uint32_t                  const gametime = game          .get_gametime ();
	Widelands::Player       &       player   = menu.iplayer().player       ();
	Widelands::Player_Number  const plnum    = player        .player_number();
	switch (menu.mode) {
	case Inbox:
		for
			(wl_index_range<uint8_t> i(0, menu.list.size());
			 i;
			 ++i)
			if (menu.list.get_record(i.current).is_checked(List::Select))
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusArchived
					 	(gametime, plnum, Message_Id(menu.list[i.current])));
		break;
	case Archive:
		for
			(wl_index_range<uint8_t> i(0, menu.list.size());
			 i;
			 ++i)
			if (menu.list.get_record(i.current).is_checked(List::Select))
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusRead
					 	(gametime, plnum, Message_Id(menu.list[i.current])));
		break;
	}
}

void GameMessageMenu::Center_Main_Mapview_On_Location::clicked() {
	GameMessageMenu & menu =
		ref_cast<GameMessageMenu, UI::Panel>(*get_parent());
	Interactive_Player & iplayer = menu.iplayer();
	List & list = menu.list;
	size_t const selection = list.selection_index();
	assert(selection < list.size());
	if
		(Message const * const message =
		 	iplayer.player().messages()[Message_Id(list[selection])])
	{
		assert(message->position());
		iplayer.move_view_to(message->position());
	}
}
