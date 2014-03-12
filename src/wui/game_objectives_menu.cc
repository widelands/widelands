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

#include "logic/player.h"
#include "wui/interactive_player.h"

using namespace Widelands;

#define BUTTON_HEIGHT 20
#define OBJECTIVE_LIST 120
#define FULL_OBJECTIVE_TEXT 240


inline Interactive_Player & GameObjectivesMenu::iplayer() const {
	return ref_cast<Interactive_Player, UI::Panel>(*get_parent());
}


GameObjectivesMenu::GameObjectivesMenu
	(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&plr, "objectives", &registry,
		 580,
		 5 + OBJECTIVE_LIST + 5 + FULL_OBJECTIVE_TEXT + 5 + BUTTON_HEIGHT + 5,
		 _("Objectives")),
	list
		(this,
		 5, 5,
		 get_inner_w() - 10, OBJECTIVE_LIST,
		 UI::Align_Left,
		 false),
	objectivetext
		(this,
		 5, 130,
		 get_inner_w() - 10, FULL_OBJECTIVE_TEXT,
		 "",
		 UI::Align_Left,
		 1),
#if 0
	m_claim_victory
		(this,
		 25,
		 5 + OBJECTIVE_LIST + 5 + FULL_OBJECTIVE_TEXT + 5,
		 BUTTON_WIDTH, BUTTON_HEIGHT,
		 g_gr->images().get("pics/but4.png"),
		 &GameObjectivesMenu::claim_victory, *this,
		 _("Claim Victory"), std::string(), false),
	m_restart_mission
		(this,
		 25 + BUTTON_WIDTH + 25,
		 5 + OBJECTIVE_LIST + 5 + FULL_OBJECTIVE_TEXT + 5,
		 BUTTON_WIDTH, BUTTON_HEIGHT,
		 g_gr->images().get("pics/but4.png"),
		 &GameObjectivesMenu::restart_mission, *this,
		 _("Restart Mission")),
#endif
	m_player(plr)
{
	list.selected.connect(boost::bind(&GameObjectivesMenu::selected, this, _1));
	if (get_usedefaultpos())
		center_to_parent();
}


void GameObjectivesMenu::think() {
	victorious(iplayer().get_playertype() == VICTORIOUS);

	//  Adjust the list according to the game state.
	Manager<Widelands::Objective> & mom = iplayer().game().map().mom();
	Manager<Widelands::Objective>::Index const nr_objectives = mom.size();
	for (Manager<Widelands::Objective>::Index i = 0; i < nr_objectives; ++i) {
		bool should_show =
			mom[i].visible() and not mom[i].done();
		uint32_t const list_size = list.size();
		for (uint32_t j = 0;; ++j)
			if (j == list_size) { //  the objective is not in our list
				if     (should_show)
					list.add(mom[i].descname().c_str(), mom[i]);
				break;
			} else if (&list[j] == &mom[i]) { //  the objective is in our list
				if (not should_show)
					list.remove(j);
				else if (list[j].descname() != mom[i].descname() || list[j].descr() != mom[i].descr()) {
					// Update
					list.remove(j);
					list.add(mom[i].descname().c_str(), mom[i]);
				}
				break;
			}
	}
	list.sort();
	if (list.size() and not list.has_selection())
		list.select(0);
}

/**
 * An entry in the objectives menu has been selected
 */
void GameObjectivesMenu::selected(uint32_t const t) {
	objectivetext.set_text
		(t == list_type::no_selection_index() ? std::string() : list[t].descr());
}

/**
 * TODO Claim Victory & exit this game
 * once this is implemented uncomment the button
 */
void GameObjectivesMenu::claim_victory() {
	m_player.end_modal(0);
}

/**
 * TODO Restart this campaign
 * once this is implemented uncomment the button
 */
void GameObjectivesMenu::restart_mission() {
	m_player.end_modal(0);
}
