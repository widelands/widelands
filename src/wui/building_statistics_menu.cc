/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include <cmath>

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/font_handler1.h"
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
constexpr int kLabelFontSize = 12;
constexpr int kTabHeight = 35 + 5 * (kBuildGridCellHeight + kLabelHeight + kLabelHeight);
constexpr int32_t kWindowWidth = kColumns * kBuildGridCellWidth;
constexpr int32_t kWindowHeight = kTabHeight + kMargin + 4 * kButtonRowHeight;

constexpr int32_t kUpdateTimeInGametimeMs = 1000;  //  1 second, gametime

using namespace Widelands;

namespace {
void set_label_font(UI::Textarea* label) {
	label->set_fontsize(kLabelFontSize);
}
}  // namespace

inline InteractivePlayer& BuildingStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

BuildingStatisticsMenu::BuildingStatisticsMenu(InteractivePlayer& parent,
                                               UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      "building_statistics",
                      &registry,
                      kWindowWidth,
                      kWindowHeight,
                      _("Building Statistics")),
     tab_panel_(this, 0, 0, g_gr->images().get("images/wui/button_secondary.png")),
     navigation_panel_(this, 0, 0, kWindowWidth, 4 * kButtonRowHeight),
     building_name_(
        &navigation_panel_, get_inner_w() / 2, 0, 0, kButtonHeight, "", UI::Align::kCenter),
     owned_label_(&navigation_panel_, kMargin, kButtonRowHeight, 0, kButtonHeight, _("Owned:")),
     construction_label_(&navigation_panel_,
                         kMargin,
                         2 * kButtonRowHeight,
                         0,
                         kButtonHeight,
                         _("Under Construction:")),
     unproductive_box_(&navigation_panel_, kMargin, 3 * kButtonRowHeight + 3, UI::Box::Horizontal),
     unproductive_label_(
        &unproductive_box_,
        /** TRANSLATORS: This is the first part of productivity with input field */
        /** TRANSLATORS: Building statistics window - 'Low Productivity <input>%:' */
        _("Low Productivity")),
     unproductive_percent_(
        &unproductive_box_,
        0,
        0,
        35,
        0,
        1,
        g_gr->images().get("images/wui/button_secondary.png"),
        kLabelFontSize - UI::g_fh1->fontset()->size_offset()),  // We need consistent height here
     unproductive_label2_(
        &unproductive_box_,
        /** TRANSLATORS: This is the second part of productivity with input field */
        /** TRANSLATORS: Building statistics window -  'Low Productivity <input>%:' */
        _("%:")),
     no_owned_label_(&navigation_panel_,
                     get_inner_w() - 2 * kButtonRowHeight - kMargin,
                     kButtonRowHeight,
                     0,
                     kButtonHeight,
                     "",
                     UI::Align::kRight),
     no_construction_label_(&navigation_panel_,
                            get_inner_w() - 2 * kButtonRowHeight - kMargin,
                            2 * kButtonRowHeight,
                            0,
                            kButtonHeight,
                            "",
                            UI::Align::kRight),
     no_unproductive_label_(&navigation_panel_,
                            get_inner_w() - 2 * kButtonRowHeight - kMargin,
                            3 * kButtonRowHeight,
                            0,
                            kButtonHeight,
                            "",
                            UI::Align::kRight),
     low_production_(33),
     has_selection_(false) {

	for (int i = 0; i < kNoOfBuildingTabs; ++i) {
		row_counters_[i] = 0;
		tabs_[i] = new UI::Box(&tab_panel_, 0, 0, UI::Box::Vertical);
	}

	tab_panel_.add("building_stats_small",
	               g_gr->images().get("images/wui/fieldaction/menu_tab_buildsmall.png"),
	               tabs_[BuildingTab::Small], _("Small Buildings"));
	tab_panel_.add("building_stats_medium",
	               g_gr->images().get("images/wui/fieldaction/menu_tab_buildmedium.png"),
	               tabs_[BuildingTab::Medium], _("Medium Buildings"));
	tab_panel_.add("building_stats_big",
	               g_gr->images().get("images/wui/fieldaction/menu_tab_buildbig.png"),
	               tabs_[BuildingTab::Big], _("Big Buildings"));
	tab_panel_.add("building_stats_mines",
	               g_gr->images().get("images/wui/fieldaction/menu_tab_buildmine.png"),
	               tabs_[BuildingTab::Mines], _("Mines"));

	// Hide the ports tab for non-seafaring maps
	if (iplayer().game().map().get_port_spaces().size() > 1) {
		tab_panel_.add("building_stats_ports",
		               g_gr->images().get("images/wui/fieldaction/menu_tab_buildport.png"),
		               tabs_[BuildingTab::Ports], _("Ports"));
	}

	const DescriptionIndex nr_buildings = parent.egbase().tribes().nrbuildings();
	building_buttons_ = std::vector<UI::Button*>(nr_buildings);
	owned_labels_ = std::vector<UI::Textarea*>(nr_buildings);
	productivity_labels_ = std::vector<UI::Textarea*>(nr_buildings);

	// Column counters
	int columns[kNoOfBuildingTabs] = {0, 0, 0, 0, 0};

	// Row containers
	UI::Box* rows[kNoOfBuildingTabs];
	for (int i = 0; i < kNoOfBuildingTabs; ++i) {
		rows[i] = new UI::Box(tabs_[i], 0, 0, UI::Box::Horizontal);
	}

	// We want to add player tribe's buildings in correct order
	const TribeDescr& tribe = iplayer().player().tribe();
	std::vector<DescriptionIndex> buildings_to_add;
	for (DescriptionIndex index : tribe.buildings()) {
		// Only add headquarter types that are owned by player.
		const BuildingDescr& descr = *tribe.get_building_descr(index);
		const Widelands::Player& player = iplayer().player();
		if (descr.is_buildable() || descr.is_enhanced() ||
		    !player.get_building_statistics(index).empty()) {
			buildings_to_add.push_back(index);
		}
	}

	// We want to add other tribes' militarysites on the bottom
	for (DescriptionIndex index = 0; index < nr_buildings; ++index) {
		const BuildingDescr& descr = *parent.egbase().tribes().get_building_descr(index);
		if (descr.type() == MapObjectType::MILITARYSITE && !tribe.has_building(index)) {
			buildings_to_add.push_back(index);
		}
	}

	for (DescriptionIndex id : buildings_to_add) {
		const BuildingDescr& descr = *tribe.get_building_descr(id);

		if (descr.type() != MapObjectType::CONSTRUCTIONSITE &&
		    descr.type() != MapObjectType::DISMANTLESITE) {
			if (descr.get_ismine()) {
				if (add_button(id, descr, BuildingTab::Mines, *rows[BuildingTab::Mines],
				               &columns[BuildingTab::Mines])) {
					rows[BuildingTab::Mines] =
					   new UI::Box(tabs_[BuildingTab::Mines], 0, 0, UI::Box::Horizontal);
				}
			} else if (descr.get_isport()) {
				if (add_button(id, descr, BuildingTab::Ports, *rows[BuildingTab::Ports],
				               &columns[BuildingTab::Ports])) {
					rows[BuildingTab::Ports] =
					   new UI::Box(tabs_[BuildingTab::Ports], 0, 0, UI::Box::Horizontal);
				}
			} else {
				switch (descr.get_size()) {
				case BaseImmovable::SMALL:
					if (add_button(id, descr, BuildingTab::Small, *rows[BuildingTab::Small],
					               &columns[BuildingTab::Small])) {
						rows[BuildingTab::Small] =
						   new UI::Box(tabs_[BuildingTab::Small], 0, 0, UI::Box::Horizontal);
					}
					break;
				case BaseImmovable::MEDIUM:
					if (add_button(id, descr, BuildingTab::Medium, *rows[BuildingTab::Medium],
					               &columns[BuildingTab::Medium])) {
						rows[BuildingTab::Medium] =
						   new UI::Box(tabs_[BuildingTab::Medium], 0, 0, UI::Box::Horizontal);
					}
					break;
				case BaseImmovable::BIG:
					if (add_button(id, descr, BuildingTab::Big, *rows[BuildingTab::Big],
					               &columns[BuildingTab::Big])) {
						rows[BuildingTab::Big] =
						   new UI::Box(tabs_[BuildingTab::Big], 0, 0, UI::Box::Horizontal);
					}
					break;
				default:
					throw wexception(
					   "Building statictics: Found building without a size: %s", descr.name().c_str());
				}
			}
		}
	}

	for (int i = 0; i < kNoOfBuildingTabs; ++i) {
		tabs_[i]->add(rows[i]);
	}

	set_label_font(&owned_label_);
	set_label_font(&construction_label_);
	set_label_font(&unproductive_label_);
	set_label_font(&unproductive_label2_);
	set_label_font(&no_owned_label_);
	set_label_font(&no_construction_label_);
	set_label_font(&no_unproductive_label_);

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
	   kButtonHeight, kButtonHeight, g_gr->images().get("images/wui/window_background.png"),
	   g_gr->images().get("images/ui_basic/scrollbar_left.png"), _("Show previous building"));

	navigation_buttons_[NavigationButton::NextOwned] = new UI::Button(
	   &navigation_panel_, "next_owned", get_inner_w() - kButtonRowHeight, kButtonRowHeight,
	   kButtonHeight, kButtonHeight, g_gr->images().get("images/wui/window_background.png"),
	   g_gr->images().get("images/ui_basic/scrollbar_right.png"), _("Show next building"));

	navigation_buttons_[NavigationButton::PrevConstruction] = new UI::Button(
	   &navigation_panel_, "previous_constructed", get_inner_w() - 2 * kButtonRowHeight,
	   2 * kButtonRowHeight, kButtonHeight, kButtonHeight,
	   g_gr->images().get("images/wui/window_background.png"),
	   g_gr->images().get("images/ui_basic/scrollbar_left.png"), _("Show previous building"));

	navigation_buttons_[NavigationButton::NextConstruction] = new UI::Button(
	   &navigation_panel_, "next_constructed", get_inner_w() - kButtonRowHeight,
	   2 * kButtonRowHeight, kButtonHeight, kButtonHeight,
	   g_gr->images().get("images/wui/window_background.png"),
	   g_gr->images().get("images/ui_basic/scrollbar_right.png"), _("Show next building"));

	navigation_buttons_[NavigationButton::PrevUnproductive] = new UI::Button(
	   &navigation_panel_, "previous_unproductive", get_inner_w() - 2 * kButtonRowHeight,
	   3 * kButtonRowHeight, kButtonHeight, kButtonHeight,
	   g_gr->images().get("images/wui/window_background.png"),
	   g_gr->images().get("images/ui_basic/scrollbar_left.png"), _("Show previous building"));

	navigation_buttons_[NavigationButton::NextUnproductive] = new UI::Button(
	   &navigation_panel_, "next_unproductive", get_inner_w() - kButtonRowHeight,
	   3 * kButtonRowHeight, kButtonHeight, kButtonHeight,
	   g_gr->images().get("images/wui/window_background.png"),
	   g_gr->images().get("images/ui_basic/scrollbar_right.png"), _("Show next building"));

	navigation_buttons_[NavigationButton::PrevOwned]->sigclicked.connect(boost::bind(
	   &BuildingStatisticsMenu::jump_building, boost::ref(*this), JumpTarget::kOwned, true));
	navigation_buttons_[NavigationButton::NextOwned]->sigclicked.connect(boost::bind(
	   &BuildingStatisticsMenu::jump_building, boost::ref(*this), JumpTarget::kOwned, false));
	navigation_buttons_[NavigationButton::PrevConstruction]->sigclicked.connect(boost::bind(
	   &BuildingStatisticsMenu::jump_building, boost::ref(*this), JumpTarget::kConstruction, true));
	navigation_buttons_[NavigationButton::NextConstruction]->sigclicked.connect(boost::bind(
	   &BuildingStatisticsMenu::jump_building, boost::ref(*this), JumpTarget::kConstruction, false));
	navigation_buttons_[NavigationButton::PrevUnproductive]->sigclicked.connect(boost::bind(
	   &BuildingStatisticsMenu::jump_building, boost::ref(*this), JumpTarget::kUnproductive, true));
	navigation_buttons_[NavigationButton::NextUnproductive]->sigclicked.connect(boost::bind(
	   &BuildingStatisticsMenu::jump_building, boost::ref(*this), JumpTarget::kUnproductive, false));

	unproductive_percent_.changed.connect(
	   boost::bind(&BuildingStatisticsMenu::low_production_changed, boost::ref(*this)));
	unproductive_percent_.ok.connect(
	   boost::bind(&BuildingStatisticsMenu::low_production_reset_focus, boost::ref(*this)));
	unproductive_percent_.cancel.connect(
	   boost::bind(&BuildingStatisticsMenu::low_production_reset_focus, boost::ref(*this)));

	update();
}

