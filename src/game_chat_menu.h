/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef GAME_CHAT_MENU_H
#define GAME_CHAT_MENU_H

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_multilineeditbox.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

class Interactive_Player;
class NetGame;

// The GameChatMenu is a rather dumb window with lots of buttons
struct GameChatMenu : public UI::UniqueWindow {
	GameChatMenu(Interactive_Player &, UI::UniqueWindow::Registry &, NetGame *);

	void think();

private:
	Interactive_Player & m_player;
	NetGame            * m_netgame;
	UI::Multiline_Textarea chatbox;
	UI::Multiline_Editbox  editbox;
	UI::Button<GameChatMenu> send;
	UI::Textarea           show_as_overlays_label;
	UI::Checkbox           show_as_overlays;

	void clicked_send();
	void changed_show_as_overlays(bool);
};

#endif
