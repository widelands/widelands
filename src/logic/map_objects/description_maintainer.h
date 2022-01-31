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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_DESCRIPTION_MAINTAINER_H
#define WL_LOGIC_MAP_OBJECTS_DESCRIPTION_MAINTAINER_H

#include <cassert>
#include <map>
#include <memory>

#include "base/wexception.h"
#include "logic/widelands.h"

namespace Widelands {

// Used for having a typesafe maintainer for description classes.
template <typename T> struct DescriptionMaintainer {
	// Adds the 'entry', will assert() if it is already registered. Returns the
	// index of the entry. Ownership is taken.
	Widelands::DescriptionIndex add(T* entry);

	// Returns the number of entries in the container.
	Widelands::DescriptionIndex size() const {
		return items_.size();
	}

	// Returns the entry with the given 'name' if it exists or nullptr.
	T* exists(const std::string& name) const;

	// Returns the index of the entry with the given 'name' or INVALID_INDEX if the entry
	// is not in the container.
	Widelands::DescriptionIndex get_index(const std::string& name) const {
		NameToIndexMap::const_iterator i = name_to_index_.find(name);
		if (i == name_to_index_.end()) {
			return Widelands::INVALID_INDEX;
		}
		return i->second;
	}

	// Returns the entry with the given 'idx' or nullptr if 'idx' is out of
	// bounds. Ownership is retained.
	T* get_mutable(const Widelands::DescriptionIndex idx) const {
		return (idx < items_.size()) ? items_[idx].get() : nullptr;
	}

	// Returns the entry at 'index'. If 'index' is out of bounds the result is
	// undefined.
	const T& get(const Widelands::DescriptionIndex index) const {
		assert(index < items_.size());
		return *items_.at(index);
	}

private:
	using NameToIndexMap = std::map<std::string, int>;
	std::vector<std::unique_ptr<T>> items_;
	NameToIndexMap name_to_index_;
};

template <typename T> Widelands::DescriptionIndex DescriptionMaintainer<T>::add(T* const item) {
	if (exists(item->name()) != nullptr) {
		throw wexception("Tried to add %s twice.", item->name().c_str());
	}
	int32_t index = items_.size();
	assert(index < (Widelands::INVALID_INDEX - 2));  // allowing for INVALID_INDEX
	items_.emplace_back(item);
	name_to_index_[item->name()] = index;
	return index;
}

template <typename T> T* DescriptionMaintainer<T>::exists(const std::string& name) const {
	auto index = get_index(name);
	if (index == Widelands::INVALID_INDEX)
		return nullptr;
	return items_[index].get();
}

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_DESCRIPTION_MAINTAINER_H
