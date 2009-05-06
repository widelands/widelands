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


BuildingHints::BuildingHints ()
:
renews_map_resource(0),
mines(0),
basic(false),
trunkproducer(false),
stoneproducer(false),
needs_water(false)
{}


void BuildingHints::parse (Profile & prof)
{
	if (Section * const hints = prof.get_section("aihints")) {
		if (char const * const s = hints->get_string("renews_map_resource"))
			renews_map_resource = strdup(s);
		if (char const * const s = hints->get_string("mines"))
			mines = strdup(s);
		if (const bool b = hints->get_bool("is_basic"))
			basic = b;
		if (const bool b = hints->get_bool("trunkproducer"))
			trunkproducer = b;
		if (const bool b = hints->get_bool("stoneproducer"))
			stoneproducer = b;
		if (const bool b = hints->get_bool("needs_water"))
			needs_water   = b;
		mines_percent
		= static_cast<uint8_t>(hints->get_int("mines_percent", 100));
	}
}
