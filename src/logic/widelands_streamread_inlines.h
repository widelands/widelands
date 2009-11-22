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

#ifndef WIDELANDS_STREAMREAD_INLINES_H
#define WIDELANDS_STREAMREAD_INLINES_H

#include "widelands_streamread.h"

#include "immovable.h"

namespace Widelands {

inline Tribe_Descr const & StreamRead::Tribe
	(Editor_Game_Base const & egbase)
{
	char const * const name = CString();
	if (Tribe_Descr const * const result = egbase.get_tribe(name))
		return *result;
	else
		throw tribe_nonexistent(name);
}

inline Tribe_Descr const * StreamRead::Tribe_allow_null
	(Editor_Game_Base const & egbase)
{
	char const * const name = CString();
	if (*name)
		if (Tribe_Descr const * const result = egbase.get_tribe(name))
			return result;
		else
			throw tribe_nonexistent(name);
	else
		return 0;
}

inline Immovable_Descr const & StreamRead::Immovable_Type
	(World const & world)
{
	char const * const name = CString();
	int32_t const index = world.get_immovable_index(name);
	if (index == -1)
		throw world_immovable_nonexistent(world.get_name(), name);
	return *world.get_immovable_descr(index);
}

inline Immovable_Descr const & StreamRead::Immovable_Type
	(Tribe_Descr const & tribe)
{
	char const * const name = CString();
	int32_t const index = tribe.get_immovable_index(name);
	if (index == -1)
		throw tribe_immovable_nonexistent(tribe.name(), name);
	return *tribe.get_immovable_descr(index);
}

inline Immovable_Descr const & StreamRead::Immovable_Type
	(Editor_Game_Base const & egbase)
{
	if (Tribe_Descr const * const tribe = Tribe_allow_null(egbase))
		return Immovable_Type(*tribe);
	else
		return Immovable_Type(egbase.map().world());
}

inline Building_Descr  const & StreamRead::Building_Type
	(Tribe_Descr const & tribe)
{
	char const * const name = CString();
	Building_Index const index = tribe.building_index(name);
	if (not index)
		throw building_nonexistent(tribe.name(), name);
	return *tribe.get_building_descr(index);
}

inline Building_Descr const & StreamRead::Building_Type
	(Editor_Game_Base const & egbase)
{
	return Building_Type(Tribe(egbase));
}

}

#endif
