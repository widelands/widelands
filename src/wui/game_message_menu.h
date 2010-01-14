/*
 * Copyright (C) 2002-2004, 2006, 2008-2010 by the Widelands Development Team
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

#ifndef GAME_MESSAGE_MENU_H
#define GAME_MESSAGE_MENU_H

#include "logic/message_queue.h"

#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"

#include "i18n.h"

#include "ref_cast.h"

namespace Widelands {
struct Game;
struct Message;
};
struct Interactive_Player;

///  Shows the not already fulfilled objectives.
struct GameMessageMenu : public UI::UniqueWindow {
	GameMessageMenu(Interactive_Player &, UI::UniqueWindow::Registry &);

	/// Shows a newly created message. Assumes that the message is not yet in
	/// the list (the message was added to the queue after the last time think()
	/// was executed.
	void show_new_message(Widelands::Message_Id, Widelands::Message const &);

	enum Mode {Inbox, Archive};
	void think();

private:
	Interactive_Player & iplayer() const;
	void                 selected(uint32_t);

	struct List : public UI::Table<uintptr_t const> {
		enum Cols {Select, Status, Title, Time_Sent};
		List(GameMessageMenu & parent) :
			UI::Table<uintptr_t const>(&parent, 5, 35, 360, 110)
		{
			selected.set(&parent, &GameMessageMenu::selected);
			add_column (50, _("Select"), UI::Align_HCenter, true);
			add_column (50, _("Status"), UI::Align_HCenter);
			add_column(136, _("Title"));
			add_column(100, _("Time sent"));
		}
	} list;
	UI::Multiline_Textarea message_body;

	struct Clear_Selection : public UI::Button {
		Clear_Selection(GameMessageMenu & parent) :
			UI::Button
				(&parent,
				 5, 5, 70, 25,
				 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
				 _("Clear"), _("Clear selection"))
		{}
		void clicked() {
			GameMessageMenu & menu =
				ref_cast<GameMessageMenu, UI::Panel>(*get_parent());
			for
				(struct {uint8_t current; uint8_t const end;} i =
				 	{0, menu.list.size()};
				 i.current < i.end;
				 ++i.current)
				menu.list.get_record(i.current).set_checked
					(List::Select, false);
		}
	} clear_selection;

	struct Invert_Selection : public UI::Button {
		Invert_Selection(GameMessageMenu & parent) :
			UI::Button
				(&parent,
				 80, 5, 70, 25,
				 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
				 _("Invert"), _("Invert selection"))
		{}
		void clicked() {
			GameMessageMenu & menu =
				ref_cast<GameMessageMenu, UI::Panel>(*get_parent());
			for
				(struct {uint8_t current; uint8_t const end;} i =
				 	{0, menu.list.size()};
				 i.current < i.end;
				 ++i.current)
				menu.list.get_record(i.current).toggle(List::Select);
		}
	} invert_selection;

	struct Archive_Or_Restore_Selected_Messages : public UI::Button {
		Archive_Or_Restore_Selected_Messages(GameMessageMenu & parent) :
			UI::Button
				(&parent,
				 155, 5, 25, 25,
				 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
				 g_gr->get_picture(PicMod_Game, "pics/message_archive.png"),
				 _("Archive selected messages"))
		{}
		void clicked();
	} archive_or_restore_selected_messages;

	struct Toggle_Between_Inbox_And_Archive : public UI::Button {
		Toggle_Between_Inbox_And_Archive(GameMessageMenu & parent) :
			UI::Button
				(&parent,
				 185, 5, 100, 25,
				 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
				 _("Show Archive"))
		{}
		void clicked() {
			GameMessageMenu & menu =
				ref_cast<GameMessageMenu, UI::Panel>(*get_parent());
			menu.list.clear();
			switch (menu.mode) {
			case Inbox:
				menu.mode = Archive;
				menu.set_title(_("Message Menu: Archive"));
				menu.archive_or_restore_selected_messages.set_pic
					(g_gr->get_picture(PicMod_Game, "pics/message_restore.png"));
				menu.archive_or_restore_selected_messages.set_tooltip
					(_("Restore selected messages"));
				set_title(_("Show Inbox"));
				break;
			case Archive:
				menu.mode = Inbox;
				menu.set_title(_("Message Menu: Inbox"));
				menu.archive_or_restore_selected_messages.set_pic
					(g_gr->get_picture(PicMod_Game, "pics/message_archive.png"));
				menu.archive_or_restore_selected_messages.set_tooltip
					(_("Archive selected messages"));
				set_title(_("Show Archive"));
				break;
			}
		}
	} toggle_between_inbox_and_archive;

	struct Center_Main_Mapview_On_Location : public UI::Button {
		Center_Main_Mapview_On_Location(GameMessageMenu & parent) :
			UI::Button
				(&parent,
				 340, 5, 25, 25,
				 g_gr->get_picture(PicMod_UI, "pics/but2.png"),
				 g_gr->get_picture(PicMod_Game, "pics/menu_goto.png"),
				 _("center main mapview on location"),
				 false)
		{}
		void clicked();
	} center_main_mapview_on_location;

	Mode mode;
};

#endif
