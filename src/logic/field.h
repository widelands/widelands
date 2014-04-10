/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef FIELD_H
#define FIELD_H

#include <cassert>
#include <limits>

#include "constants.h"
#include "logic/nodecaps.h"
#include "logic/roadtype.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"
#include "logic/world.h"

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

struct Terrain_Descr;
class Bob;
struct BaseImmovable;

// Field is used so often, make sure it is as small as possible.
#pragma pack(push, 1)
/// a field like it is represented in the game
/// \todo This is all one evil hack :(
struct Field {
	friend class Map;
	friend class Bob;
	friend struct BaseImmovable;

	enum Buildhelp_Index {
		Buildhelp_Flag   = 0,
		Buildhelp_Small  = 1,
		Buildhelp_Medium = 2,
		Buildhelp_Big    = 3,
		Buildhelp_Mine   = 4,
		Buildhelp_Port   = 5,
		Buildhelp_None   = 6
	};

	typedef uint8_t Height;
	typedef uint8_t Resource_Amount;

	struct Terrains         {Terrain_Index   d : 4, r : 4;};
	static_assert(sizeof(Terrains) == 1, "assert(sizeof(Terrains) == 1) failed.");
	struct Resources        {Resource_Index  d : 4, r : 4;};
	static_assert(sizeof(Resources) == 1, "assert(sizeof(Resources) == 1) failed.");
	struct Resource_Amounts {Resource_Amount d : 4, r : 4;};
	static_assert(sizeof(Resource_Amounts) == 1, "assert(sizeof(Resource_Amounts) == 1) failed.");

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
	typedef Player_Number Owner_Info_and_Selections_Type;
	static const uint8_t Border_Bit =
		std::numeric_limits<Owner_Info_and_Selections_Type>::digits - 1;
	static const Owner_Info_and_Selections_Type Border_Bitmask = 1 << Border_Bit;
	static const Owner_Info_and_Selections_Type Player_Number_Bitmask =
		Border_Bitmask - 1;
	static const Owner_Info_and_Selections_Type Owner_Info_Bitmask =
		Player_Number_Bitmask + Border_Bitmask;
	static_assert(MAX_PLAYERS <= Player_Number_Bitmask, "Bitmask is too big.");

	// Data Members
	/** linked list, \sa Bob::m_linknext*/
	Bob           * bobs;
	BaseImmovable * immovable;

	uint8_t caps                    : 7;
	uint8_t buildhelp_overlay_index : 3;
	uint8_t roads                   : 6;

	Height height;
	int8_t brightness;

	Owner_Info_and_Selections_Type owner_info_and_selections;

	Resource_Index m_resources; ///< Resource type on this field, if any
	uint8_t m_starting_res_amount; ///< Initial amount of m_resources
	uint8_t m_res_amount; ///< Current amount of m_resources

	Terrains terrains;

public:
	Height get_height() const {return height;}
	NodeCaps nodecaps() const {return static_cast<NodeCaps>(caps);}
	uint16_t get_caps()     const {return caps;}

	Terrains      get_terrains() const {return terrains;}
	Terrain_Index terrain_d   () const {return terrains.d;}
	Terrain_Index terrain_r   () const {return terrains.r;}
	void          set_terrains(const Terrains & i) {terrains = i;}
	void set_terrain
		(const TCoords<FCoords>::TriangleIndex& t, Terrain_Index const i)

	{
		if (t == TCoords<FCoords>::D) set_terrain_d(i);
		else set_terrain_r(i);
	}
	void set_terrain_d(Terrain_Index const i) {terrains.d = i;}
	void set_terrain_r(Terrain_Index const i) {terrains.r = i;}

	Bob * get_first_bob() const {return bobs;}
	const BaseImmovable * get_immovable() const {return immovable;}
	BaseImmovable * get_immovable() {return immovable;}

	void set_brightness
		(int32_t l, int32_t r, int32_t tl, int32_t tr, int32_t bl, int32_t br);
	int8_t get_brightness() const {return brightness;}

	/**
	 * Does not change the border bit of this or neighbouring fields. That must
	 * be done separately.
	 */
	void set_owned_by(const Player_Number n) {
		assert(n <= MAX_PLAYERS);
		owner_info_and_selections =
			n | (owner_info_and_selections & ~Player_Number_Bitmask);
	}

	Player_Number get_owned_by() const {
		assert
			((owner_info_and_selections & Player_Number_Bitmask) <= MAX_PLAYERS);
		return owner_info_and_selections & Player_Number_Bitmask;
	}
	bool is_border() const {return owner_info_and_selections & Border_Bitmask;}

	///
	/// Returns true when the node is owned by player_number and is not a border
	/// node. This is fast; only one compare (and a mask because the byte is
	/// shared with selection).
	///
	/// player_number must be in the range 1 .. Player_Number_Bitmask or the
	/// behaviour is undefined.
	bool is_interior(const Player_Number player_number) const {
		assert(0 < player_number);
		assert    (player_number <= Player_Number_Bitmask);
		return player_number == (owner_info_and_selections & Owner_Info_Bitmask);
	}

	void set_border(const bool b) {
		owner_info_and_selections =
			(owner_info_and_selections & ~Border_Bitmask) | (b << Border_Bit);
	}

	uint8_t get_buildhelp_overlay_index() const {return buildhelp_overlay_index;}
	void set_buildhelp_overlay_index(Buildhelp_Index const i) {
		buildhelp_overlay_index = i;
	}

	int32_t get_roads() const {return roads;}
	int32_t get_road(int32_t const dir) const {
		return (roads >> dir) & Road_Mask;
	}
	void set_road(int32_t const dir, int32_t const type) {
		roads &= ~(Road_Mask << dir);
		roads |= type << dir;
	}

	/// \todo This should return Resource_Index
	uint8_t get_resources() const {return m_resources;}
	uint8_t get_resources_amount() const {return m_res_amount;}
	void set_resources(uint8_t const res, uint8_t const amount) {
		m_resources  = res;
		m_res_amount = amount;
	}

	///<\todo This should take uint8_t
	void set_starting_res_amount(int32_t const amount) {
		m_starting_res_amount = amount;
	}
	/// \todo This should return uint8_t
	int32_t get_starting_res_amount() const {return m_starting_res_amount;}

	/// \note you must reset this field's + neighbor's brightness when you
	/// change the height. Map::change_height does this. This function is not
	/// private, because the loader will use them directly But realize, most of
	/// the times you will need Map::set_field_height().
	void set_height(Height const h) {
		height =
			static_cast<int8_t>(h) < 0 ? 0 :
			MAX_FIELD_HEIGHT       < h ? MAX_FIELD_HEIGHT : h;
	}
};
#pragma pack(pop)

// Check that Field is tightly packed.
static_assert(sizeof(Field) <= sizeof(void *) * 2 + 10, "Field is not tightly packed.");
}

#endif
