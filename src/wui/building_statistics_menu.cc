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
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h" // NOCOM
#include "base/macros.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/tribe.h"
#include "ui_basic/button.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/plot_area.h"

constexpr int kTabHeight = 35;
constexpr int kBuildGridCellSize = 50;
constexpr int kColumns = 5;

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

#define LOW_PROD 33

#define UPDATE_TIME 1000  //  1 second, gametime


namespace Columns {enum {Name, Size, Prod, Owned, Build};}

inline InteractivePlayer & BuildingStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

BuildingStatisticsMenu::BuildingStatisticsMenu
	(InteractivePlayer & parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&parent, "building_statistics",
		 &registry,
		 WINDOW_WIDTH, WINDOW_HEIGHT,
		 _("Building Statistics")),
	tabs_(this, 0, 0, g_gr->images().get("pics/but1.png")),
	small_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	medium_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	big_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	mines_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	ports_tab_(&tabs_, 0, 0, UI::Box::Vertical),


	old_design_(&tabs_, 0, 0, UI::Box::Vertical),
	m_table
		(&old_design_, HMARGIN, VMARGIN, BUILDING_LIST_WIDTH, BUILDING_LIST_HEIGHT),
	m_progbar
		(&old_design_,
		 LABEL_X, PROGRESS_BAR_Y, WINDOW_WIDTH - LABEL_X - HMARGIN, 20,
		 UI::ProgressBar::Horizontal),
	m_total_productivity_label
		(&old_design_,
		 LABEL_X, TOTAL_PRODUCTIVITY_Y, LABEL_WIDTH, 24,
		 _("Total Productivity:"), UI::Align_CenterLeft),
	m_lastupdate         (0),
	m_last_building_index(0),
	m_last_table_index   (0)
{
	tabs_.add("building_stats_small", g_gr->images().get("pics/menu_tab_buildsmall.png"),
				 &small_tab_, _("Small Buildings"));
	tabs_.add("building_stats_medium", g_gr->images().get("pics/menu_tab_buildmedium.png"),
				 &medium_tab_, _("Medium Buildings"));
	tabs_.add("building_stats_big", g_gr->images().get("pics/menu_tab_buildbig.png"),
				 &big_tab_, _("Big Buildings"));
	tabs_.add("building_stats_mines", g_gr->images().get("pics/menu_tab_buildmine.png"),
				 &mines_tab_, _("Mines"));
	tabs_.add("building_stats_ports", g_gr->images().get("pics/menu_tab_buildport.png"),
				 &ports_tab_, _("Ports"));
	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	//  building list
	m_table.add_column(310, _("Name"));
	m_table.add_column (70, _("Type"), "",     UI::Align_HCenter);
	m_table.add_column (70, _("Prod"), "",     UI::Align_Right);
	m_table.add_column (70, _("Owned"), "",    UI::Align_Right);
	m_table.add_column (70, _("Build"), "",    UI::Align_Right);
	m_table.selected.connect(boost::bind(&BuildingStatisticsMenu::table_changed, this, _1));
	m_table.set_column_compare
		(Columns::Size,
		 boost::bind
		 	(&BuildingStatisticsMenu::compare_building_size, this, _1, _2));
	m_table.focus();

	//  toggle when to run button
	m_progbar.set_total(100);

	old_design_.add(&m_table, UI::Align_Left);
	old_design_.add(&m_total_productivity_label, UI::Align_Left);
	old_design_.add(&m_progbar, UI::Align_Left);

	UI::Box* old_bottom = new UI::Box(&old_design_, 0, 0, UI::Box::Horizontal);

	m_owned_label = new UI::Textarea
		(old_bottom,
		 LABEL_X, OWNED_Y, LABEL_WIDTH, 24,
		 _("Owned:"), UI::Align_CenterLeft);
	m_owned = new UI::Textarea
		(old_bottom, VALUE_X, OWNED_Y, 100, 24, UI::Align_CenterLeft);

	m_btn[PrevOwned] =
		new UI::Button
			(old_bottom, "previous_owned",
			 JUMP_PREV_BUTTON_X, OWNED_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_left.png"),
			 _("Show previous"),
			 false);

	m_btn[NextOwned] =
		new UI::Button
			(old_bottom, "next_owned",
			 JUMP_NEXT_BUTTON_X, OWNED_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_right.png"),
			 _("Show next"),
			 false);

	old_bottom->add(m_owned_label, UI::Align_Left);
	old_bottom->add(m_owned, UI::Align_Left);
	old_bottom->add(m_btn[PrevOwned], UI::Align_Left);
	old_bottom->add(m_btn[NextOwned], UI::Align_Left);
	old_design_.add(old_bottom, UI::Align_Left);

	old_bottom = new UI::Box(&old_design_, 0, 0, UI::Box::Horizontal);

	m_in_build_label = new UI::Textarea
		(old_bottom,
		 LABEL_X, IN_BUILD_Y, LABEL_WIDTH, 24,
		 _("Being built:"), UI::Align_CenterLeft);
	m_in_build = new UI::Textarea
		(old_bottom, VALUE_X, IN_BUILD_Y, 100, 24, UI::Align_CenterLeft);

	m_btn[PrevConstruction] =
		new UI::Button
			(old_bottom, "previous_constructed",
			 JUMP_PREV_BUTTON_X, IN_BUILD_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_left.png"),
			 _("Show previous"),
			 false);

	m_btn[NextConstruction] =
		new UI::Button
			(old_bottom, "next_constructed",
			 JUMP_NEXT_BUTTON_X, IN_BUILD_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_right.png"),
			 _("Show next"),
			 false);

	old_bottom->add(m_in_build_label, UI::Align_Left);
	old_bottom->add(m_in_build, UI::Align_Left);
	old_bottom->add(m_btn[PrevConstruction], UI::Align_Left);
	old_bottom->add(m_btn[NextConstruction], UI::Align_Left);
	old_design_.add(old_bottom, UI::Align_Left);

	old_bottom = new UI::Box(&old_design_, 0, 0, UI::Box::Horizontal);

	m_unproductive_label = new UI::Textarea
		(old_bottom,
		 LABEL_X, UNPRODUCTIVE_Y, LABEL_WIDTH + 100, 24,
		 _("Jump to unproductive"), UI::Align_CenterLeft);

	m_btn[PrevUnproductive] =
		new UI::Button
			(old_bottom, "previous_unproductive",
			 JUMP_PREV_BUTTON_X, UNPRODUCTIVE_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_left.png"),
			 _("Show previous"),
			 false);

	m_btn[NextUnproductive] =
		new UI::Button
			(old_bottom, "next_unproductive",
			 JUMP_NEXT_BUTTON_X, UNPRODUCTIVE_Y, 24, 24,
			 g_gr->images().get("pics/but4.png"),
			 g_gr->images().get("pics/scrollbar_right.png"),
			 _("Show next"),
			 false);

	old_bottom->add(m_unproductive_label, UI::Align_Left);
	old_bottom->add(m_btn[PrevUnproductive], UI::Align_Left);
	old_bottom->add(m_btn[NextUnproductive], UI::Align_Left);
	old_design_.add(old_bottom, UI::Align_Left);

	tabs_.add("building_stats_old", g_gr->images().get("pics/genstats_nrbuildings.png"),
					 &old_design_, "Old Design");
	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	m_btn[PrevOwned]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::clicked_jump, boost::ref(*this), PrevOwned));

	m_btn[NextOwned]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::clicked_jump, boost::ref(*this), NextOwned));

	m_btn[PrevConstruction]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::clicked_jump, boost::ref(*this), PrevConstruction));

	m_btn[NextConstruction]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::clicked_jump, boost::ref(*this), NextConstruction));
	m_btn[PrevUnproductive]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::clicked_jump, boost::ref(*this), PrevUnproductive));

	m_btn[NextUnproductive]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::clicked_jump, boost::ref(*this), NextUnproductive));

	init();
}


