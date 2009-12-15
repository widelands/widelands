/*
 * Copyright (C) 2004, 2006, 2008 by the Widelands Development Team
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

#ifndef AI_HINTS_H
#define AI_HINTS_H

#include <SDL_types.h>

struct Section;

/// This struct is used to read out the data given in [aihints] section of a
/// buildings conf file. It is used to tell the computer player about the
/// special properties of a building.
struct BuildingHints {
	BuildingHints (Section *);
	~BuildingHints();

	char const * get_renews_map_resource() const {return renews_map_resource;}

	char const * get_mines              () const {return mines;}

	bool         is_basic               () const {return basic;}

	bool         prod_build_material    () const {return build_material;}

	bool         is_trunkproducer       () const {return trunkproducer;}

	bool         is_stoneproducer       () const {return stoneproducer;}

	bool         get_needs_water        () const {return needs_water;}

	uint8_t      get_mines_percent      () const {return mines_percent;}

private:
	BuildingHints & operator= (BuildingHints const &);
	explicit BuildingHints    (BuildingHints const &);

	char  * renews_map_resource;
	char  * mines;
	bool    basic;
	bool    build_material; // whether the building produces build material
	bool    trunkproducer;
	bool    stoneproducer;
	bool    needs_water;
	uint8_t mines_percent;
};

#endif
