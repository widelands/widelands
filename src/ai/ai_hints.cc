/*
 * Copyright (C) 2004, 2008-2009 by the Widelands Development Team
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

#include "ai_hints.h"

#include "profile/profile.h"

#include <cstring>


BuildingHints::~BuildingHints ()
{
	free(renews_map_resource);
	free(mines);
}

BuildingHints::BuildingHints (Section * const section) :
	renews_map_resource(0),
	mines              (0),
	basic              (section ? section->get_bool("is_basic")      : false),
	trunkproducer      (section ? section->get_bool("trunkproducer") : false),
	stoneproducer      (section ? section->get_bool("stoneproducer") : false),
	needs_water        (section ? section->get_bool("needs_water")   : false),
	mines_percent      (section ? section->get_int ("mines_percent", 100) : 0)
{
	if (section) {
		if (char const * const s = section->get_string("renews_map_resource"))
			renews_map_resource = strdup(s);
		if (char const * const s = section->get_string("mines"))
			mines               = strdup(s);
	}
}
