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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "building_statistics_menu.h"

#include "building.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "player.h"
#include "productionsite.h"
#include "rendertarget.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_progressbar.h"
#include "ui_textarea.h"
#include "tribe.h"
#include "wui_plot_area.h"

#include "upcast.h"

#include <vector>

#define BUILDING_LIST_HEIGHT 220
#define BUILDING_LIST_WIDTH  320

#define LOW_PROD 25

#define UPDATE_TIME 1000  // 1 second, real time

/*
===============
Building_Statistics_Menu::Building_Statistics_Menu

Create all the buttons etc...
===============
*/
Building_Statistics_Menu::Building_Statistics_Menu
(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&parent, &registry, 400, 400, _("Building Statistics")),
m_parent(&parent),

m_table
(this,
 (get_inner_w() - BUILDING_LIST_WIDTH) / 2, 30,
 BUILDING_LIST_WIDTH, BUILDING_LIST_HEIGHT)

{

   // First, we must decide about the size
   int32_t spacing=5;
   int32_t offsx=spacing;
   int32_t offsy=30;
   int32_t posx=offsx;
   int32_t posy=offsy;

   // Building list
	m_table.add_column(_("Name"), 160);
	m_table.add_column(_("Prod"),  40);
	m_table.add_column(_("Owned"), 40);
	m_table.add_column(_("Build"), 40);
   m_table.selected.set(this, &Building_Statistics_Menu::table_changed);

   posy += BUILDING_LIST_HEIGHT + 2*spacing;
   m_end_of_table_y = posy;

   // let place for Picture
   posx = get_inner_w() / 4 + spacing;

   // Toggle when to run button
   UI::Textarea* ta = new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("Total Productivity: "), Align_CenterLeft);
   m_progbar = new UI::Progress_Bar(this, posx + ta->get_w() + spacing, posy, get_inner_w() - (posx + ta->get_w() + spacing) - spacing, 24, UI::Progress_Bar::Horizontal);
   m_progbar->set_total(100);
   posy += 25;

   // owned
   new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("Owned: "), Align_CenterLeft);
	m_owned = new UI::Textarea
		(this, posx+ta->get_w(), posy, 100, 24, Align_CenterLeft);

	m_btn[Prev_Owned] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 58, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Prev_Owned,
		 _("Snow previous"),
		 false);

	m_btn[Next_Owned] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 29, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Next_Owned,
		 _("Snow next"),
		 false);

   posy += 25;

   // build
   new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("In Build: "), Align_CenterLeft);
	m_build = new UI::Textarea
		(this, posx+ta->get_w(), posy, 100, 24, Align_CenterLeft);

	m_btn[Prev_Construction] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 58, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Prev_Construction,
		 _("Snow previous"),
		 false);

	m_btn[Next_Construction] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 29, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Next_Construction,
		 _("Snow next"),
		 false);

   posy += 25;

   // Jump to unproductive
   new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("Jump to unproductive: "), Align_CenterLeft);

	m_btn[Prev_Unproductive] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 58, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Prev_Unproductive,
		 _("Snow previous"),
		 false);

	m_btn[Next_Unproductive] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 29, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Next_Unproductive,
		 _("Snow next"),
		 false);

   posy += 25;

   // TODO: help button
	new UI::Button<Building_Statistics_Menu>
		(this,
		 spacing, get_inner_w() - 37, 32, 32,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
		 &Building_Statistics_Menu::clicked_help, this);

   m_lastupdate = m_parent->get_game()->get_gametime();
   m_anim = 0;
   update();

   m_last_building_index = 0;
}

/*
===============
Building_Statistics_Menu::~Building_Statistics_Menu

Unregister from the registry pointer
===============
*/
Building_Statistics_Menu::~Building_Statistics_Menu()
{
}

/*
 * Think
 *
 * Update this statistic
 */
