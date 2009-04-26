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
#include "graphic/graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "mapviewpixelconstants.h"
#include "player.h"
#include "productionsite.h"
#include "rendertarget.h"
#include "tribe.h"
#include "wui_plot_area.h"

#include "ui_button.h"

#include "upcast.h"

#include <vector>

#define WINDOW_WIDTH         440
#define WINDOW_HEIGHT        440
#define VMARGIN                5
#define HMARGIN                5
#define VSPACING               5
#define HSPACING               5
#define BUILDING_LIST_HEIGHT 285
#define BUILDING_LIST_WIDTH  (WINDOW_WIDTH - HMARGIN - HMARGIN)
#define LABEL_X              178
#define LABEL_WIDTH          122
#define VALUE_X              (LABEL_X + LABEL_WIDTH)
#define JUMP_PREV_BUTTON_X   (WINDOW_WIDTH - HMARGIN - 24 - HSPACING - 24)
#define JUMP_NEXT_BUTTON_X   (WINDOW_WIDTH - HMARGIN - 24)
#define TOTAL_PRODUCTIVITY_Y (VMARGIN + BUILDING_LIST_HEIGHT + VSPACING + 22)
#define PROGRESS_BAR_Y       (TOTAL_PRODUCTIVITY_Y + 24)
#define OWNED_Y              (PROGRESS_BAR_Y       + 24)
#define IN_BUILD_Y           (OWNED_Y              + 24)
#define UNPRODUCTIVE_Y       (IN_BUILD_Y           + 24)
#define FLAG_POINT           Point(125, WINDOW_HEIGHT - 8)

#define LOW_PROD 25

#define UPDATE_TIME 1000  // 1 second, real time


inline Interactive_Player & Building_Statistics_Menu::iplayer() const {
	return dynamic_cast<Interactive_Player &>(*get_parent());
}


/*
===============
Building_Statistics_Menu::Building_Statistics_Menu

Create all the buttons etc...
===============
*/
Building_Statistics_Menu::Building_Statistics_Menu
	(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow
	(&parent, &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Building Statistics")),
m_table
	(this, HMARGIN, VMARGIN, BUILDING_LIST_WIDTH, BUILDING_LIST_HEIGHT),
m_progbar
	(this,
	 LABEL_X, PROGRESS_BAR_Y, WINDOW_WIDTH - LABEL_X - HMARGIN, 20,
	 UI::Progress_Bar::Horizontal),
m_total_productivity_label
	(this,
	 LABEL_X, TOTAL_PRODUCTIVITY_Y, LABEL_WIDTH, 24,
	 _("Total Productivity: "), Align_CenterLeft),
m_owned_label
	(this, LABEL_X, OWNED_Y, LABEL_WIDTH, 24, _("Owned: "), Align_CenterLeft),
m_owned
	(this, VALUE_X, OWNED_Y, 100, 24, Align_CenterLeft),
m_in_build_label
	(this,
	 LABEL_X, IN_BUILD_Y, LABEL_WIDTH, 24,
	 _("In Build: "), Align_CenterLeft),
m_in_build
	(this, VALUE_X, IN_BUILD_Y, 100, 24, Align_CenterLeft),
m_unproductive_label
	(this,
	 LABEL_X, UNPRODUCTIVE_Y, LABEL_WIDTH, 24,
	 _("Jump to unproductive: "), Align_CenterLeft),
m_anim               (0),
m_lastupdate         (0),
m_last_building_index(0)
{
	//  building list
	m_table.add_column (32);
	m_table.add_column(174, _("Name"));
	m_table.add_column (50, _("Size"),     Align_HCenter);
	m_table.add_column (50, _("Prod"),     Align_Right);
	m_table.add_column (50, _("Owned"),    Align_Right);
	m_table.add_column (50, _("Build"),    Align_HCenter);
	m_table.selected.set(this, &Building_Statistics_Menu::table_changed);

	//  toggle when to run button
	m_progbar.set_total(100);

	m_btn[Prev_Owned] = new UI::Callback_IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 JUMP_PREV_BUTTON_X, OWNED_Y, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, *this, Prev_Owned,
		 _("Show previous"),
		 false);

	m_btn[Next_Owned] = new UI::Callback_IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 JUMP_NEXT_BUTTON_X, OWNED_Y, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, *this, Next_Owned,
		 _("Show next"),
		 false);

	m_btn[Prev_Construction] = new UI::Callback_IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 JUMP_PREV_BUTTON_X, IN_BUILD_Y, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, *this, Prev_Construction,
		 _("Show previous"),
		 false);

	m_btn[Next_Construction] = new UI::Callback_IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 JUMP_NEXT_BUTTON_X, IN_BUILD_Y, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, *this, Next_Construction,
		 _("Show next"),
		 false);

	m_btn[Prev_Unproductive] = new UI::Callback_IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 JUMP_PREV_BUTTON_X, UNPRODUCTIVE_Y, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, *this, Prev_Unproductive,
		 _("Show previous"),
		 false);

	m_btn[Next_Unproductive] = new UI::Callback_IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 JUMP_NEXT_BUTTON_X, UNPRODUCTIVE_Y, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, *this, Next_Unproductive,
		 _("Show next"),
		 false);
}


