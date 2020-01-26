/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "editor/ui_menus/scenario_tool_worker_options_menu.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

inline EditorInteractive& ScenarioToolWorkerOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

ScenarioToolWorkerOptionsMenu::ScenarioToolWorkerOptionsMenu(EditorInteractive& parent,
                                                             ScenarioPlaceWorkerTool& tool,
                                                             UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 450, 300, _("Workers and Ships"), tool),
     tool_(tool),
     box_(this, 0, 0, UI::Box::Vertical),
     bottombox_(&box_, 0, 0, UI::Box::Horizontal),
     players_(&box_,
              "player",
              0,
              0,
              200,
              8,
              24,
              "",
              UI::DropdownType::kTextual,
              UI::PanelStyle::kWui,
              UI::ButtonStyle::kWuiSecondary),
     item_types_(&box_, 0, 0, 24, 24, 20),
     experience_(&bottombox_,
                 0,
                 0,
                 200,
                 80,
                 tool.get_experience(),
                 0,
                 0,
                 UI::PanelStyle::kWui,
                 _("Experience")),
     ware_(&bottombox_,
           "ware",
           0,
           0,
           24,
           8,
           24,
           _("Ware type a new worker will be carrying"),
           UI::DropdownType::kPictorial,
           UI::PanelStyle::kWui,
           UI::ButtonStyle::kWuiSecondary),
     shipname_(&bottombox_, 0, 0, 150, UI::PanelStyle::kWui),
     selected_items_(&box_,
                     0,
                     0,
                     300,
                     10,
                     UI::PanelStyle::kWui,
                     "",
                     UI::Align::kCenter,
                     UI::MultilineTextarea::ScrollMode::kNoScrolling) {
	const Widelands::Map& map = parent.egbase().map();
	const Widelands::PlayerNumber max = map.get_nrplayers();
	const Widelands::PlayerNumber sel = tool_.get_player();
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const std::string name = map.get_scenario_player_name(p);
		const std::string tribe = map.get_scenario_player_tribe(p);
		players_.add(
		   (boost::format(_("Player %1$s (%2$s)")) % std::to_string(static_cast<int>(p)) % name)
		      .str(),
		   p,
		   g_gr->images().get(
		      tribe.empty() ?
		         "images/ui_fsmenu/random.png" :
		         Widelands::get_tribeinfo(parent.egbase().map().get_scenario_player_tribe(p)).icon),
		   sel == p);
	}
	players_.selected.connect(boost::bind(&ScenarioToolWorkerOptionsMenu::select_player, this));

	const Widelands::Tribes* tribes = &parent.egbase().tribes();
	const size_t nrwo = tribes->nrworkers();
	const size_t nrt = tribes->nrtribes();
	const size_t nrwa = tribes->nrwares();
	auto tribe_of_worker = [tribes, nrt](Widelands::DescriptionIndex i) {
		for (size_t t = 0; t < nrt; ++t) {
			const Widelands::TribeDescr& td = *tribes->get_tribe_descr(t);
			if (td.has_worker(i)) {
				return td.descname();
			}
		}
		NEVER_HERE();
	};
	{
		const Widelands::ShipDescr* s = tribes->get_ship_descr(0);
		item_types_.add(
		   s->name(), s->icon(), reinterpret_cast<void*>(Widelands::INVALID_INDEX), s->descname());
	}
	for (size_t i = 0; i < nrwo; ++i) {
		const Widelands::WorkerDescr* w = tribes->get_worker_descr(i);
		item_types_.add(
		   w->name(), w->icon(), reinterpret_cast<void*>(i),
		   /** TRANSLATORS: Worker name (Tribe name) */
		   (boost::format(_("%1$s (%2$s)")) % w->descname() % tribe_of_worker(i).c_str()).str());
	}
	ware_.add(_("(Empty)"), Widelands::INVALID_INDEX,
	          g_gr->images().get("images/wui/editor/no_ware.png"),
	          tool_.get_carried_ware() == Widelands::INVALID_INDEX, _("No ware"));
	for (size_t i = 0; i < nrwa; ++i) {
		const Widelands::WareDescr* w = tribes->get_ware_descr(i);
		ware_.add(w->descname(), i, w->icon(), i == tool_.get_carried_ware(), w->descname());
	}
	ware_.selected.connect(boost::bind(&ScenarioToolWorkerOptionsMenu::select_ware, this));

	shipname_.set_text(tool_.get_shipname());
	shipname_.changed.connect([this]() { tool_.set_shipname(shipname_.text()); });
	item_types_.icon_clicked.connect(
	   boost::bind(&ScenarioToolWorkerOptionsMenu::toggle_item, this, _1));

	box_.add(&players_, UI::Box::Resizing::kFullSize);
	box_.add(&item_types_, UI::Box::Resizing::kExpandBoth);
	box_.add(&selected_items_, UI::Box::Resizing::kExpandBoth);
	box_.add(&bottombox_, UI::Box::Resizing::kFullSize);
	bottombox_.add(&experience_);
	bottombox_.add(&ware_);
	bottombox_.add(new UI::Textarea(&bottombox_, _("Ship name:")));
	bottombox_.add(&shipname_, UI::Box::Resizing::kFullSize);
	set_center_panel(&box_);
	update_text_and_spinner();
	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolWorkerOptionsMenu::select_player() {
	const Widelands::PlayerNumber p = players_.get_selected();
	assert(p > 0);
	assert(p <= eia().egbase().map().get_nrplayers());
	tool_.set_player(p);
	select_correct_tool();
}

