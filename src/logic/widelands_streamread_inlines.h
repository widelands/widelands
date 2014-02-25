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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WIDELANDS_STREAMREAD_INLINES_H
#define WIDELANDS_STREAMREAD_INLINES_H

#include "logic/immovable.h"
#include "logic/widelands_streamread.h"

namespace Widelands {

inline const Tribe_Descr & StreamRead::Tribe
	(const Editor_Game_Base & egbase)
{
	char const * const name = CString();
	if (Tribe_Descr const * const result = egbase.get_tribe(name))
		return *result;
	else
		throw tribe_nonexistent(name);
}

inline Tribe_Descr const * StreamRead::Tribe_allow_null
	(const Editor_Game_Base & egbase)
{
	char const * const name = CString();
	if (*name)
		if (Tribe_Descr const * const result = egbase.get_tribe(name))
			return result;
		else
			throw tribe_nonexistent(name);
	else
		return nullptr;
}

inline const Immovable_Descr & StreamRead::Immovable_Type
	(const World & world)
{
	char const * const name = CString();
	int32_t const index = world.get_immovable_index(name);
	if (index == -1)
		throw world_immovable_nonexistent(world.get_name(), name);
	return *world.get_immovable_descr(index);
}

inline const Immovable_Descr & StreamRead::Immovable_Type
	(const Editor_Game_Base & egbase)
{
	if (Tribe_Descr const * const tribe = Tribe_allow_null(egbase))
		return Immovable_Type(*tribe);
	else
		return Immovable_Type(egbase.map().world());
}

inline const Building_Descr  & StreamRead::Building_Type
	(const Tribe_Descr & tribe)
{
	char const * const name = CString();
	Building_Index const index = tribe.building_index(name);
	if (not index)
		throw building_nonexistent(tribe.name(), name);
	return *tribe.get_building_descr(index);
}

inline const Building_Descr & StreamRead::Building_Type
	(const Editor_Game_Base & egbase)
{
	return Building_Type(Tribe(egbase));
}

}

#endif
