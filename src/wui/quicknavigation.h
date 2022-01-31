/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#ifndef WL_WUI_QUICKNAVIGATION_H
#define WL_WUI_QUICKNAVIGATION_H

#include "wui/mapview.h"

constexpr uint16_t kQuicknavSlots = 9;

/**
 * Provide quick navigation shortcuts.
 *
 * \note This functionality is really only used by \ref InteractiveBase,
 * but it is moved in its own structure to avoid overloading that class.
 */
struct QuickNavigation {
	struct Landmark {
		MapView::View view;
		bool set;

		Landmark() : set(false) {
		}
	};

	explicit QuickNavigation(MapView* map_view);

	// Set the landmark for 'index' to 'view'. 'index' must be < 10.
	void set_landmark(size_t index, const MapView::View& view);

	// Returns a pointer to the first element in the landmarks array
	const Landmark* landmarks() const {
		return landmarks_;
	}

	bool handle_key(bool down, SDL_Keysym key);

private:
	void view_changed();
	void jumped();

	MapView* map_view_;

	bool havefirst_;
	MapView::View current_;

	// Landmarks that were set explicitly by the player, mapped on the 1-9 keys.
	Landmark landmarks_[kQuicknavSlots];

	// navigation with ',' and '.'
	std::list<MapView::View> previous_locations_;
	std::list<MapView::View> next_locations_;
	// Ignore the initial (0,0,1×) view
	bool location_jumping_started_;
	void insert_if_applicable(std::list<MapView::View>&);
};

#endif  // end of include guard: WL_WUI_QUICKNAVIGATION_H
