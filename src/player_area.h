/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#ifndef __S__PLAYER_AREA_H
#define __S__PLAYER_AREA_H

#include "widelands_geometry.h"

#include "widelands.h"

namespace Widelands {

template <typename Area_type = Area<> > struct Player_Area : public Area_type {
	Player_Area() {}
	Player_Area(const Player_Number pn, const Area_type area)
		: Area_type(area), player_number(pn)
	{}
	Player_Number player_number;
};

};

#endif // __S__PLAYER_AREA_H
