/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "wui/game_objectives_menu.h"

#include "logic/objective.h"
#include "logic/player.h"
#include "wui/interactive_player.h"

using namespace Widelands;

#define BUTTON_HEIGHT 20
#define OBJECTIVE_LIST 120
#define FULL_OBJECTIVE_TEXT 240


inline InteractivePlayer & GameObjectivesMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}


GameObjectivesMenu::GameObjectivesMenu
	(UI::Panel * parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(parent, "objectives", &registry,
		 580,
		 5 + OBJECTIVE_LIST + 5 + FULL_OBJECTIVE_TEXT + 5 + BUTTON_HEIGHT + 5,
		 _("Objectives")),
	list
		(this,
		 5, 5,
		 get_inner_w() - 10, OBJECTIVE_LIST,
		 UI::Align::kLeft,
		 false),
	objectivetext
		(this,
		 5, 130,
		 get_inner_w() - 10, FULL_OBJECTIVE_TEXT,
		 "",
		 UI::Align::kLeft,
		 1)
{
	list.selected.connect(boost::bind(&GameObjectivesMenu::selected, this, _1));
	if (get_usedefaultpos())
		center_to_parent();
}

void GameObjectivesMenu::think() {
	//  Adjust the list according to the game state.
	for (const auto& pair : iplayer().game().map().objectives()) {
		const Objective& obj = *(pair.second);
		bool should_show = obj.visible() && !obj.done();
		uint32_t const list_size = list.size();
		for (uint32_t j = 0;; ++j)
			if (j == list_size) {  //  the objective is not in our list
				if (should_show)
					list.add(obj.descname(), obj);
				break;
			} else if (&list[j] == &obj) {  //  the objective is in our list
				if (!should_show)
					list.remove(j);
				else if (list[j].descname() != obj.descname() || list[j].descr() != obj.descr()) {
					// Update
					list.remove(j);
					list.add(obj.descname(), obj);
				}
				break;
			}
	}
	list.sort();
	if (list.size() && !list.has_selection())
		list.select(0);
}

/**
 * An entry in the objectives menu has been selected
 */
void GameObjectivesMenu::selected(uint32_t const t) {
	objectivetext.set_text
		(t == ListType::no_selection_index() ? std::string() : list[t].descr());
}
