/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__GAME_CHAT_MENU_H
#define __S__GAME_CHAT_MENU_H

#include "interactive_player.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_multilineeditbox.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

class NetGame;

// The GameChatMenu is a rather dumb window with lots of buttons
class GameChatMenu : public UIUniqueWindow {
public:
	GameChatMenu(Interactive_Player &, UIUniqueWindowRegistry &, NetGame *);

   void  think( void );

private:
	Interactive_Player & m_player;
	NetGame            * m_netgame;
	UIMultiline_Textarea chatbox;
	UIMultiline_Editbox  editbox;
	UIButton             send;
	UITextarea           show_as_overlays_label;
	UICheckbox           show_as_overlays;
	
	void clicked_send(int);
	void changed_show_as_overlays(bool);
};

#endif