/*
 * Update this statistic
 */
void Building_Statistics_Menu::think() {
	Widelands::Game const & game = iplayer().game();
	int32_t const gametime = game.get_gametime();

	if ((gametime - m_lastupdate) > UPDATE_TIME) {
		update();
		m_lastupdate = gametime;
	}
}

/*
 * draw()
 *
 * Draw this window
 */
void Building_Statistics_Menu::draw(RenderTarget & dst) {
	Widelands::Player const & player = iplayer().player();
	if (m_anim)
		dst.drawanim
			(FLAG_POINT - Point(TRIANGLE_WIDTH / 2, TRIANGLE_HEIGHT),
			 m_anim, 0, &player);
	dst.drawanim
		(FLAG_POINT, player.tribe().get_flag_anim(), 0, &player);

	// Draw all the panels etc. above the background
	UI::Window::draw(dst);
}

/*
 * validate if this pointer is ok
 */
int32_t Building_Statistics_Menu::validate_pointer
	(int32_t * const id, int32_t const size)
{
	if (*id < 0)
		*id = size - 1;
	if (size <= *id)
		*id = 0;

	return *id;
}


void Building_Statistics_Menu::clicked_jump(Jump_Targets id) {
	assert(m_table.has_selection());
	const std::vector<Widelands::Player::Building_Stats> & vec =
		iplayer().get_player()->get_building_statistics
			(Widelands::Building_Index
			 	(static_cast<size_t>(m_table.get_selected())));
	Widelands::Map const & map = iplayer().egbase().map();

	bool found = true; //  we think, we always find a proper building

	switch (id) {
	case Prev_Owned:
		--m_last_building_index;
		break;
	case Next_Owned:
		++m_last_building_index;
		break;
	case Prev_Construction: {
		int32_t const curindex = m_last_building_index;
		while
			(validate_pointer(&(--m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite) break;
		break;
	}
	case Next_Construction: {
		int32_t const curindex = m_last_building_index;
		while
			(validate_pointer(&(++m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite) break;
		break;
	}
	case Prev_Unproductive: {
		int32_t const curindex = m_last_building_index;
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
		break;
	}
	case Next_Unproductive: {
		int32_t const curindex = m_last_building_index;
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
		break;
	}
	default:
		assert(false);
	}

	validate_pointer(&m_last_building_index, vec.size());

	if (found)
		iplayer().move_view_to(vec[m_last_building_index].pos);
}

/*
 * The table has been selected
 */
void Building_Statistics_Menu::table_changed(uint32_t) {update();}

namespace Columns {enum {Icon, Name, Size, Prod, Owned, Build};};

/*
 * Update table
 */
void Building_Statistics_Menu::update() {
	m_owned   .set_text("");
	m_in_build.set_text("");
	m_progbar .set_state(0);

	Widelands::Tribe_Descr const & tribe = iplayer().player().tribe();
	Widelands::Map         const & map   = iplayer().game().map();
	Widelands::Building_Index      const nr_buildings = tribe.get_nrbuildings();
	for
		(Widelands::Building_Index i = Widelands::Building_Index::First();
		 i < nr_buildings;
		 ++i)
	{
		Widelands::Building_Descr const & building = *tribe.get_building_descr(i);
		if (not (building.buildable() or building.get_enhanced_building()))
			continue;

		std::vector<Widelands::Player::Building_Stats> const & vec =
			iplayer().get_player()->get_building_statistics(i);

		//  walk all entries, add new ones if needed
		UI::Table<const intptr_t>::Entry_Record * te = 0;
		const uint32_t table_size = m_table.size();
		for (uint32_t l = 0; l < table_size; ++l) {
			UI::Table<const intptr_t>::Entry_Record & er = m_table.get_record(l);
			if (UI::Table<const intptr_t>::get(er) == i.value()) {
				te = &er;
				break;
			}
		}

		//  If not in list, add new one, as long as this building is enabled.
		if (not te) {
			if (! iplayer().player().is_building_allowed(i))
				continue;
			te = &m_table.add(i.value());
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
						(*map[vec[l].pos].get_immovable()).get_statistics_percent();
			}
		}

		const bool is_selected = //  Is this entry selected?
			m_table.has_selection() and m_table.get_selected() == i.value();

		if (is_selected) {
			m_anim = building.get_ui_anim();
			m_btn[Prev_Owned]       ->set_enabled(nr_owned);
			m_btn[Next_Owned]       ->set_enabled(nr_owned);
			m_btn[Prev_Construction]->set_enabled(nr_build);
			m_btn[Next_Construction]->set_enabled(nr_build);
		}

		te->set_picture(Columns::Icon, building.get_buildicon(), "FEL");

		//  add new Table Entry
		char buffer[100];
		te->set_string(Columns::Name, building.descname());

		{
			char const * str = "";
			char const * pic = "pics/novalue.png";
			if (building.get_ismine()) {
				str = "M";
				pic = "pics/mine.png";
			} else switch (building.get_size()) {
			case Widelands::BaseImmovable::SMALL:
				str = "1";
				pic = "pics/small.png";
				break;
			case Widelands::BaseImmovable::MEDIUM:
				str = "2";
				pic = "pics/medium.png";
				break;
			case Widelands::BaseImmovable::BIG:
				str = "3";
				pic = "pics/big.png";
				break;
			default:
				assert(false);
			}
			te->set_picture
				(Columns::Size, g_gr->get_picture(PicMod_UI, pic), str);
		}

		if (productionsite and nr_owned) {
			const uint32_t percent = static_cast<uint32_t>
				(static_cast<float>(total_prod) / static_cast<float>(nr_owned));
			snprintf(buffer, sizeof(buffer), "%i", percent);
			if (is_selected)  {
				m_progbar.set_state(percent);
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
		te->set_string(Columns::Prod, buffer);

		//  number of this buildings
		snprintf(buffer, sizeof(buffer), "%3i", nr_owned); // space-pad for sort
		te->set_string(Columns::Owned, buffer);
		if (is_selected)
			m_owned.set_text(buffer);

		//  number of currently builds
		snprintf(buffer, sizeof(buffer), "%3i", nr_build); // space-pad for sort
		te->set_string(Columns::Build, buffer);
		if (is_selected)
			m_in_build.set_text(buffer);
	}

	//  disable all buttons, if nothing to select
	if (not m_table.has_selection()) {
		m_btn[Prev_Owned]       ->set_enabled(false);
		m_btn[Next_Owned]       ->set_enabled(false);
		m_btn[Prev_Construction]->set_enabled(false);
		m_btn[Next_Construction]->set_enabled(false);
		m_btn[Prev_Unproductive]->set_enabled(false);
		m_btn[Next_Unproductive]->set_enabled(false);
	}
}
