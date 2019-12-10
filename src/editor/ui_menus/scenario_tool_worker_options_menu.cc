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
		: EditorToolOptionsMenu(parent, registry, 200, 100, _("Workers and Ships"), tool),
		tool_(tool),
		box_(this, 0, 0, UI::Box::Vertical),
		players_(&box_, "player", 0, 0, get_inner_w(), 8, get_inner_h(), "",
				UI::DropdownType::kTextual, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary),
		item_types_(&box_, 0, 0, 24, 24, 12),
		experience_(&box_, 0, 0, 200, 50, 0, 0, 0, UI::PanelStyle::kWui, _("Experience")),
		shipname_(&box_, 0, 0, 200, UI::PanelStyle::kWui),
		selected_items_(&box_, 0, 0, 100, 10, UI::PanelStyle::kWui,
			"", UI::Align::kCenter, UI::MultilineTextarea::ScrollMode::kNoScrolling) {
	const Widelands::Map& map = parent.egbase().map();
	const Widelands::PlayerNumber max = map.get_nrplayers();
	const Widelands::PlayerNumber sel = tool_.get_player();
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const std::string name = map.get_scenario_player_name(p);
		const std::string tribe = map.get_scenario_player_tribe(p);
		players_.add((boost::format(_("Player %1$s (%2$s)")) % std::to_string(static_cast<int>(p)) % name).str(), p,
				g_gr->images().get(tribe.empty() ? "images/ui_fsmenu/random.png" :
						Widelands::get_tribeinfo(parent.egbase().map().get_scenario_player_tribe(p)).icon),
						sel == p, (boost::format(_("Claim fields for %s")) % name).str());
	}
	players_.selected.connect(boost::bind(&ScenarioToolWorkerOptionsMenu::select_player, this));

	const Widelands::Tribes& tribes = parent.egbase().tribes();
	const size_t nrw = tribes.nrworkers();
	{
		const Widelands::ShipDescr* s = tribes.get_ship_descr(0);
		item_types_.add(s->name(), s->icon(), reinterpret_cast<void*>(Widelands::INVALID_INDEX), s->descname());
	}
	for (size_t i = 0; i < nrw; ++i) {
		const Widelands::WorkerDescr* w = tribes.get_worker_descr(i);
		item_types_.add(w->name(), w->icon(), reinterpret_cast<void*>(i), w->descname());
	}

	update_text();
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

void ScenarioToolWorkerOptionsMenu::update_text() {
	const size_t nr_items = tool_.get_descr().size();
	const Widelands::EditorGameBase* egbase = &eia().egbase();
	if (nr_items == 0) {
		selected_items_.set_text(_("Nothing selected"));
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
	for (auto it = tool_.get_descr().begin(); it != tool_.get_descr().end(); ++it) {
		const std::string name = *it == nullptr ? egbase->tribes().get_ship_descr(egbase->player(
				tool_.get_player()).tribe().ship())->descname() : (boost::format(_("(%1$s) %2$s")) %
						tribe_of_worker((*it)->worker_index()) % (*it)->descname()).str();
		if (text.empty()) {
			text = name;
		} else {
			/** TRANSLATORS: Selected items: Item 1 · Item 2 · Item 3 · … */
			text = (boost::format(_("%1$s · %2$s")) % text % name).str();
		}
	}
	assert(!text.empty());
	selected_items_.set_text(text);
}

