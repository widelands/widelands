/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_MESSAGE_MENU_H
#define WL_WUI_GAME_MESSAGE_MENU_H

#include "base/i18n.h"
#include "logic/message.h"
#include "logic/message_queue.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class Game;
struct Message;
}
class InteractivePlayer;

///  Shows the not already fulfilled objectives.
struct GameMessageMenu : public UI::UniqueWindow {
	GameMessageMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);

	/// Shows a newly created message. Assumes that the message is not yet in
	/// the list (the message was added to the queue after the last time think()
	/// was executed.
	void show_new_message(Widelands::MessageId, const Widelands::Message&);

	enum Mode { Inbox, Archive };
	void think() override;
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	enum Cols { ColTitle, ColType, ColStatus, ColTimeSent };
	enum class ReadUnread : uint8_t { allMessages, readMessages, newMessages };

	InteractivePlayer& iplayer() const;
	void selected(uint32_t);
	void double_clicked(uint32_t);

	bool compare_title(uint32_t a, uint32_t b);
	bool compare_status(uint32_t a, uint32_t b);
	bool compare_type(uint32_t a, uint32_t b);
	bool compare_time_sent(uint32_t a, uint32_t b);
	void archive_or_restore();
	void toggle_mode();
	void center_view();
	void filter_messages(Widelands::Message::Type);
	void toggle_filter_messages_button(UI::Button&, Widelands::Message::Type);
	void set_filter_messages_tooltips();
	std::string display_message_type_icon(Widelands::Message);
	void update_record(UI::Table<uintptr_t>::EntryRecord& er, const Widelands::Message&);

	UI::Table<uintptr_t>* list;
	UI::MultilineTextarea message_body;
	UI::Button* archivebtn_;
	UI::Button* togglemodebtn_;
	UI::Button* centerviewbtn_;
	Mode mode;
	// Buttons for message types
	UI::Button* geologistsbtn_;
	UI::Button* economybtn_;
	UI::Button* seafaringbtn_;
	UI::Button* warfarebtn_;
	UI::Button* scenariobtn_;
	Widelands::Message::Type message_filter_;
};

#endif  // end of include guard: WL_WUI_GAME_MESSAGE_MENU_H
