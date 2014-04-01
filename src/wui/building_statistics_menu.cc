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

#include "wui/building_statistics_menu.h"

#include <vector>

#include <boost/bind.hpp>

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "i18n.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/tribe.h"
#include "ui_basic/button.h"
#include "upcast.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/plot_area.h"

#define WINDOW_WIDTH         625
#define WINDOW_HEIGHT        440
#define VMARGIN                5
#define HMARGIN                5
#define VSPACING               5
#define HSPACING               5
#define BUILDING_LIST_HEIGHT 285
#define BUILDING_LIST_WIDTH  (WINDOW_WIDTH - HMARGIN - HMARGIN)
#define LABEL_X              200
#define LABEL_WIDTH          150
#define VALUE_X              (LABEL_X + LABEL_WIDTH)
#define JUMP_PREV_BUTTON_X   (WINDOW_WIDTH - HMARGIN - 24 - HSPACING - 24)
#define JUMP_NEXT_BUTTON_X   (WINDOW_WIDTH - HMARGIN - 24)
#define TOTAL_PRODUCTIVITY_Y (VMARGIN + BUILDING_LIST_HEIGHT + VSPACING + 22)
#define PROGRESS_BAR_Y       (TOTAL_PRODUCTIVITY_Y + 24)
#define OWNED_Y              (PROGRESS_BAR_Y       + 24)
#define IN_BUILD_Y           (OWNED_Y              + 24)
#define UNPRODUCTIVE_Y       (IN_BUILD_Y           + 24)
#define FLAG_POINT           Point(125, WINDOW_HEIGHT - 8)

#define LOW_PROD 33

#define UPDATE_TIME 1000  //  1 second, gametime


namespace Columns {enum {Name, Size, Prod, Owned, Build};}

inline Interactive_Player & Building_Statistics_Menu::iplayer() const {
	return ref_cast<Interactive_Player, UI::Panel>(*get_parent());
}

Building_Statistics_Menu::Building_Statistics_Menu
	(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&parent, "building_statistics",
		 &registry,
		 WINDOW_WIDTH, WINDOW_HEIGHT,
		 _("Building Statistics")),
	m_table
		(this, HMARGIN, VMARGIN, BUILDING_LIST_WIDTH, BUILDING_LIST_HEIGHT),
	m_progbar
		(this,
		 LABEL_X, PROGRESS_BAR_Y, WINDOW_WIDTH - LABEL_X - HMARGIN, 20,
		 UI::Progress_Bar::Horizontal),
	m_total_productivity_label
		(this,
		 LABEL_X, TOTAL_PRODUCTIVITY_Y, LABEL_WIDTH, 24,
		 _("Total Productivity:"), UI::Align_CenterLeft),
	m_owned_label
		(this,
		 LABEL_X, OWNED_Y, LABEL_WIDTH, 24,
		 _("Owned:"), UI::Align_CenterLeft),
	m_owned
		(this, VALUE_X, OWNED_Y, 100, 24, UI::Align_CenterLeft),
	m_in_build_label
		(this,
		 LABEL_X, IN_BUILD_Y, LABEL_WIDTH, 24,
		 _("Being built:"), UI::Align_CenterLeft),
	m_in_build
		(this, VALUE_X, IN_BUILD_Y, 100, 24, UI::Align_CenterLeft),
	m_unproductive_label
		(this,
		 LABEL_X, UNPRODUCTIVE_Y, LABEL_WIDTH, 24,
		 _("Jump to unproductive"), UI::Align_CenterLeft),
	m_anim               (0),
	m_lastupdate         (0),
	m_last_building_index(0),
	m_last_table_index   (0)
{
	//  building list
	m_table.add_column(310, _("Name"));
	m_table.add_column (70, _("Type"), "",     UI::Align_HCenter);
	m_table.add_column (70, _("Prod"), "",     UI::Align_Right);
	m_table.add_column (70, _("Owned"), "",    UI::Align_Right);
	m_table.add_column (70, _("Build"), "",    UI::Align_Right);
	m_table.selected.connect(boost::bind(&Building_Statistics_Menu::table_changed, this, _1));
	m_table.set_column_compare
		(Columns::Size,
		 boost::bind
		 	(&Building_Statistics_Menu::compare_building_size, this, _1, _2));

	//  toggle when to run button
	m_progbar.set_total(100);

	m_btn[Prev_Owned] =
		new UI::Button
			(this, "previous_owned",
			 JUMP_PREV_BUTTON_X, OWNED_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_left.png"),
			 _("Show previous"),
			 false);
	m_btn[Prev_Owned]->sigclicked.connect
		(boost::bind(&Building_Statistics_Menu::clicked_jump, boost::ref(*this), Prev_Owned));

	m_btn[Next_Owned] =
		new UI::Button
			(this, "next_owned",
			 JUMP_NEXT_BUTTON_X, OWNED_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_right.png"),
			 _("Show next"),
			 false);
	m_btn[Next_Owned]->sigclicked.connect
		(boost::bind(&Building_Statistics_Menu::clicked_jump, boost::ref(*this), Next_Owned));

	m_btn[Prev_Construction] =
		new UI::Button
			(this, "previous_constructed",
			 JUMP_PREV_BUTTON_X, IN_BUILD_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_left.png"),
			 _("Show previous"),
			 false);
	m_btn[Prev_Construction]->sigclicked.connect
		(boost::bind(&Building_Statistics_Menu::clicked_jump, boost::ref(*this), Prev_Construction));

	m_btn[Next_Construction] =
		new UI::Button
			(this, "next_constructed",
			 JUMP_NEXT_BUTTON_X, IN_BUILD_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_right.png"),
			 _("Show next"),
			 false);
	m_btn[Next_Construction]->sigclicked.connect
		(boost::bind(&Building_Statistics_Menu::clicked_jump, boost::ref(*this), Next_Construction));

	m_btn[Prev_Unproductive] =
		new UI::Button
			(this, "previous_unproductive",
			 JUMP_PREV_BUTTON_X, UNPRODUCTIVE_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_left.png"),
			 _("Show previous"),
			 false);
	m_btn[Prev_Unproductive]->sigclicked.connect
		(boost::bind(&Building_Statistics_Menu::clicked_jump, boost::ref(*this), Prev_Unproductive));

	m_btn[Next_Unproductive] =
		new UI::Button
			(this, "next_unproductive",
			 JUMP_NEXT_BUTTON_X, UNPRODUCTIVE_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_right.png"),
			 _("Show next"),
			 false);
	m_btn[Next_Unproductive]->sigclicked.connect
		(boost::bind(&Building_Statistics_Menu::clicked_jump, boost::ref(*this), Next_Unproductive));
}


