/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef WIDELANDS_STREAMWRITE_INLINES_H
#define WIDELANDS_STREAMWRITE_INLINES_H

#include "widelands_streamwrite.h"

#include "building.h"

namespace Widelands {

inline void StreamWrite::Tribe(Tribe_Descr const & tribe) {
	String(tribe.name());
}

inline void StreamWrite::Tribe(Tribe_Descr const * tribe) {
	CString(tribe ? tribe->name().c_str() : "");
}

inline void StreamWrite::Immovable_Type(Immovable_Descr const & immovable) {
	Tribe (immovable.get_owner_tribe());
	String(immovable.name());
}

inline void StreamWrite::Building_Type(Building_Descr const & building) {
	Tribe (building.tribe());
	String(building.name());
}

}

#endif
