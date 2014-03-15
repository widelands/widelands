/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#include "logic/widelands_streamread.h"

#include "logic/tribe.h"

namespace Widelands {

const Immovable_Descr & StreamRead::Immovable_Type
	(const Tribe_Descr & tribe)
{
	std::string name = CString();
	int32_t const index = tribe.get_immovable_index(name);
	if (index == -1)
		throw tribe_immovable_nonexistent(tribe.name(), name);
	return *tribe.get_immovable_descr(index);
}

} // namespace Widelands
