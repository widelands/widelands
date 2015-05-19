/*
 * Copyright (C) 2002-2004, 2006, 2008-2009, 2011 by the Widelands Development Team
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

#include <vector>

#include "logic/building.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_player.h"

using namespace Widelands;

/// This window shows statistics for all the buildings that the player owns.
/// It also allows to jump through buildings on the map.
struct BuildingStatisticsMenu : public UI::UniqueWindow {
	BuildingStatisticsMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);

	void think() override;
	void update();

private:
	/// Which building state to jump through
	enum class JumpTarget {kOwned, kConstruction, kUnproductive};
	enum NavigationButton {
		PrevOwned,
		NextOwned,
		PrevConstruction,
		NextConstruction,
		PrevUnproductive,
		NextUnproductive
	};

	/// Adds a button for the building type belonging to the id and descr to the tab.
	/// Returns true when a new row needs to be created.
	bool add_button(
	   BuildingIndex id, const BuildingDescr& descr, UI::Box& tab, UI::Box& row, int* column);

	/// Jumps to the next / previous appropriate building
	void jump_building(JumpTarget target, bool reverse);

	/// Sets the current building type for the bottom navigation
	void set_current_building_type(BuildingIndex id);

	/// Helper function for jump_building to go round robin
	int32_t validate_pointer(int32_t*, int32_t);

	InteractivePlayer& iplayer() const;

	/// UI tabs
	UI::TabPanel tabs_;
	UI::Box small_tab_;
	UI::Box medium_tab_;
	UI::Box big_tab_;
	UI::Box mines_tab_;
	UI::Box ports_tab_;

	/// Button with building icon
	std::vector<UI::Button*> building_buttons_;
	/// Labels with owned / under construction buildings
	std::vector<UI::Textarea*> owned_labels_;
	/// Labels with buildings' productivity
	// TODO(GunChleoc): These need to be multiline, so we can give them a color.
	// Turn into normal textareas in fh1 branch.
	std::vector<UI::MultilineTextarea*> productivity_labels_;

	/// The buttons for stepping through buildings
	UI::Button* navigation_buttons_[6];
	UI::Textarea owned_label_;
	UI::Textarea construction_label_;
	UI::Textarea unproductive_label_;
	UI::Textarea no_owned_label_;
	UI::Textarea no_construction_label_;
	UI::Textarea no_unproductive_label_;

	/// The building type we are currently navigating
	BuildingIndex current_building_type_;
	/// The last building that was jumped to
	int32_t last_building_index_;
	/// The type of last building that was jumped to
	BuildingIndex last_building_type_;
	/// The last time the information in this Panel got updated
	uint32_t lastupdate_;
};

#endif  // end of include guard: WL_WUI_BUILDING_STATISTICS_MENU_H
