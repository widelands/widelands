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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ai/ai_hints.h"

#include <cstring>

#include "profile/profile.h"


BuildingHints::~BuildingHints ()
{
	free(renews_map_resource);
	free(mines);
}

BuildingHints::BuildingHints (Section * const section) :
	renews_map_resource(nullptr),
	mines              (nullptr),
	basic              (section ? section->get_bool("is_basic")         : false),
	build_material     (section ? section->get_bool("build_material")   : true),
	logproducer      (section ? section->get_bool("logproducer")    : false),
	stoneproducer      (section ? section->get_bool("stoneproducer")    : false),
	needs_water        (section ? section->get_bool("needs_water")      : false),
	recruitment        (section ? section->get_bool("recruitment")      : false),
	space_consumer     (section ? section->get_bool("space_consumer")   : false),
	mines_percent      (section ? section->get_int ("mines_percent", 100) : 0)
{
	if (section) {
		if (char const * const s = section->get_string("renews_map_resource"))
			renews_map_resource = strdup(s);
		if (char const * const s = section->get_string("mines"))
			mines               = strdup(s);
	}
}
