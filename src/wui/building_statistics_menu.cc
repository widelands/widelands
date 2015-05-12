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

#include "wui/building_statistics_menu.h"

#include <vector>

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h" // NOCOM
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/tribe.h"
#include "wui/interactive_player.h"

constexpr int kTabHeight = 35;
constexpr int kBuildGridCellSize = 50;
constexpr int kColumns = 5;
constexpr int32_t kWindowWidth = kColumns * kBuildGridCellSize;
constexpr int32_t kWindowHeight =  485;

constexpr uint8_t kLowProduction = 33;
constexpr int32_t kUpdateTime = 1000;  //  1 second, gametime

namespace {
// Formats a main text + help text for a tooltip and adds a hint for the shift button.
const std::string format_tooltip(const std::string& title, const std::string& helptext) {
	const std::string color = (boost::format("%02x%02x%02x")
								% int(UI_FONT_TOOLTIP_CLR.r)
								% int(UI_FONT_TOOLTIP_CLR.g)
								% int(UI_FONT_TOOLTIP_CLR.b)).str();

	const std::string tooltip_format("<rt><p><font face=serif size=%i bold=1 color=%s>%s"
										"<br><font size=%i>%s<br>%s</font></font></p></rt>");

	return (boost::format(tooltip_format)
			  % UI_FONT_SIZE_SMALL
			  % color
			  % title
			  % 11
			  % helptext
			  % _("Hold down SHIFT to step through the buildings from back to front.")).str();
}

} // namespace

inline InteractivePlayer& BuildingStatisticsMenu::iplayer() const {
	return dynamic_cast<InteractivePlayer&>(*get_parent());
}

