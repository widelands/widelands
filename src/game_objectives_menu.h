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

#ifndef __S__GAME_OBJECTIVE_MENU_H
#define __S__GAME_OBJECTIVE_MENU_H

#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_unique_window.h"

namespace Widelands {
class Game;
struct MapObjective;
};
class Interactive_Player;

// The GameObjectives Menu shows the not already
// fullfilled scenario objectives.
struct GameObjectivesMenu : public UI::UniqueWindow {
	GameObjectivesMenu
		(Interactive_Player &, UI::UniqueWindow::Registry &, Widelands::Game &);

private:
	UI::Listselect<Widelands::MapObjective &>list;
	UI::Multiline_Textarea objectivetext;

	void selected(uint32_t);
};

#endif
