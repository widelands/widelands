/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#include "logic/mapastar.h"

#include "logic/map_objects/map_object.h"
#include "logic/path.h"

namespace Widelands {

/**
 * Recover a shortest path from one of the sources introduced by @ref MapAStar::push
 * to the given destination @p dest, which must have been discovered
 * in the A-star process already.
 */
void MapAStarBase::pathto(Coords dest, Path& path) const {
	path.end_ = dest;
	path.path_.clear();

	Coords cur = dest;
	for (;;) {
		const Pathfield& pf(pathfield(cur));

		assert(pf.cycle == pathfields->cycle);

		if (pf.backlink == IDLE) {
			break;
		}

		path.path_.push_back(pf.backlink);

		map.get_neighbour(cur, get_reverse_dir(pf.backlink), &cur);
	}

	path.start_ = cur;
}

}  // namespace Widelands