BuildingStatisticsMenu::BuildingStatisticsMenu
	(InteractivePlayer & parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&parent, "building_statistics",
		 &registry,
		 kWindowWidth, kWindowHeight,
		 _("Building Statistics")),
	is_shift_pressed_(false),
	tabs_(this, 0, 0, g_gr->images().get("pics/but1.png")),
	small_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	medium_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	big_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	mines_tab_(&tabs_, 0, 0, UI::Box::Vertical),
	ports_tab_(&tabs_, 0, 0, UI::Box::Vertical) {
	tabs_.add("building_stats_small", g_gr->images().get("pics/menu_tab_buildsmall.png"),
				 &small_tab_, _("Small Buildings"));
	tabs_.add("building_stats_medium", g_gr->images().get("pics/menu_tab_buildmedium.png"),
				 &medium_tab_, _("Medium Buildings"));
	tabs_.add("building_stats_big", g_gr->images().get("pics/menu_tab_buildbig.png"),
				 &big_tab_, _("Big Buildings"));
	tabs_.add("building_stats_mines", g_gr->images().get("pics/menu_tab_buildmine.png"),
				 &mines_tab_, _("Mines"));
	tabs_.add("building_stats_ports", g_gr->images().get("pics/menu_tab_buildport.png"),
				 &ports_tab_, _("Ports"));
	tabs_.set_size(kWindowWidth, kWindowHeight);

	const TribeDescr& tribe = iplayer().player().tribe();

	const BuildingIndex nr_buildings = tribe.get_nrbuildings();
	building_buttons_ = std::vector<UI::Button*>(nr_buildings);
	owned_buttons_ = std::vector<UI::Button*>(nr_buildings);
	productivity_buttons_ = std::vector<UI::Button*>(nr_buildings);

	int small_column = 0;
	int medium_column = 0;
	int big_column = 0;
	int mines_column = 0;
	int ports_column = 0;

	UI::Box* mines_row = new UI::Box(&mines_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* ports_row = new UI::Box(&ports_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* big_row = new UI::Box(&big_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* medium_row = new UI::Box(&medium_tab_, 0, 0, UI::Box::Horizontal);
	UI::Box* small_row = new UI::Box(&small_tab_, 0, 0, UI::Box::Horizontal);

	for(BuildingIndex id = 0; id < nr_buildings; ++id) {
		const BuildingDescr& descr = *tribe.get_building_descr(id);

		if (descr.type() != MapObjectType::CONSTRUCTIONSITE &&
			 descr.type() != MapObjectType::DISMANTLESITE) {
			if (descr.get_ismine()) {
				add_button(id, descr, *mines_row);
				++mines_column;
				if (mines_column == kColumns) {
					mines_tab_.add(mines_row, UI::Align_Left);
					mines_column = 0;
					mines_row = new UI::Box(&mines_tab_, 0, 0, UI::Box::Horizontal);
				}
			} else if (descr.get_isport()) {
				add_button(id, descr, *ports_row);
				++ports_column;
				if (ports_column == kColumns) {
					ports_tab_.add(ports_row, UI::Align_Left);
					ports_column = 0;
					ports_row = new UI::Box(&ports_tab_, 0, 0, UI::Box::Horizontal);
				}
			} else {
				switch (descr.get_size()) {
					case BaseImmovable::SMALL:
						add_button(id, descr, *small_row);
						++small_column;
						if (small_column == kColumns) {
							small_tab_.add(small_row, UI::Align_Left);
							small_column = 0;
							small_row = new UI::Box(&small_tab_, 0, 0, UI::Box::Horizontal);
						}
						break;
					case BaseImmovable::MEDIUM:
						add_button(id, descr, *medium_row);
						++medium_column;
						if (medium_column == kColumns) {
							medium_tab_.add(medium_row, UI::Align_Left);
							medium_column = 0;
							medium_row = new UI::Box(&medium_tab_, 0, 0, UI::Box::Horizontal);
						}
						break;
					case BaseImmovable::BIG:
						add_button(id, descr, *big_row);
						++big_column;
						if (big_column == kColumns) {
							big_tab_.add(big_row, UI::Align_Left);
							big_column = 0;
							big_row = new UI::Box(&big_tab_, 0, 0, UI::Box::Horizontal);
						}
						break;
					default:
						throw wexception("Building statictics: Found building without a size: %s", descr.name().c_str());
				}
			}
		}
	}
	mines_tab_.add(mines_row, UI::Align_Left);
	ports_tab_.add(ports_row, UI::Align_Left);
	small_tab_.add(small_row, UI::Align_Left);
	medium_tab_.add(medium_row, UI::Align_Left);
	big_tab_.add(big_row, UI::Align_Left);
}

// Adds 3 buttons per building type:
// - Building image, steps through all buildings of the type
// - Buildings owned, steps through constructionsites
// - Productivity, steps though buildings with low productivity
void BuildingStatisticsMenu::add_button(BuildingIndex id, const BuildingDescr& descr, UI::Box& tab) {
	UI::Box* button_box = new UI::Box(&tab, 0, 0, UI::Box::Vertical);
	building_buttons_[id] =
			new UI::Button(button_box, (boost::format("building_button%s") % id).str(), 0, 0,
								kBuildGridCellSize, kBuildGridCellSize,
								g_gr->images().get("pics/but1.png"),
								&g_gr->animations()
								.get_animation(descr.get_animation("idle"))
								.representative_image_from_disk(),
								descr.descname(), false, true);
	button_box->add(building_buttons_[id], UI::Align_Left);

	std::string buttonlabel;
	if (!descr.global() && (descr.is_buildable() || descr.is_enhanced())) {
		/** TRANSLATORS Buildings: owned / under construction */
		buttonlabel = (boost::format(_("%1% / %2%")) % 0 % 0).str();
	} else {
		buttonlabel = (boost::format(_("%1% / %2%")) % 0 % "–").str();
	}
	owned_buttons_[id] =
			new UI::Button(button_box, (boost::format("owned_button%s") % id).str(), 0, 0,
								kBuildGridCellSize, 20,
								g_gr->images().get("pics/but1.png"),
								buttonlabel,
								_("Owned / Under Construction"), false, true);
	button_box->add(owned_buttons_[id], UI::Align_Left);

	std::string productivity_tooltip;
	if (descr.type() == MapObjectType::PRODUCTIONSITE &&
		 descr.type() != MapObjectType::MILITARYSITE &&
		 descr.type() != MapObjectType::WAREHOUSE) {
		buttonlabel =  "–";
		productivity_tooltip = _("Productivity");
	} else {
		buttonlabel = " ";
		productivity_tooltip = "";
	}
	productivity_buttons_[id] =
			new UI::Button(button_box, (boost::format("prod_button%s") % id).str(), 0, 0,
								kBuildGridCellSize, 20,
								g_gr->images().get("pics/but1.png"),
								buttonlabel,
								productivity_tooltip, false, true);
	button_box->add(productivity_buttons_[id], UI::Align_Left);

	tab.add(button_box, UI::Align_Left);

	building_buttons_[id]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::jump_building, boost::ref(*this), id, JumpTarget::Owned));
	owned_buttons_[id]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::jump_building, boost::ref(*this), id, JumpTarget::Construction));
	productivity_buttons_[id]->sigclicked.connect
		(boost::bind(&BuildingStatisticsMenu::jump_building, boost::ref(*this), id, JumpTarget::Unproductive));
}

bool BuildingStatisticsMenu::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		// only on down events
		switch (code.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				is_shift_pressed_ = true;
				break;
			default:
				break;
		}
	} else {
		switch (code.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				is_shift_pressed_ = false;
				break;
			default:
				break;
		}
	}
	return false;
}

