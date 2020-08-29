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
#include "ui_basic/messagebox.h"

inline EditorInteractive& ScenarioToolInfrastructureOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

constexpr uint16_t kIconGridCellSize = 48;
constexpr uint16_t kIconGridColumns = 10;

ScenarioToolInfrastructureOptionsMenu::ScenarioToolInfrastructureOptionsMenu(
   EditorInteractive& parent,
   ScenarioInfrastructureTool& tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 400, 200, _("Place Infrastructure"), tool),
     tool_(tool),
     box_(this, 0, 0, UI::Box::Vertical),
     buttonsbox_(&box_, 0, 0, UI::Box::Horizontal),
     auto_infra_cur_(
        &buttonsbox_,
        "auto_infra_cur",
        0,
        0,
        100,
        30,
        UI::ButtonStyle::kWuiSecondary,
        _("Place one Headquarters"),
        _("Automatically place a Headquarters building at this player’s starting position")),
     auto_infra_all_(
        &buttonsbox_,
        "auto_infra_all",
        0,
        0,
        100,
        30,
        UI::ButtonStyle::kWuiSecondary,
        _("Place all Headquarters"),
        _("Automatically place a Headquarters building at every player’s starting position")),
     tabs_(&box_, UI::TabPanelStyle::kWuiDark),
     force_(&box_,
            Vector2i(0, 0),
            _("Force"),
            _("Allow building on unsuitable spots, destroy nearby immovables, "
              "and conquer the surrounding land")),
     selected_items_(&box_,
                     0,
                     0,
                     0,
                     0,
                     UI::PanelStyle::kWui,
                     "",
                     UI::Align::kCenter,
                     UI::MultilineTextarea::ScrollMode::kNoScrolling) {
	const Widelands::Map& map = parent.egbase().map();
	const Widelands::PlayerNumber max = map.get_nrplayers();
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const Widelands::TribeDescr& tribe = *parent.egbase().tribes().get_tribe_descr(
		   parent.egbase().tribes().tribe_index(map.get_scenario_player_tribe(p)));
		UI::TabPanel* tab = new UI::TabPanel(&tabs_, UI::TabPanelStyle::kWuiLight);
		{
			UI::Box* box = new UI::Box(tab, 0, 0, UI::Box::Vertical);

			UI::Checkbox* c = new UI::Checkbox(
			   box, Vector2i(0, 0), _("Construction Site"), _("Place a construction site if allowed"));
			box->add(c, UI::Box::Resizing::kFullSize);
			c->set_state(tool_.get_construct());
			c->changed.connect([this, c]() {
				tool_.set_construct(c->get_state());
				select_correct_tool();
			});

			UI::TabPanel* buildingtabs = new UI::TabPanel(box, UI::TabPanelStyle::kWuiLight);
			UI::IconGrid* ig_small = new UI::IconGrid(buildingtabs, 0, 0, 50, 50, kIconGridColumns);
			UI::IconGrid* ig_medium = new UI::IconGrid(buildingtabs, 0, 0, 50, 50, kIconGridColumns);
			UI::IconGrid* ig_big = new UI::IconGrid(buildingtabs, 0, 0, 50, 50, kIconGridColumns);
			UI::IconGrid* ig_port = new UI::IconGrid(buildingtabs, 0, 0, 50, 50, kIconGridColumns);
			UI::IconGrid* ig_mine = new UI::IconGrid(buildingtabs, 0, 0, 50, 50, kIconGridColumns);
			for (UI::IconGrid* i : {ig_small, ig_medium, ig_big, ig_port, ig_mine}) {
				i->icon_clicked.connect(
				   [this, i](int32_t x) { toggle_selected(i, Widelands::MapObjectType::BUILDING, x); });
			}

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
				} else {
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
				}
				assert(i);
				i->add(descr->name(), descr->representative_image(), reinterpret_cast<void*>(di),
				       descr->descname());
			}
			const size_t nrb = eia().egbase().tribes().nrbuildings();
			for (Widelands::DescriptionIndex di = 0; di < nrb; ++di) {
				if (buildings.count(di)) {
					continue;
				}
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
				} else {
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
				}
				assert(i);
				i->add(descr->name(), descr->representative_image(), reinterpret_cast<void*>(di),
				       descr->descname());
			}