BuildingStatisticsMenu::~BuildingStatisticsMenu() {
	building_buttons_.clear();
	owned_labels_.clear();
	productivity_labels_.clear();
}

/**
 * Adds 3 buttons per building type.
 *
 * - Building image, steps through all buildings of the type
 * - Buildings owned, steps through constructionsites
 * - Productivity, steps though buildings with low productivity and stopped buildings
 */
bool BuildingStatisticsMenu::add_button(
   DescriptionIndex id, const BuildingDescr& descr, int tab_index, UI::Box& row, int* column) {

	UI::Box* button_box = new UI::Box(&row, 0, 0, UI::Box::Vertical);
	building_buttons_[id] = new UI::Button(
	   button_box, (boost::format("building_button%s") % id).str(), 0, 0, kBuildGridCellWidth,
	   kBuildGridCellHeight, g_gr->images().get("images/wui/button_secondary.png"),
	   descr.representative_image(&iplayer().get_player()->get_playercolor()), "",
	   UI::Button::Style::kFlat);
	building_buttons_[id]->set_disable_style(UI::ButtonDisableStyle::kMonochrome |
	                                         UI::ButtonDisableStyle::kFlat);
	button_box->add(building_buttons_[id]);

	owned_labels_[id] =
	   new UI::Textarea(button_box, 0, 0, kBuildGridCellWidth, kLabelHeight, UI::Align::kCenter);
	owned_labels_[id]->set_fontsize(kLabelFontSize);
	owned_labels_[id]->set_fixed_width(kBuildGridCellWidth);
	button_box->add(owned_labels_[id]);

	productivity_labels_[id] =
	   new UI::Textarea(button_box, 0, 0, kBuildGridCellWidth, kLabelHeight, UI::Align::kCenter);
	productivity_labels_[id]->set_fontsize(kLabelFontSize);
	productivity_labels_[id]->set_fixed_width(kBuildGridCellWidth);
	button_box->add(productivity_labels_[id]);

	row.add(button_box);

	building_buttons_[id]->sigclicked.connect(
	   boost::bind(&BuildingStatisticsMenu::set_current_building_type, boost::ref(*this), id));

	// For dynamic window height
	if (*column == 0) {
		++row_counters_[tab_index];
	}

	// Check if the row is full
	++*column;
	if (*column == kColumns) {
		tabs_[tab_index]->add(&row);
		tabs_[tab_index]->add_space(6);
		*column = 0;
		return true;
	}
	return false;
}

