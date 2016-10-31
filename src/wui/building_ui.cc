/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "wui/buildingwindow.h"

#include <boost/lexical_cast.hpp>

#include "base/macros.h"
#include "logic/map_objects/tribes/building.h"
#include "ui_basic/window.h"

using Widelands::Building;

/**
 * Create the building's options window if necessary and bring it to
 * the top to be seen by the player.
 */
void Building::show_options(InteractiveGameBase& igbase, bool avoid_fastclick, Vector2i pos) {
	if (optionswindow_) {
		if (optionswindow_->is_minimal())
			optionswindow_->restore();
		optionswindow_->move_to_top();
	} else {
		create_options_window(igbase, optionswindow_);
		if (upcast(BuildingWindow, bw, optionswindow_)) {
			bw->set_avoid_fastclick(avoid_fastclick);
		}
		// Run a first think here so that certain things like caps buttons
		// get properly initialized
		optionswindow_->think();
	}
	if (pos.x >= 0 && pos.y >= 0) {
		optionswindow_->set_pos(pos);
	}
}

/**
 * Remove the building's options window.
 */
void Building::hide_options() {
	for (boost::signals2::connection& c : options_window_connections)
		c.disconnect();
	delete optionswindow_;
	optionswindow_ = nullptr;
}

/**
 * refreshs the option window of a building - useful if some ui elements have to be removed or added
 */
void Building::refresh_options(InteractiveGameBase& igb) {
	// Only do something if there is actually a window
	if (optionswindow_) {
		Vector2i window_position = optionswindow_->get_pos();
		hide_options();
		show_options(igb, true);
		optionswindow_->set_pos(window_position);
	}
}