void BuildingStatisticsMenu::init() {
	const TribeDescr& tribe = iplayer().player().tribe();

	const BuildingIndex nr_buildings = tribe.get_nrbuildings();
	building_buttons_ = std::vector<UI::Button*>(nr_buildings);
	owned_buttons_ = std::vector<UI::Button*>(nr_buildings);
	productivity_buttons_ = std::vector<UI::Button*>(nr_buildings);

	int small_column = 0;
	int medium_column = 0;
	int big_column = 0;
	int mines_column = 0;
	int ports_column = 0;

	UI::Box* mines_row = new UI::Box(&mines_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* ports_row = new UI::Box(&ports_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* big_row = new UI::Box(&big_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* medium_row = new UI::Box(&medium_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* small_row = new UI::Box(&small_tab_, 0, 0, UI::Box::Horizontal);


	for(BuildingIndex id = 0; id < nr_buildings; ++id) {
		const BuildingDescr& descr = *tribe.get_building_descr(id);

		if (descr.type() != MapObjectType::CONSTRUCTIONSITE &&
			 descr.type() != MapObjectType::DISMANTLESITE) {
			if (descr.get_ismine()) {
				mines_.push_back(id);
				add_button(id, descr, *mines_row);
				++mines_column;
				if (mines_column == kColumns) {
					mines_tab_.add(mines_row, UI::Align_Left);
					mines_column = 0;
					mines_row = new UI::Box(&mines_tab_, 0, 0, UI::Box::Horizontal);
				}
			} else if (descr.get_isport()) {
				ports_.push_back(id);
				add_button(id, descr, *ports_row);
				++ports_column;
				if (ports_column == kColumns) {
					ports_tab_.add(ports_row, UI::Align_Left);
					ports_column = 0;
					ports_row = new UI::Box(&ports_tab_, 0, 0, UI::Box::Horizontal);
				}
			} else {
				switch (descr.get_size()) {
					case BaseImmovable::SMALL:
						small_buildings_.push_back(id);
						add_button(id, descr, *small_row);
						++small_column;
						if (small_column == kColumns) {
							small_tab_.add(small_row, UI::Align_Left);
							small_column = 0;
							small_row = new UI::Box(&small_tab_, 0, 0, UI::Box::Horizontal);
						}
						break;
					case BaseImmovable::MEDIUM:
						medium_buildings_.push_back(id);
						add_button(id, descr, *medium_row);
						++medium_column;
						if (medium_column == kColumns) {
							medium_tab_.add(medium_row, UI::Align_Left);
							medium_column = 0;
							medium_row = new UI::Box(&medium_tab_, 0, 0, UI::Box::Horizontal);
						}
						break;
					case BaseImmovable::BIG:
						big_buildings_.push_back(id);
						add_button(id, descr, *big_row);
						++big_column;
						if (big_column == kColumns) {
							big_tab_.add(big_row, UI::Align_Left);
							big_column = 0;
							big_row = new UI::Box(&big_tab_, 0, 0, UI::Box::Horizontal);
						}
						break;
					default:
						throw wexception("Building statictics: Found building without a size: %s", descr.name().c_str());
				}
			}
		}
	}
	mines_tab_.add(mines_row, UI::Align_Left);
	ports_tab_.add(ports_row, UI::Align_Left);
	small_tab_.add(small_row, UI::Align_Left);
	medium_tab_.add(medium_row, UI::Align_Left);
	big_tab_.add(big_row, UI::Align_Left);
}

void BuildingStatisticsMenu::add_button(BuildingIndex id, const BuildingDescr& descr, UI::Box& tab) {
	UI::Box* button_box = new UI::Box(&tab, 0, 0, UI::Box::Vertical);
	building_buttons_[id] =
			new UI::Button(button_box, (boost::format("building_button%s") % id).str(), 0, 0,
								kBuildGridCellSize, kBuildGridCellSize,
								g_gr->images().get("pics/but1.png"),
								&g_gr->animations()
								.get_animation(descr.get_animation("idle"))
								.representative_image_from_disk(),
								descr.descname(), true, true);
	button_box->add(building_buttons_[id], UI::Align_Left);

	owned_buttons_[id] =
			new UI::Button(button_box, (boost::format("prod_button%s") % id).str(), 0, 0,
								kBuildGridCellSize, 20,
								g_gr->images().get("pics/but1.png"),
								/** TRANSLATORS Buildings: owned / under construction */
								(boost::format(_("%1% / %2%")) % 0 % 0).str(),
								_("Owned / Under Construction"), true, true);
	button_box->add(owned_buttons_[id], UI::Align_Left);

	productivity_buttons_[id] =
			new UI::Button(button_box, (boost::format("prod_button%s") % id).str(), 0, 0,
								kBuildGridCellSize, 20,
								g_gr->images().get("pics/but1.png"),
								"–",
								_("Productivity"), true, true);
	button_box->add(productivity_buttons_[id], UI::Align_Left);

	tab.add(button_box, UI::Align_Left);
}


/*
 * Update this statistic
 */
void BuildingStatisticsMenu::think() {
	const Game & game = iplayer().game();
	int32_t const gametime = game.get_gametime();

	if ((gametime - m_lastupdate) > UPDATE_TIME) {
		update();
		m_lastupdate = gametime;
	}
}

/*
 * validate if this pointer is ok
 */
int32_t BuildingStatisticsMenu::validate_pointer
	(int32_t * const id, int32_t const size)
{
	if (*id < 0)
		*id = size - 1;
	if (size <= *id)
		*id = 0;

	return *id;
}


void BuildingStatisticsMenu::clicked_jump(JumpTargets const id) {
	assert(m_table.has_selection());
	if (m_last_table_index != m_table.selection_index())
		m_last_building_index = 0;
	m_last_table_index = m_table.selection_index();
	const std::vector<Player::BuildingStats> & vec =
		iplayer().get_player()->get_building_statistics
			(BuildingIndex
				(static_cast<size_t>(m_table.get_selected())));
	const Map & map = iplayer().egbase().map();

	bool found = true; //  we think, we always find a proper building

	switch (id) {
	case PrevOwned:
		--m_last_building_index;
		break;
	case NextOwned:
		++m_last_building_index;
		break;
	case PrevConstruction: {
		int32_t const curindex = m_last_building_index;
		while
			(validate_pointer(&(--m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite)
				break;
		break;
	}
	case NextConstruction: {
		int32_t const curindex = m_last_building_index;
		while
			(validate_pointer(&(++m_last_building_index), vec.size()) != curindex)
			if (vec[m_last_building_index].is_constructionsite)
				break;
		break;
	}
	case PrevUnproductive: {
		int32_t const curindex = m_last_building_index;
		found = false;
		while (validate_pointer(&(--m_last_building_index), vec.size()) != curindex)
			if (!vec[m_last_building_index].is_constructionsite) {
				if
					(upcast
						(ProductionSite,
					 	 productionsite,
					 	 map[vec[m_last_building_index].pos].get_immovable()))
					if (productionsite->get_statistics_percent() < LOW_PROD) {
						found = true;
						break;
					}
			}
		if (!found) // Now look at the old
			if
				(upcast
					(ProductionSite,
				 	 productionsite,
				 	 map[vec[m_last_building_index].pos].get_immovable()))
				if (productionsite->get_statistics_percent() < LOW_PROD)
					found = true;
		break;
	}
	case NextUnproductive: {
		int32_t const curindex = m_last_building_index;
		found = false;
		while
			(validate_pointer(&(++m_last_building_index), vec.size()) != curindex)
			if (!vec[m_last_building_index].is_constructionsite) {
				if
					(upcast
						(ProductionSite,
					 	 productionsite,
					 	 map[vec[m_last_building_index].pos].get_immovable()))
					if (productionsite->get_statistics_percent() < LOW_PROD) {
						found = true;
						break;
					}
			}
		if (!found) // Now look at the old
			if
				(upcast
					(ProductionSite,
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
void BuildingStatisticsMenu::table_changed(uint32_t) {update();}

/**
 * Callback to sort table based on building size.
 */
bool BuildingStatisticsMenu::compare_building_size
	(uint32_t const rowa, uint32_t const rowb)
{
	const TribeDescr & tribe = iplayer().player().tribe();
	BuildingIndex a = BuildingIndex(m_table[rowa]);
	BuildingIndex b = BuildingIndex(m_table[rowb]);
	const BuildingDescr * descra = tribe.get_building_descr(a);
	const BuildingDescr * descrb = tribe.get_building_descr(b);

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
void BuildingStatisticsMenu::update() {
	m_owned   ->set_text("");
	m_in_build->set_text("");
	m_progbar .set_state(0);

	const Player      & player = iplayer().player();
	const TribeDescr & tribe  = player.tribe();
	const Map         & map   = iplayer().game().map();
	BuildingIndex      const nr_buildings = tribe.get_nrbuildings();

	for(BuildingIndex i = 0; i < nr_buildings; ++i) {
		const BuildingDescr& building = *tribe.get_building_descr(i);
		// NOCOM decide how to handle enhanced and glocal sites
		if(!(building.is_buildable() || building.is_enhanced() || building.global())) {
			continue;
		}

		const std::vector<Player::BuildingStats>& stats_vector = player.get_building_statistics(i);

		uint32_t nr_owned   = 0;
		uint32_t nr_build   = 0;
		uint32_t total_prod = 0;
		upcast(ProductionSiteDescr const, productionsite, &building);
		for (uint32_t l = 0; l < stats_vector.size(); ++l) {
			if (stats_vector[l].is_constructionsite)
				++nr_build;
			else {
				++nr_owned;
				if (productionsite)
					total_prod +=
						dynamic_cast<ProductionSite&>
							(*map[stats_vector[l].pos].get_immovable())
						.get_statistics_percent();
			}
		}

		if (productionsite && nr_owned) {
			int const percent =
				static_cast<int>
					(static_cast<float>(total_prod) / static_cast<float>(nr_owned));
			productivity_buttons_[i]->set_title((boost::format("%i%%") % percent).str());
		} else {
			productivity_buttons_[i]->set_title("–");
		}

		/** TRANSLATORS Buildings: owned / under construction */
		owned_buttons_[i]->set_title((boost::format(_("%1% / %2%")) % nr_owned % nr_build).str());

	}


	// NOCOM Old code
	for
		(BuildingIndex i = 0;
		 i < nr_buildings;
		 ++i)
	{
		const BuildingDescr & building =
			*tribe.get_building_descr(i);
		if
			(!(building.is_buildable()
			 || building.is_enhanced()
			 || building.global()))
			continue;

		const std::vector<Player::BuildingStats> & vec =
			player.get_building_statistics(i);

		//  walk all entries, add new ones if needed
		UI::Table<uintptr_t const>::EntryRecord * te = nullptr;
		const uint32_t table_size = m_table.size();
		for (uint32_t l = 0; l < table_size; ++l) {
			UI::Table<uintptr_t const>::EntryRecord & er = m_table.get_record(l);
			if (UI::Table<uintptr_t const>::get(er) == i) {
				te = &er;
				break;
			}
		}

		//  If not in list, add new one, as long as this building is enabled.
		if (!te) {
			if (! iplayer().player().is_building_type_allowed(i))
				continue;
			te = &m_table.add(i);
		}

		uint32_t nr_owned   = 0;
		uint32_t nr_build   = 0;
		uint32_t total_prod = 0;
		upcast(ProductionSiteDescr const, productionsite, &building);
		for (uint32_t l = 0; l < vec.size(); ++l) {
			if (vec[l].is_constructionsite)
				++nr_build;
			else {
				++nr_owned;
				if (productionsite)
					total_prod +=
						dynamic_cast<ProductionSite&>
							(*map[vec[l].pos].get_immovable())
						.get_statistics_percent();
			}
		}

		const bool is_selected = //  Is this entry selected?
			m_table.has_selection() && m_table.get_selected() == i;

		if (is_selected) {
			m_btn[PrevOwned]       ->set_enabled(nr_owned);
			m_btn[NextOwned]       ->set_enabled(nr_owned);
			m_btn[PrevConstruction]->set_enabled(nr_build);
			m_btn[NextConstruction]->set_enabled(nr_build);
		}

		//  add new Table Entry
		te->set_picture
			(Columns::Name, building.get_icon(), building.descname());

		{
			char const * pic = "pics/novalue.png";
			if (building.get_ismine()) {
				pic = "pics/menu_tab_buildmine.png";
			} else if (building.get_isport()) {
				pic = "pics/menu_tab_buildport.png";
			}
			else switch (building.get_size()) {
			case BaseImmovable::SMALL:
				pic = "pics/menu_tab_buildsmall.png";
				break;
			case BaseImmovable::MEDIUM:
				pic = "pics/menu_tab_buildmedium.png";
				break;
			case BaseImmovable::BIG:
				pic = "pics/menu_tab_buildbig.png";
				break;
			default:
				assert(false);
				break;
			}
			te->set_picture(Columns::Size, g_gr->images().get(pic));
		}

		if (productionsite && nr_owned) {
			uint32_t const percent =
				static_cast<uint32_t>
					(static_cast<float>(total_prod) / static_cast<float>(nr_owned));
			te->set_string(Columns::Prod, (boost::format("%3u") % percent).str()); //  space-pad for sort
			if (is_selected)  {
				m_progbar.set_state(percent);
				m_btn[PrevUnproductive]->set_enabled(true);
				m_btn[NextUnproductive]->set_enabled(true);
			}
		} else {
			te->set_string(Columns::Prod,  " ");
			if (is_selected) {
				m_btn[PrevUnproductive]->set_enabled(false);
				m_btn[NextUnproductive]->set_enabled(false);
			}
		}

		//  number of these buildings
		const std::string owned_string =
		   (boost::format("%3u") % nr_owned).str();  //  space-pad for sort
		te->set_string(Columns::Owned, owned_string);
		if (is_selected) {
			m_owned->set_text(owned_string);
		}

		//  number of these buildings currently being built
		const std::string build_string =
		   (boost::format("%3u") % nr_build).str();  //  space-pad for sort
		te->set_string(Columns::Build, build_string);
		if (is_selected) {
			m_in_build->set_text(build_string);
		}
	}

	//  disable all buttons, if nothing to select
	if (!m_table.has_selection()) {
		m_btn[PrevOwned]       ->set_enabled(false);
		m_btn[NextOwned]       ->set_enabled(false);
		m_btn[PrevConstruction]->set_enabled(false);
		m_btn[NextConstruction]->set_enabled(false);
		m_btn[PrevUnproductive]->set_enabled(false);
		m_btn[NextUnproductive]->set_enabled(false);
	}
}
