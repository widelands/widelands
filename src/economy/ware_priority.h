/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_WARE_PRIORITY_H
#define WL_ECONOMY_WARE_PRIORITY_H

#include <cstdint>
#include <limits>

class StreamRead;
class StreamWrite;

namespace Widelands {

class WarePriority {
public:
	WarePriority(const WarePriority&) = default;
	WarePriority& operator=(const WarePriority&) = default;

	~WarePriority() {
	}

	uint32_t to_weighting_factor() const {
		return value_;
	}

	bool operator==(const WarePriority& w) const {
		return value_ == w.value_;
	}
	// For ordering in sets and maps
	bool operator<(const WarePriority& w) const {
		return value_ < w.value_;
	}
	bool operator<=(const WarePriority& w) const {
		return value_ <= w.value_;
	}

	// For saveloading and (de)serializing of playercommands
	void write(StreamWrite&) const;
	explicit WarePriority(StreamRead&);

	// Predefined constants. Use these instead of instantiating this directly.
	static const WarePriority kVeryLow;
	static const WarePriority kLow;
	static const WarePriority kNormal;
	static const WarePriority kHigh;
	static const WarePriority kVeryHigh;

private:
	constexpr explicit WarePriority(uint32_t v) noexcept : value_(v) {
	}

	uint32_t value_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_WARE_PRIORITY_H
