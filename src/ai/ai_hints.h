/*
 * Copyright (C) 2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef AI_HINTS_H
#define AI_HINTS_H

#include <SDL_types.h>
#include <boost/noncopyable.hpp>

struct Section;

/// This struct is used to read out the data given in [aihints] section of a
/// buildings conf file. It is used to tell the computer player about the
/// special properties of a building.
struct BuildingHints : boost::noncopyable {
	BuildingHints (Section *);
	~BuildingHints();

	char const * get_renews_map_resource() const {return renews_map_resource;}

	char const * get_mines              () const {return mines;}

	bool         is_basic               () const {return basic;}

	bool         prod_build_material    () const {return build_material;}

	bool         is_logproducer       () const {return logproducer;}

	bool         is_stoneproducer       () const {return stoneproducer;}

	bool         get_needs_water        () const {return needs_water;}

	bool         for_recruitment        () const {return recruitment;}

	bool         is_space_consumer      () const {return space_consumer;}

	uint8_t      get_mines_percent      () const {return mines_percent;}

private:
	char  * renews_map_resource;
	char  * mines;
	bool    basic;
	bool    build_material; // whether the building produces build material
	bool    logproducer;
	bool    stoneproducer;
	bool    needs_water;
	bool    recruitment; // whether building recruits special workers
	bool    space_consumer;
	uint8_t mines_percent;
};

#endif
