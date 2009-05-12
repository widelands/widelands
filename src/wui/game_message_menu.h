/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/button.h"
#include "message.h"

namespace Widelands {
struct Game;
struct Message;
};
struct Interactive_Player;

///  Shows the not already fulfilled objectives.
struct GameMessageMenu : public UI::UniqueWindow {
	GameMessageMenu(Interactive_Player &, UI::UniqueWindow::Registry &);
	void think();

private:
	Interactive_Player & iplayer() const;
	void                 selected(uint32_t);

	void clicked_hide();
	void clicked_view();

	typedef UI::Listselect<Widelands::Message &> list_type;
	list_type              list;
	UI::Multiline_Textarea messagetext;
	UI::Callback_Button<GameMessageMenu> hide;
	UI::Callback_Button<GameMessageMenu> view;
};

#endif
