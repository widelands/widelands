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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_streamread.h"

#include "tribe.h"

namespace Widelands {

Immovable_Descr const & StreamRead::Immovable_Type
	(Tribe_Descr const & tribe)
{
	std::string name = CString();
	const std::vector<std::string> & compat = tribe.compatibility_immovable(name);
	if (compat.size() == 2 && compat[0] == "replace")
		name = compat[1];
	int32_t const index = tribe.get_immovable_index(name);
	if (index == -1)
		throw tribe_immovable_nonexistent(tribe.name(), name);
	return *tribe.get_immovable_descr(index);
}

} // namespace Widelands