void Building_Statistics_Menu::think() {
	Widelands::Game const & game = m_parent->game();
	int32_t const gametime = game.get_gametime();

	if
		((gametime - m_lastupdate)
		 /
		 std::max<Widelands::Game::Speed>(1, game.get_speed())
		 >
		 UPDATE_TIME)
	{
      update();
		m_lastupdate = gametime;
	}
}

/*
 * draw()
 *
 * Draw this window
 */
void Building_Statistics_Menu::draw(RenderTarget* dst) {
	if (m_anim)
		dst->drawanim
			(Point
			 (5 + get_inner_w() / 8,
			  m_end_of_table_y + (get_inner_h() - m_end_of_table_y) / 2),
			 m_anim,
			 0,
			 0);

	// Draw all the panels etc. above the background
	UI::Window::draw(dst);
}

/*
 * validate if this pointer is ok
 */
int32_t Building_Statistics_Menu::validate_pointer(int32_t* id, int32_t size) {
	if (*id < 0)
      *id = size-1;
	if (*id >= size)
      *id = 0;

   return *id;
}

/**
 * \todo Implement help
*/
void Building_Statistics_Menu::clicked_help() {
}


void Building_Statistics_Menu::clicked_jump(Jump_Targets id) {
	assert(m_table.has_selection());
	const std::vector<Widelands::Player::Building_Stats> & vec =
		m_parent->get_player()->get_building_statistics(m_table.get_selected());
	const Widelands::Map & map = m_parent->egbase().map();

   bool found = true; // We think, we always find a proper building

	switch (id) {
	case Prev_Owned:
		--m_last_building_index;
		break;
	case Next_Owned:
		++m_last_building_index;
		break;
	case Prev_Construction: {
            int32_t curindex = m_last_building_index;
		while
			(validate_pointer(&(--m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite) break;
	}
		break;
	case Next_Construction: {
            int32_t curindex = m_last_building_index;
		while
			(validate_pointer(&(++m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite) break;
	}
		break;
	case Prev_Unproductive: {
            int32_t curindex = m_last_building_index;
            found = false;
		while
			(validate_pointer(&(--m_last_building_index), vec.size()) != curindex)
			if (not vec[m_last_building_index].is_constructionsite) {
				if
					(upcast
					 (Widelands::ProductionSite,
					  productionsite,
					  map[vec[m_last_building_index].pos].get_immovable()))
					if (productionsite->get_statistics_percent() < LOW_PROD) {
                        found = true;
                        break;
					}
			}
		if (not found) // Now look at the old
			if
				(upcast
				 (Widelands::ProductionSite,
				  productionsite,
				  map[vec[m_last_building_index].pos].get_immovable()))
				if (productionsite->get_statistics_percent() < LOW_PROD)
                     found = true;
	}
         break;
	case Next_Unproductive: {
            int32_t curindex = m_last_building_index;
            found = false;
		while
			(validate_pointer(&(++m_last_building_index), vec.size()) != curindex)
			if (not vec[m_last_building_index].is_constructionsite) {
				if
					(upcast
					 (Widelands::ProductionSite,
					  productionsite,
					  map[vec[m_last_building_index].pos].get_immovable()))
					if (productionsite->get_statistics_percent() < LOW_PROD) {
                        found = true;
                        break;
					}
			}
		if (not found) // Now look at the old
			if
				(upcast
				 (Widelands::ProductionSite,
				  productionsite,
				  map[vec[m_last_building_index].pos].get_immovable()))
				if (productionsite->get_statistics_percent() < LOW_PROD)
                        found = true;
	}
	}

   validate_pointer(&m_last_building_index, vec.size());

	if (found) m_parent->move_view_to(vec[m_last_building_index].pos);
}

/*
 * The table has been selected
 */
void Building_Statistics_Menu::table_changed(uint32_t) {update();}

/*
 * Update table
 */
void Building_Statistics_Menu::update() {
   m_owned->set_text("");
   m_build->set_text("");
   m_progbar->set_state(0);

   // List all buildings
	Widelands::Tribe_Descr const & tribe = m_parent->player().tribe();
	Widelands::Map         const & map   = m_parent->get_game()->map();
	for (Widelands::Building_Descr::Index i = 0; i < tribe.get_nrbuildings(); ++i) {
		Widelands::Building_Descr const & building = *tribe.get_building_descr(i);
		{
			const std::string & name = building.name();
			if (name == "constructionsite" or name == "headquarters") continue;
		}

		std::vector<Widelands::Player::Building_Stats> const & vec =
			m_parent->get_player()->get_building_statistics(i);

      // walk all entries, add new ones if needed
		UI::Table<const intptr_t>::Entry_Record * te = 0;
		const uint32_t table_size = m_table.size();
		for (uint32_t l = 0; l < table_size; ++l) {
			UI::Table<const intptr_t>::Entry_Record & er = m_table.get_record(l);
			if (UI::Table<const intptr_t>::get(er) == i) {
				te = &er;
            break;
			}
		}

      // If not in list, add new one, as long as this building is
      // enabled
		if (not te) {
			if (! m_parent->player().is_building_allowed(i))
				continue;
			te = &m_table.add(i, building.get_buildicon());
		}

		uint32_t nr_owned   = 0;
		uint32_t nr_build   = 0;
		uint32_t total_prod = 0;
		upcast(Widelands::ProductionSite_Descr const, productionsite, &building);
		for (uint32_t l = 0; l < vec.size(); ++l) {
			if (vec[l].is_constructionsite) ++nr_build;
			else {
				++nr_owned;
				if (productionsite)
					total_prod +=
						dynamic_cast<Widelands::ProductionSite &>
						(*map.get_field(vec[l].pos)->get_immovable())
						.get_statistics_percent();

			}
		}

          // Is this entry selected?
		const bool is_selected =
			m_table.has_selection() and m_table.get_selected() == i;

		if (is_selected) {
			m_anim = building.get_ui_anim();
			m_btn[Prev_Owned]       ->set_enabled(nr_owned);
			m_btn[Next_Owned]       ->set_enabled(nr_owned);
			m_btn[Prev_Construction]->set_enabled(nr_build);
			m_btn[Next_Construction]->set_enabled(nr_build);
		}

          // Add new Table Entry
          char buffer[100];
		te->set_string(0, building.descname());

          // Product
		if (productionsite and nr_owned) {
			const uint32_t percent = static_cast<uint32_t>
				(static_cast<float>(total_prod) / static_cast<float>(nr_owned));
			snprintf(buffer, sizeof(buffer), "%i", percent);
			if (is_selected)  {
				m_progbar->set_state(percent);
				m_btn[Prev_Unproductive]->set_enabled(true);
				m_btn[Next_Unproductive]->set_enabled(true);
			}
		} else {
			snprintf(buffer, sizeof(buffer), "-");
			if (is_selected) {
				m_btn[Prev_Unproductive]->set_enabled(false);
				m_btn[Next_Unproductive]->set_enabled(false);
			}
		}
          te->set_string(1, buffer);

          // Number of this buildings
		snprintf(buffer, sizeof(buffer), "%i", nr_owned);
          te->set_string(2, buffer);
		if (is_selected)
            m_owned->set_text(buffer);

          // Number of currently builds
		snprintf(buffer, sizeof(buffer), "%i", nr_build);
          te->set_string(3, buffer);
		if (is_selected)
            m_build->set_text(buffer);
	}

   // Disable all buttons, if nothing to select
	if (not m_table.has_selection()) {
      m_btn[Prev_Owned]       ->set_enabled(false);
      m_btn[Next_Owned]       ->set_enabled(false);
      m_btn[Prev_Construction]->set_enabled(false);
      m_btn[Next_Construction]->set_enabled(false);
      m_btn[Prev_Unproductive]->set_enabled(false);
      m_btn[Next_Unproductive]->set_enabled(false);
	}
}
