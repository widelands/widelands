/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/player.h"

constexpr int kBuildGridCellHeight = 50;
constexpr int kBuildGridCellWidth = 55;
constexpr int kMargin = 5;
constexpr int kColumns = 5;
constexpr int kButtonHeight = 20;
constexpr int kButtonRowHeight = kButtonHeight + kMargin;
constexpr int kLabelHeight = 18;
constexpr int32_t kWindowWidth = kColumns * kBuildGridCellWidth;

constexpr Duration kUpdateTimeInGametimeMs = Duration(1000);  //  1 second, gametime

inline InteractivePlayer& BuildingStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

BuildingStatisticsMenu::BuildingStatisticsMenu(InteractivePlayer& parent,
                                               UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, "building_statistics", &registry, kWindowWidth, 100, _("Building Statistics")),
     style_(g_style_manager->building_statistics_style()),
     tab_panel_(this, UI::TabPanelStyle::kWuiDark),
     navigation_panel_(this, 0, 0, kWindowWidth, 4 * kButtonRowHeight),
     building_name_(
        &navigation_panel_, get_inner_w() / 2, 0, 0, kButtonHeight, "", UI::Align::kCenter),
     owned_label_(&navigation_panel_,
                  kMargin,
                  kButtonRowHeight,
                  0,
                  kButtonHeight,
                  _("Owned:"),
                  UI::Align::kLeft,
                  style_.building_statistics_details_font()),
     construction_label_(&navigation_panel_,
                         kMargin,
                         2 * kButtonRowHeight,
                         0,
                         kButtonHeight,
                         _("Under Construction:"),
                         UI::Align::kLeft,
                         style_.building_statistics_details_font()),
     unproductive_box_(&navigation_panel_, kMargin, 3 * kButtonRowHeight + 3, UI::Box::Horizontal),
     unproductive_label_(
        &unproductive_box_,
        /** TRANSLATORS: This is the first part of productivity with input field */
        /** TRANSLATORS: Building statistics window - 'Low Productivity <input>%:' */
        _("Low Productivity")),
     // We need consistent height here - test
     unproductive_percent_(&unproductive_box_, 0, 0, 35, UI::PanelStyle::kWui),
     unproductive_label2_(
        &unproductive_box_,
        /** TRANSLATORS: This is the second part of productivity with input field */
        /** TRANSLATORS: Building statistics window -  'Low Productivity <input>%:' */
        _("%:"),
        UI::Align::kLeft,
        style_.building_statistics_details_font()),
     no_owned_label_(&navigation_panel_,
                     get_inner_w() - 2 * kButtonRowHeight - kMargin,
                     kButtonRowHeight,
                     0,
                     kButtonHeight,
                     "",
                     UI::Align::kRight,
                     style_.building_statistics_details_font()),
     no_construction_label_(&navigation_panel_,
                            get_inner_w() - 2 * kButtonRowHeight - kMargin,
                            2 * kButtonRowHeight,
                            0,
                            kButtonHeight,
                            "",
                            UI::Align::kRight,
                            style_.building_statistics_details_font()),
     no_unproductive_label_(&navigation_panel_,
                            get_inner_w() - 2 * kButtonRowHeight - kMargin,
                            3 * kButtonRowHeight,
                            0,
                            kButtonHeight,
                            "",
                            UI::Align::kRight,
                            style_.building_statistics_details_font()),
     current_building_type_(Widelands::INVALID_INDEX),
     last_building_index_(0),
     last_building_type_(Widelands::INVALID_INDEX),
     lastupdate_(0),
     was_minimized_(false),
     low_production_(33),
     has_selection_(false),
     nr_building_types_(parent.egbase().tribes().nrbuildings()) {

	building_buttons_ = std::vector<UI::Button*>(nr_building_types_);
	owned_labels_ = std::vector<UI::Textarea*>(nr_building_types_);
	productivity_labels_ = std::vector<UI::Textarea*>(nr_building_types_);

	unproductive_percent_.set_font_style_and_margin(
	   style_.building_statistics_details_font(), style_.editbox_margin());

	unproductive_label_.set_size(unproductive_label_.get_w(), kButtonRowHeight);
	unproductive_percent_.set_text(std::to_string(low_production_));
	unproductive_percent_.set_max_length(4);
	unproductive_label2_.set_size(unproductive_label2_.get_w(), kButtonRowHeight);
	unproductive_box_.add(&unproductive_label_);
	unproductive_box_.add_space(2);
	unproductive_box_.add(&unproductive_percent_);
	unproductive_box_.add(&unproductive_label2_);
	unproductive_box_.set_size(
	   unproductive_label_.get_w() + unproductive_percent_.get_w() + unproductive_label2_.get_w(),
	   kButtonRowHeight);

	navigation_buttons_[NavigationButton::PrevOwned] = new UI::Button(
	   &navigation_panel_, "previous_owned", get_inner_w() - 2 * kButtonRowHeight, kButtonRowHeight,
	   kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiMenu,
	   g_image_cache->get("images/ui_basic/scrollbar_left.png"), _("Show previous building"));

	navigation_buttons_[NavigationButton::NextOwned] = new UI::Button(
	   &navigation_panel_, "next_owned", get_inner_w() - kButtonRowHeight, kButtonRowHeight,
	   kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiMenu,
	   g_image_cache->get("images/ui_basic/scrollbar_right.png"), _("Show next building"));

	navigation_buttons_[NavigationButton::PrevConstruction] = new UI::Button(
	   &navigation_panel_, "previous_constructed", get_inner_w() - 2 * kButtonRowHeight,
	   2 * kButtonRowHeight, kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiMenu,
	   g_image_cache->get("images/ui_basic/scrollbar_left.png"), _("Show previous building"));

	navigation_buttons_[NavigationButton::NextConstruction] = new UI::Button(
	   &navigation_panel_, "next_constructed", get_inner_w() - kButtonRowHeight,
	   2 * kButtonRowHeight, kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiMenu,
	   g_image_cache->get("images/ui_basic/scrollbar_right.png"), _("Show next building"));

	navigation_buttons_[NavigationButton::PrevUnproductive] = new UI::Button(
	   &navigation_panel_, "previous_unproductive", get_inner_w() - 2 * kButtonRowHeight,
	   3 * kButtonRowHeight, kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiMenu,
	   g_image_cache->get("images/ui_basic/scrollbar_left.png"), _("Show previous building"));

	navigation_buttons_[NavigationButton::NextUnproductive] = new UI::Button(
	   &navigation_panel_, "next_unproductive", get_inner_w() - kButtonRowHeight,
	   3 * kButtonRowHeight, kButtonHeight, kButtonHeight, UI::ButtonStyle::kWuiMenu,
	   g_image_cache->get("images/ui_basic/scrollbar_right.png"), _("Show next building"));

	navigation_buttons_[NavigationButton::PrevOwned]->sigclicked.connect(
	   [this]() { jump_building(JumpTarget::kOwned, true); });
	navigation_buttons_[NavigationButton::NextOwned]->sigclicked.connect(
	   [this]() { jump_building(JumpTarget::kOwned, false); });
	navigation_buttons_[NavigationButton::PrevConstruction]->sigclicked.connect(
	   [this]() { jump_building(JumpTarget::kConstruction, true); });
	navigation_buttons_[NavigationButton::NextConstruction]->sigclicked.connect(
	   [this]() { jump_building(JumpTarget::kConstruction, false); });
	navigation_buttons_[NavigationButton::PrevUnproductive]->sigclicked.connect(
	   [this]() { jump_building(JumpTarget::kUnproductive, true); });
	navigation_buttons_[NavigationButton::NextUnproductive]->sigclicked.connect(
	   [this]() { jump_building(JumpTarget::kUnproductive, false); });

	unproductive_percent_.changed.connect([this]() { low_production_changed(); });
	unproductive_percent_.ok.connect([this]() { low_production_reset_focus(); });
	unproductive_percent_.cancel.connect([this]() { low_production_reset_focus(); });

	init();
}

