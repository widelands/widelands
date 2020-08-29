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

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"

inline EditorInteractive& ScenarioToolWorkerOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

ScenarioToolWorkerOptionsMenu::ScenarioToolWorkerOptionsMenu(EditorInteractive& parent,
                                                             ScenarioPlaceWorkerTool& tool,
                                                             UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 300, 200, _("Workers and Ships"), tool),
     tool_(tool),
     box_(this, 0, 0, UI::Box::Vertical),
     bottombox_(&box_, 0, 0, UI::Box::Horizontal),
     tabs_(&box_, UI::TabPanelStyle::kWuiDark),
     ware_(&bottombox_,
           "ware",
           0,
           0,
           32,
           8,
           32,
           _("Ware type a new worker will be carrying"),
           UI::DropdownType::kPictorial,
           UI::PanelStyle::kWui,
           UI::ButtonStyle::kWuiSecondary),
     experience_(&bottombox_,
                 0,
                 0,
                 200,
                 80,
                 tool.get_experience(),
                 0,
                 0,
                 UI::PanelStyle::kWui,
                 _("Experience:")),
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
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const Widelands::TribeDescr& tribe = *parent.egbase().tribes().get_tribe_descr(
		   parent.egbase().tribes().tribe_index(map.get_scenario_player_tribe(p)));
		UI::IconGrid* i = new UI::IconGrid(&tabs_, 0, 0, 32, 32, 10);
		{
			const Widelands::ShipDescr* s = parent.egbase().tribes().get_ship_descr(tribe.ship());
			i->add(
			   s->name(), s->icon(), reinterpret_cast<void*>(Widelands::INVALID_INDEX), s->descname());
		}
		for (Widelands::DescriptionIndex di : tribe.workers()) {
			const Widelands::WorkerDescr* w = tribe.get_worker_descr(di);
			i->add(w->name(), w->icon(), reinterpret_cast<void*>(di), w->descname());
		}
		i->icon_clicked.connect([this](int32_t x) { toggle_item(x); });
		items_.push_back(std::unique_ptr<UI::IconGrid>(i));
		tabs_.add(
		   "player_" + std::to_string(static_cast<unsigned>(p)), map.get_scenario_player_name(p), i);
	}
	tabs_.sigclicked.connect([this]() { select_tab(); });
	ware_.clicked.connect([this]() { select_ware(); });

	bottombox_.add(&experience_, UI::Box::Resizing::kFullSize);
	bottombox_.add_inf_space();
	bottombox_.add(&ware_, UI::Box::Resizing::kFullSize);
	box_.add(&tabs_, UI::Box::Resizing::kFullSize);
	box_.add(&selected_items_, UI::Box::Resizing::kFullSize);
	box_.add(&bottombox_, UI::Box::Resizing::kFullSize);
	set_center_panel(&box_);
	tabs_.activate(tool_.get_player() - 1);
	select_tab();
	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolWorkerOptionsMenu::select_tab() {
	const Widelands::Map& map = eia().egbase().map();
	const Widelands::Tribes& tribes = eia().egbase().tribes();

	const Widelands::PlayerNumber p = tabs_.active() + 1;
	assert(p);
	assert(p <= map.get_nrplayers());
	tool_.set_player(p);

	const Widelands::TribeDescr& tribe =
	   *tribes.get_tribe_descr(tribes.tribe_index(map.get_scenario_player_tribe(p)));
	auto& sel = tool_.get_descr();
	for (auto it = sel.begin(); it != sel.end();) {
		if (*it && !tribe.has_worker(tribe.worker_index((*it)->name()))) {
			it = sel.erase(it);
		} else {
			++it;
		}
	}
	if (!tribe.has_ware(tool_.get_carried_ware())) {
		tool_.set_carried_ware(Widelands::INVALID_INDEX);
	}

	ware_.clear();
	ware_.add(_("(Empty)"), Widelands::INVALID_INDEX,
	          g_image_cache->get("images/wui/editor/no_ware.png"),
	          tool_.get_carried_ware() == Widelands::INVALID_INDEX);
	for (Widelands::DescriptionIndex di : tribe.wares()) {
		const Widelands::WareDescr* w = tribe.get_ware_descr(di);
		ware_.add(w->descname(), di, w->icon(), tool_.get_carried_ware() == di);
	}
	assert(ware_.has_selection());

	update_text_and_spinner();
	select_correct_tool();
}

void ScenarioToolWorkerOptionsMenu::select_ware() {
	tool_.set_carried_ware(ware_.get_selected());
	select_correct_tool();
}

void ScenarioToolWorkerOptionsMenu::toggle_item(int32_t idx) {
	select_correct_tool();
	const Widelands::DescriptionIndex index =
	   static_cast<int32_t>(reinterpret_cast<intptr_t>(items_[tabs_.active()]->get_data(idx)));
	const Widelands::WorkerDescr* descr =
	   index == Widelands::INVALID_INDEX ? nullptr : eia().egbase().tribes().get_worker_descr(index);
	auto& list = tool_.get_descr();
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
	std::string text = "";
	int32_t max_xp = 0;
	for (auto it = tool_.get_descr().begin(); it != tool_.get_descr().end(); ++it) {
		const std::string name =
		   *it ? (*it)->descname() :
		         egbase->tribes()
		            .get_ship_descr(egbase->player(tool_.get_player()).tribe().ship())
		            ->descname();
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