void BuildingStatisticsMenu::jump_building(JumpTarget target, bool reverse) {
	bool found = true;
	if (last_building_type_ != current_building_type_) {
		last_building_index_ = 0;
	}
	last_building_type_ = current_building_type_;

	const std::vector<Player::BuildingStats>& stats_vector =
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
		const Map& map = iplayer().egbase().map();
		int32_t const curindex = last_building_index_;
		found = false;
		if (reverse) {
			while (validate_pointer(&(--last_building_index_), stats_vector.size()) != curindex) {
				if (!stats_vector[last_building_index_].is_constructionsite) {
					if (upcast(MilitarySite, militarysite,
					           map[stats_vector[last_building_index_].pos].get_immovable())) {
						if (militarysite->stationed_soldiers().size() <
						    militarysite->soldier_capacity()) {
							found = true;
							break;
						}
					} else if (upcast(ProductionSite, productionsite,
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
					if (upcast(MilitarySite, militarysite,
					           map[stats_vector[last_building_index_].pos].get_immovable())) {
						if (militarysite->stationed_soldiers().size() <
						    militarysite->soldier_capacity()) {
							found = true;
							break;
						}
					} else if (upcast(ProductionSite, productionsite,
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
			if (upcast(MilitarySite, militarysite,
			           map[stats_vector[last_building_index_].pos].get_immovable())) {
				if (militarysite->stationed_soldiers().size() < militarysite->soldier_capacity()) {
					found = true;
				}
			} else if (upcast(ProductionSite, productionsite,
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
		iplayer().scroll_to_field(
		   stats_vector[last_building_index_].pos, MapView::Transition::Smooth);
	}
	low_production_reset_focus();
	update();
}

/*
 * Update this statistic
 */
void BuildingStatisticsMenu::think() {
	// Adjust height to current tab
	if (is_minimal()) {
		tab_panel_.set_size(0, 0);
	} else {
		int tab_height =
		   35 +
		   row_counters_[tab_panel_.active()] * (kBuildGridCellHeight + kLabelHeight + kLabelHeight);
		tab_panel_.set_size(kWindowWidth, tab_height);
		set_size(
		   get_w(), tab_height + kMargin + 4 * kButtonRowHeight + get_tborder() + get_bborder());
		navigation_panel_.set_pos(Vector2i(0, tab_height + kMargin));
	}

	// Update statistics
	const int32_t gametime = iplayer().game().get_gametime();

	if (was_minimized_ || (gametime - lastupdate_) > kUpdateTimeInGametimeMs) {
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
	const Player& player = iplayer().player();
	const TribeDescr& tribe = player.tribe();
	const Map& map = iplayer().game().map();
	const DescriptionIndex nr_buildings = iplayer().egbase().tribes().nrbuildings();

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

	for (DescriptionIndex id = 0; id < nr_buildings; ++id) {
		const BuildingDescr& building = *tribe.get_building_descr(id);
		if (building_buttons_[id] == nullptr) {
			continue;
		}
		assert(productivity_labels_[id] != nullptr);
		assert(owned_labels_[id] != nullptr);

		const std::vector<Player::BuildingStats>& stats_vector = player.get_building_statistics(id);

		uint32_t nr_owned = 0;
		uint32_t nr_build = 0;
		uint32_t total_prod = 0;
		uint32_t total_soldier_capacity = 0;
		uint32_t total_stationed_soldiers = 0;
		uint32_t nr_unproductive = 0;

		for (uint32_t l = 0; l < stats_vector.size(); ++l) {
			if (stats_vector[l].is_constructionsite)
				++nr_build;
			else {
				++nr_owned;
				BaseImmovable& immovable = *map[stats_vector[l].pos].get_immovable();
				if (building.type() == MapObjectType::PRODUCTIONSITE ||
				    building.type() == MapObjectType::TRAININGSITE) {
					ProductionSite& productionsite = dynamic_cast<ProductionSite&>(immovable);
					int percent = productionsite.get_statistics_percent();
					total_prod += percent;

					if (percent < low_production_ || productionsite.is_stopped()) {
						++nr_unproductive;
					}
				} else if (building.type() == MapObjectType::MILITARYSITE) {
					MilitarySite& militarysite = dynamic_cast<MilitarySite&>(immovable);
					total_soldier_capacity += militarysite.soldier_capacity();
					total_stationed_soldiers += militarysite.stationed_soldiers().size();
					if (total_stationed_soldiers < total_soldier_capacity) {
						++nr_unproductive;
					}
				}
			}
		}

		productivity_labels_[id]->set_text(" ");
		productivity_labels_[id]->set_visible(false);

		if (building.type() == MapObjectType::PRODUCTIONSITE ||
		    building.type() == MapObjectType::TRAININGSITE) {
			if (nr_owned) {
				int const percent =
				   static_cast<int>(static_cast<float>(total_prod) / static_cast<float>(nr_owned));

				RGBColor color;
				if (percent < low_production_) {
					color = UI_FONT_CLR_BAD;
				} else if (percent < ((low_production_ < 50) ?
				                         2 * low_production_ :
				                         low_production_ + ((100 - low_production_) / 2))) {
					color = UI_FONT_CLR_OK;
				} else {
					color = UI_FONT_CLR_GOOD;
				}
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
		} else if (building.type() == MapObjectType::MILITARYSITE) {
			if (nr_owned) {
				RGBColor color;
				if (total_stationed_soldiers < total_soldier_capacity / 2) {
					color = UI_FONT_CLR_BAD;
				} else if (total_stationed_soldiers < total_soldier_capacity) {
					color = UI_FONT_CLR_OK;
				} else {
					color = UI_FONT_CLR_GOOD;
				}
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
				/** TRANSLATORS Label for number of buildings that are waiting for soldiers */
				unproductive_label_.set_text(_("Lacking Soldiers:"));
				unproductive_box_.set_visible(true);
				unproductive_label_.set_visible(true);
				no_unproductive_label_.set_visible(true);
			}
		}

		std::string owned_text = "";
		if (player.tribe().has_building(id) && (building.is_buildable() || building.is_enhanced())) {
			/** TRANSLATORS Buildings: owned / under construction */
			owned_text = (boost::format(_("%1%/%2%")) % nr_owned % nr_build).str();
		} else {
			owned_text = (boost::format(_("%1%/%2%")) % nr_owned % "–").str();
		}
		set_labeltext(owned_labels_[id], owned_text, UI_FONT_CLR_FG);
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
			if (player.tribe().has_building(id) && building.is_buildable()) {
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
	textarea->set_color(color);
	textarea->set_text(text);
	textarea->set_visible(true);
}

void BuildingStatisticsMenu::set_current_building_type(DescriptionIndex id) {
	assert(building_buttons_[id] != nullptr);

	// Reset button states
	for (UI::Button* building_button : building_buttons_) {
		if (building_button == nullptr) {
			continue;
		}
		building_button->set_style(UI::Button::Style::kFlat);
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
	int number = std::atoi(cutoff.c_str());

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
