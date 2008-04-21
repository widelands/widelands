/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef FIELD_H
#define FIELD_H

#include "widelands_geometry.h"
#include "compile_assert.h"
#include "constants.h"
#include "world.h"

#include "widelands.h"

#include <cassert>
#include <limits>


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
// Probably it would be better to make Field a struct, rather than a class (things wouldn't
// change much, it's just a question of style and code understanding)

enum FieldCaps {
	CAPS_NONE = 0,
	/** can we build normal buildings? (use BUILDCAPS_SIZEMASK for binary masking)*/
	BUILDCAPS_SMALL = 1,
	BUILDCAPS_MEDIUM = 2,
	BUILDCAPS_BIG = 3,
	BUILDCAPS_SIZEMASK = 3,

	/** can we build a flag on this field?*/
	BUILDCAPS_FLAG = 4,

	/** can we build a mine on this field (completely independent from build size!)*/
	BUILDCAPS_MINE = 8,

	/** (only if BUILDCAPS_BIG): can we build a harbour on this field?
	 * this should be automatically set for BUILDCAPS_BIG fields that have a swimmable second-order neighbour*/
	BUILDCAPS_PORT = 16,

	/** can we build any building on this field?*/
	BUILDCAPS_BUILDINGMASK = BUILDCAPS_SIZEMASK|BUILDCAPS_MINE|BUILDCAPS_PORT,

	/** Can Map_Objects walk or swim here? Also used for Map_Object::get_movecaps()
	 * If MOVECAPS_WALK, any walking being can walk to this field*/
	MOVECAPS_WALK = 32,

	/// If MOVECAPS_SWIM, any swimming being (including ships) can go there.
	/// Additionally, swimming beings can temporarily visit nodes that are
	/// walkable but not swimmable if those nodes are at the start or end of
	/// their path. Without this clause, harbours would be kind of impossible ;)
	/// This clause stops ducks from "swimwalking" along the coast.
	MOVECAPS_SWIM = 64,
};

enum Roads {
	Road_None = 0,
	Road_Normal = 1,
	Road_Busy = 2,
	Road_Water = 3,
	Road_Mask = 3,

	Road_East      = 0, //  shift values
	Road_SouthEast = 2,
	Road_SouthWest = 4,
};

struct Terrain_Descr;
class Bob;
struct BaseImmovable;

/// a field like it is represented in the game
/// \todo This is all one evil hack :(
struct Field {
	friend struct Map;
	friend class Bob;
	friend struct BaseImmovable;

	enum Buildhelp_Index {
		Buildhelp_Flag   = 0,
		Buildhelp_Small  = 1,
		Buildhelp_Medium = 2,
		Buildhelp_Big    = 3,
		Buildhelp_Mine   = 4,
		Buildhelp_None   = 5
	};

	typedef uint8_t Height;
	typedef uint8_t Resource_Amount;

	struct Terrains         {Terrain_Descr::Index  d : 4, r : 4;};
	struct Resources        {Resource_Descr::Index d : 4, r : 4;};
	struct Resource_Amounts {Resource_Amount       d : 4, r : 4;};

private:
	Height height;
	int8_t   brightness;

	FieldCaps       caps                    : 7;
	Buildhelp_Index buildhelp_overlay_index : 3;
	uint8_t           roads                   : 6;

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
	static const uint8_t Selection_B_Bit =
		std::numeric_limits<Owner_Info_and_Selections_Type>::digits - 1;
	static const uint8_t Selection_A_Bit = Selection_B_Bit - 1;
	static const uint8_t Border_Bit      = Selection_A_Bit - 1;
	static const Owner_Info_and_Selections_Type Selection_B_Bitmask =
		1 << Selection_B_Bit;
	static const Owner_Info_and_Selections_Type Selection_A_Bitmask =
		1 << Selection_A_Bit;
	static const Owner_Info_and_Selections_Type Border_Bitmask = 1 << Border_Bit;
	static const Owner_Info_and_Selections_Type Player_Number_Bitmask =
		Border_Bitmask - 1;
	static const Owner_Info_and_Selections_Type Owner_Info_Bitmask =
		Player_Number_Bitmask + Border_Bitmask;
	compile_assert(MAX_PLAYERS <= Player_Number_Bitmask);
	Owner_Info_and_Selections_Type owner_info_and_selections;

	Resource_Index m_resources; ///< Resource type on this field, if any
	uint8_t m_starting_res_amount; ///< Initial amount of m_resources
	uint8_t m_res_amount; ///< Current amount of m_resources

