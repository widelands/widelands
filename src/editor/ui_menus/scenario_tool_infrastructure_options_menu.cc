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

#include <memory>

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
   : EditorToolOptionsMenu(parent, registry, 500, 300, _("Set Field Owner"), tool), tool_(tool) {
	main_box_.reset(new UI::Box(this, 0, 0, UI::Box::Vertical));
	set_center_panel(main_box_.get());
	players_.reset(new UI::Dropdown<Widelands::PlayerNumber>(
	   main_box_.get(), "players", 0, 0, 300, 8, 24, _("Player"), UI::DropdownType::kTextual,
	   UI::PanelStyle::kWui, UI::ButtonStyle::kWuiSecondary));
	players_->selected.connect(
	   boost::bind(&ScenarioToolInfrastructureOptionsMenu::select_player, this));
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

	item_categories_.reset(new UI::TabPanel(main_box_.get(), UI::TabPanelStyle::kWuiDark));
	{
		UI::IconGrid* i = new UI::IconGrid(item_categories_.get(), 0, 0, 50, 50, 1);
		i->add("flag", g_gr->images().get("images/wui/fieldaction/menu_build_flag.png"),
		       reinterpret_cast<void*>(Widelands::INVALID_INDEX), _("Flag"));
		i->icon_clicked.connect(boost::bind(&ScenarioToolInfrastructureOptionsMenu::toggle_selected,
		                                    this, i, Widelands::MapObjectType::FLAG, _1));
		item_categories_->add(std::to_string(static_cast<int>(Widelands::MapObjectType::FLAG)),
		                      g_gr->images().get("images/wui/fieldaction/menu_build_flag.png"), i,
		                      _("Flag"));
		item_grids_.push_back(std::unique_ptr<UI::IconGrid>(i));
	}
	const size_t nr_tribes = parent.egbase().tribes().nrtribes();
	for (Widelands::MapObjectType t :
	     {Widelands::MapObjectType::BUILDING, Widelands::MapObjectType::IMMOVABLE}) {
		UI::TabPanel* tab = new UI::TabPanel(item_categories_.get(), UI::TabPanelStyle::kWuiDark);
		for (size_t tribe = 0; tribe < nr_tribes; ++tribe) {
			const Widelands::TribeDescr* td = parent.egbase().tribes().get_tribe_descr(tribe);
			const Image* icon = g_gr->images().get(Widelands::get_tribeinfo(td->name()).icon);
			assert(td);
			switch (t) {
			case Widelands::MapObjectType::BUILDING: {
				UI::TabPanel* sizetabs = new UI::TabPanel(tab, UI::TabPanelStyle::kWuiDark);
				UI::IconGrid* ig_small = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
				UI::IconGrid* ig_medium = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
				UI::IconGrid* ig_big = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
				UI::IconGrid* ig_port = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
				UI::IconGrid* ig_mine = new UI::IconGrid(sizetabs, 0, 0, 50, 50, kIconGridColumns);
				for (Widelands::DescriptionIndex di : td->buildings()) {
					const Widelands::BuildingDescr* descr = td->get_building_descr(di);
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
					i->add(std::to_string(static_cast<int>(t)) + "__" + descr->name(),
					       descr->representative_image(), reinterpret_cast<void*>(di),
					       descr->descname());
				}
				for (UI::IconGrid* i : {ig_small, ig_medium, ig_big, ig_port, ig_mine}) {
					i->icon_clicked.connect(boost::bind(
					   &ScenarioToolInfrastructureOptionsMenu::toggle_selected, this, i, t, _1));
					item_grids_.push_back(std::unique_ptr<UI::IconGrid>(i));
				}
				sizetabs->add(td->name() + "_small",
				              g_gr->images().get("images/wui/fieldaction/menu_tab_buildsmall.png"),
				              ig_small, _("Small Buildings"));
				sizetabs->add(td->name() + "_medium",
				              g_gr->images().get("images/wui/fieldaction/menu_tab_buildmedium.png"),
				              ig_medium, _("Medium Buildings"));
				sizetabs->add(td->name() + "_big",
				              g_gr->images().get("images/wui/fieldaction/menu_tab_buildbig.png"),
				              ig_big, _("Big Buildings"));
				sizetabs->add(td->name() + "_port",
				              g_gr->images().get("images/wui/fieldaction/menu_tab_buildport.png"),
				              ig_port, _("Ports"));
				sizetabs->add(td->name() + "_mine",
				              g_gr->images().get("images/wui/fieldaction/menu_tab_buildmine.png"),
				              ig_mine, _("Mines"));
				item_tabs_.push_back(std::unique_ptr<UI::TabPanel>(sizetabs));
				tab->add(std::to_string(static_cast<int>(t)) + "_" + std::to_string(tribe), icon,
				         sizetabs, td->descname());
				break;
			}
			case Widelands::MapObjectType::IMMOVABLE: {
				UI::IconGrid* i = new UI::IconGrid(tab, 0, 0, 50, 50, kIconGridColumns);
				for (Widelands::DescriptionIndex di : td->immovables()) {
					const Widelands::ImmovableDescr* descr = td->get_immovable_descr(di);
					i->add(std::to_string(static_cast<int>(t)) + "_" + std::to_string(tribe) + "_" +
					          descr->name(),
					       descr->representative_image(), reinterpret_cast<void*>(di),
					       descr->descname());
				}
				i->icon_clicked.connect(boost::bind(
				   &ScenarioToolInfrastructureOptionsMenu::toggle_selected, this, i, t, _1));
				tab->add(std::to_string(static_cast<int>(t)) + "_" + std::to_string(tribe), icon, i,
				         td->descname());
				item_grids_.push_back(std::unique_ptr<UI::IconGrid>(i));
				break;
			}
			default:
				NEVER_HERE();
			}
		}
		item_categories_->add(
		   std::to_string(static_cast<int>(t)),
		   g_gr->images().get(t == Widelands::MapObjectType::BUILDING ?
		                         "images/wui/stats/genstats_nrbuildings.png" :
		                         "images/wui/menus/toggle_immovables.png"),
		   tab, t == Widelands::MapObjectType::BUILDING ? _("Buildings") : _("Immovables"));
		item_tabs_.push_back(std::unique_ptr<UI::TabPanel>(tab));
	}
	main_box_->add(item_categories_.get(), UI::Box::Resizing::kExpandBoth);

	selected_items_.reset(new UI::MultilineTextarea(
	   main_box_.get(), 0, 0, 100, 10, UI::PanelStyle::kWui, "", UI::Align::kCenter,
	   UI::MultilineTextarea::ScrollMode::kNoScrolling));
	main_box_->add(selected_items_.get(), UI::Box::Resizing::kFullSize);

	subscriber_ = Notifications::subscribe<Widelands::NoteEditorPlayerEdited>(
	   [this](const Widelands::NoteEditorPlayerEdited& n) {
		   if (n.map == &eia().egbase().map()) {
			   update_players();
		   }
	   });

	update_players();
	update_text();

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolInfrastructureOptionsMenu::update_players() {
	const Widelands::Map& map = eia().egbase().map();
	const Widelands::PlayerNumber sel = tool_.get_player();
	const Widelands::PlayerNumber max = map.get_nrplayers();
	players_->clear();
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const std::string name = map.get_scenario_player_name(p);
		const std::string tribe = map.get_scenario_player_tribe(p);
		players_->add(
		   (boost::format(_("Player %1$s (%2$s)")) % std::to_string(static_cast<int>(p)) % name)
		      .str(),
		   p,
		   g_gr->images().get(
		      tribe.empty() ?
		         "images/ui_fsmenu/random.png" :
		         Widelands::get_tribeinfo(eia().egbase().map().get_scenario_player_tribe(p)).icon),
		   sel == p);
	}
}

void ScenarioToolInfrastructureOptionsMenu::select_player() {
	const Widelands::PlayerNumber p = players_->get_selected();
	assert(p <= eia().egbase().map().get_nrplayers());
	tool_.set_player(p);
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