#define ADD_TAB(size, tt)                                                                          \
	buildingtabs->add(#size,                                                                        \
	                  g_image_cache->get("images/wui/fieldaction/menu_tab_build" #size ".png"),     \
	                  ig_##size, tt);
			ADD_TAB(small, _("Small buildings"))
			ADD_TAB(medium, _("Medium buildings"))
			ADD_TAB(big, _("Big buildings"))
			ADD_TAB(port, _("Ports"))
			ADD_TAB(mine, _("Mines"))
#undef ADD_TAB
			box->add(buildingtabs, UI::Box::Resizing::kFullSize);
			tab->add("buildings", g_image_cache->get("images/wui/stats/genstats_nrbuildings.png"), box,
			         _("Buildings"));
		}
		{
			const Image* icon = g_image_cache->get("images/wui/fieldaction/menu_build_flag.png");
			UI::IconGrid* i = new UI::IconGrid(tab, 0, 0, kIconGridCellSize, kIconGridCellSize, 1);
			i->add("flag", icon, reinterpret_cast<void*>(Widelands::INVALID_INDEX), _("Flag"));
			i->icon_clicked.connect(
			   [this, i](int32_t x) { toggle_selected(i, Widelands::MapObjectType::FLAG, x); });
			tab->add("flag", icon, i, _("Flag"));
		}
		{
			UI::IconGrid* i =
			   new UI::IconGrid(tab, 0, 0, kIconGridCellSize, kIconGridCellSize, kIconGridColumns);
			for (Widelands::DescriptionIndex di : tribe.immovables()) {
				const Widelands::ImmovableDescr* d = tribe.get_immovable_descr(di);
				i->add(d->name(), d->icon(), reinterpret_cast<void*>(di), d->descname());
			}
			i->icon_clicked.connect(
			   [this, i](int32_t x) { toggle_selected(i, Widelands::MapObjectType::IMMOVABLE, x); });
			tab->add("immovables", g_image_cache->get("images/wui/menus/toggle_immovables.png"), i,
			         _("Immovables"));
		}
		tabs_.add("player_" + std::to_string(static_cast<unsigned>(p)),
		          map.get_scenario_player_name(p), tab);
	}
	tabs_.sigclicked.connect([this]() { select_tab(); });

	force_.set_state(tool_.get_force());
	force_.changed.connect([this]() {
		tool_.set_force(force_.get_state());
		select_correct_tool();
	});
	auto_infra_cur_.sigclicked.connect([this]() {
		make_auto_infra(false);
		select_correct_tool();
	});
	auto_infra_all_.sigclicked.connect([this]() {
		make_auto_infra(true);
		select_correct_tool();
	});

	buttonsbox_.add(&auto_infra_cur_, UI::Box::Resizing::kExpandBoth);
	buttonsbox_.add(&auto_infra_all_, UI::Box::Resizing::kExpandBoth);
	box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
	box_.add(&force_, UI::Box::Resizing::kFullSize);
	box_.add(&buttonsbox_, UI::Box::Resizing::kFullSize);
	box_.add(&selected_items_, UI::Box::Resizing::kFullSize);
	tabs_.activate(tool_.get_player() - 1);
	select_tab();
	set_center_panel(&box_);
	layout();
	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolInfrastructureOptionsMenu::make_auto_infra(bool all) {
	std::list<std::string> errors;
	std::list<Widelands::Building*> placed;
	Widelands::EditorGameBase& egbase = eia().egbase();
	// TODO(Nordfriese): Instead of iterating all warehouse types until we find one that seems to
	// fit,
	// add a "DescriptionIndex headquarters_" to TribeDescr which we can use here
	auto is_hq = [&egbase](Widelands::DescriptionIndex di) {
		const Widelands::BuildingDescr& b = *egbase.tribes().get_building_descr(di);
		return b.type() == Widelands::MapObjectType::WAREHOUSE &&
		       b.get_size() == Widelands::BaseImmovable::Size::BIG && !b.get_isport() &&
		       !b.get_ismine() && !b.is_destructible() && !b.can_be_dismantled() &&
		       !b.is_buildable() && !b.is_enhanced() && !b.needs_seafaring() &&
		       !b.needs_waterways() && b.get_built_over_immovable() == Widelands::INVALID_INDEX;
	};
	std::set<unsigned> for_whom;
	if (all) {
		for (unsigned p = egbase.map().get_nrplayers(); p; --p) {
			for_whom.insert(p);
		}
	} else {
		for_whom.insert(tabs_.active() + 1);
	}
	for (unsigned p : for_whom) {
		Widelands::Player& player = *egbase.get_player(p);
		Widelands::DescriptionIndex index =
		   player.tribe().building_index(player.tribe().name() + "_headquarters");
		if (index == Widelands::INVALID_INDEX || !is_hq(index)) {
			// So the HQ for this tribe does not follow the naming conventions. So we try to find some
			// other suitable building…
			index = Widelands::INVALID_INDEX;
			for (Widelands::DescriptionIndex di : player.tribe().buildings()) {
				if (is_hq(di)) {
					index = di;
					errors.push_back(
					   (boost::format(_("Player %1$u (tribe %2$s): Headquarters does not follow naming "
					                    "conventions, identified %3$s as headquarters")) %
					    p % player.tribe().descname().c_str() %
					    egbase.tribes().get_building_descr(di)->name().c_str())
					      .str());
				}
			}
		}
		if (index != Widelands::INVALID_INDEX) {
			const Widelands::BuildingDescr& b = *egbase.tribes().get_building_descr(index);
			if (b.suitability(
			       egbase.map(), egbase.map().get_fcoords(egbase.map().get_starting_pos(p)))) {
				placed.push_back(&player.force_building(
				   egbase.map().get_starting_pos(p), {std::make_pair(index, "")}));
			} else {
				errors.push_back(
				   (boost::format(
				       _("Player %1$u (tribe %2$s): Starting position is not suited for a %3$s")) %
				    p % player.tribe().descname().c_str() % b.name().c_str())
				      .str());
			}
		} else {
			errors.push_back(
			   (boost::format(
			       _("Player %1$u (tribe %2$s): Headquarters building could not be identified")) %
			    p % player.tribe().descname().c_str())
			      .str());
		}
	}
	for (Widelands::Building* b : placed) {
		eia().show_building_window(b->get_position(), true, false);
	}
	std::string text;
	if (errors.empty()) {
		text =
		   _("All buildings placed successfully. Don’t forget to add the initial wares and workers.");
	} else {
		text = _("Not all buildings could be placed!");
		text += "\n";
		text += (boost::format(
		            ngettext("%u warning generated:", "%u warnings generated:", errors.size())) %
		         errors.size())
		           .str();
		text += "\n";
		for (const std::string& e : errors) {
			text += "\n" + e;
		}
	}
	UI::WLMessageBox m(get_parent(), errors.empty() ? _("Success") : _("Error"), text,
	                   UI::WLMessageBox::MBoxType::kOk);
	m.run<UI::Panel::Returncodes>();
}

