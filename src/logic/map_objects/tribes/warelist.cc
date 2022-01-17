/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "logic/map_objects/tribes/warelist.h"

#include "base/log.h"

namespace Widelands {

/**
 * Delete the list. Print a warning message if the storage is not empty.
 *
 * This is because most of the time, a WareList should be zeroed by cleanup
 * operations before the destructor is called. If you are sure of what you're
 * doing, call clear().
 */
WareList::~WareList() {
	for (uint32_t id = 0; id < wares_.size(); ++id) {
		if (wares_[id]) {
			log_warn("WareList: %i items of %i left.\n", wares_[id], id);
		}
	}
}

/**
 * Add the given number of items (default = 1) to the storage.
 */
void WareList::add(DescriptionIndex const i, const Quantity count) {
	if (!count) {
		return;
	}

	if (wares_.size() <= i) {
		wares_.resize(i + 1, 0);
	}
	wares_[i] += count;
	assert(wares_[i] >= count);
}

void WareList::add(const WareList& wl) {
	DescriptionIndex const nr_wares = wl.get_nrwareids();
	if (wares_.size() < nr_wares) {
		wares_.reserve(nr_wares);
	}
	for (DescriptionIndex i = 0; i < nr_wares; ++i) {
		if (wl.wares_[i]) {
			add(i, wl.wares_[i]);
		}
	}
}

/**
 * Remove the given number of items (default = 1) from the storage.
 */
void WareList::remove(DescriptionIndex const i, const Quantity count) {
	if (!count) {
		return;
	}

	assert(i < wares_.size());
	assert(wares_[i] >= count);
	wares_[i] -= count;
}

void WareList::remove(const WareList& wl) {
	DescriptionIndex const nr_wares = wl.get_nrwareids();
	for (DescriptionIndex i = 0; i < nr_wares; ++i) {
		if (wl.wares_[i]) {
			remove(i, wl.wares_[i]);
		}
	}
}

/**
 * Return the number of wares of a given type stored in this storage.
 */
Quantity WareList::stock(DescriptionIndex const id) const {
	return id < wares_.size() ? wares_[id] : 0;
}

/**
 * Two WareLists are only equal when they contain the exact same stock of
 * all wares types.
 */
bool WareList::operator==(const WareList& wl) const {
	uint32_t i = 0;

	while (i < wl.wares_.size()) {
		const Quantity count = wl.wares_[i];
		if (i < wares_.size()) {
			if (count != wares_[i]) {
				return false;
			}
		} else {
			if (count) {  // wl2 has 0 stock per definition
				return false;
			}
		}
		++i;
	}

	while (i < wares_.size()) {
		if (wares_[i]) {  // wl1 has 0 stock per definition
			return false;
		}
		++i;
	}

	return true;
}
}  // namespace Widelands
