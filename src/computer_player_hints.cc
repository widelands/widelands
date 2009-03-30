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

#include "computer_player_hints.h"

#include "profile.h"

#include <cstring>


BuildingHints::BuildingHints () : need_map_resource(0) {}


void BuildingHints::parse (Profile & prof)
{
	if (Section * const hints = prof.get_section("aihints"))
		if (char const * const s = hints->get_string("need_map_resource"))
			need_map_resource = strdup(s);
}
