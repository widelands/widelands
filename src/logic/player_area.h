/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#ifndef PLAYER_AREA_H
#define PLAYER_AREA_H

#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

template <typename Area_type = Area<> > struct Player_Area : public Area_type {
	Player_Area() : player_number(0) {}
	Player_Area(const Player_Number pn, const Area_type area)
		: Area_type(area), player_number(pn)
	{}
	Player_Number player_number;
};

}

#endif