void ScenarioToolInfrastructureOptionsMenu::select_tab() {
	Widelands::EditorGameBase& egbase = eia().egbase();

	const Widelands::PlayerNumber p = tabs_.active() + 1;
	assert(p);
	assert(p <= egbase.map().get_nrplayers());
	tool_.set_player(p);

	const Widelands::TribeDescr& tribe = *egbase.tribes().get_tribe_descr(
	   egbase.tribes().tribe_index(egbase.map().get_scenario_player_tribe(p)));
	auto& list = tool_.get_indices();
	for (auto it = list.begin(); it != list.end();) {
		bool erase = false;
		switch (it->first) {
		case Widelands::MapObjectType::FLAG:
			break;
		case Widelands::MapObjectType::IMMOVABLE:
			erase = !tribe.has_immovable(it->second);
			break;
		case Widelands::MapObjectType::BUILDING:
			erase = !tribe.has_building(it->second) && tribe.get_building_descr(it->second)->type() !=
			                                              Widelands::MapObjectType::MILITARYSITE;
			break;
		default:
			NEVER_HERE();
		}
		if (erase) {
			it = list.erase(it);
		} else {
			++it;
		}
	}

	update_text();
	select_correct_tool();
}

void ScenarioToolInfrastructureOptionsMenu::update_text() {
	const size_t nr_items = tool_.get_indices().size();
	if (nr_items == 0) {
		selected_items_.set_text(pgettext("dropdown", "Not Selected"));
		return;
	}
	auto name_of = [this](size_t i) {
		const auto& pair = tool_.get_indices()[i];
		switch (pair.first) {
		case Widelands::MapObjectType::BUILDING:
			return eia().egbase().tribes().get_building_descr(pair.second)->descname();
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
	selected_items_.set_text(text);
}

void ScenarioToolInfrastructureOptionsMenu::toggle_selected(UI::IconGrid* ig,
                                                            Widelands::MapObjectType type,
                                                            int32_t grid_idx) {
	auto& list = tool_.get_indices();
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
