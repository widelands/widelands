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

#include "wui/encyclopedia_window.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "graphic/graphic.h"
#include "logic/building.h"
#include "logic/player.h"
#include "logic/production_program.h"
#include "logic/productionsite.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/tribes/tribes.h"
#include "logic/ware_descr.h"
#include "logic/warelist.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"
#include "wui/interactive_player.h"

#define WINDOW_WIDTH std::min(700, g_gr->get_xres() - 40)
#define WINDOW_HEIGHT std::min(550, g_gr->get_yres() - 40)
constexpr uint32_t quantityColumnWidth = 100;
constexpr uint32_t wareColumnWidth = 250;
#define PRODSITE_GROUPS_WIDTH (WINDOW_WIDTH - wareColumnWidth - quantityColumnWidth - 10)
constexpr int kPadding = 5;
constexpr int kTabHeight = 35;

using namespace Widelands;

inline InteractivePlayer & EncyclopediaWindow::iaplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

namespace {
const std::string heading(const std::string& text) {
	return ((boost::format("<rt><p font-size=18 font-weight=bold font-color=D1D1D1>"
								  "%s<br></p><p font-size=8> <br></p></rt>") % text).str());
}
} // namespace


EncyclopediaWindow::EncyclopediaWindow
	(InteractivePlayer & parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&parent, "encyclopedia",
		 &registry,
		 WINDOW_WIDTH, WINDOW_HEIGHT,
		 _("Tribal Encyclopedia")),
	tabs_(this, 0, 0, nullptr),
	buildings_tab_box_(&tabs_, 0, 0, UI::Box::Horizontal),
	buildings_box_(&buildings_tab_box_, 0, 0, UI::Box::Horizontal),
	buildings_    (&buildings_box_, 0, 0,
						WINDOW_WIDTH / 2 - 1.5 * kPadding, WINDOW_HEIGHT - kTabHeight - 2 * kPadding),
	building_text_(&buildings_box_, 0, 0,
						WINDOW_WIDTH / 2 - 1.5 * kPadding, WINDOW_HEIGHT - kTabHeight - 2 * kPadding),

	wares_tab_box_(&tabs_, 0, 0, UI::Box::Horizontal),
	wares_box_(&wares_tab_box_, 0, 0, UI::Box::Vertical),
	wares_details_box_(&wares_box_, 0, 0, UI::Box::Horizontal),
	wares_            (&wares_box_, 0, 0, WINDOW_WIDTH - 2 * kPadding, WINDOW_HEIGHT - 270 - 2 * kPadding),
	ware_text_        (&wares_box_, 0, 0, WINDOW_WIDTH - 2 * kPadding, 80, ""),
	prod_sites_       (&wares_details_box_, 0, 0, PRODSITE_GROUPS_WIDTH, 145),
	cond_table_
		(&wares_details_box_,
		 0, 0, WINDOW_WIDTH - PRODSITE_GROUPS_WIDTH - 2 * kPadding, 145),

	workers_tab_box_(&tabs_, 0, 0, UI::Box::Horizontal),
	workers_box_(&workers_tab_box_, 0, 0, UI::Box::Horizontal),
	workers_    (&workers_box_, 0, 0,
						WINDOW_WIDTH / 2 - 1.5 * kPadding, WINDOW_HEIGHT - kTabHeight - 2 * kPadding),
	worker_text_(&workers_box_, 0, 0,
						WINDOW_WIDTH / 2 - 1.5 * kPadding, WINDOW_HEIGHT - kTabHeight - 2 * kPadding)
{
	// Buildings
	buildings_box_.add(&buildings_, UI::Align_Left);
	buildings_box_.add_space(kPadding);
	buildings_box_.add(&building_text_, UI::Align_Left);

	buildings_tab_box_.add_space(kPadding);
	buildings_tab_box_.add(&buildings_box_, UI::Align_Left);

	// Wares
	wares_details_box_.add(&prod_sites_, UI::Align_Left);
	wares_details_box_.add(&cond_table_, UI::Align_Left);
	wares_details_box_.set_size(WINDOW_WIDTH,
										 tabs_.get_inner_h() - wares_.get_h() - ware_text_.get_h() - 4 * kPadding);

	wares_box_.add(&wares_, UI::Align_Left);
	wares_box_.add_space(kPadding);
	wares_box_.add(&ware_text_, UI::Align_Left);
	wares_box_.add_space(kPadding);
	wares_box_.add(&wares_details_box_, UI::Align_Left);
	wares_box_.set_size(WINDOW_WIDTH, wares_.get_h() + ware_text_.get_h() + 2 * kPadding);

	wares_tab_box_.add_space(kPadding);
	wares_tab_box_.add(&wares_box_, UI::Align_Left);

	// Workers
	workers_box_.add(&workers_, UI::Align_Left);
	workers_box_.add_space(kPadding);
	workers_box_.add(&worker_text_, UI::Align_Left);

	workers_tab_box_.add_space(kPadding);
	workers_tab_box_.add(&workers_box_, UI::Align_Left);

	tabs_.add("encyclopedia_wares", g_gr->images().get("pics/genstats_nrwares.png"),
				 &wares_tab_box_, _("Wares"));
	tabs_.add("encyclopedia_workers", g_gr->images().get("pics/genstats_nrworkers.png"),
				 &workers_tab_box_, _("Workers"));
	tabs_.add("encyclopedia_buildings", g_gr->images().get("pics/genstats_nrbuildings.png"),
				 &buildings_tab_box_, _("Buildings"));
	tabs_.set_size(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Content
	buildings_.selected.connect(boost::bind(&EncyclopediaWindow::building_selected, this, _1));

	wares_.selected.connect(boost::bind(&EncyclopediaWindow::ware_selected, this, _1));
	prod_sites_.selected.connect(boost::bind(&EncyclopediaWindow::prod_site_selected, this, _1));
	cond_table_.add_column
			/** TRANSLATORS: Column title in the Tribal Encyclopedia */
			(wareColumnWidth, ngettext("Consumed Ware Type", "Consumed Ware Types", 0));
	cond_table_.add_column (quantityColumnWidth, _("Quantity"));
	cond_table_.focus();

	workers_.selected.connect(boost::bind(&EncyclopediaWindow::worker_selected, this, _1));

	fill_buildings();
	fill_wares();
	fill_workers();

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}


void EncyclopediaWindow::fill_buildings() {
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<Building> building_vec;

	const Tribes& tribes = iaplayer().egbase().tribes();
	for (BuildingIndex i = 0; i < tribes.nrbuildings(); ++i) {
		const BuildingDescr* building = tribes.get_building_descr(i);
		if (tribe.has_building(i) || building->type() == MapObjectType::MILITARYSITE) {
			Building b(i, building);
			building_vec.push_back(b);
		}
	}

	std::sort(building_vec.begin(), building_vec.end());

	for (uint32_t i = 0; i < building_vec.size(); i++) {
		Building cur = building_vec[i];
		buildings_.add(cur.descr_->descname(), cur.index_, cur.descr_->icon());
	}
}

void EncyclopediaWindow::building_selected(uint32_t) {
	const TribeDescr& tribe = iaplayer().player().tribe();
	const Widelands::BuildingDescr& selected_building = *tribe.get_building_descr(buildings_.get_selected());

	assert(tribe.has_building(tribe.building_index(selected_building.name())) ||
			 selected_building.type() == MapObjectType::MILITARYSITE);
	try {
		std::unique_ptr<LuaTable> t(
				iaplayer().egbase().lua().run_script("tribes/scripting/help/building_help.lua"));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->push_arg(tribe.name());
		cr->push_arg(selected_building.name());
		cr->resume();
		const std::string help_text = cr->pop_string();
		building_text_.set_text(help_text);
	} catch (LuaError& err) {
		building_text_.set_text(err.what());
	}
	building_text_.scroll_to_top();
}

void EncyclopediaWindow::fill_wares() {
	const TribeDescr & tribe = iaplayer().player().tribe();
	std::vector<Ware> ware_vec;

	for (const WareIndex& ware_index : tribe.wares()) {
		Ware w(ware_index, tribe.get_ware_descr(ware_index));
		ware_vec.push_back(w);
	}

	std::sort(ware_vec.begin(), ware_vec.end());

	for (uint32_t i = 0; i < ware_vec.size(); i++) {
		Ware cur = ware_vec[i];
		wares_.add(cur.descr_->descname(), cur.index_, cur.descr_->icon());
	}
}

void EncyclopediaWindow::ware_selected(uint32_t) {
	const TribeDescr & tribe = iaplayer().player().tribe();
	selected_ware_ = tribe.get_ware_descr(wares_.get_selected());

	try {
		std::unique_ptr<LuaTable> t(
			iaplayer().egbase().lua().run_script("tribes/scripting/help/ware_help.lua"));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->push_arg(tribe.name());
		cr->push_arg(selected_ware_);
		cr->resume();
		const std::string help_text = cr->pop_string();
		ware_text_.set_text(help_text);
	} catch (LuaError& err) {
		ware_text_.set_text(err.what());
	}

	ware_text_.scroll_to_top();

	prod_sites_.clear();
	cond_table_.clear();

	for (const BuildingIndex& building_index : selected_ware_->producers()) {
		const BuildingDescr* building_descr = tribe.get_building_descr(building_index);
		if (tribe.has_building(building_index)) {
			prod_sites_.add(building_descr->descname(), building_index, building_descr->icon());
		}
	}

	if (!prod_sites_.empty()) {
		prod_sites_.select(0);
	}
}

void EncyclopediaWindow::prod_site_selected(uint32_t) {
	assert(prod_sites_.has_selection());
	size_t no_of_wares = 0;
	cond_table_.clear();
	const TribeDescr & tribe = iaplayer().player().tribe();

	if (upcast(ProductionSiteDescr const, descr, tribe.get_building_descr(prod_sites_.get_selected()))) {
		WareIndex selected_ware_index = tribe.ware_index(selected_ware_->name());
		// Only check programs for the production sites that produce the ware.
		if (descr->is_output_ware_type(selected_ware_index)) {
			for (const std::pair<std::string, ProductionProgram *>& program : descr->programs()) {
				// Filter for the production programs that actually produce this ware.
				bool add_program = false;
				for (const ProductionProgram::Action* action : program.second->actions()) {
					for (const WareAmount& produced_ware : action->produced_wares()) {
						if (produced_ware.first == selected_ware_index) {
							add_program = true;
							break;
						}
					}
				}
				if (add_program) {
					// Now iterate through the programs and fetch the exact wares consumed.
					for (const ProductionProgram::Action* action : program.second->actions()) {
						for (const ProductionProgram::WareTypeGroup& group : action->consumed_wares()) {
							const std::set<WareIndex> & ware_types = group.first;
							assert(ware_types.size());
							no_of_wares = no_of_wares + ware_types.size();

							std::vector<std::string> ware_type_descnames;
							for (const WareIndex& ware_index : ware_types) {
								ware_type_descnames.push_back(tribe.get_ware_descr(ware_index)->descname());
							}
							std::string ware_type_names =
									i18n::localize_list(ware_type_descnames, i18n::ConcatenateWith::OR);

							//  Make sure to detect if someone changes the type so that it
							//  needs more than 3 decimal digits to represent.
							static_assert(sizeof(group.second) == 1, "Number is too big for 3 char string.");

							//  picture only of first ware type in group
							UI::Table<uintptr_t>::EntryRecord & tableEntry = cond_table_.add(0);
							tableEntry.set_picture
								(0, tribe.get_ware_descr(*ware_types.begin())->icon(), ware_type_names);
							tableEntry.set_string(1, std::to_string(static_cast<unsigned int>(group.second)));
							cond_table_.set_sort_column(0);
							cond_table_.sort();
						} // groups
					} // action
				} // add_program
			} // programs
		} // is_output_ware_type
	}
	cond_table_.set_column_title(0, ngettext("Consumed Ware Type", "Consumed Ware Types", no_of_wares));
}

void EncyclopediaWindow::fill_workers() {
	const TribeDescr& tribe = iaplayer().player().tribe();
	std::vector<Worker> worker_vec;

	for (const WareIndex& i: tribe.workers()) {
		WorkerDescr const * worker = tribe.get_worker_descr(i);
		Worker w(i, worker);
		worker_vec.push_back(w);
	}

	std::sort(worker_vec.begin(), worker_vec.end());

	for (uint32_t i = 0; i < worker_vec.size(); i++) {
		Worker cur = worker_vec[i];
		workers_.add(cur.descr_->descname(), cur.index_, cur.descr_->icon());
	}
}

void EncyclopediaWindow::worker_selected(uint32_t) {
	const TribeDescr& tribe = iaplayer().player().tribe();
	const Widelands::WorkerDescr& selected_worker = *tribe.get_worker_descr(workers_.get_selected());

	try {
		std::unique_ptr<LuaTable> t(
			iaplayer().egbase().lua().run_script("tribes/scripting/help/worker_help.lua"));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->push_arg(tribe.name());
		cr->push_arg(&selected_worker);
		cr->resume();
		const std::string help_text = cr->pop_string();
		worker_text_.set_text((boost::format("%s%s")
									  % heading(selected_worker.descname())
									  % help_text).str());
	} catch (LuaError& err) {
		worker_text_.set_text(err.what());
	}

	worker_text_.scroll_to_top();
}
