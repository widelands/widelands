/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#ifndef WL_WUI_SHIPWINDOW_H
#define WL_WUI_SHIPWINDOW_H

#include <functional>
#include <memory>

#include "logic/game.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/walkingdir.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_base.h"
#include "wui/itemwaresdisplay.h"

/**
 * Display information about a ship.
 */
class ShipWindow : public UI::UniqueWindow {
public:
	ShipWindow(InteractiveBase& ib, UI::UniqueWindow::Registry& reg, Widelands::Ship* ship);

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kShipWindow;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&, Widelands::MapObjectLoader&);

private:
	void think() override;

	UI::Button* make_button(UI::Panel* parent,
	                        const std::string& name,
	                        const std::string& title,
	                        const std::string& picname,
	                        const std::function<void()>& callback);
	void set_button_visibility();
	void no_port_error_message();

	void act_goto();
	void act_destination();
	void act_sink();
	void act_debug();
	void act_cancel_expedition();
	void act_scout_towards(Widelands::WalkingDir);
	void act_construct_port();
	void act_explore_island(Widelands::IslandExploreDirection);

	InteractiveBase& ibase_;
	Widelands::OPtr<Widelands::Ship> ship_;

	UI::Box vbox_;
	UI::Box navigation_box_;
	UI::Button* btn_goto_;
	UI::Button* btn_destination_;
	UI::Button* btn_sink_;
	UI::Button* btn_debug_;
	UI::Button* btn_cancel_expedition_;
	UI::Button* btn_explore_island_cw_;
	UI::Button* btn_explore_island_ccw_;
	// format: DIRECTION - 1, as 0 is normally the current location.
	UI::Button* btn_scout_[Widelands::LAST_DIRECTION];
	UI::Button* btn_construct_port_;
	ItemWaresDisplay* display_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteShip>> shipnotes_subscriber_;
	DISALLOW_COPY_AND_ASSIGN(ShipWindow);
};

#endif  // end of include guard: WL_WUI_SHIPWINDOW_H
