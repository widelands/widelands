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

class Profile;

/// This struct is used to read out the data given in [aihints] section of a
/// buildings conf file. It is used to tell the computer player about the
/// special properties of a building.
struct BuildingHints {
	BuildingHints ();

	void parse (Profile &);

	const char * get_renews_map_resource() const throw ()
	{return renews_map_resource;}

	const char * get_mines() const throw ()
	{return mines;}

	bool is_basic()
	{return basic;}

	bool is_trunkproducer()
	{return trunkproducer;}

	bool is_stoneproducer()
	{return stoneproducer;}

	bool get_needs_water()
	{return needs_water;}

	uint8_t get_mines_percent()
	{return mines_percent;}

private:
	char  * renews_map_resource;
	char  * mines;
	bool    basic;
	bool    trunkproducer;
	bool    stoneproducer;
	bool    needs_water;
	uint8_t mines_percent;
};

#endif
