/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/pathfield.h"

#include <memory>

#include "base/wexception.h"

namespace Widelands {

Pathfields::Pathfields(uint32_t const nrfields) : fields(new Pathfield[nrfields]), cycle(0) {
}

PathfieldManager::PathfieldManager() : nrfields_(0) {
}

void PathfieldManager::set_size(uint32_t const nrfields) {
	if (nrfields_ != nrfields) {
		list_.clear();
	}

	nrfields_ = nrfields;
}

std::shared_ptr<Pathfields> PathfieldManager::allocate() {
	for (std::shared_ptr<Pathfields>& pathfield : list_) {
		if (pathfield.use_count() == 1) {
			++pathfield->cycle;
			if (pathfield->cycle == 0u) {
				clear(pathfield);
			}
			return pathfield;
		}
	}

	if (list_.size() >= 8) {
		throw wexception("PathfieldManager::allocate: unbounded nesting?");
	}

	std::shared_ptr<Pathfields> pf(new Pathfields(nrfields_));
	clear(pf);
	list_.push_back(pf);
	return pf;
}

void PathfieldManager::clear(const std::shared_ptr<Pathfields>& pf) const {
	for (uint32_t i = 0; i < nrfields_; ++i) {
		pf->fields[i].cycle = 0;
	}
	pf->cycle = 1;
}
}  // namespace Widelands
