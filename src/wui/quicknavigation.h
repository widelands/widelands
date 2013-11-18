/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef QUICKNAVIGATION_H
#define QUICKNAVIGATION_H

#include <vector>

#include <SDL_keyboard.h>
#include <boost/function.hpp>
#include <stdint.h>

#include "point.h"

namespace Widelands {
class Editor_Game_Base;
}

/**
 * Provide quick navigation shortcuts.
 *
 * \note This functionality is really only used by \ref Interactive_Base,
 * but it is moved in its own structure to avoid overloading that class.
 */
struct QuickNavigation {
	typedef boost::function<void (Point)> SetViewFn;

	QuickNavigation(const Widelands::Editor_Game_Base & egbase, uint32_t screenwidth, uint32_t screenheight);

	void set_setview(const SetViewFn & fn);

	void view_changed(Point point, bool jump);

	bool handle_key(bool down, SDL_keysym key);

private:
	void setview(Point where);

	const Widelands::Editor_Game_Base & m_egbase;
	uint32_t m_screenwidth;
	uint32_t m_screenheight;

	/**
	 * This is the callback function that we call to request a change in view position.
	 */
	SetViewFn m_setview;

	bool m_havefirst;
	bool m_update;
	Point m_anchor;
	Point m_current;

	/**
	 * Keeps track of what the player has looked at to allow jumping back and forth
	 * in the history.
	 */
	/*@{*/
	std::vector<Point> m_history;
	std::vector<Point>::size_type m_history_index;
	/*@}*/

	struct Landmark {
		Point point;
		bool set;

		Landmark() : set(false) {}
	};

	/**
	 * Landmarks that were set explicitly by the player, mapped on the 0-9 keys.
	 */
	Landmark m_landmarks[10];
};

#endif // QUICKNAVIGATION_H
