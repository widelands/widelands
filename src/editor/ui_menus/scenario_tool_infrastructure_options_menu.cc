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

#include "editor/ui_menus/scenario_tool_infrastructure_options_menu.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

inline EditorInteractive& ScenarioToolInfrastructureOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

constexpr uint8_t kIconGridColumns = 7;

ScenarioToolInfrastructureOptionsMenu::ScenarioToolInfrastructureOptionsMenu(
   EditorInteractive& parent,
   ScenarioInfrastructureTool& tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 500, 300, _("Place Infrastructure"), tool),
     tool_(tool) {
	main_box_.reset(new UI::Box(this, 0, 0, UI::Box::Vertical));
	set_center_panel(main_box_.get());
	players_.reset(new UI::Dropdown<Widelands::PlayerNumber>(
	   main_box_.get(), "players", 0, 0, 300, 8, 24, _("Player"), UI::DropdownType::kTextual,
	   UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary));
	main_box_->add(players_.get(), UI::Box::Resizing::kFullSize);

	force_.reset(new UI::Checkbox(main_box_.get(), Vector2i(0, 0), _("Force"),
	                              _("Enable all building types, allow building on unsuited spots, "
	                                "destroy nearby immovables, and conquer the surrounding land")));
	main_box_->add(force_.get(), UI::Box::Resizing::kFullSize);
	force_->set_state(tool_.get_force());
	force_->changed.connect([this]() {
		tool_.set_force(force_->get_state());
		select_correct_tool();
	});

	construct_.reset(new UI::Checkbox(main_box_.get(), Vector2i(0, 0), _("Constructionsite"),
	                                  _("Place a constructionsite if possible")));
	main_box_->add(construct_.get(), UI::Box::Resizing::kFullSize);
	construct_->set_state(tool_.get_construct());
	construct_->changed.connect([this]() {
		tool_.set_construct(construct_->get_state());
		select_correct_tool();
	});

	{
		const Widelands::Map& map = parent.egbase().map();
		const Widelands::PlayerNumber sel = tool_.get_player();
		const Widelands::PlayerNumber max = map.get_nrplayers();
		for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
			const std::string name = map.get_scenario_player_name(p);
			const std::string tribe = map.get_scenario_player_tribe(p);
			players_->add(
			   (boost::format(_("Player %1$s (%2$s)")) % std::to_string(static_cast<int>(p)) % name)
			      .str(),
			   p, g_gr->images().get(Widelands::get_tribeinfo(map.get_scenario_player_tribe(p)).icon),
			   sel == p);
		}
	}

	const Widelands::Tribes& tribes = parent.egbase().tribes();
	const size_t nr_tribes = tribes.nrtribes();
	item_categories_.reset(new UI::TabPanel(main_box_.get(), UI::TabPanelStyle::kWuiDark));
	// Flag
	{
		UI::IconGrid* i = new UI::IconGrid(item_categories_.get(), 0, 0, 50, 50, 1);
		i->add("flag", g_gr->images().get("images/wui/fieldaction/menu_build_flag.png"),
		       reinterpret_cast<void*>(Widelands::INVALID_INDEX), _("Flag"));
		i->icon_clicked.connect(boost::bind(&ScenarioToolInfrastructureOptionsMenu::toggle_selected,
		                                    this, i, Widelands::MapObjectType::FLAG, _1));
		item_categories_->add(
		   "flag", g_gr->images().get("images/wui/fieldaction/menu_build_flag.png"), i, _("Flag"));
	}
	// Immovables
	{
		UI::TabPanel* tab = new UI::TabPanel(item_categories_.get(), UI::TabPanelStyle::kWuiDark);
		for (size_t tribe = 0; tribe < nr_tribes; ++tribe) {
			const Widelands::TribeDescr* td = parent.egbase().tribes().get_tribe_descr(tribe);
			const Image* icon = g_gr->images().get(Widelands::get_tribeinfo(td->name()).icon);
			assert(td);
			UI::IconGrid* i = new UI::IconGrid(tab, 0, 0, 50, 50, kIconGridColumns);
			for (Widelands::DescriptionIndex di : td->immovables()) {
				const Widelands::ImmovableDescr* descr = td->get_immovable_descr(di);
				i->add(std::to_string(tribe) + "_" + descr->name(), descr->representative_image(),
				       reinterpret_cast<void*>(di), descr->descname());
			}
			i->icon_clicked.connect(
			   boost::bind(&ScenarioToolInfrastructureOptionsMenu::toggle_selected, this, i,
			               Widelands::MapObjectType::IMMOVABLE, _1));
			tab->add(std::to_string(tribe), icon, i, td->descname());
		}
		item_categories_->add("immovables",
		                      g_gr->images().get("images/wui/menus/toggle_immovables.png"), tab,
		                      _("Immovables"));
	}
	create_buildings_tab();
	main_box_->add(item_categories_.get(), UI::Box::Resizing::kExpandBoth);

	players_->selected.connect(
	   boost::bind(&ScenarioToolInfrastructureOptionsMenu::select_player, this));

	selected_items_.reset(new UI::MultilineTextarea(
	   main_box_.get(), 0, 0, 100, 10, UI::PanelStyle::kWui, "", UI::Align::kCenter,
	   UI::MultilineTextarea::ScrollMode::kNoScrolling));
	main_box_->add(selected_items_.get(), UI::Box::Resizing::kFullSize);

	update_text();

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolInfrastructureOptionsMenu::create_buildings_tab() {
	assert(item_categories_);
	item_categories_->remove_last_tab("buildings");

	UI::TabPanel* sizetabs = new UI::TabPanel(item_categories_.get(), UI::TabPanelStyle::kWuiDark);
	UI::IconGrid* ig_small = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
	UI::IconGrid* ig_medium = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
	UI::IconGrid* ig_big = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
	UI::IconGrid* ig_port = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
	UI::IconGrid* ig_mine = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
	for (UI::IconGrid* i : {ig_small, ig_medium, ig_big, ig_port, ig_mine}) {
		i->icon_clicked.connect(boost::bind(&ScenarioToolInfrastructureOptionsMenu::toggle_selected,
		                                    this, i, Widelands::MapObjectType::BUILDING, _1));
	}

	const Widelands::TribeDescr& tribe = eia().egbase().player(players_->get_selected()).tribe();
	std::set<Widelands::DescriptionIndex> buildings;
	for (Widelands::DescriptionIndex di : tribe.buildings()) {
		assert(!buildings.count(di));
		buildings.insert(di);
		const Widelands::BuildingDescr* descr = tribe.get_building_descr(di);
		if (descr->type() == Widelands::MapObjectType::CONSTRUCTIONSITE ||
		    descr->type() == Widelands::MapObjectType::DISMANTLESITE) {
			continue;
		}
		UI::IconGrid* i = nullptr;
		if (descr->get_ismine()) {
			i = ig_mine;
		} else if (descr->get_isport()) {
			i = ig_port;
		} else
			switch (descr->get_size()) {
			case Widelands::BaseImmovable::BIG:
				i = ig_big;
				break;
			case Widelands::BaseImmovable::MEDIUM:
				i = ig_medium;
				break;
			case Widelands::BaseImmovable::SMALL:
				i = ig_small;
				break;
			default:
				NEVER_HERE();
			}
		assert(i);
		i->add(descr->name(), descr->representative_image(), reinterpret_cast<void*>(di),
		       descr->descname());
	}
	const size_t nrb = eia().egbase().tribes().nrbuildings();
	for (Widelands::DescriptionIndex di = 0; di < nrb; ++di) {
		if (buildings.count(di))
			continue;
		buildings.insert(di);
		const Widelands::BuildingDescr* descr = tribe.get_building_descr(di);
		if (descr->type() != Widelands::MapObjectType::MILITARYSITE) {
			continue;
		}
		UI::IconGrid* i = nullptr;
		if (descr->get_ismine()) {
			i = ig_mine;
		} else if (descr->get_isport()) {
			i = ig_port;
		} else
			switch (descr->get_size()) {
			case Widelands::BaseImmovable::BIG:
				i = ig_big;
				break;
			case Widelands::BaseImmovable::MEDIUM:
				i = ig_medium;
				break;
			case Widelands::BaseImmovable::SMALL:
				i = ig_small;
				break;
			default:
				NEVER_HERE();
			}
		assert(i);
		i->add(descr->name(), descr->representative_image(), reinterpret_cast<void*>(di),
		       descr->descname());
	}

#define ADD_TAB(size, tt)                                                                          \
	sizetabs->add(#size, g_gr->images().get("images/wui/fieldaction/menu_tab_build" #size ".png"),  \
	              ig_##size, tt);
	ADD_TAB(small, _("Small buildings"))
	ADD_TAB(medium, _("Medium buildings"))
	ADD_TAB(big, _("Big buildings"))
	ADD_TAB(port, _("Ports"))
	ADD_TAB(mine, _("Mines"))
#undef ADD_TAB

	item_categories_->add("buildings",
	                      g_gr->images().get("images/wui/stats/genstats_nrbuildings.png"), sizetabs,
	                      _("Buildings"));
}

void ScenarioToolInfrastructureOptionsMenu::select_player() {
	const Widelands::PlayerNumber p = players_->get_selected();
	assert(p <= eia().egbase().map().get_nrplayers());
	tool_.set_player(p);
	create_buildings_tab();
	select_correct_tool();
}

void ScenarioToolInfrastructureOptionsMenu::update_text() {
	const size_t nr_items = tool_.get_index().size();
	if (nr_items == 0) {
		selected_items_->set_text(_("Nothing selected"));
		return;
	}
	auto tribe_of_building = [this](Widelands::DescriptionIndex i) {
		const size_t nr = eia().egbase().tribes().nrtribes();
		for (size_t t = 0; t < nr; ++t) {
			const Widelands::TribeDescr& td = *eia().egbase().tribes().get_tribe_descr(t);
			if (td.has_building(i)) {
				return td.descname();
			}
		}
		NEVER_HERE();
	};
	auto name_of = [this, tribe_of_building](size_t i) {
		const auto& pair = tool_.get_index()[i];
		switch (pair.first) {
		case Widelands::MapObjectType::BUILDING:
			/** TRANSLATORS: "(Tribename) Building", e.g. "(Frisians) Headquarters" */
			return (boost::format(_("(%1$s) %2$s")) % tribe_of_building(pair.second) %
			        eia().egbase().tribes().get_building_descr(pair.second)->descname())
			   .str();
		case Widelands::MapObjectType::IMMOVABLE:
			return eia().egbase().tribes().get_immovable_descr(pair.second)->descname();
		case Widelands::MapObjectType::FLAG:
			return std::string(_("Flag"));
		default:
			NEVER_HERE();
		}
	};
	std::string text = name_of(0);
	for (size_t i = 1; i < nr_items; ++i) {
		/** TRANSLATORS: Selected items: Item 1 · Item 2 · Item 3 · … */
		text = (boost::format(_("%1$s · %2$s")) % text % name_of(i)).str();
	}
	selected_items_->set_text(text);
}

void ScenarioToolInfrastructureOptionsMenu::toggle_selected(UI::IconGrid* ig,
                                                            Widelands::MapObjectType type,
                                                            int32_t grid_idx) {
	auto& list = tool_.get_index();
	const Widelands::DescriptionIndex di =
	   static_cast<int32_t>(reinterpret_cast<intptr_t>(ig->get_data(grid_idx)));
	auto pair = std::make_pair(type, di);
	if (SDL_GetModState() & KMOD_CTRL) {
		auto it = std::find(list.begin(), list.end(), pair);
		if (it == list.end()) {
			list.push_back(pair);
		} else {
			list.erase(it);
		}
	} else {
		list.clear();
		list.push_back(pair);
	}
	update_text();
	select_correct_tool();
}
