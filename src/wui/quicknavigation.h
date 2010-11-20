
namespace Widelands {
class Map;}
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef QUICKNAVIGATION_H
#define QUICKNAVIGATION_H

#include <boost/function.hpp>
#include <stdint.h>
#include <vector>

#include <SDL_keyboard.h>

#include "point.h"

namespace Widelands {
struct Editor_Game_Base;
}

/**
 * Provide quick navigation shortcuts.
 *
 * \note This functionality is really only used by \ref Interactive_Base,
 * but it is moved in its own structure to avoid overloading that class.
 */
struct QuickNavigation {
	typedef boost::function<void (Point)> SetViewFn;

	QuickNavigation
		(Widelands::Editor_Game_Base const & egbase,
		 uint32_t screenwidth, uint32_t screenheight);

	void set_setview(SetViewFn const &);

	void view_changed(Point, bool jump);

	bool handle_key(bool down, SDL_keysym);

private:
	void setview(Point where);

	Widelands::Editor_Game_Base const & m_egbase;
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