BuildingStatisticsMenu::~BuildingStatisticsMenu() {
	building_buttons_.clear();
	owned_labels_.clear();
	productivity_labels_.clear();
}

void BuildingStatisticsMenu::reset() {
	update();  // In case a building got removed, make sure to deselect it first

	const int last_selected_tab = tab_assignments_[tab_panel_.active()];

	tab_panel_.remove_last_tab("building_stats_ports");
	tab_panel_.remove_last_tab("building_stats_mines");
	tab_panel_.remove_last_tab("building_stats_big");
	tab_panel_.remove_last_tab("building_stats_medium");
	tab_panel_.remove_last_tab("building_stats_small");

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
	building_name_.set_text("");
	if (has_selection_) {
		if (building_buttons_[current_building_type_] != nullptr) {
			set_current_building_type(current_building_type_);
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
	for (int tab_index = 0; tab_index < kNoOfBuildingTabs; ++tab_index) {
		int current_column = 0;
		tabs_[tab_index] = new UI::Box(&tab_panel_, 0, 0, UI::Box::Vertical);
		UI::Box* row = new UI::Box(tabs_[tab_index], 0, 0, UI::Box::Horizontal);
		row_counters[tab_index] = 0;

		for (const Widelands::DescriptionIndex id : buildings_to_add[tab_index]) {
			const Widelands::BuildingDescr& descr =
			   *iplayer().egbase().tribes().get_building_descr(id);
			add_button(id, descr, row);
			++current_column;
			if (current_column == 1) {
				++row_counters[tab_index];
			} else if (current_column == kColumns) {
				tabs_[tab_index]->add(row, UI::Box::Resizing::kFullSize);
				tabs_[tab_index]->add_space(6);
				row = new UI::Box(tabs_[tab_index], 0, 0, UI::Box::Horizontal);
				current_column = 0;
			}
		}
		// Add final row
		if (current_column != 0) {
			tabs_[tab_index]->add(row, UI::Box::Resizing::kFullSize);
		}
	}

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
	if ((player.is_building_type_allowed(index) && (descr.is_buildable() || descr.is_enhanced())) ||
	    !player.get_building_statistics(index).empty()) {
		return true;
	}
	return false;
}

bool BuildingStatisticsMenu::foreign_tribe_building_is_valid(
   const Widelands::Player& player, Widelands::DescriptionIndex index) const {
	if (!player.tribe().has_building(index) && !player.get_building_statistics(index).empty()) {
		const Widelands::BuildingDescr& descr =
		   *iplayer().egbase().tribes().get_building_descr(index);
		if (descr.type() == Widelands::MapObjectType::CONSTRUCTIONSITE ||
		    descr.type() == Widelands::MapObjectType::DISMANTLESITE) {
			return false;
		}
		return true;
	}
	return false;
}

int BuildingStatisticsMenu::find_tab_for_building(const Widelands::BuildingDescr& descr) const {
	assert(descr.type() != Widelands::MapObjectType::CONSTRUCTIONSITE);
	assert(descr.type() != Widelands::MapObjectType::DISMANTLESITE);
	if (descr.get_ismine()) {
		return BuildingTab::Mines;
	} else if (descr.get_isport()) {
		return BuildingTab::Ports;
	} else {
		switch (descr.get_size()) {
		case Widelands::BaseImmovable::SMALL:
			return BuildingTab::Small;
		case Widelands::BaseImmovable::MEDIUM:
			return BuildingTab::Medium;
		case Widelands::BaseImmovable::BIG:
			return BuildingTab::Big;
		default:
			throw wexception(
			   "Building statictics: Found building without a size: %s", descr.name().c_str());
		}
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
	UI::Box* button_box = new UI::Box(row, 0, 0, UI::Box::Vertical);
	building_buttons_[id] =
	   new UI::Button(button_box, (boost::format("building_button%s") % id).str(), 0, 0,
	                  kBuildGridCellWidth, kBuildGridCellHeight, UI::ButtonStyle::kWuiBuildingStats,
	                  descr.representative_image(&iplayer().get_player()->get_playercolor()), "",
	                  UI::Button::VisualState::kFlat);
	building_buttons_[id]->set_disable_style(UI::ButtonDisableStyle::kMonochrome |
	                                         UI::ButtonDisableStyle::kFlat);
	button_box->add(building_buttons_[id]);

	owned_labels_[id] =
	   new UI::Textarea(button_box, 0, 0, kBuildGridCellWidth, kLabelHeight, "", UI::Align::kCenter,
	                    style_.building_statistics_button_font());
	owned_labels_[id]->set_fixed_width(kBuildGridCellWidth);
	button_box->add(owned_labels_[id]);

	productivity_labels_[id] =
	   new UI::Textarea(button_box, 0, 0, kBuildGridCellWidth, kLabelHeight, "", UI::Align::kCenter,
	                    style_.building_statistics_button_font());
	productivity_labels_[id]->set_fixed_width(kBuildGridCellWidth);
	button_box->add(productivity_labels_[id]);

	row->add(button_box);

	building_buttons_[id]->sigclicked.connect([this, id]() { set_current_building_type(id); });
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
						auto* soldier_control = militarysite->soldier_control();
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
						auto* soldier_control = militarysite->soldier_control();
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
				auto* soldier_control = militarysite->soldier_control();
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
	}

	if (found) {
		validate_pointer(&last_building_index_, stats_vector.size());
		iplayer().map_view()->scroll_to_field(
		   stats_vector[last_building_index_].pos, MapView::Transition::Smooth);
	}
	low_production_reset_focus();
	update();
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

	// Adjust height to current tab
	if (is_minimal()) {
		tab_panel_.set_size(0, 0);
	} else {
		const int tab_height =
		   35 +
		   row_counters_[tab_panel_.active()] * (kBuildGridCellHeight + kLabelHeight + kLabelHeight) +
		   kMargin;
		tab_panel_.set_size(kWindowWidth, tab_height);
		set_size(
		   get_w(), tab_height + kMargin + navigation_panel_.get_h() + get_tborder() + get_bborder());
		navigation_panel_.set_pos(Vector2i(0, tab_height + kMargin));
	}
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

	owned_label_.set_visible(false);
	no_owned_label_.set_visible(false);
	navigation_buttons_[NavigationButton::NextOwned]->set_visible(false);
	navigation_buttons_[NavigationButton::PrevOwned]->set_visible(false);
	construction_label_.set_visible(false);
	no_construction_label_.set_visible(false);
	navigation_buttons_[NavigationButton::NextConstruction]->set_visible(false);
	navigation_buttons_[NavigationButton::PrevConstruction]->set_visible(false);
	unproductive_box_.set_visible(false);
	unproductive_label_.set_visible(false);
	unproductive_percent_.set_visible(false);
	unproductive_label2_.set_visible(false);
	no_unproductive_label_.set_visible(false);
	navigation_buttons_[NavigationButton::NextUnproductive]->set_visible(false);
	navigation_buttons_[NavigationButton::PrevUnproductive]->set_visible(false);

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
			if (nr_owned) {
				int const percent =
				   static_cast<int>(static_cast<float>(total_prod) / static_cast<float>(nr_owned));

				const RGBColor& color =
				   (percent < low_production_) ?
				      style_.low_color() :
				      (percent < ((low_production_ < 50) ?
				                     2 * low_production_ :
				                     low_production_ + ((100 - low_production_) / 2))) ?
				      style_.medium_color() :
				      style_.high_color();

				/** TRANSLATORS: Percent in building statistics window, e.g. 85% */
				/** TRANSLATORS: If you wish to add a space, translate as '%i %%' */
				const std::string perc_str = (boost::format(_("%i%%")) % percent).str();
				set_labeltext(productivity_labels_[id], perc_str, color);
			}
			if (has_selection_ && id == current_building_type_) {
				no_unproductive_label_.set_text(nr_unproductive > 0 ? std::to_string(nr_unproductive) :
				                                                      "");
				navigation_buttons_[NavigationButton::NextUnproductive]->set_enabled(nr_unproductive >
				                                                                     0);
				navigation_buttons_[NavigationButton::PrevUnproductive]->set_enabled(nr_unproductive >
				                                                                     0);
				navigation_buttons_[NavigationButton::NextUnproductive]->set_visible(true);
				navigation_buttons_[NavigationButton::PrevUnproductive]->set_visible(true);
				unproductive_label_.set_text(_("Low Productivity"));
				unproductive_box_.set_visible(true);
				unproductive_label_.set_visible(true);
				unproductive_percent_.set_visible(true);
				unproductive_label2_.set_visible(true);
				no_unproductive_label_.set_visible(true);
			}
		} else if (building.type() == Widelands::MapObjectType::MILITARYSITE) {
			if (nr_owned) {
				const RGBColor& color = (total_stationed_soldiers < total_soldier_capacity / 2) ?
				                           style_.low_color() :
				                           (total_stationed_soldiers < total_soldier_capacity) ?
				                           style_.medium_color() :
				                           style_.high_color();
				const std::string perc_str =
				   (boost::format(_("%1%/%2%")) % total_stationed_soldiers % total_soldier_capacity)
				      .str();
				set_labeltext(productivity_labels_[id], perc_str, color);
			}
			if (has_selection_ && id == current_building_type_) {
				no_unproductive_label_.set_text(nr_unproductive > 0 ? std::to_string(nr_unproductive) :
				                                                      "");
				navigation_buttons_[NavigationButton::NextUnproductive]->set_enabled(
				   total_soldier_capacity > total_stationed_soldiers);
				navigation_buttons_[NavigationButton::PrevUnproductive]->set_enabled(
				   total_soldier_capacity > total_stationed_soldiers);
				navigation_buttons_[NavigationButton::NextUnproductive]->set_visible(true);
				navigation_buttons_[NavigationButton::PrevUnproductive]->set_visible(true);
				/** TRANSLATORS: Label for number of buildings that are waiting for soldiers */
				unproductive_label_.set_text(_("Lacking Soldiers:"));
				unproductive_box_.set_visible(true);
				unproductive_label_.set_visible(true);
				no_unproductive_label_.set_visible(true);
			}
		}

		std::string owned_text;
		const bool can_construct_this_building =
		   player.tribe().has_building(id) && (building.is_buildable() || building.is_enhanced());
		if (can_construct_this_building) {
			/** TRANSLATORS: Buildings: owned / under construction */
			owned_text = (boost::format(_("%1%/%2%")) % nr_owned % nr_build).str();
		} else {
			owned_text = (boost::format(_("%1%/%2%")) % nr_owned % "–").str();
		}
		set_labeltext(
		   owned_labels_[id], owned_text, style_.building_statistics_details_font().color());
		owned_labels_[id]->set_visible((nr_owned + nr_build) > 0);

		building_buttons_[id]->set_enabled((nr_owned + nr_build) > 0);
		if (has_selection_ && id == current_building_type_) {
			no_owned_label_.set_text(nr_owned > 0 ? std::to_string(nr_owned) : "");
			navigation_buttons_[NavigationButton::NextOwned]->set_enabled(nr_owned > 0);
			navigation_buttons_[NavigationButton::PrevOwned]->set_enabled(nr_owned > 0);
			owned_label_.set_visible(true);
			no_owned_label_.set_visible(true);
			navigation_buttons_[NavigationButton::NextOwned]->set_visible(true);
			navigation_buttons_[NavigationButton::PrevOwned]->set_visible(true);
			if (can_construct_this_building) {
				no_construction_label_.set_text(nr_build > 0 ? std::to_string(nr_build) : "");
				navigation_buttons_[NavigationButton::NextConstruction]->set_enabled(nr_build > 0);
				navigation_buttons_[NavigationButton::PrevConstruction]->set_enabled(nr_build > 0);
				construction_label_.set_visible(true);
				no_construction_label_.set_visible(true);
				navigation_buttons_[NavigationButton::NextConstruction]->set_visible(true);
				navigation_buttons_[NavigationButton::PrevConstruction]->set_visible(true);
			}
		}
		building_buttons_[id]->set_tooltip(building.descname());
	}
}

