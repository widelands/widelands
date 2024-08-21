/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/building_statistics_menu.h"

#include <algorithm>

#include "base/i18n.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/style_manager.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/walkingdir.h"
#include "logic/player.h"
#include "wui/mapviewpixelfunctions.h"

constexpr int kBuildGridCellHeight = 50;
constexpr int kBuildGridCellWidth = 55;
constexpr int kMargin = 5;
constexpr int kColumns = 5;
constexpr int kButtonHeight = 20;
constexpr int kLabelHeight = 18;
constexpr int kSpinboxWidth = 4 * kBuildGridCellWidth;
constexpr int32_t kWindowWidth = kColumns * kBuildGridCellWidth;

constexpr Duration kUpdateTimeInGametimeMs(1000);  //  1 second, gametime

inline InteractivePlayer& BuildingStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

BuildingStatisticsMenu::BuildingStatisticsMenu(InteractivePlayer& parent,
                                               UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "building_statistics",
                      &registry,
                      kWindowWidth,
                      100,
                      _("Building Statistics")),
     style_(g_style_manager->building_statistics_style()),
     main_box_(this, UI::PanelStyle::kWui, "main_box", 0, 0, UI::Box::Vertical, 0, 0, kMargin),
     tab_panel_(&main_box_, UI::TabPanelStyle::kWuiDark, "tabs"),

     hbox_owned_(
        &main_box_, UI::PanelStyle::kWui, "owned_hbox", 0, 0, UI::Box::Horizontal, 0, 0, kMargin),
     hbox_construction_(&main_box_,
                        UI::PanelStyle::kWui,
                        "construction_hbox",
                        0,
                        0,
                        UI::Box::Horizontal,
                        0,
                        0,
                        kMargin),
     hbox_unproductive_(&main_box_,
                        UI::PanelStyle::kWui,
                        "unproductive_hbox",
                        0,
                        0,
                        UI::Box::Horizontal,
                        0,
                        0,
                        kMargin),

     label_name_(&main_box_,
                 UI::PanelStyle::kWui,
                 "label_name",
                 UI::FontStyle::kWuiLabel,
                 _("(no building selected)"),
                 UI::Align::kCenter),
     label_owned_(&hbox_owned_,
                  UI::PanelStyle::kWui,
                  "label_owned",
                  UI::FontStyle::kWuiLabel,
                  _("Owned:"),
                  UI::Align::kLeft),
     label_construction_(&hbox_construction_,
                         UI::PanelStyle::kWui,
                         "label_construction",
                         UI::FontStyle::kWuiLabel,
                         _("Under construction:"),
                         UI::Align::kLeft),
     label_unproductive_(&hbox_unproductive_,
                         UI::PanelStyle::kWui,
                         "label_unproductive",
                         UI::FontStyle::kWuiLabel,
                         "" /* text will be set later */,
                         UI::Align::kLeft),
     label_nr_owned_(&hbox_owned_,
                     UI::PanelStyle::kWui,
                     "owned",
                     UI::FontStyle::kWuiLabel,
                     "",
                     UI::Align::kRight),
     label_nr_construction_(&hbox_construction_,
                            UI::PanelStyle::kWui,
                            "construction",
                            UI::FontStyle::kWuiLabel,
                            "",
                            UI::Align::kRight),
     label_nr_unproductive_(&hbox_unproductive_,
                            UI::PanelStyle::kWui,
                            "unproductive",
                            UI::FontStyle::kWuiLabel,
                            "",
                            UI::Align::kRight),
     label_threshold_(&main_box_,
                      UI::PanelStyle::kWui,
                      "label_threshold",
                      UI::FontStyle::kWuiLabel,
                      _("Low productivity threshold:"),
                      UI::Align::kLeft),

     b_prev_owned_(&hbox_owned_,
                   "previous_owned",
                   0,
                   0,
                   kButtonHeight,
                   kButtonHeight,
                   UI::ButtonStyle::kWuiMenu,
                   g_image_cache->get("images/ui_basic/scrollbar_left.png"),
                   _("Show previous building")),
     b_next_owned_(&hbox_owned_,
                   "next_owned",
                   0,
                   0,
                   kButtonHeight,
                   kButtonHeight,
                   UI::ButtonStyle::kWuiMenu,
                   g_image_cache->get("images/ui_basic/scrollbar_right.png"),
                   _("Show next building")),

     b_prev_construction_(&hbox_construction_,
                          "previous_construction",
                          0,
                          0,
                          kButtonHeight,
                          kButtonHeight,
                          UI::ButtonStyle::kWuiMenu,
                          g_image_cache->get("images/ui_basic/scrollbar_left.png"),
                          _("Show previous building")),
     b_next_construction_(&hbox_construction_,
                          "next_construction",
                          0,
                          0,
                          kButtonHeight,
                          kButtonHeight,
                          UI::ButtonStyle::kWuiMenu,
                          g_image_cache->get("images/ui_basic/scrollbar_right.png"),
                          _("Show next building")),

     b_prev_unproductive_(&hbox_unproductive_,
                          "previous_unproductive",
                          0,
                          0,
                          kButtonHeight,
                          kButtonHeight,
                          UI::ButtonStyle::kWuiMenu,
                          g_image_cache->get("images/ui_basic/scrollbar_left.png"),
                          _("Show previous building")),
     b_next_unproductive_(&hbox_unproductive_,
                          "next_unproductive",
                          0,
                          0,
                          kButtonHeight,
                          kButtonHeight,
                          UI::ButtonStyle::kWuiMenu,
                          g_image_cache->get("images/ui_basic/scrollbar_right.png"),
                          _("Show next building")),

     unproductive_threshold_(&main_box_,
                             "unproductive_threshold",
                             0,
                             0,
                             kSpinboxWidth,
                             kSpinboxWidth,
                             low_production_,
                             0,
                             100,
                             UI::PanelStyle::kWui,
                             "",
                             UI::SpinBox::Units::kPercent,
                             UI::SpinBox::Type::kBig),
     nr_building_types_(parent.egbase().descriptions().nr_buildings()),
     last_traffic_type_((iplayer().game().map().get_waterway_max_length() >= 2) ?
                           TrafficStat::kLast :
                           TrafficStat::kWaterway) {
	const Widelands::Player& player = iplayer().player();
	const Widelands::TribeDescr& tribe = player.tribe();
	const Widelands::Descriptions& descriptions = iplayer().egbase().descriptions();
	traffic_stats_ = {
	   TrafficStatData(_("Flag"), g_animation_manager->get_representative_image(
	                                 tribe.flag_animation(), &player.get_playercolor())),
	   TrafficStatData(_("Road"), descriptions.get_worker_descr(tribe.carriers()[0])
	                                 ->representative_image(&player.get_playercolor())),
	   TrafficStatData(_("Busy Road"), descriptions.get_worker_descr(tribe.carriers()[1])
	                                      ->representative_image(&player.get_playercolor()))};
	if (last_traffic_type_ == TrafficStat::kLast) {
		traffic_stats_.emplace_back(
		   _("Waterway"), descriptions.get_worker_descr(tribe.ferry())
		                     ->representative_image(&player.get_playercolor()));
	}

	building_buttons_ = std::vector<UI::Button*>(nr_building_types_);
	owned_labels_ = std::vector<UI::Textarea*>(nr_building_types_);
	productivity_labels_ = std::vector<UI::Textarea*>(nr_building_types_);

	hbox_owned_.add(&label_owned_, UI::Box::Resizing::kFullSize);
	hbox_owned_.add_inf_space();
	hbox_owned_.add(&label_nr_owned_, UI::Box::Resizing::kFullSize);
	hbox_owned_.add(&b_prev_owned_);
	hbox_owned_.add(&b_next_owned_);

	hbox_construction_.add(&label_construction_, UI::Box::Resizing::kFullSize);
	hbox_construction_.add_inf_space();
	hbox_construction_.add(&label_nr_construction_, UI::Box::Resizing::kFullSize);
	hbox_construction_.add(&b_prev_construction_);
	hbox_construction_.add(&b_next_construction_);

	hbox_unproductive_.add(&label_unproductive_, UI::Box::Resizing::kFullSize);
	hbox_unproductive_.add_inf_space();
	hbox_unproductive_.add(&label_nr_unproductive_, UI::Box::Resizing::kFullSize);
	hbox_unproductive_.add(&b_prev_unproductive_);
	hbox_unproductive_.add(&b_next_unproductive_);

	main_box_.set_size(200, 100);  // guard against SpinBox asserts
	main_box_.add(&tab_panel_, UI::Box::Resizing::kFullSize);
	main_box_.add(&label_name_, UI::Box::Resizing::kFullSize);
	main_box_.add(&hbox_owned_, UI::Box::Resizing::kFullSize);
	main_box_.add(&hbox_construction_, UI::Box::Resizing::kFullSize);
	main_box_.add(&hbox_unproductive_, UI::Box::Resizing::kFullSize);

	main_box_.add_space(kMargin);
	main_box_.add(&label_threshold_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add(&unproductive_threshold_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	unproductive_threshold_.set_tooltip(_("Buildings will be considered unproductive if their "
	                                      "productivity falls below this percentage"));

	b_prev_owned_.sigclicked.connect([this]() { jump(JumpTarget::kOwned, true); });
	b_next_owned_.sigclicked.connect([this]() { jump(JumpTarget::kOwned, false); });
	b_prev_construction_.sigclicked.connect([this]() { jump(JumpTarget::kConstruction, true); });
	b_next_construction_.sigclicked.connect([this]() { jump(JumpTarget::kConstruction, false); });
	b_prev_unproductive_.sigclicked.connect([this]() { jump(JumpTarget::kUnproductive, true); });
	b_next_unproductive_.sigclicked.connect([this]() { jump(JumpTarget::kUnproductive, false); });

	unproductive_threshold_.changed.connect([this]() { low_production_changed(); });

	set_center_panel(&main_box_);

	init();
}

BuildingStatisticsMenu::~BuildingStatisticsMenu() {
	building_buttons_.clear();
	owned_labels_.clear();
	productivity_labels_.clear();

	traffic_stats_.clear();
}

void BuildingStatisticsMenu::reset() {
	update();  // In case a building got removed, make sure to deselect it first

	const int last_selected_tab = tab_assignments_[tab_panel_.active()];

	tab_panel_.remove_last_tab("building_stats_ports");
	tab_panel_.remove_last_tab("building_stats_mines");
	tab_panel_.remove_last_tab("building_stats_big");
	tab_panel_.remove_last_tab("building_stats_medium");
	tab_panel_.remove_last_tab("building_stats_small");
	tab_panel_.remove_last_tab("building_stats_traffic");

	// Clean state if buildings disappear from list
	building_buttons_.clear();
	building_buttons_.resize(nr_building_types_);
	owned_labels_.clear();
	owned_labels_.resize(nr_building_types_);
	productivity_labels_.clear();
	productivity_labels_.resize(nr_building_types_);

	// Ensure that defunct buttons disappear
	for (UI::Box* tab : tabs_) {
		if (tab != nullptr) {
			tab->die();
		}
	}

	init(last_selected_tab);

	// Reset navigator
	label_name_.set_visible(false);
	if (has_selection_) {
		if (building_buttons_[current_building_type_] != nullptr) {
			set_current_building_type(current_building_type_);
		} else if (current_traffic_type_ != TrafficStat::kLast) {
			set_current_traffic_type(current_traffic_type_);
		} else {
			has_selection_ = false;
		}
	}
}

void BuildingStatisticsMenu::init(int last_selected_tab) {
	// We want to add player tribe's buildings in correct order
	const Widelands::Player& player = iplayer().player();
	const Widelands::TribeDescr& tribe = player.tribe();
	const bool map_allows_seafaring = iplayer().game().map().allows_seafaring();
	const bool map_allows_waterways = iplayer().game().map().get_waterway_max_length() >= 2;
	std::vector<Widelands::DescriptionIndex> buildings_to_add[kNoOfBuildingTabs];
	// Add the player's own tribe's buildings.
	for (Widelands::DescriptionIndex index : tribe.buildings()) {
		if (own_building_is_valid(player, index, map_allows_seafaring, map_allows_waterways)) {
			buildings_to_add[find_tab_for_building(*tribe.get_building_descr(index))].push_back(index);
		}
	}

	// We want to add other tribes' buildings on the bottom. Only add the ones that the player owns.
	for (Widelands::DescriptionIndex index = 0; index < nr_building_types_; ++index) {
		if (foreign_tribe_building_is_valid(player, index)) {
			buildings_to_add[find_tab_for_building(*tribe.get_building_descr(index))].push_back(index);
		}
	}

	// Now create the tab contents and add the building buttons
	int row_counters[kNoOfBuildingTabs];
	for (int tab_index = 1; tab_index < kNoOfBuildingTabs; ++tab_index) {
		int current_column = 0;
		tabs_[tab_index] = new UI::Box(&tab_panel_, UI::PanelStyle::kWui,
		                               format("tab_box_%d", tab_index), 0, 0, UI::Box::Vertical);
		UI::Box* row = new UI::Box(tabs_[tab_index], UI::PanelStyle::kWui,
		                           format("row_box_%d", tab_index), 0, 0, UI::Box::Horizontal);
		row_counters[tab_index] = 0;

		for (const Widelands::DescriptionIndex id : buildings_to_add[tab_index]) {
			const Widelands::BuildingDescr& descr =
			   *iplayer().egbase().descriptions().get_building_descr(id);
			add_button(id, descr, row);
			++current_column;
			if (current_column == 1) {
				++row_counters[tab_index];
			} else if (current_column == kColumns) {
				tabs_[tab_index]->add(row, UI::Box::Resizing::kFullSize);
				tabs_[tab_index]->add_space(6);
				row = new UI::Box(tabs_[tab_index], UI::PanelStyle::kWui,
				                  format("row_box_%d_%u", tab_index, id), 0, 0, UI::Box::Horizontal);
				current_column = 0;
			}
		}
		// Add final row
		if (current_column != 0) {
			tabs_[tab_index]->add(row, UI::Box::Resizing::kFullSize);
		}
	}

	// Add traffic (tecnically not buildings)
	tabs_[0] = new UI::Box(
	   &tab_panel_, UI::PanelStyle::kWui, format("tab_box_%d", 0), 0, 0, UI::Box::Vertical);
	UI::Box* traffic_row = new UI::Box(
	   tabs_[0], UI::PanelStyle::kWui, format("row_box_%d", 0), 0, 0, UI::Box::Horizontal);
	for (uint8_t tt = TrafficStat::kFlag; tt < last_traffic_type_; ++tt) {
		add_button(static_cast<TrafficStat>(tt), traffic_row);
	}
	tabs_[0]->add(traffic_row, UI::Box::Resizing::kFullSize);
	row_counters[0] = 1;

	// Show the tabs that have buttons on them
	int tab_counter = 0;
	auto add_tab = [this, row_counters, &tab_counter, last_selected_tab](
	                  int tab_index, const std::string& name, const std::string& image,
	                  const std::string& descr) {
		if (row_counters[tab_index] > 0) {
			tab_panel_.add(name, g_image_cache->get(image), tabs_[tab_index], descr);
			if (last_selected_tab == tab_index) {
				tab_panel_.activate(tab_counter);
			}
			tab_assignments_[tab_counter] = tab_index;
			row_counters_[tab_counter] = row_counters[tab_index];
			++tab_counter;
		}
	};
	add_tab(BuildingTab::Traffic, "building_stats_traffic",
	        "images/wui/fieldaction/menu_tab_buildroad.png", _("Traffic"));
	add_tab(BuildingTab::Small, "building_stats_small",
	        "images/wui/fieldaction/menu_tab_buildsmall.png", _("Small buildings"));
	add_tab(BuildingTab::Medium, "building_stats_medium",
	        "images/wui/fieldaction/menu_tab_buildmedium.png", _("Medium buildings"));
	add_tab(BuildingTab::Big, "building_stats_big", "images/wui/fieldaction/menu_tab_buildbig.png",
	        _("Big buildings"));
	add_tab(BuildingTab::Mines, "building_stats_mines",
	        "images/wui/fieldaction/menu_tab_buildmine.png", _("Mines"));
	add_tab(BuildingTab::Ports, "building_stats_ports",
	        "images/wui/fieldaction/menu_tab_buildport.png", _("Ports"));

	update();

	initialization_complete();
}

bool BuildingStatisticsMenu::own_building_is_valid(const Widelands::Player& player,
                                                   Widelands::DescriptionIndex index,
                                                   bool map_allows_seafaring,
                                                   bool map_allows_waterways) const {
	const Widelands::BuildingDescr& descr = *player.tribe().get_building_descr(index);

	if (!descr.is_useful_on_map(map_allows_seafaring, map_allows_waterways) &&
	    player.get_building_statistics(index).empty()) {
		return false;
	}
	if (descr.type() == Widelands::MapObjectType::CONSTRUCTIONSITE ||
	    descr.type() == Widelands::MapObjectType::DISMANTLESITE) {
		return false;
	}
	// Only add allowed buildings or buildings that are owned by the player.
	return (
	   (player.is_building_type_allowed(index) && (descr.is_buildable() || descr.is_enhanced())) ||
	   !player.get_building_statistics(index).empty());
}

bool BuildingStatisticsMenu::foreign_tribe_building_is_valid(
   const Widelands::Player& player, Widelands::DescriptionIndex index) const {
	if (!player.tribe().has_building(index) && !player.get_building_statistics(index).empty()) {
		const Widelands::BuildingDescr& descr =
		   *iplayer().egbase().descriptions().get_building_descr(index);
		return (descr.type() != Widelands::MapObjectType::CONSTRUCTIONSITE &&
		        descr.type() != Widelands::MapObjectType::DISMANTLESITE);
	}
	return false;
}

int BuildingStatisticsMenu::find_tab_for_building(const Widelands::BuildingDescr& descr) const {
	assert(descr.type() != Widelands::MapObjectType::CONSTRUCTIONSITE);
	assert(descr.type() != Widelands::MapObjectType::DISMANTLESITE);
	if (descr.get_ismine()) {
		return BuildingTab::Mines;
	}
	if (descr.get_isport()) {
		return BuildingTab::Ports;
	}
	switch (descr.get_size()) {
	case Widelands::BaseImmovable::SMALL:
		return BuildingTab::Small;
	case Widelands::BaseImmovable::MEDIUM:
		return BuildingTab::Medium;
	case Widelands::BaseImmovable::BIG:
		return BuildingTab::Big;
	default:
		throw wexception(
		   "Building statistics: Found building without a size: %s", descr.name().c_str());
	}

	NEVER_HERE();
}

void BuildingStatisticsMenu::update_building_list() {
	const Widelands::Player& player = iplayer().player();
	const bool map_allows_seafaring = iplayer().game().map().allows_seafaring();
	const bool map_allows_waterways = iplayer().game().map().get_waterway_max_length() >= 2;
	for (Widelands::DescriptionIndex index = 0; index < nr_building_types_; ++index) {
		const bool should_have_this_building =
		   own_building_is_valid(player, index, map_allows_seafaring, map_allows_waterways) ||
		   foreign_tribe_building_is_valid(player, index);
		const bool has_this_building = building_buttons_[index] != nullptr;
		if (should_have_this_building != has_this_building) {
			reset();
			return;
		}
	}
}

/**
 * Adds 3 buttons per building type.
 *
 * - Building image, steps through all buildings of the type
 * - Buildings owned, steps through constructionsites
 * - Productivity, steps though buildings with low productivity and stopped buildings
 */
void BuildingStatisticsMenu::add_button(Widelands::DescriptionIndex id,
                                        const Widelands::BuildingDescr& descr,
                                        UI::Box* row) {
	UI::Box* button_box = new UI::Box(
	   row, UI::PanelStyle::kWui, format("buttons_box_%s", descr.name()), 0, 0, UI::Box::Vertical);
	building_buttons_[id] =
	   new UI::Button(button_box, format("building_button%d", id), 0, 0, kBuildGridCellWidth,
	                  kBuildGridCellHeight, UI::ButtonStyle::kWuiBuildingStats,
	                  descr.representative_image(&iplayer().get_player()->get_playercolor()), "",
	                  UI::Button::VisualState::kFlat);
	building_buttons_[id]->set_disable_style(UI::ButtonDisableStyle::kMonochrome |
	                                         UI::ButtonDisableStyle::kFlat);
	button_box->add(building_buttons_[id]);

	owned_labels_[id] = new UI::Textarea(
	   button_box, UI::PanelStyle::kWui, format("label_%s", descr.name()), UI::FontStyle::kWuiLabel,
	   0, 0, kBuildGridCellWidth, kLabelHeight, "", UI::Align::kCenter);
	owned_labels_[id]->set_style_override(style_.building_statistics_button_font());
	owned_labels_[id]->set_fixed_width(kBuildGridCellWidth);
	button_box->add(owned_labels_[id]);

	productivity_labels_[id] = new UI::Textarea(
	   button_box, UI::PanelStyle::kWui, format("productivity_%s", descr.name()),
	   UI::FontStyle::kWuiLabel, 0, 0, kBuildGridCellWidth, kLabelHeight, "", UI::Align::kCenter);
	productivity_labels_[id]->set_style_override(style_.building_statistics_button_font());
	productivity_labels_[id]->set_fixed_width(kBuildGridCellWidth);
	button_box->add(productivity_labels_[id]);

	row->add(button_box);

	building_buttons_[id]->sigclicked.connect([this, id]() { set_current_building_type(id); });
}

void BuildingStatisticsMenu::add_button(BuildingStatisticsMenu::TrafficStat ts, UI::Box* row) {
	const std::string& name = traffic_stats_[ts].name;
	UI::Box* button_box = new UI::Box(
	   row, UI::PanelStyle::kWui, format("buttons_box_%s", name), 0, 0, UI::Box::Vertical);

	traffic_stats_[ts].button =
	   new UI::Button(button_box, format("building_button_%s", name), 0, 0, kBuildGridCellWidth,
	                  kBuildGridCellHeight, UI::ButtonStyle::kWuiBuildingStats,
	                  traffic_stats_[ts].image, name, UI::Button::VisualState::kFlat);
	traffic_stats_[ts].button->set_disable_style(UI::ButtonDisableStyle::kMonochrome |
	                                             UI::ButtonDisableStyle::kFlat);
	button_box->add(traffic_stats_[ts].button);

	traffic_stats_[ts].owned_label = new UI::Textarea(
	   button_box, UI::PanelStyle::kWui, format("label_%s", name), UI::FontStyle::kWuiLabel, 0, 0,
	   kBuildGridCellWidth, kLabelHeight, "", UI::Align::kCenter);
	traffic_stats_[ts].owned_label->set_style_override(style_.building_statistics_button_font());
	traffic_stats_[ts].owned_label->set_fixed_width(kBuildGridCellWidth);
	button_box->add(traffic_stats_[ts].owned_label);

	traffic_stats_[ts].productivity_label = new UI::Textarea(
	   button_box, UI::PanelStyle::kWui, format("productivity_%s", name), UI::FontStyle::kWuiLabel,
	   0, 0, kBuildGridCellWidth, kLabelHeight, "", UI::Align::kCenter);
	traffic_stats_[ts].productivity_label->set_style_override(
	   style_.building_statistics_button_font());
	traffic_stats_[ts].productivity_label->set_fixed_width(kBuildGridCellWidth);
	button_box->add(traffic_stats_[ts].productivity_label);

	row->add(button_box);

	traffic_stats_[ts].button->sigclicked.connect([this, ts]() { set_current_traffic_type(ts); });
}

void BuildingStatisticsMenu::jump(JumpTarget target, bool reverse) {
	if (current_building_type_ != Widelands::INVALID_INDEX) {
		jump_building(target, reverse);
	} else if (current_traffic_type_ != TrafficStat::kLast) {
		jump_traffic(target, reverse);
	}
}

void BuildingStatisticsMenu::jump_building(JumpTarget target, bool reverse) {
	bool found = true;
	if (last_building_type_ != current_building_type_) {
		last_building_index_ = 0;
	}
	last_building_type_ = current_building_type_;

	const std::vector<Widelands::Player::BuildingStats>& stats_vector =
	   iplayer().get_player()->get_building_statistics(current_building_type_);

	switch (target) {
	case JumpTarget::kOwned: {
		int32_t const curindex = last_building_index_;
		if (reverse) {
			while (validate_pointer(&(--last_building_index_), stats_vector.size()) != curindex) {
				if (!stats_vector[last_building_index_].is_constructionsite) {
					break;
				}
			}
		} else {
			while (validate_pointer(&(++last_building_index_), stats_vector.size()) != curindex) {
				if (!stats_vector[last_building_index_].is_constructionsite) {
					break;
				}
			}
		}
		break;
	}
	case JumpTarget::kConstruction: {
		int32_t const curindex = last_building_index_;
		if (reverse) {
			while (validate_pointer(&(--last_building_index_), stats_vector.size()) != curindex) {
				if (stats_vector[last_building_index_].is_constructionsite) {
					break;
				}
			}
		} else {
			while (validate_pointer(&(++last_building_index_), stats_vector.size()) != curindex) {
				if (stats_vector[last_building_index_].is_constructionsite) {
					break;
				}
			}
		}
		break;
	}
	case JumpTarget::kUnproductive: {
		const Widelands::Map& map = iplayer().egbase().map();
		int32_t const curindex = last_building_index_;
		found = false;
		if (reverse) {
			while (validate_pointer(&(--last_building_index_), stats_vector.size()) != curindex) {
				if (!stats_vector[last_building_index_].is_constructionsite) {
					if (upcast(Widelands::MilitarySite, militarysite,
					           map[stats_vector[last_building_index_].pos].get_immovable())) {
						const auto* soldier_control = militarysite->soldier_control();
						assert(soldier_control != nullptr);
						if (soldier_control->stationed_soldiers().size() <
						    soldier_control->soldier_capacity()) {
							found = true;
							break;
						}
					} else if (upcast(Widelands::ProductionSite, productionsite,
					                  map[stats_vector[last_building_index_].pos].get_immovable())) {
						if (productionsite->is_stopped() ||
						    productionsite->get_statistics_percent() < low_production_) {
							found = true;
							break;
						}
					}
				}
			}
		} else {
			while (validate_pointer(&(++last_building_index_), stats_vector.size()) != curindex) {
				if (!stats_vector[last_building_index_].is_constructionsite) {
					if (upcast(Widelands::MilitarySite, militarysite,
					           map[stats_vector[last_building_index_].pos].get_immovable())) {
						const auto* soldier_control = militarysite->soldier_control();
						assert(soldier_control != nullptr);
						if (soldier_control->stationed_soldiers().size() <
						    soldier_control->soldier_capacity()) {
							found = true;
							break;
						}
					} else if (upcast(Widelands::ProductionSite, productionsite,
					                  map[stats_vector[last_building_index_].pos].get_immovable())) {
						if (productionsite->is_stopped() ||
						    productionsite->get_statistics_percent() < low_production_) {
							found = true;
							break;
						}
					}
				}
			}
		}
		if (!found) {  // Now look at the old
			if (upcast(Widelands::MilitarySite, militarysite,
			           map[stats_vector[last_building_index_].pos].get_immovable())) {
				const auto* soldier_control = militarysite->soldier_control();
				assert(soldier_control != nullptr);
				if (soldier_control->stationed_soldiers().size() <
				    soldier_control->soldier_capacity()) {
					found = true;
				}
			} else if (upcast(Widelands::ProductionSite, productionsite,
			                  map[stats_vector[last_building_index_].pos].get_immovable())) {
				if (productionsite->is_stopped() ||
				    productionsite->get_statistics_percent() < low_production_) {
					found = true;
				}
			}
		}
		break;
	}

	default:
		NEVER_HERE();
	}

	if (found) {
		validate_pointer(&last_building_index_, stats_vector.size());
		iplayer().map_view()->scroll_to_field(
		   stats_vector[last_building_index_].pos, MapView::Transition::Smooth);
	}
	update();
}

void BuildingStatisticsMenu::jump_traffic(JumpTarget target, bool reverse) {
	assert(target == JumpTarget::kUnproductive);
	assert(current_traffic_type_ != TrafficStat::kLast);
	assert(!traffic_stats_[current_traffic_type_].jump_targets.empty());
	Vector2f cv = iplayer().map_view()->get_centered_view().viewpoint;
	Widelands::Coords cur =
	   MapviewPixelFunctions::calc_node_and_triangle(iplayer().game().map(), cv.x, cv.y).node;
	if (reverse) {
		auto it = traffic_stats_[current_traffic_type_].jump_targets.lower_bound(cur);
		if (it == traffic_stats_[current_traffic_type_].jump_targets.begin()) {
			it = traffic_stats_[current_traffic_type_].jump_targets.end();
		}
		--it;
		iplayer().map_view()->scroll_to_field(*it, MapView::Transition::Smooth);
	} else {
		auto it = traffic_stats_[current_traffic_type_].jump_targets.upper_bound(cur);
		if (it == traffic_stats_[current_traffic_type_].jump_targets.end()) {
			it = traffic_stats_[current_traffic_type_].jump_targets.begin();
		}
		iplayer().map_view()->scroll_to_field(*it, MapView::Transition::Smooth);
	}
}

/*
 * Update this statistic
 */
void BuildingStatisticsMenu::think() {
	// Update statistics
	const Time& gametime = iplayer().game().get_gametime();

	if (was_minimized_ || (gametime - lastupdate_) > kUpdateTimeInGametimeMs) {
		update_building_list();
		update();
		lastupdate_ = gametime;
	}
	// Make sure we don't have a delay with displaying labels when we restore the window.
	was_minimized_ = is_minimal();
}

/*
 * validate if this pointer is ok
 */
int32_t BuildingStatisticsMenu::validate_pointer(int32_t* const id, int32_t const size) {
	if (*id < 0) {
		*id = size - 1;
	}
	if (size <= *id) {
		*id = 0;
	}

	return *id;
}

/*
 * Update Buttons
 */
void BuildingStatisticsMenu::update() {
	const Widelands::Player& player = iplayer().player();
	const Widelands::TribeDescr& tribe = player.tribe();

	hbox_owned_.set_visible(false);
	hbox_construction_.set_visible(false);
	hbox_unproductive_.set_visible(false);

	for (Widelands::DescriptionIndex id = 0; id < nr_building_types_; ++id) {
		const Widelands::BuildingDescr& building = *tribe.get_building_descr(id);
		if (building_buttons_[id] == nullptr) {
			continue;
		}
		assert(productivity_labels_[id] != nullptr);
		assert(owned_labels_[id] != nullptr);

		const std::vector<Widelands::Player::BuildingStats>& stats_vector =
		   player.get_building_statistics(id);

		uint32_t nr_owned = 0;
		uint32_t nr_build = 0;
		uint32_t total_prod = 0;
		uint32_t total_soldier_capacity = 0;
		uint32_t total_stationed_soldiers = 0;
		uint32_t nr_unproductive = 0;

		for (const Widelands::Player::BuildingStats& stats : stats_vector) {
			if (stats.is_constructionsite) {
				++nr_build;
			} else {
				++nr_owned;
				Widelands::BaseImmovable& immovable =
				   *iplayer().game().map()[stats.pos].get_immovable();
				if (building.type() == Widelands::MapObjectType::PRODUCTIONSITE ||
				    building.type() == Widelands::MapObjectType::TRAININGSITE) {
					Widelands::ProductionSite& productionsite =
					   dynamic_cast<Widelands::ProductionSite&>(immovable);
					int percent = productionsite.get_statistics_percent();
					total_prod += percent;

					if (percent < low_production_ || productionsite.is_stopped()) {
						++nr_unproductive;
					}
				} else if (building.type() == Widelands::MapObjectType::MILITARYSITE) {
					const Widelands::SoldierControl* soldier_control =
					   dynamic_cast<Widelands::Building&>(immovable).soldier_control();
					assert(soldier_control != nullptr);
					total_soldier_capacity += soldier_control->soldier_capacity();
					total_stationed_soldiers += soldier_control->stationed_soldiers().size();
					if (total_stationed_soldiers < total_soldier_capacity) {
						++nr_unproductive;
					}
				}
			}
		}

		productivity_labels_[id]->set_visible(false);

		if (building.type() == Widelands::MapObjectType::PRODUCTIONSITE ||
		    building.type() == Widelands::MapObjectType::TRAININGSITE) {
			if (nr_owned != 0u) {
				int const percent =
				   static_cast<int>(static_cast<float>(total_prod) / static_cast<float>(nr_owned));

				const RGBColor& color =
				   (percent < low_production_) ? style_.low_color() :
				   (percent < ((low_production_ < 50) ?
                              2 * low_production_ :
                              low_production_ + ((100 - low_production_) / 2))) ?
                                             style_.medium_color() :
                                             style_.high_color();

				/** TRANSLATORS: Percent in building statistics window, e.g. 85% */
				/** TRANSLATORS: If you wish to add a space, translate as '%i %%' */
				const std::string perc_str = format(_("%i%%"), percent);
				set_labeltext(productivity_labels_[id], perc_str, color);
			}
			if (has_selection_ && id == current_building_type_) {
				label_nr_unproductive_.set_text(nr_unproductive > 0 ? std::to_string(nr_unproductive) :
                                                                  "");
				b_next_unproductive_.set_enabled(nr_unproductive > 0);
				b_prev_unproductive_.set_enabled(nr_unproductive > 0);
				hbox_unproductive_.set_visible(true);
				label_unproductive_.set_text(_("Low productivity:"));
			}
		} else if (building.type() == Widelands::MapObjectType::MILITARYSITE) {
			if (nr_owned != 0u) {
				const RGBColor& color =
				   (total_stationed_soldiers < total_soldier_capacity / 2) ? style_.low_color() :
				   (total_stationed_soldiers < total_soldier_capacity)     ? style_.medium_color() :
                                                                         style_.high_color();
				const std::string perc_str =
				   format(_("%1%/%2%"), total_stationed_soldiers, total_soldier_capacity);
				set_labeltext(productivity_labels_[id], perc_str, color);
			}
			if (has_selection_ && id == current_building_type_) {
				label_nr_unproductive_.set_text(nr_unproductive > 0 ? std::to_string(nr_unproductive) :
                                                                  "");
				b_next_unproductive_.set_enabled(total_soldier_capacity > total_stationed_soldiers);
				b_prev_unproductive_.set_enabled(total_soldier_capacity > total_stationed_soldiers);
				hbox_unproductive_.set_visible(true);
				/** TRANSLATORS: Label for number of buildings that are waiting for soldiers */
				label_unproductive_.set_text(_("Lacking soldiers:"));
			}
		}

		std::string owned_text;
		const bool can_construct_this_building =
		   player.tribe().has_building(id) && (building.is_buildable() || building.is_enhanced());
		if (can_construct_this_building) {
			/** TRANSLATORS: Buildings: owned / under construction */
			owned_text = format(_("%1%/%2%"), nr_owned, nr_build);
		} else {
			owned_text = format(_("%1%/%2%"), nr_owned, "–");
		}
		set_labeltext(
		   owned_labels_[id], owned_text, style_.building_statistics_details_font().color());
		owned_labels_[id]->set_visible((nr_owned + nr_build) > 0);

		building_buttons_[id]->set_enabled((nr_owned + nr_build) > 0);
		if (has_selection_ && id == current_building_type_) {
			label_nr_owned_.set_text(nr_owned > 0 ? std::to_string(nr_owned) : "");
			b_next_owned_.set_enabled(nr_owned > 0);
			b_prev_owned_.set_enabled(nr_owned > 0);
			hbox_owned_.set_visible(true);
			if (can_construct_this_building) {
				label_nr_construction_.set_text(nr_build > 0 ? std::to_string(nr_build) : "");
				b_next_construction_.set_enabled(nr_build > 0);
				b_prev_construction_.set_enabled(nr_build > 0);
				hbox_construction_.set_visible(true);
			}
		}
		building_buttons_[id]->set_tooltip(building.descname());
	}

	// only do traffic computations if tab is active, or one of the items is selected
	if (tab_assignments_[tab_panel_.active()] == BuildingTab::Traffic ||
	    (current_traffic_type_ != TrafficStat::kLast)) {

		for (uint8_t tt = TrafficStat::kFlag; tt < last_traffic_type_; ++tt) {
			traffic_stats_[tt].jump_targets.clear();
		}

		const bool map_allows_waterways = iplayer().game().map().get_waterway_max_length() >= 2;
		auto traffic_stat_counts = std::vector<std::pair<uint32_t, uint32_t>>(last_traffic_type_);
		std::fill(traffic_stat_counts.begin(), traffic_stat_counts.end(), std::make_pair(0u, 0u));

		for (const auto& eco : player.economies()) {
			if (eco.second->type() == Widelands::wwWARE) {
				for (Widelands::Flag* f : eco.second->flags()) {
					++traffic_stat_counts[TrafficStat::kFlag].first;
					if (f->current_wares() > 5) {
						++traffic_stat_counts[TrafficStat::kFlag].second;
						if (current_traffic_type_ == TrafficStat::kFlag) {
							traffic_stats_[TrafficStat::kFlag].jump_targets.insert(f->get_position());
						}
					}
					for (uint8_t road_id = Widelands::WalkingDir::LAST_DIRECTION;
					     road_id >= Widelands::WalkingDir::FIRST_DIRECTION; --road_id) {
						if (Widelands::Road* const r = f->get_road(road_id)) {
							if (&r->base_flag() == f) {
								++traffic_stat_counts[TrafficStat::kRoadNorm].first;
								if (r->get_carrier(0).get(iplayer().game()) == nullptr) {
									++traffic_stat_counts[TrafficStat::kRoadNorm].second;
									if (current_traffic_type_ == TrafficStat::kRoadNorm) {
										traffic_stats_[TrafficStat::kRoadNorm].jump_targets.insert(
										   r->get_positions(iplayer().game())[r->get_idle_index() - 1]);
									}
								}
								if (r->is_busy()) {
									++traffic_stat_counts[TrafficStat::kRoadBusy].first;
									if (r->get_carrier(1).get(iplayer().game()) == nullptr) {
										++traffic_stat_counts[TrafficStat::kRoadBusy].second;
										if (current_traffic_type_ == TrafficStat::kRoadBusy) {
											traffic_stats_[TrafficStat::kRoadBusy].jump_targets.insert(
											   r->get_positions(iplayer().game())[r->get_idle_index() - 1]);
										}
									}
								}
							}
						}
						if (Widelands::Waterway* const w = f->get_waterway(road_id)) {
							assert(map_allows_waterways);
							if (&w->base_flag() == f) {  // avoid double counting
								++traffic_stat_counts[TrafficStat::kWaterway].first;
								if (w->get_ferry().get(iplayer().game()) == nullptr) {
									++traffic_stat_counts[TrafficStat::kWaterway].second;
									if (current_traffic_type_ == TrafficStat::kWaterway) {
										traffic_stats_[TrafficStat::kWaterway].jump_targets.insert(
										   w->get_positions(iplayer().game())[w->get_idle_index() - 1]);
									}
								}
							}
						}
					}
				}
			}
		}
		for (uint8_t tt = TrafficStat::kFlag; tt < last_traffic_type_; ++tt) {
			traffic_stats_[tt].owned_label->set_visible(false);
			traffic_stats_[tt].productivity_label->set_visible(false);
			if (traffic_stat_counts[tt].first != 0u) {
				traffic_stats_[tt].button->set_enabled(true);
				set_labeltext(traffic_stats_[tt].owned_label,
				              std::to_string(traffic_stat_counts[tt].first),
				              style_.building_statistics_details_font().color());
			} else {
				traffic_stats_[tt].button->set_enabled(false);
			}
			if (traffic_stat_counts[tt].second != 0u) {
				set_labeltext(traffic_stats_[tt].productivity_label,
				              std::to_string(traffic_stat_counts[tt].second), style_.low_color());
			}
		}
		if ((current_traffic_type_ != TrafficStat::kLast)) {
			hbox_unproductive_.set_visible(true);
			bool e = traffic_stat_counts[current_traffic_type_].second > 0;
			label_nr_unproductive_.set_text(
			   e ? std::to_string(traffic_stat_counts[current_traffic_type_].second) : "");
			b_next_unproductive_.set_enabled(e);
			b_prev_unproductive_.set_enabled(e);
			switch (current_traffic_type_) {
			case TrafficStat::kFlag:
				label_unproductive_.set_text(_("Congested:"));
				break;
			case TrafficStat::kRoadNorm:
				FALLS_THROUGH;
			case TrafficStat::kRoadBusy:
				label_unproductive_.set_text(_("Lacking carrier:"));
				break;
			case TrafficStat::kWaterway:
				label_unproductive_.set_text(_("Lacking ferry:"));
				break;
			case TrafficStat::kLast:
				FALLS_THROUGH;
			default:
				NEVER_HERE();
			}
		}
	}
}

void BuildingStatisticsMenu::set_labeltext(UI::Textarea* textarea,
                                           const std::string& text,
                                           const RGBColor& color) {
	UI::FontStyleInfo style(style_.building_statistics_button_font());
	style.set_color(color);
	textarea->set_style_override(style);
	textarea->set_text(text);
	textarea->set_visible(true);
}

void BuildingStatisticsMenu::reset_button_states() {
	for (TrafficStatData& tsd : traffic_stats_) {
		tsd.button->set_visual_state(UI::Button::VisualState::kFlat);
	}
	for (UI::Button* building_button : building_buttons_) {
		if (building_button == nullptr) {
			continue;
		}
		building_button->set_visual_state(UI::Button::VisualState::kFlat);
	}
}

void BuildingStatisticsMenu::set_current_building_type(Widelands::DescriptionIndex id) {
	assert(building_buttons_[id] != nullptr);

	reset_button_states();

	// Update for current button
	current_building_type_ = id;
	building_buttons_[current_building_type_]->set_perm_pressed(true);
	label_name_.set_text(iplayer().player().tribe().get_building_descr(id)->descname());
	has_selection_ = true;
	update();
}

void BuildingStatisticsMenu::set_current_traffic_type(TrafficStat ts) {
	reset_button_states();
	current_building_type_ = Widelands::INVALID_INDEX;
	current_traffic_type_ = ts;
	traffic_stats_[current_traffic_type_].button->set_perm_pressed(true);
	label_name_.set_text(traffic_stats_[ts].name);
	has_selection_ = true;
	update();
}

void BuildingStatisticsMenu::low_production_changed() {
	low_production_ = unproductive_threshold_.get_value();
	update();
}

constexpr uint16_t kCurrentPacketVersion = 2;
UI::Window& BuildingStatisticsMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (1 <= packet_version && packet_version <= kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r =
			   dynamic_cast<InteractivePlayer&>(ib).menu_windows_.stats_buildings;
			r.create();
			assert(r.window);
			BuildingStatisticsMenu& m = dynamic_cast<BuildingStatisticsMenu&>(*r.window);
			m.unproductive_threshold_.set_value(fr.unsigned_8());
			m.low_production_changed();
			m.tab_panel_.activate(fr.unsigned_8());
			const std::string sel = fr.string();
			if (!sel.empty()) {
				Widelands::DescriptionIndex idx = ib.egbase().descriptions().safe_building_index(sel);
				/* Check if the button for the building still exists. There are valid cases where
				 * it might not, since some buildings are only selectable here under specific
				 * circumstances and vanish after closing and reopening the window.
				 */
				if (m.building_buttons_.at(idx) != nullptr) {
					m.set_current_building_type(idx);
				}
			}
			m.last_building_index_ = fr.signed_32();
			if (packet_version >= 2) {
				TrafficStat ts = static_cast<TrafficStat>(fr.unsigned_8());
				if (ts < m.last_traffic_type_) {
					m.current_traffic_type_ = ts;
				}
			}
			return m;
		}
		throw Widelands::UnhandledVersionError(
		   "Building Statistics Menu", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("building statistics menu: %s", e.what());
	}
}
void BuildingStatisticsMenu::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(low_production_);
	fw.unsigned_8(tab_panel_.active());
	fw.string(
	   current_building_type_ == Widelands::INVALID_INDEX ?
         "" :
         iplayer().egbase().descriptions().get_building_descr(current_building_type_)->name());
	fw.signed_32(last_building_index_);
	fw.unsigned_8(static_cast<uint8_t>(current_traffic_type_));
}
