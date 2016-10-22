/*
 * Copyright (C) 2010-2016 by the Widelands Development Team
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

#ifndef WL_WUI_QUICKNAVIGATION_H
#define WL_WUI_QUICKNAVIGATION_H

#include <vector>

#include <SDL_keyboard.h>
#include <boost/function.hpp>
#include <stdint.h>

#include "base/vector.h"
#include "wui/mapview.h"

namespace Widelands {
class EditorGameBase;
}

/**
 * Provide quick navigation shortcuts.
 *
 * \note This functionality is really only used by \ref InteractiveBase,
 * but it is moved in its own structure to avoid overloading that class.
 */
struct QuickNavigation {
	struct View {
		Vector2f viewpoint;
		float zoom;
	};

	struct Landmark {
		View view;
		bool set;

		Landmark() : set(false) {
		}
	};

	QuickNavigation(const Widelands::EditorGameBase& egbase,
	                MapView* map_view);

	// Set the landmark for 'index' to 'view'. 'index' must be < 10.
	void set_landmark(size_t index, const View& view);

	// Returns a pointer to the first element in the landmarks array
	const std::vector<Landmark>& landmarks() const {
		return landmarks_;
	}

	bool handle_key(bool down, SDL_Keysym key);

private:
	void setview(const View& where);
	void view_changed(bool jump);

	const Widelands::EditorGameBase& egbase_;
	MapView* map_view_;

	bool havefirst_;
	bool update_;
	Vector2f anchor_;  // center of last view in mappixel.
	View current_;

	/**
	 * Keeps track of what the player has looked at to allow jumping back and forth
	 * in the history.
	 */
	/*@{*/
	std::vector<View> history_;
	size_t history_index_;
	/*@}*/

	/**
	 * Landmarks that were set explicitly by the player, mapped on the 0-9 keys.
	 */
	std::vector<Landmark> landmarks_;
};

#endif  // end of include guard: WL_WUI_QUICKNAVIGATION_H
