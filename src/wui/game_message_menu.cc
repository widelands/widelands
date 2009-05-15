/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "game_message_menu.h"

#include "interactive_player.h"
#include "message_queue.h"
#include "logic/player.h"
#include "trigger/trigger_time.h"


inline Interactive_Player & GameMessageMenu::iplayer() const {
	return dynamic_cast<Interactive_Player &>(*get_parent());
}


GameMessageMenu::GameMessageMenu
	(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
	:
	UI::UniqueWindow
		(&plr, &registry, 340, 5 + 25 + 5 + 110 + 5 + 220 + 5, _("Message Menu")),
	list(this, 5,   30, get_inner_w() - 10, 110, Align_Left, false),
	messagetext
		(this, 5, 5 + 25 + 5 + 110, get_inner_w() - 10, 220, "", Align_Left, 1),
	hide
		(this,
		 5, 5,
		 25, 25,
		 2,
		 &GameMessageMenu::clicked_hide, *this, _("del")),
	view
		(this,
		 5 + 25 + 5, 5,
		 25, 25,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_goto.png"),
		 &GameMessageMenu::clicked_view, *this, _("go to location")),
	sort
		(this,
		 5 + 25 + 5 + 25 + 5,5,
		 25, 25,
		 2,
		 g_gr->get_picture(PicMod_Game, "pics/menu_sort.png"),
		 &GameMessageMenu::clicked_sort, *this, _("sort messages")),
	m_direction(1)
{
	list.selected.set(this, &GameMessageMenu::selected);
	if (get_usedefaultpos())
		center_to_parent();
}


void GameMessageMenu::think() {
	std::vector<Widelands::Message> & mmm =
		Widelands::MessageQueue::get(iplayer().player());
	int const nr_messages = mmm.size();
	log (" we have %i messages \n",nr_messages);
	for (int i = 0; i < nr_messages; ++i) {
		if (mmm[i].get_is_visible()) {
			for (uint32_t j = 0;;++j) {
				if (j == list.size()) {
					log ("think: message %s\n",mmm[i].visname().c_str());
					if (m_direction < 0) {
						list.add_front(mmm[i].visname().c_str(),mmm[i]);
					} else {
						list.add(mmm[i].visname().c_str(), mmm[i]);
					}
					break;
				} else if (&list[j] == &mmm[i]) {
					break;
				}
			}
		}
	}
	//list.sort();
	if (list.size() and not list.has_selection())
		list.select(0);

}

/*
 * Something has been selected
 */
void GameMessageMenu::selected(uint32_t const t) {
	messagetext.set_text
		(t == list_type::no_selection_index() ? std::string() : list[t].descr());
}

void GameMessageMenu::clicked_hide() {
	size_t selection = list.selection_index();
	if (selection < list.size()) {
		list[selection].set_is_visible(false);
		list.remove_selected();
		if (selection < list.size())
		{
			list.select(selection);
		}
	}
}

void GameMessageMenu::clicked_view() {
	//zoom to the location where the message was centered
	size_t selection = list.selection_index();
	if (selection < list.size()) {
		iplayer().move_view_to(list[selection].get_coords());
	}
}

void GameMessageMenu::clicked_sort() {
	m_direction = -1 * m_direction;
	list.clear();
}