void BuildingStatisticsMenu::jump_building(BuildingIndex id, JumpTarget target) {
	if (last_building_type_ != id) {
		last_building_index_ = 0;
	}
	last_building_type_ = id;

	const std::vector<Player::BuildingStats>& stats_vector =
		iplayer().get_player()->get_building_statistics(id);

	switch (target) {
		case JumpTarget::Owned: {
			if (is_shift_pressed_) {
				--last_building_index_;
			} else {
				++last_building_index_;
			}
			break;
		}
		case JumpTarget::Construction: {
			int32_t const curindex = last_building_index_;

			if (is_shift_pressed_) {
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
		case JumpTarget::Unproductive: {
			const Map & map = iplayer().egbase().map();

			int32_t const curindex = last_building_index_;
			bool found = false;

			if (is_shift_pressed_) {
				while (validate_pointer(&(--last_building_index_), stats_vector.size()) != curindex) {
					if (!stats_vector[last_building_index_].is_constructionsite) {
						if
							(upcast
								(ProductionSite,
								 productionsite,
								 map[stats_vector[last_building_index_].pos].get_immovable())) {
							if (productionsite->is_stopped() || productionsite->get_statistics_percent() < kLowProduction) {
								found = true;
								break;
							}
						}
					}
				}
			} else {
				while (validate_pointer(&(++last_building_index_), stats_vector.size()) != curindex) {
					if (!stats_vector[last_building_index_].is_constructionsite) {
						if
							(upcast
								(ProductionSite,
								 productionsite,
								 map[stats_vector[last_building_index_].pos].get_immovable())) {
							if (productionsite->is_stopped() || productionsite->get_statistics_percent() < kLowProduction) {
								found = true;
								break;
							}
						}
					}
				}
			}
			if (!found) { // Now look at the old
				if
					(upcast
						(ProductionSite,
						 productionsite,
						 map[stats_vector[last_building_index_].pos].get_immovable())) {
					if (productionsite->is_stopped() ||productionsite->get_statistics_percent() < kLowProduction) {
						found = true;
					}
				}
			}
			break;
		}
		default:
			assert(false);
			break;
	}

	validate_pointer(&last_building_index_, stats_vector.size());
	iplayer().move_view_to(stats_vector[last_building_index_].pos);
}


/*
 * Update this statistic
 */
void BuildingStatisticsMenu::think() {
	const Game& game = iplayer().game();
	int32_t const gametime = game.get_gametime();

	if ((gametime - lastupdate_) > kUpdateTime) {
		update();
		lastupdate_ = gametime;
	}
}

/*
 * validate if this pointer is ok
 */
int32_t BuildingStatisticsMenu::validate_pointer
	(int32_t * const id, int32_t const size)
{
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
	const TribeDescr& tribe  = player.tribe();
	const Map& map = iplayer().game().map();
	const BuildingIndex nr_buildings = tribe.get_nrbuildings();
	std::string button_tooltip;

	for(BuildingIndex i = 0; i < nr_buildings; ++i) {
		const BuildingDescr& building = *tribe.get_building_descr(i);
		if(!(building.is_buildable() || building.is_enhanced() || building.global())) {
			continue;
		}

		const std::vector<Player::BuildingStats>& stats_vector = player.get_building_statistics(i);

		uint32_t nr_owned   = 0;
		uint32_t nr_build   = 0;
		uint32_t total_prod = 0;
		upcast(ProductionSiteDescr const, productionsite, &building);
		for (uint32_t l = 0; l < stats_vector.size(); ++l) {
			if (stats_vector[l].is_constructionsite)
				++nr_build;
			else {
				++nr_owned;
				if (productionsite)
					total_prod +=
						dynamic_cast<ProductionSite&>
							(*map[stats_vector[l].pos].get_immovable())
						.get_statistics_percent();
			}
		}

		if (building.type() == MapObjectType::PRODUCTIONSITE &&
			 building.type() != MapObjectType::MILITARYSITE &&
			 building.type() != MapObjectType::WAREHOUSE) {
			if (nr_owned) {
				int const percent =
					static_cast<int>
						(static_cast<float>(total_prod) / static_cast<float>(nr_owned));
				productivity_buttons_[i]->set_title((boost::format("%i%%") % percent).str());
				productivity_buttons_[i]->set_enabled(true);
			} else {
				productivity_buttons_[i]->set_title("–");
				productivity_buttons_[i]->set_enabled(false);
			}
			button_tooltip = _("Productivity");
			if (productivity_buttons_[i]->enabled()) {
				button_tooltip =
						format_tooltip(
							button_tooltip,
							_("Click to step through buildings with low productivity and stopped buildings."));
			}
			productivity_buttons_[i]->set_tooltip(button_tooltip);
		} else {
			productivity_buttons_[i]->set_title(" ");
			productivity_buttons_[i]->set_enabled(false);
		}

		if (!building.global() && (building.is_buildable() || building.is_enhanced())) {
			/** TRANSLATORS Buildings: owned / under construction */
			owned_buttons_[i]->set_title((boost::format(_("%1% / %2%")) % nr_owned % nr_build).str());
			owned_buttons_[i]->set_enabled((nr_owned + nr_build) > 0);
		} else {
			owned_buttons_[i]->set_title((boost::format(_("%1% / %2%")) % nr_owned %  "–").str());
			owned_buttons_[i]->set_enabled(false);
		}
		building_buttons_[i]->set_enabled((nr_owned + nr_build) > 0);

		button_tooltip = building.descname();
		if (building_buttons_[i]->enabled()) {
			button_tooltip =
					format_tooltip(button_tooltip, _("Click to step through all buildings of this type."));
		}
		building_buttons_[i]->set_tooltip(button_tooltip);

		button_tooltip = _("Owned / Under Construction");
		if (owned_buttons_[i]->enabled()) {
			button_tooltip =
					format_tooltip(button_tooltip, _("Click to step through buildings under construction."));
		}
		owned_buttons_[i]->set_tooltip(button_tooltip);
	}
}
