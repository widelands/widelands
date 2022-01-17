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

#ifndef WL_LOGIC_FIELD_H
#define WL_LOGIC_FIELD_H

#include <atomic>
#include <cassert>

#include "base/wexception.h"
#include "graphic/playercolor.h"
#include "graphic/road_segments.h"
#include "logic/map_objects/walkingdir.h"
#include "logic/nodecaps.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

#define MAX_FIELD_HEIGHT 60
#define MAX_FIELD_HEIGHT_DIFF 5

// Think, if we shouldn't call for each field a new() in map::set_size
// and a delete
// Okay, as it stands now, Field can be safely memset()ed to 0.
//
// No. In fact, you should view Field more as a plain old structure rather than
// a class. If you think of Fields as a class you get into a whole lot of
// problems (like the 6 neighbour field pointers we used to have). - Nicolai
// Probably it would be better to make Field a struct, rather than a class
// (things wouldn't change much, it's just a question of style and code
// understanding)
// Making it a struct doesn't add anything. struct is used interchangeably with
// class all around the code

class Bob;
struct BaseImmovable;

// Field is used so often, make sure it is as small as possible.
#pragma pack(push, 1)
/// a field like it is represented in the game
// TODO(unknown): This is all one evil hack :(
class Field {
public:
	friend class Map;
	friend class Bob;
	friend struct BaseImmovable;

	enum BuildhelpIndex {
		Buildhelp_Flag = 0,
		Buildhelp_Small = 1,
		Buildhelp_Medium = 2,
		Buildhelp_Big = 3,
		Buildhelp_Mine = 4,
		Buildhelp_Port = 5,
		Buildhelp_None = 6
	};

	using Height = uint8_t;
	using ResourceAmount = uint8_t;

	struct Terrains {
		DescriptionIndex d, r;
	};
	static_assert(sizeof(Terrains) == sizeof(DescriptionIndex) * 2,
	              "assert(sizeof(Terrains) == sizeof(DescriptionIndex) * 2) failed.");
	struct Resources {
		DescriptionIndex d : 4, r : 4;
	};
	static_assert(sizeof(Resources) <= sizeof(DescriptionIndex),
	              "assert(sizeof(Resources) <= sizeof(DescriptionIndex)) failed.");
	struct ResourceAmounts {
		ResourceAmount d : 4, r : 4;
	};
	static_assert(sizeof(ResourceAmounts) == 1, "assert(sizeof(ResourceAmounts) == 1) failed.");

	Field() : brightness(0), owner_info_and_selections(Widelands::neutral()) {
	}

	Field& operator=(const Field& other) {
		bobs = other.bobs;
		immovable = other.immovable;
		caps = other.caps;
		max_caps = other.max_caps;
		road_east = other.road_east;
		road_southeast = other.road_southeast;
		road_southwest = other.road_southwest;
		height = other.height;
		brightness = other.brightness.load();
		owner_info_and_selections = other.owner_info_and_selections.load();
		resources = other.resources;
		initial_res_amount = other.initial_res_amount;
		res_amount = other.res_amount;
		terrains = other.terrains;
		return *this;
	}

	Height get_height() const {
		return height;
	}
	NodeCaps nodecaps() const {
		return static_cast<NodeCaps>(caps);
	}
	NodeCaps maxcaps() const {
		return static_cast<NodeCaps>(max_caps);
	}
	uint16_t get_caps() const {
		return caps;
	}

	Terrains get_terrains() const {
		return terrains;
	}
	// The terrain on the downward triangle
	DescriptionIndex terrain_d() const {
		return terrains.d;
	}
	// The terrain on the triangle to the right
	DescriptionIndex terrain_r() const {
		return terrains.r;
	}
	void set_terrains(const Terrains& i) {
		terrains = i;
	}
	void set_terrain(const TriangleIndex& t, DescriptionIndex const i)

	{
		if (t == TriangleIndex::D)
			set_terrain_d(i);
		else
			set_terrain_r(i);
	}
	void set_terrain_d(DescriptionIndex const i) {
		terrains.d = i;
	}
	void set_terrain_r(DescriptionIndex const i) {
		terrains.r = i;
	}

	Bob* get_first_bob() const {
		return bobs;
	}
	const BaseImmovable* get_immovable() const {
		return immovable;
	}
	BaseImmovable* get_immovable() {
		return immovable;
	}

	void set_brightness(int32_t l, int32_t r, int32_t tl, int32_t tr, int32_t bl, int32_t br);
	int8_t get_brightness() const {
		return brightness;
	}

	/**
	 * Does not change the border bit of this or neighbouring fields. That must
	 * be done separately.
	 */
	void set_owned_by(const PlayerNumber n) {
		assert(n <= kMaxPlayers);
		owner_info_and_selections = n | (owner_info_and_selections & ~Player_Number_Bitmask);
	}

