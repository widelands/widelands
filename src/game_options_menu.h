/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__GAME_OPTIONS_MENU_H
#define __S__GAME_OPTIONS_MENU_H

#include "interactive_player.h"
#include "ui_unique_window.h"

// The GameOptionsMenu is a rather dumb window with lots of buttons
class GameOptionsMenu : public UIUniqueWindow {
public:
	GameOptionsMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry, Interactive_Player::Game_Main_Menu_Windows* windows);
	virtual ~GameOptionsMenu();

private:
	Interactive_Player	*m_player;
   Interactive_Player::Game_Main_Menu_Windows* m_windows;
   void clicked(int);
};

#endif