void BuildingStatisticsMenu::set_labeltext(UI::Textarea* textarea,
                                           const std::string& text,
                                           const RGBColor& color) {
	UI::FontStyleInfo style(style_.building_statistics_button_font());
	style.set_color(color);
	textarea->set_style(style);
	textarea->set_text(text);
	textarea->set_visible(true);
}

void BuildingStatisticsMenu::set_current_building_type(Widelands::DescriptionIndex id) {
	assert(building_buttons_[id] != nullptr);

	// Reset button states
	for (UI::Button* building_button : building_buttons_) {
		if (building_button == nullptr) {
			continue;
		}
		building_button->set_visual_state(UI::Button::VisualState::kFlat);
	}

	// Update for current button
	current_building_type_ = id;
	building_buttons_[current_building_type_]->set_perm_pressed(true);
	building_name_.set_text(iplayer().player().tribe().get_building_descr(id)->descname());
	low_production_reset_focus();
	has_selection_ = true;
	update();
}

void BuildingStatisticsMenu::low_production_changed() {
	const std::string cutoff = unproductive_percent_.text();
	int number = boost::lexical_cast<int>(cutoff.c_str());

	// Make sure that the user specified a correct number
	if (std::to_string(number) == cutoff && 0 <= number && number <= 100) {
		low_production_ = number;
		update();
	}
}

void BuildingStatisticsMenu::low_production_reset_focus() {
	unproductive_percent_.set_can_focus(false);
	unproductive_percent_.set_can_focus(true);
}