	Terrains terrains;

	/** linked list, \sa Bob::m_linknext*/
	Bob* bobs;
	BaseImmovable* immovable;

public:
	Height get_height() const throw () {return height;}
	FieldCaps get_caps() const {return caps;}

	Terrains             get_terrains() const throw () {return terrains;}
	Terrain_Descr::Index terrain_d   () const throw () {return terrains.d;}
	Terrain_Descr::Index terrain_r   () const throw () {return terrains.r;}
	void set_terrains (const Terrains             i) throw () {terrains   = i;}
	void set_terrain
		(const TCoords<FCoords>::TriangleIndex t, const Terrain_Descr::Index i)
		throw ()
	{(t == TCoords<FCoords>::D ? terrains.d : terrains.r) = i;}
	void set_terrain_d(const Terrain_Descr::Index i) throw () {terrains.d = i;}
	void set_terrain_r(const Terrain_Descr::Index i) throw () {terrains.r = i;}

	Bob * get_first_bob() const throw () {return bobs;}
	const BaseImmovable * get_immovable() const throw () {return immovable;}
	BaseImmovable* get_immovable() {return immovable;}

	void set_brightness(int32_t l, int32_t r, int32_t tl, int32_t tr, int32_t bl, int32_t br);
	int8_t get_brightness() const {return brightness;}

	/**
	 * Does not change the border bit of this or neighbouring fileds. That must
	 * be done separately.
	 */
	void set_owned_by(const Player_Number n) throw () {
		assert(n <= MAX_PLAYERS);
		owner_info_and_selections =
			n | (owner_info_and_selections & ~Player_Number_Bitmask);
	}

	Player_Number get_owned_by() const throw () {
		assert
			((owner_info_and_selections & Player_Number_Bitmask) <= MAX_PLAYERS);
		return owner_info_and_selections & Player_Number_Bitmask;
	}
	bool is_border() const throw ()
	{return owner_info_and_selections & Border_Bitmask;}

	/**
	 * Returns true when the field is owned by player_number and is not a border
	 * field. This is fast; only one compare (and a mask because the byte is
	 * shared with selection).
	 *
	 * player_number must be in the range 1 .. Player_Number_Bitmask or the
	 * behaviour is undefined
	 */
	bool is_interior(const Player_Number player_number) const throw () {
		assert(0 < player_number);
		assert    (player_number <= Player_Number_Bitmask);
		return player_number == (owner_info_and_selections & Owner_Info_Bitmask);
	}

	void set_border(const bool b) throw () {
		owner_info_and_selections =
			(owner_info_and_selections & ~Border_Bitmask) | (b << Border_Bit);
	}

	uint8_t get_buildhelp_overlay_index() const {return buildhelp_overlay_index;}
	void set_buildhelp_overlay_index(const Buildhelp_Index i)
	{buildhelp_overlay_index = i;}

	int32_t get_roads() const {return roads;}
	int32_t get_road(int32_t dir) const {return (roads >> dir) & Road_Mask;}
	void set_road(int32_t dir, int32_t type) {
		roads &= ~(Road_Mask << dir);
		roads |= type << dir;
	}

	uint8_t get_resources() const {return m_resources;} ///<\todo This should return Resource_Index
	uint8_t get_resources_amount() const {return m_res_amount;}
	void set_resources(uint8_t res, uint8_t amount) {m_resources = res; m_res_amount=amount;}
	void set_starting_res_amount(int32_t amount) {m_starting_res_amount=amount;} ///<\todo This should take uint8_t
	int32_t get_starting_res_amount() const {return m_starting_res_amount;} ///<\todo This should return uint8_t

	/// \note you must reset this field's + neighbor's brightness when you
	/// change the height. Map::change_height does this. This function is not
	/// private, because the loader will use them directly But realize, most of
	/// the times you will need Map::set_field_height().
	void set_height(Height h) {
		if (static_cast<int8_t>(h) < 0) h = 0;
		if (h > MAX_FIELD_HEIGHT) h = MAX_FIELD_HEIGHT;
		height = h;
	}
};
//TODO:
/**
 * \todo This following line is wrong on 64bit systems.
 * Question: does anything rely on this size? If yes, that needs to be fixed
 * ASAP! Or is it just a paranoia check? In that case it still needs fixing.
 *
 * compile_assert(sizeof(Field) == 12 + 2 * sizeof(void *));
 */

};

#endif
