/*
 * Copyright (C) 2002-2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef GAME_MESSAGE_MENU_H
#define GAME_MESSAGE_MENU_H

#include "container_iterate.h"
#include "i18n.h"
#include "logic/message_queue.h"
#include "ref_cast.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class Game;
struct Message;
};
struct Interactive_Player;

///  Shows the not already fulfilled objectives.
struct GameMessageMenu : public UI::UniqueWindow {
	GameMessageMenu(Interactive_Player &, UI::UniqueWindow::Registry &);

	/// Shows a newly created message. Assumes that the message is not yet in
	/// the list (the message was added to the queue after the last time think()
	/// was executed.
	void show_new_message(Widelands::Message_Id, const Widelands::Message &);

	enum Mode {Inbox, Archive};
	void think() override;
	virtual bool handle_key(bool down, SDL_keysym code) override;

private:
	enum Cols {ColSelect, ColStatus, ColTitle, ColTimeSent};

	Interactive_Player & iplayer() const;
	void selected(uint32_t);
	void double_clicked(uint32_t);

	bool status_compare(uint32_t a, uint32_t b);
	void do_clear_selection();
	void do_invert_selection();
	void archive_or_restore();
	void toggle_mode();
	void center_view();
	void update_record(UI::Table<uintptr_t>::Entry_Record & er, const Widelands::Message &);

	UI::Table<uintptr_t> * list;
	UI::Multiline_Textarea message_body;
	UI::Button * m_archivebtn;
	UI::Button * m_togglemodebtn;
	UI::Button * m_centerviewbtn;
	Mode mode;
};

#endif
