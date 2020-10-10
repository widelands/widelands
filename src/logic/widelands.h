/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

#ifndef WL_LOGIC_WIDELANDS_H
#define WL_LOGIC_WIDELANDS_H

#include <cstdint>
#include <limits>
#include <vector>

#include "base/wexception.h"

class FileRead;
class FileWrite;

namespace Widelands {

//  Type definitions for the game logic.
using MilitaryInfluence = uint16_t;

/// 5 bits used, so 0 .. 31
/// Data type must match kMaxPlayers in graphics/playercolor.h
using PlayerNumber = uint8_t;
inline PlayerNumber neutral() {
	return 0;
}
#define iterate_player_numbers(p, nr_players)                                                      \
	for (Widelands::PlayerNumber p = 1; p < nr_players + 1; ++p)

/**
 * Every player has a team number. Team number 0 is special,
 * indicating that the player is not part of any team.
 *
 * Players having the same positive team number are allied.
 */
using TeamNumber = uint8_t;

/** This is used as index for wares, workers, terrain and other Descriptions.
 *
 * So we can have at most 254 types of these,
 * as some are predefined as invalid by the declarations below.
 *
 * A DescriptionMaintainer can be used to lookup the actual item.
 * See TerrainDescription, WareDescr, WorkerDescr
 * EditorCategory, BuildingDescr, ImmovableDescr, ShipDescr, TribeDescr
 * and others.
 */
using DescriptionIndex = uint16_t;

constexpr DescriptionIndex INVALID_INDEX = std::numeric_limits<uint16_t>::max();
constexpr DescriptionIndex kInvalidWare = INVALID_INDEX - 1;
constexpr DescriptionIndex kNoResource = INVALID_INDEX - 1;

using ResourceAmount = uint8_t;  /// 4 bits used, so 0 .. 15.

using Quantity = uint32_t;  // e.g. the number of a type of ware in a warehouse.

using Vision = uint16_t;

// The difference of two points in time, in milliseconds gametime.
struct Duration {
	// The default-constructed Duration is the special value "endless"
	constexpr explicit Duration(uint32_t v = std::numeric_limits<uint32_t>::max()) : value_(v) {
	}
	void operator+=(const Duration& delta) {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to add invalid Durations");
		}
		value_ += delta.get();
	}
	void operator-=(const Duration& delta) {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to subtract invalid Durations");
		}
		if (get() < delta.get()) {
			throw wexception("Duration: Subtraction result would be negative");
		}
		value_ -= delta.get();
	}
	void operator/=(uint32_t d) {
		if (!is_valid()) {
			throw wexception("Attempt to divide invalid Duration");
		}
		if (d <= 0) {
			throw wexception("Attempt to divide Duration by zero");
		}
		value_ /= d;
	}

	// Intervals arithmetics
	Duration operator+(const Duration& d) const {
		if (!is_valid() || !d.is_valid()) {
			throw wexception("Attempt to add invalid Durations");
		}
		return Duration(get() + d.get());
	}
	Duration operator-(const Duration& d) const {
		if (!is_valid() || !d.is_valid()) {
			throw wexception("Attempt to subtract invalid Durations");
		}
		if (get() < d.get()) {
			throw wexception("Duration: Subtraction result would be negative");
		}
		return Duration(get() - d.get());
	}
	Duration operator*(uint32_t d) const {
		if (!is_valid()) {
			throw wexception("Attempt to multiply invalid Durations");
		}
		return Duration(get() * d);
	}
	Duration operator/(uint32_t d) const {
		if (!is_valid()) {
			throw wexception("Attempt to divide invalid Duration");
		}
		if (d <= 0) {
			throw wexception("Attempt to divide Duration by zero");
		}
		return Duration(get() / d);
	}

	uint32_t get() const {
		return value_;
	}

	inline bool operator<(const Duration& m) const {
		return get() < m.get();
	}
	inline bool operator>(const Duration& m) const {
		return get() > m.get();
	}
	inline bool operator<=(const Duration& m) const {
		return get() <= m.get();
	}
	inline bool operator>=(const Duration& m) const {
		return get() >= m.get();
	}
	inline bool operator==(const Duration& m) const {
		return get() == m.get();
	}
	inline bool operator!=(const Duration& m) const {
		return get() != m.get();
	}

	// Special values
	inline bool is_invalid() const {
		return *this == Duration();
	}
	inline bool is_valid() const {
		return !is_invalid();
	}

	// Saveloading
	explicit Duration(FileRead&);
	void save(FileWrite&) const;

private:
	uint32_t value_;
};

// A time point, in milliseconds gametime.
struct Time {
	// The default-constructed Time is the special value "never"
	constexpr explicit Time(uint32_t v = std::numeric_limits<uint32_t>::max()) : value_(v) {
	}

	// Adding/subtracting intervals
	Time operator+(const Duration& delta) const {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to add invalid Time or Duration");
		}
		return Time(get() + delta.get());
	}
	Time operator-(const Duration& delta) const {
		if (!is_valid() || !delta.is_valid()) {
			throw wexception("Attempt to subtract invalid Time or Duration");
		}
		if (get() < delta.get()) {
			throw wexception("Time-Duration-Subtraction result would be negative");
		}
		return Time(get() - delta.get());
	}

	// Obtaining a time difference
	Duration operator-(const Time& t) const {
		if (!is_valid() || !t.is_valid()) {
			throw wexception("Attempt to subtract invalid Time");
		}
		if (get() < t.get()) {
			throw wexception("Time: Subtraction result would be negative");
		}
		return Duration(get() - t.get());
	}

	uint32_t get() const {
		return value_;
	}

	void increment(const Duration& d = Duration(1)) {
		if (!is_valid() || !d.is_valid()) {
			throw wexception("Attempt to increment invalid Time or Duration");
		}
		value_ += d.get();
	}

	inline bool operator<(const Time& m) const {
		return get() < m.get();
	}
	inline bool operator>(const Time& m) const {
		return get() > m.get();
	}
	inline bool operator<=(const Time& m) const {
		return get() <= m.get();
	}
	inline bool operator>=(const Time& m) const {
		return get() >= m.get();
	}
	inline bool operator==(const Time& m) const {
		return get() == m.get();
	}
	inline bool operator!=(const Time& m) const {
		return get() != m.get();
	}

	// Special values
	inline bool is_invalid() const {
		return *this == Time();
	}
	inline bool is_valid() const {
		return !is_invalid();
	}

	// Saveloading
	explicit Time(FileRead&);
	void save(FileWrite&) const;

private:
	uint32_t value_;
};

using Serial = uint32_t;  /// Serial number for MapObject.
constexpr Serial kInvalidSerial = std::numeric_limits<uint32_t>::max();

using Direction = uint8_t;

struct SoldierStrength {
	uint8_t health, attack, defense, evade;
	bool operator==(const SoldierStrength& other) const {
		return health == other.health && attack == other.attack && defense == other.defense &&
		       evade == other.evade;
	}
	bool operator<(const SoldierStrength& other) const {
		return health < other.health ||
		       (health == other.health &&
		        (attack < other.attack ||
		         (attack == other.attack &&
		          (defense < other.defense || (defense == other.defense && evade < other.evade)))));
	}
};

// For suggested teams info during map preload
using SuggestedTeam = std::vector<PlayerNumber>;  // Players in a team
// Recommended teams to play against each other
using SuggestedTeamLineup = std::vector<SuggestedTeam>;

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_WIDELANDS_H
