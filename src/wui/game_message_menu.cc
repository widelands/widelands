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

#include "interactive_player.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/playercommand.h"

using Widelands::Message;
using Widelands::MessageQueue;

inline Interactive_Player & GameMessageMenu::iplayer() const {
	return ref_cast<Interactive_Player, UI::Panel>(*get_parent());
}


GameMessageMenu::GameMessageMenu
	(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
	:
	UI::UniqueWindow(&plr, &registry, 370, 375, _("Message Menu: Inbox")),
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
}


static char const * const status_picture_filename[] = {
	"pics/message_new.png",
	"pics/message_read.png",
	"pics/message_archived.png"
};


#define SET_TITLE_AND_TIME                                                    \
   te.set_string(List::Title, message.title());                               \
   uint32_t time = message.sent();                                            \
   char timestring[] = "000:00:00.000";                                       \
   timestring[12] +=  time        % 10;                                       \
   timestring[11] += (time /= 10) % 10;                                       \
   timestring[10] += (time /= 10) % 10;                                       \
   timestring [8] += (time /= 10) % 10;                                       \
   timestring [7] += (time /= 10) %  6;                                       \
   timestring [5] += (time /=  6) % 10;                                       \
   timestring [4] += (time /= 10) %  6;                                       \
   timestring [2] += (time /=  6) % 10;                                       \
   timestring [1] += (time /= 10) % 10;                                       \
   timestring [0] +=  time /= 10;                                             \
   te.set_string(List::Time_Sent, time < 10 ? timestring : "-------------");  \


void GameMessageMenu::show_new_message
	(Widelands::Message_Id const id, Widelands::Message const & message)
{
	assert(iplayer().player().messages()[id] == &message);
	assert(not list.find(id));
	Message::Status const status = message.status();
	if ((mode == Archive) != (status == Message::Archived))
		toggle_between_inbox_and_archive.clicked();
	List::Entry_Record & te = list.add(id, true);
	te.set_picture
		(List::Status,
		 g_gr->get_picture(PicMod_UI, status_picture_filename[status]));
	SET_TITLE_AND_TIME;
}



static std::string const status_sort_string[] = {"new", "read"};

void GameMessageMenu::think() {
	MessageQueue & mq = iplayer().player().messages();
#if 0
	log
		("GameMessageMenu::think: %u new, %u read, %u archived\n",
		 mq.nr_messages(Message::New),
		 mq.nr_messages(Message::Read),
		 mq.nr_messages(Message::Archived));
#endif
	switch (mode) {
	case Inbox:
		//  Remove those that are still in the list but no longer in the queue.
		for (uint32_t j = list.size(); j;)
			if (Message const * const message = mq[list[--j]]) //  Still in queue.
				switch (message->status()) {
				case Message::New:
					//  Nothing to do. If it is new, it must have been new before.
					//  before. (The list is cleared when switching mode.)
					break;
				case Message::Read:
					//  It is read now, but might have been new before, so set the
					//  icon for new.
					list.get_record(j).set_picture
						(List::Status,
						 g_gr->get_picture
						 	(PicMod_UI, status_picture_filename[Message::Read]),
						 status_sort_string[Message::Read]);
					break;
				case Message::Archived:
					list.remove(j);
					break;
				default:
					assert(false);
				}
			else //  No longer in the queue and must be removed from the list.
				list.remove(j);

		//  Add those that are in the queue but not yet in the list.
		container_iterate_const(MessageQueue, mq, i) {
			Widelands::Message_Id const id      =  i.current->first;
			Message       const &       message = *i.current->second;
			Message::Status       const status  = message.status();
			if (status == Message::Archived)
				continue;
			assert(status < 2);
			for (uint32_t j = 0;; ++j)
				if (j == list.size()) {
					List::Entry_Record & te = list.add(id);
					te.set_picture
						(List::Status,
						 g_gr->get_picture
						 	(PicMod_UI, status_picture_filename[status]),
						 status_sort_string[status]);
					SET_TITLE_AND_TIME;
					break;
				} else if (list[j] == static_cast<uintptr_t>(id))
					break;
		}
		break;
	case Archive:
		//  Remove those that are still in the list but no longer in the queue.
		for (uint32_t j = list.size(); j;)
			if (Message const * const message = mq[list[--j]]) //  Still in queue.
				switch (message->status()) {
				case Message::New:
					//  fallthrough
				case Message::Read:
					list.remove(j);
					break;
				case Message::Archived:
					//  Nothing to do. If it is archived, it must have been archived
					//  before. (The list is cleared when switching mode.)
					break;
				default:
					assert(false);
				}
			else //  No longer in the queue and must be removed from the list.
				list.remove(j);

		//  Add those that are in the queue but not yet in the list.
		container_iterate_const(MessageQueue, mq, i) {
			Widelands::Message_Id const id      =  i.current->first;
			Message       const &       message = *i.current->second;
			if (message.status() != Message::Archived)
				continue;
			for (uint32_t j = 0;; ++j)
				if (j == list.size()) {
					List::Entry_Record & te = list.add(id);
					te.set_picture
						(List::Status,
						 g_gr->get_picture
						 	(PicMod_UI, status_picture_filename[Message::Archived]));
					SET_TITLE_AND_TIME;
					break;
				} else if (list[j] == static_cast<uintptr_t>(id))
					break;
		}
		break;
	}
	if (list.size()) {
		if (not list.has_selection())
			list.select(0);
	} else {
		center_main_mapview_on_location.set_enabled(false);
		message_body.set_text(std::string());
	}
}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	Widelands::Player & player = iplayer().player  ();
	MessageQueue      & mq     = player   .messages();
	if (t != List::no_selection_index()) {
		uint32_t const id = list[t];
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
			(struct {uint8_t current; uint8_t const end;} i =
			 	{0, menu.list.size()};
			 i.current < i.end;
			 ++i.current)
			if (menu.list.get_record(i.current).is_checked(List::Select))
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusArchived
					 	(gametime, plnum, menu.list[i.current]));
		break;
	case Archive:
		for
			(struct {uint8_t current; uint8_t const end;} i =
			 	{0, menu.list.size()};
			 i.current < i.end;
			 ++i.current)
			if (menu.list.get_record(i.current).is_checked(List::Select))
				game.send_player_command
					(*new Widelands::Cmd_MessageSetStatusRead
					 	(gametime, plnum, menu.list[i.current]));
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
		 	iplayer.player().messages()[list[selection]])
	{
		assert(message->position());
		iplayer.move_view_to(message->position());
	}
}