/*
 * Update this statistic
 */
void Building_Statistics_Menu::think() {
	const Widelands::Game & game = iplayer().game();
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
	UI::Window::draw(dst);

	const Widelands::Player & player = iplayer().player();
	if (m_anim)
		dst.drawanim
			(FLAG_POINT - Point(TRIANGLE_WIDTH / 2, TRIANGLE_HEIGHT),
			 m_anim, 0, &player);
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


void Building_Statistics_Menu::clicked_jump(Jump_Targets const id) {
	assert(m_table.has_selection());
	if (m_last_table_index != m_table.selection_index())
		m_last_building_index = 0;
	m_last_table_index = m_table.selection_index();
	const std::vector<Widelands::Player::Building_Stats> & vec =
		iplayer().get_player()->get_building_statistics
			(Widelands::Building_Index
				(static_cast<size_t>(m_table.get_selected())));
	const Widelands::Map & map = iplayer().egbase().map();

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
			if (vec[m_last_building_index].is_constructionsite)
				break;
		break;
	}
	case Next_Construction: {
		int32_t const curindex = m_last_building_index;
		while
			(validate_pointer(&(++m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite)
				break;
		break;
	}
	case Prev_Unproductive: {
		int32_t const curindex = m_last_building_index;
		found = false;
		while (validate_pointer(&(--m_last_building_index), vec.size()) != curindex)
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
		break;
	}

	validate_pointer(&m_last_building_index, vec.size());

	if (found)
		iplayer().move_view_to(vec[m_last_building_index].pos);
}

/*
 * The table has been selected
 */
void Building_Statistics_Menu::table_changed(uint32_t) {update();}

/**
 * Callback to sort table based on building size.
 */
bool Building_Statistics_Menu::compare_building_size
	(uint32_t const rowa, uint32_t const rowb)
{
	const Widelands::Tribe_Descr & tribe = iplayer().player().tribe();
	Widelands::Building_Index a = Widelands::Building_Index(m_table[rowa]);
	Widelands::Building_Index b = Widelands::Building_Index(m_table[rowb]);
	const Widelands::Building_Descr * descra = tribe.get_building_descr(a);
	const Widelands::Building_Descr * descrb = tribe.get_building_descr(b);

	if (!descra || !descrb)
		return false; // shouldn't happen, but be defensive

	// mines come last
	if (descrb->get_ismine())
		return !descra->get_ismine();
	else if (descra->get_ismine())
		return false;

	// smallest first
	return descra->get_size() < descrb->get_size();
}


/*
 * Update table
 */
void Building_Statistics_Menu::update() {
	m_owned   .set_text("");
	m_in_build.set_text("");
	m_progbar .set_state(0);

	const Widelands::Player      & player = iplayer().player();
	const Widelands::Tribe_Descr & tribe  = player.tribe();
	const Widelands::Map         & map   = iplayer().game().map();
	Widelands::Building_Index      const nr_buildings = tribe.get_nrbuildings();
	for
		(Widelands::Building_Index i = Widelands::Building_Index::First();
		 i < nr_buildings;
		 ++i)
	{
		const Widelands::Building_Descr & building =
			*tribe.get_building_descr(i);
		if
			(not (building.is_buildable()
			 or building.is_enhanced()
			 or building.global()))
			continue;

		const std::vector<Widelands::Player::Building_Stats> & vec =
			player.get_building_statistics(i);

		//  walk all entries, add new ones if needed
		UI::Table<uintptr_t const>::Entry_Record * te = nullptr;
		const uint32_t table_size = m_table.size();
		for (uint32_t l = 0; l < table_size; ++l) {
			UI::Table<uintptr_t const>::Entry_Record & er = m_table.get_record(l);
			if (UI::Table<uintptr_t const>::get(er) == i.value()) {
				te = &er;
				break;
			}
		}

		//  If not in list, add new one, as long as this building is enabled.
		if (not te) {
			if (! iplayer().player().is_building_type_allowed(i))
				continue;
			te = &m_table.add(i.value());
		}

		uint32_t nr_owned   = 0;
		uint32_t nr_build   = 0;
		uint32_t total_prod = 0;
		upcast(Widelands::ProductionSite_Descr const, productionsite, &building);
		for (uint32_t l = 0; l < vec.size(); ++l) {
			if (vec[l].is_constructionsite)
				++nr_build;
			else {
				++nr_owned;
				if (productionsite)
					total_prod +=
						ref_cast<Widelands::ProductionSite, Widelands::BaseImmovable>
							(*map[vec[l].pos].get_immovable())
						.get_statistics_percent();
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

		//  add new Table Entry
		char buffer[100];
		te->set_picture
			(Columns::Name, building.get_buildicon(), building.descname());

		{
			char const * pic = "pics/novalue.png";
			if (building.get_ismine()) {
				pic = "pics/menu_tab_buildmine.png";
			} else if (building.get_isport()) {
				pic = "pics/menu_tab_buildport.png";
			}
			else switch (building.get_size()) {
			case Widelands::BaseImmovable::SMALL:
				pic = "pics/menu_tab_buildsmall.png";
				break;
			case Widelands::BaseImmovable::MEDIUM:
				pic = "pics/menu_tab_buildmedium.png";
				break;
			case Widelands::BaseImmovable::BIG:
				pic = "pics/menu_tab_buildbig.png";
				break;
			default:
				assert(false);
				break;
			}
			te->set_picture(Columns::Size, g_gr->images().get(pic));
		}

		if (productionsite and nr_owned) {
			uint32_t const percent =
				static_cast<uint32_t>
					(static_cast<float>(total_prod) / static_cast<float>(nr_owned));
			snprintf(buffer, sizeof(buffer), "%3u", percent);
			if (is_selected)  {
				m_progbar.set_state(percent);
				m_btn[Prev_Unproductive]->set_enabled(true);
				m_btn[Next_Unproductive]->set_enabled(true);
			}
		} else {
			snprintf(buffer, sizeof(buffer), " ");
			if (is_selected) {
				m_btn[Prev_Unproductive]->set_enabled(false);
				m_btn[Next_Unproductive]->set_enabled(false);
			}
		}
		te->set_string(Columns::Prod, buffer);

		//  number of this buildings
		snprintf(buffer, sizeof(buffer), "%3u", nr_owned); //  space-pad for sort
		te->set_string(Columns::Owned, buffer);
		if (is_selected)
			m_owned.set_text(buffer);

		//  number of currently builds
		snprintf(buffer, sizeof(buffer), "%3u", nr_build); //  space-pad for sort
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
