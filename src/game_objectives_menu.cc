/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "game_objectives_menu.h"

#include "interactive_player.h"
#include "map_objective_manager.h"
#include "player.h"
#include "trigger/trigger_null.h"


GameObjectivesMenu::GameObjectivesMenu
(Interactive_Player & plr, UI::UniqueWindow::Registry & registry, Game & game)
:
UI::UniqueWindow
(&plr, &registry, 340, 5 + 120 + 5 + 240 + 5, _("Objectives Menu")),
list         (this, 5,   5, get_inner_w() - 10, 120, Align_Left, false),
objectivetext(this, 5, 130, get_inner_w() - 10, 240, "", Align_Left, 1)
{
   // Listselect with Objectives
	MapObjectiveManager & mom = game.map().get_mom();
	for (MapObjectiveManager::Index i = 0; i < mom.get_nr_objectives(); ++i) {
		MapObjective & obj = mom.get_objective_by_nr(i);
		if (not obj.get_is_visible()) continue;
		if (obj.get_trigger()->is_set()) continue;

		list.add(obj.name().c_str(), obj);
		if (obj.get_is_optional())
			list.set_entry_color(list.size() - 1, RGBColor(255, 0, 0));
	}
   list.selected.set(this, &GameObjectivesMenu::selected);

   // If any objectives, select the first one
	if (list.size()) list.select(0);

	if (get_usedefaultpos())
		center_to_parent();
}


/*
 * Something has been selected
 */
void GameObjectivesMenu::selected(uint t)
{objectivetext.set_text(list[t].descr());}
