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

#ifndef WL_WUI_BUILDING_STATISTICS_MENU_H
#define WL_WUI_BUILDING_STATISTICS_MENU_H

#include "graphic/color.h"
#include "logic/map_objects/tribes/building.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_player.h"

namespace {

constexpr int kNoOfBuildingTabs = 5;

}  // namespace

/// This window shows statistics for all the buildings that the player owns.
/// It also allows to jump through buildings on the map.
struct BuildingStatisticsMenu : public UI::UniqueWindow {
	BuildingStatisticsMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);
	~BuildingStatisticsMenu() override;

	void think() override;

	/// Update state of current building buttons
	void update();

private:
	/// Array indices for the tabs
	enum BuildingTab { Small, Medium, Big, Mines, Ports };

	/// Which building state to jump through
	enum class JumpTarget { kOwned, kConstruction, kUnproductive };

	/// Array indices for the navigation buttons
	enum NavigationButton {
		PrevOwned,
		NextOwned,
		PrevConstruction,
		NextConstruction,
		PrevUnproductive,
		NextUnproductive
	};

	/// Initialize the buttons
	void reset();
	void init(int last_selected_tab = 0);

	/// Whether a building that is used by the player's tribe should be added
	bool own_building_is_valid(const Widelands::Player& player,
	                           Widelands::DescriptionIndex index,
	                           bool map_allows_seafaring,
	                           bool map_allows_waterways) const;
	/// Whether a building that isn't used by the player's tribe should be added
	bool foreign_tribe_building_is_valid(const Widelands::Player& player,
	                                     Widelands::DescriptionIndex index) const;
	/// Determine which tab a building button should end up on, according to building size etc.
	int find_tab_for_building(const Widelands::BuildingDescr& descr) const;

	/// If the buildings that should be shown have changes, update the list and reinitialize
	void update_building_list();

	/// Adds a button for the building type belonging to the id and descr to the tab.
	/// Returns true when a new row needs to be created.
	void
	add_button(Widelands::DescriptionIndex id, const Widelands::BuildingDescr& descr, UI::Box* row);

	/// Jumps to the next / previous appropriate building
	void jump_building(JumpTarget target, bool reverse);

	/// Sets the label for the given textarea to text in the chosen color
	void set_labeltext(UI::Textarea* textarea, const std::string& text, const RGBColor& color);

	/// Sets the current building type for the bottom navigation
	void set_current_building_type(Widelands::DescriptionIndex id);

	/// Change the percentage where buildings are deemed unproductive
	void low_production_changed();
	/// Unfocuses the editbox to free the keyboard input
	void low_production_reset_focus();

	/// Helper function for jump_building to go round robin
	int32_t validate_pointer(int32_t*, int32_t);

	InteractivePlayer& iplayer() const;

	/// Style
	const UI::BuildingStatisticsStyleInfo& style_;

	/// UI tabs
	UI::TabPanel tab_panel_;
	UI::Box* tabs_[kNoOfBuildingTabs];
	/// How many button rows each tab has
	int row_counters_[kNoOfBuildingTabs];
	/// We can have gaps in the tab sequence, so we need to map the indices for remembering the last
	/// selected tab
	int tab_assignments_[kNoOfBuildingTabs];

	/// Button with building icon
	std::vector<UI::Button*> building_buttons_;
	/// Labels with owned / under construction buildings
	std::vector<UI::Textarea*> owned_labels_;
	/// Labels with buildings' productivity
	std::vector<UI::Textarea*> productivity_labels_;

	/// The buttons for stepping through buildings
	UI::Panel navigation_panel_;
	UI::Button* navigation_buttons_[6];
	UI::Textarea building_name_;
	UI::Textarea owned_label_;
	UI::Textarea construction_label_;
	UI::Box unproductive_box_;
	UI::Textarea unproductive_label_;
	UI::EditBox unproductive_percent_;
	UI::Textarea unproductive_label2_;
	UI::Textarea no_owned_label_;
	UI::Textarea no_construction_label_;
	UI::Textarea no_unproductive_label_;

	/// The building type we are currently navigating
	Widelands::DescriptionIndex current_building_type_;
	/// The last building that was jumped to
	int32_t last_building_index_;
	/// The type of last building that was jumped to
	Widelands::DescriptionIndex last_building_type_;
	/// The last time the information in this Panel got updated
	Time lastupdate_;
	/// Whether the window was minimized the last time that think() was executed
	uint32_t was_minimized_;

	/// At which percent to deem buildings as unproductive
	int low_production_;

	/// Whether a building has been selected
	bool has_selection_;

	/// The total number of building types available for all the tribes
	const Widelands::DescriptionIndex nr_building_types_;
};

#endif  // end of include guard: WL_WUI_BUILDING_STATISTICS_MENU_H