void ScenarioToolWorkerOptionsMenu::select_ware() {
	tool_.set_carried_ware(ware_.get_selected());
	select_correct_tool();
}

void ScenarioToolWorkerOptionsMenu::toggle_item(int32_t idx) {
	select_correct_tool();
	const Widelands::DescriptionIndex index =
	   static_cast<int32_t>(reinterpret_cast<intptr_t>(item_types_.get_data(idx)));
	const Widelands::WorkerDescr* descr =
	   index == Widelands::INVALID_INDEX ? nullptr : eia().egbase().tribes().get_worker_descr(index);
	std::list<const Widelands::WorkerDescr*>& list = tool_.get_descr();
	if (SDL_GetModState() & KMOD_CTRL) {
		for (auto it = list.begin(); it != list.end(); ++it) {
			if (*it == descr) {
				list.erase(it);
				return update_text_and_spinner();
			}
		}
	} else {
		list.clear();
	}
	list.push_back(descr);
	update_text_and_spinner();
}

void ScenarioToolWorkerOptionsMenu::update_text_and_spinner() {
	const size_t nr_items = tool_.get_descr().size();
	const Widelands::EditorGameBase* egbase = &eia().egbase();
	if (nr_items == 0) {
		selected_items_.set_text(_("Nothing selected"));
		experience_.set_value(0);
		experience_.set_interval(0, 0);
		return;
	}
	auto tribe_of_worker = [this, egbase](Widelands::DescriptionIndex i) {
		const size_t nr = egbase->tribes().nrtribes();
		for (size_t t = 0; t < nr; ++t) {
			const Widelands::TribeDescr& td = *egbase->tribes().get_tribe_descr(t);
			if (td.has_worker(i)) {
				return td.descname();
			}
		}
		NEVER_HERE();
	};
	std::string text = "";
	int32_t max_xp = 0;
	for (auto it = tool_.get_descr().begin(); it != tool_.get_descr().end(); ++it) {
		const std::string name =
		   *it == nullptr ? egbase->tribes()
		                       .get_ship_descr(egbase->player(tool_.get_player()).tribe().ship())
		                       ->descname() :
		                    (boost::format(_("(%1$s) %2$s")) %
		                     tribe_of_worker((*it)->worker_index()) % (*it)->descname())
		                       .str();
		if (*it && (*it)->becomes() != Widelands::INVALID_INDEX) {
			max_xp = std::max(max_xp, (*it)->get_needed_experience() - 1);
		}
		if (text.empty()) {
			text = name;
		} else {
			/** TRANSLATORS: Selected items: Item 1 · Item 2 · Item 3 · … */
			text = (boost::format(_("%1$s · %2$s")) % text % name).str();
		}
	}
	experience_.set_interval(0, max_xp);
	experience_.set_value(std::min(max_xp, experience_.get_value()));
	assert(!text.empty());
	selected_items_.set_text(text);
}
