/*
 * Copyright (C) 2002 by Florian Bluemel
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

#include "tribe.h"

Tribe::Tribe(const char*)
{
	// load tribe file
}

Tribe::~Tribe()
{
}

Pic* Tribe::get_pic(int n)
{
	// return pointer to pic n
	return NULL;
}

Building* Tribe::create_building(int n)
{
	return NULL;
}

Creature* Tribe::create_creature(int n)
{
	return NULL;
}

Ware* Tribe::get_ware(int n)
{
	return NULL;
}