/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_PLAYER_MENU_ALLOWED_BUILDINGS_MENU_H
#define EDITOR_PLAYER_MENU_ALLOWED_BUILDINGS_MENU_H

#include "logic/widelands.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

namespace Widelands {class Player;}

/**
 * Let's the user choose which buildings should be available
 * for this player for this scenario. Used to throttle AI and
 * to advance technology slowly through the missions
 */
struct Editor_Player_Menu_Allowed_Buildings_Menu : public UI::UniqueWindow {
	Editor_Player_Menu_Allowed_Buildings_Menu
		(UI::Panel * parent, Widelands::Player &, UI::UniqueWindow::Registry *);
	virtual ~Editor_Player_Menu_Allowed_Buildings_Menu();

private:
	Widelands::Player & m_player;
	UI::Textarea              m_allowed_label, m_forbidden_label;
	UI::Listselect<Widelands::Building_Index> m_allowed, m_forbidden;
	UI::Button   m_forbid_button, m_allow_button;
	void allowed_selected        (uint32_t);
	void forbidden_selected      (uint32_t);
	void allowed_double_clicked  (uint32_t);
	void forbidden_double_clicked(uint32_t);
	void clicked                 (bool allow);
	void update();
};

#endif