	PlayerNumber get_owned_by() const {
		assert((owner_info_and_selections & Player_Number_Bitmask) <= kMaxPlayers);
		return owner_info_and_selections & Player_Number_Bitmask;
	}
	bool is_border() const {
		return owner_info_and_selections & Border_Bitmask;
	}

	///
	/// Returns true when the node is owned by player_number and is not a border
	/// node. This is fast; only one compare (and a mask because the byte is
	/// shared with selection).
	///
	/// player_number must be in the range 1 .. Player_Number_Bitmask or the
	/// behaviour is undefined.
	bool is_interior(const PlayerNumber player_number) const {
		assert(0 < player_number);
		assert(player_number <= Player_Number_Bitmask);
		return player_number == (owner_info_and_selections & Owner_Info_Bitmask);
	}

	void set_border(const bool b) {
		owner_info_and_selections = (owner_info_and_selections & ~Border_Bitmask) | (b << Border_Bit);
	}

	RoadSegment get_road(uint8_t dir) const {
		switch (dir) {
		case WalkingDir::WALK_E:
			return road_east;
		case WalkingDir::WALK_SE:
			return road_southeast;
		case WalkingDir::WALK_SW:
			return road_southwest;
		default:
			throw wexception("Queried road going in invalid direction %i", dir);
		}
	}
	void set_road(uint8_t dir, RoadSegment type) {
		switch (dir) {
		case WalkingDir::WALK_E:
			road_east = type;
			break;
		case WalkingDir::WALK_SE:
			road_southeast = type;
			break;
		case WalkingDir::WALK_SW:
			road_southwest = type;
			break;
		default:
			throw wexception("Attempt to set road going in invalid direction %i", dir);
		}
	}

	// Resources can be set through Map::set_resources()
	// TODO(unknown): This should return DescriptionIndex
	DescriptionIndex get_resources() const {
		return resources;
	}
	ResourceAmount get_resources_amount() const {
		return res_amount;
	}
	// TODO(unknown): This should return uint8_t
	ResourceAmount get_initial_res_amount() const {
		return initial_res_amount;
	}

	/// \note you must reset this field's + neighbor's brightness when you
	/// change the height. Map::change_height does this. This function is not
	/// private, because the loader will use them directly But realize, most of
	/// the times you will need Map::set_field_height().
	void set_height(Height const h) {
		height = static_cast<int8_t>(h) < 0 ? 0 : MAX_FIELD_HEIGHT < h ? MAX_FIELD_HEIGHT : h;
	}

private:
	/**
	 * A field can be selected in one of 2 selections. This allows the user to
	 * use selection tools to select a set of fields and then perform a command
	 * on those fields.
	 *
	 * Selections can be edited with some operations. A selection can be
	 * 1. inverted,
	 * 2. unioned with the other selection,
	 * 3. intersected with the other selection or
	 * 4. differenced with the other selection.
	 *
	 * Each field can be owned by a player.
	 * The 2 highest bits are selected_a and selected_b.
	 * The next highest bit is the border bit.
	 * The low bits are the player number of the owner.
	 */
	using OwnerInfoAndSelectionsType = PlayerNumber;
	static const uint8_t Border_Bit = std::numeric_limits<OwnerInfoAndSelectionsType>::digits - 1;
	static const OwnerInfoAndSelectionsType Border_Bitmask = 1 << Border_Bit;
	static const OwnerInfoAndSelectionsType Player_Number_Bitmask = Border_Bitmask - 1;
	static const OwnerInfoAndSelectionsType Owner_Info_Bitmask =
	   Player_Number_Bitmask + Border_Bitmask;
	static_assert(kMaxPlayers <= Player_Number_Bitmask, "Bitmask is too big.");

	// Data Members. Initialize everything to make cppcheck happy.
	/** linked list, \sa Bob::linknext_ */
	Bob* bobs = nullptr;
	BaseImmovable* immovable = nullptr;

	uint8_t caps = 0U;
	uint8_t max_caps = 0U;

	RoadSegment road_east = RoadSegment::kNone;
	RoadSegment road_southeast = RoadSegment::kNone;
	RoadSegment road_southwest = RoadSegment::kNone;

	Height height = 0U;
	std::atomic<int8_t> brightness;

	std::atomic<OwnerInfoAndSelectionsType> owner_info_and_selections;

	DescriptionIndex resources = INVALID_INDEX;  ///< Resource type on this field, if any
	ResourceAmount initial_res_amount = 0U;      ///< Initial amount of resources
	ResourceAmount res_amount = 0U;              ///< Current amount of resources

	Terrains terrains = Terrains{INVALID_INDEX, INVALID_INDEX};
};
#pragma pack(pop)

// Check that Field is tightly packed.
static_assert(sizeof(Field) == sizeof(void*) * 2 + sizeof(RoadSegment) * 3 +
                                  sizeof(DescriptionIndex) * 3 + sizeof(uint8_t) * 7,
              "Field is not tightly packed.");
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_FIELD_H
