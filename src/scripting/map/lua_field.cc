/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#include "scripting/map/lua_field.h"

#include "logic/map_objects/descriptions.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"
#include "scripting/map/lua_resource_description.h"
#include "scripting/map/lua_terrain_description.h"
#include "wui/mapviewpixelfunctions.h"

namespace LuaMaps {

/* RST
Field
-----

.. class:: Field

   This class represents one Field in Widelands. The field may contain
   immovables like Flags or Buildings and can be connected via Roads. Every
   Field has two Triangles associated with itself: The right and the down one.

   You cannot instantiate this directly, access it via
   ``wl.Game().map.get_field()`` instead.
*/

const char LuaField::className[] = "Field";
const MethodType<LuaField> LuaField::Methods[] = {
   METHOD(LuaField, __eq),     METHOD(LuaField, __tostring),   METHOD(LuaField, region),
   METHOD(LuaField, has_caps), METHOD(LuaField, has_max_caps),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaField, indicate),
#endif
   {nullptr, nullptr},
};
const PropertyType<LuaField> LuaField::Properties[] = {
   PROP_RO(LuaField, __hash),
   PROP_RO(LuaField, x),
   PROP_RO(LuaField, y),
   PROP_RO(LuaField, rn),
   PROP_RO(LuaField, ln),
   PROP_RO(LuaField, trn),
   PROP_RO(LuaField, tln),
   PROP_RO(LuaField, bln),
   PROP_RO(LuaField, brn),
   PROP_RO(LuaField, immovable),
   PROP_RO(LuaField, bobs),
   PROP_RW(LuaField, terr),
   PROP_RW(LuaField, terd),
   PROP_RW(LuaField, height),
   PROP_RW(LuaField, raw_height),
   PROP_RO(LuaField, viewpoint_x),
   PROP_RO(LuaField, viewpoint_y),
   PROP_RW(LuaField, resource),
   PROP_RW(LuaField, resource_amount),
   PROP_RW(LuaField, initial_resource_amount),
   PROP_RO(LuaField, claimers),
   PROP_RO(LuaField, owner),
   PROP_RO(LuaField, buildable),
   PROP_RO(LuaField, has_roads),
   {nullptr, nullptr, nullptr},
};

void LuaField::__persist(lua_State* L) {
	PERS_INT32("x", coords_.x);
	PERS_INT32("y", coords_.y);
}

void LuaField::__unpersist(lua_State* L) {
	UNPERS_INT32("x", coords_.x)
	UNPERS_INT32("y", coords_.y)
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: __hash

      (RO) The hashed coordinates of the field's position. Used to identify a class in a Set.
*/
int LuaField::get___hash(lua_State* L) {
	lua_pushuint32(L, coords_.hash());
	return 1;
}

/* RST
   .. attribute:: x, y

      (RO) The x/y coordinate of this field
*/
int LuaField::get_x(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, coords_.x);
	return 1;
}
int LuaField::get_y(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, coords_.y);
	return 1;
}

/* RST
   .. attribute:: height

      (RW) The height of this field. The default height is 10, you can increase
      or decrease this value to build mountains. Note though that if you change
      this value too much, all surrounding fields will also change their
      heights because the slope is constrained. If you are changing the height
      of many terrains at once, use :attr:`raw_height` instead and then call
      :any:`recalculate` afterwards.
*/
int LuaField::get_height(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_height());
	return 1;
}
int LuaField::set_height(lua_State* L) {
	uint32_t height = luaL_checkuint32(L, -1);
	Widelands::FCoords f = fcoords(L);

	if (f.field->get_height() == height) {
		return 0;
	}

	if (height > MAX_FIELD_HEIGHT) {
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);
	}

	Widelands::EditorGameBase& egbase = get_egbase(L);
	egbase.mutable_map()->set_height(egbase, f, height);

	return 0;
}

/* RST
   .. attribute:: raw_height

      (RW) The same as :attr:`height`, but setting this will not trigger a
      recalculation of the surrounding fields. You can use this field to
      change the height of many fields on a map quickly, then use
      :any:`recalculate` to make sure that everything is in order.
*/
// UNTESTED
int LuaField::get_raw_height(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_height());
	return 1;
}
int LuaField::set_raw_height(lua_State* L) {
	uint32_t height = luaL_checkuint32(L, -1);
	Widelands::FCoords f = fcoords(L);

	if (f.field->get_height() == height) {
		return 0;
	}

	if (height > MAX_FIELD_HEIGHT) {
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);
	}

	f.field->set_height(height);

	return 0;
}

/* RST
   .. attribute:: viewpoint_x, viewpoint_y

      (RO) Returns the position in pixels to move the view to to center
      this field for the current interactive player.
*/
int LuaField::get_viewpoint_x(lua_State* L) {
	Vector2f point =
	   MapviewPixelFunctions::to_map_pixel_with_normalization(get_egbase(L).map(), coords_);
	lua_pushdouble(L, point.x);
	return 1;
}
int LuaField::get_viewpoint_y(lua_State* L) {
	Vector2f point =
	   MapviewPixelFunctions::to_map_pixel_with_normalization(get_egbase(L).map(), coords_);
	lua_pushdouble(L, point.y);
	return 1;
}

/* RST
   .. attribute:: resource

      (RW) The name of the resource that is available in this field or
      "none".

      :see also: :attr:`resource_amount`
*/
int LuaField::get_resource(lua_State* L) {
	const Widelands::ResourceDescription* res_desc =
	   get_egbase(L).descriptions().get_resource_descr(fcoords(L).field->get_resources());

	lua_pushstring(L, res_desc != nullptr ? res_desc->name().c_str() : "none");

	return 1;
}
int LuaField::set_resource(lua_State* L) {
	auto& egbase = get_egbase(L);
	Widelands::DescriptionIndex res = egbase.descriptions().resource_index(luaL_checkstring(L, -1));

	if (res == Widelands::INVALID_INDEX) {
		report_error(L, "Illegal resource: '%s'", luaL_checkstring(L, -1));
	}

	auto c = fcoords(L);
	const auto current_amount = c.field->get_resources_amount();
	auto* map = egbase.mutable_map();
	map->initialize_resources(c, res, c.field->get_initial_res_amount());
	map->set_resources(c, current_amount);
	return 0;
}

/* RST
   .. attribute:: resource_amount

      (RW) How many items of the resource is available in this field.

      :see also: :attr:`resource`
*/
int LuaField::get_resource_amount(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_resources_amount());
	return 1;
}
int LuaField::set_resource_amount(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::FCoords c = fcoords(L);
	Widelands::DescriptionIndex res = c.field->get_resources();
	int32_t amount = luaL_checkint32(L, -1);
	const Widelands::ResourceDescription* res_desc = egbase.descriptions().get_resource_descr(res);
	Widelands::ResourceAmount max_amount = (res_desc != nullptr) ? res_desc->max_amount() : 0;

	if (amount < 0 || amount > max_amount) {
		report_error(L, "Illegal amount: %i, must be >= 0 and <= %u", amount,
		             static_cast<unsigned int>(max_amount));
	}

	auto* map = egbase.mutable_map();
	if (egbase.is_game()) {
		map->set_resources(c, amount);
	} else {
		// in editor, reset also initial amount
		map->initialize_resources(c, res, amount);
	}
	return 0;
}

/* RST
   .. attribute:: initial_resource_amount

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) Starting value of resource.

      :see also: :attr:`resource`
*/
int LuaField::get_initial_resource_amount(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_initial_res_amount());
	return 1;
}
int LuaField::set_initial_resource_amount(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::FCoords c = fcoords(L);
	Widelands::DescriptionIndex res = c.field->get_resources();
	int32_t amount = luaL_checkint32(L, -1);
	const Widelands::ResourceDescription* res_desc = egbase.descriptions().get_resource_descr(res);
	Widelands::ResourceAmount max_amount = (res_desc != nullptr) ? res_desc->max_amount() : 0;

	if (amount < 0 || amount > max_amount) {
		report_error(L, "Illegal amount: %i, must be >= 0 and <= %u", amount,
		             static_cast<unsigned int>(max_amount));
	}

	egbase.mutable_map()->initialize_resources(c, res, amount);
	return 0;
}

/* RST
   .. attribute:: immovable

      (RO) The immovable that stands on this field or :const:`nil`. If you want
      to remove an immovable, you can use :func:`wl.map.MapObject.remove`.
*/
int LuaField::get_immovable(lua_State* L) {
	Widelands::BaseImmovable* bi = get_egbase(L).map().get_immovable(coords_);

	if (bi == nullptr) {
		return 0;
	}
	upcasted_map_object_to_lua(L, bi);

	return 1;
}

/* RST
   .. attribute:: bobs

      (RO) An :class:`array` of :class:`~wl.map.Bob` that are associated
      with this field.
*/
// UNTESTED
int LuaField::get_bobs(lua_State* L) {
	Widelands::Bob* b = fcoords(L).field->get_first_bob();

	lua_newtable(L);
	uint32_t cidx = 1;
	while (b != nullptr) {
		lua_pushuint32(L, cidx++);
		upcasted_map_object_to_lua(L, b);
		lua_rawset(L, -3);
		b = b->get_next_bob();
	}
	return 1;
}

/* RST
   .. attribute:: terr, terd

      (RW) The terrain of the right/down triangle. This is a string value
      containing the name of the terrain as it is defined in the world
      configuration. You can change the terrain by simply assigning another
      valid name to these variables. If you are changing the terrain from or to
      water, the map will not recalculate whether it allows seafaring, because
      this recalculation can take up a lot of performance. If you need this
      recalculated, you can do so by calling :any:`recalculate_seafaring` after
      you're done changing terrains.
*/
int LuaField::get_terr(lua_State* L) {
	const Widelands::TerrainDescription* td =
	   get_egbase(L).descriptions().get_terrain_descr(fcoords(L).field->terrain_r());
	lua_pushstring(L, td->name().c_str());
	return 1;
}
int LuaField::set_terr(lua_State* L) {
	const char* name = luaL_checkstring(L, -1);
	Widelands::EditorGameBase& egbase = get_egbase(L);
	try {
		const Widelands::DescriptionIndex td = egbase.mutable_descriptions()->load_terrain(name);
		egbase.mutable_map()->change_terrain(
		   egbase, Widelands::TCoords<Widelands::FCoords>(fcoords(L), Widelands::TriangleIndex::R),
		   td);
	} catch (const Widelands::GameDataError& e) {
		report_error(L, "set_terr: %s", e.what());
	}

	lua_pushstring(L, name);
	return 1;
}

int LuaField::get_terd(lua_State* L) {
	const Widelands::TerrainDescription* td =
	   get_egbase(L).descriptions().get_terrain_descr(fcoords(L).field->terrain_d());
	lua_pushstring(L, td->name().c_str());
	return 1;
}
int LuaField::set_terd(lua_State* L) {
	const char* name = luaL_checkstring(L, -1);
	Widelands::EditorGameBase& egbase = get_egbase(L);
	try {
		const Widelands::DescriptionIndex td = egbase.mutable_descriptions()->load_terrain(name);
		egbase.mutable_map()->change_terrain(
		   egbase, Widelands::TCoords<Widelands::FCoords>(fcoords(L), Widelands::TriangleIndex::D),
		   td);
	} catch (const Widelands::GameDataError& e) {
		report_error(L, "set_terd: %s", e.what());
	}

	lua_pushstring(L, name);
	return 1;
}

/* RST
   .. attribute:: rn, ln, brn, bln, trn, tln

      (RO) The neighbour fields of this field. The abbreviations stand for:

      * ``rn`` -- Right neighbour
      * ``ln`` -- Left neighbour
      * ``brn`` -- Bottom right neighbour
      * ``bln`` -- Bottom left neighbour
      * ``trn`` -- Top right neighbour
      * ``tln`` -- Top left neighbour

      Note that the widelands map wraps at its borders, that is the following
      holds:

      .. code-block:: lua

         wl.map.Field(wl.map.get_width()-1, 10).rn == wl.map.Field(0, 10)
*/
#define GET_X_NEIGHBOUR(X)                                                                         \
	int LuaField::get_##X(lua_State* L) {                                                           \
		Widelands::Coords n;                                                                         \
		get_egbase(L).map().get_##X(coords_, &n);                                                    \
		to_lua<LuaField>(L, new LuaField(n.x, n.y));                                                 \
		return 1;                                                                                    \
	}
GET_X_NEIGHBOUR(rn)
GET_X_NEIGHBOUR(ln)
GET_X_NEIGHBOUR(trn)
GET_X_NEIGHBOUR(tln)
GET_X_NEIGHBOUR(bln)
GET_X_NEIGHBOUR(brn)

#undef GET_X_NEIGHBOUR

/* RST
   .. attribute:: owner

      (RO) The current owner of the field or :const:`nil` if noone owns it. See
      also :attr:`claimers`.
*/
int LuaField::get_owner(lua_State* L) {
	Widelands::PlayerNumber current_owner = fcoords(L).field->get_owned_by();
	if (current_owner != 0u) {
		get_factory(L).push_player(L, current_owner);
		return 1;
	}
	return 0;
}

/* RST
   .. attribute:: buildable

      (RO) Returns :const:`true` if a flag or building could be built on this field,
      independently of whether anybody currently owns this field.
*/
int LuaField::get_buildable(lua_State* L) {
	const Widelands::NodeCaps caps = fcoords(L).field->nodecaps();
	const bool is_buildable = ((caps & Widelands::BUILDCAPS_FLAG) != 0) ||
	                          ((caps & Widelands::BUILDCAPS_BUILDINGMASK) != 0);
	lua_pushboolean(L, static_cast<int>(is_buildable));
	return 1;
}

/* RST
   .. attribute:: has_roads

      (RO) Whether any roads lead to the field.
      Note that waterways are currently treated like roads.

      :returns: :const:`true` if any of the 6 directions has a road on it, :const:`false` otherwise.
*/
int LuaField::get_has_roads(lua_State* L) {

	const Widelands::FCoords& fc = fcoords(L);
	Widelands::Field* f = fc.field;
	if (f->get_road(Widelands::WalkingDir::WALK_E) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	if (f->get_road(Widelands::WalkingDir::WALK_SE) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	if (f->get_road(Widelands::WalkingDir::WALK_SW) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}

	Widelands::FCoords neighbor;
	const Widelands::Map& map = get_egbase(L).map();
	map.get_ln(fc, &neighbor);
	if (neighbor.field->get_road(Widelands::WalkingDir::WALK_E) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	map.get_tln(fc, &neighbor);
	if (neighbor.field->get_road(Widelands::WalkingDir::WALK_SE) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	map.get_trn(fc, &neighbor);
	if (neighbor.field->get_road(Widelands::WalkingDir::WALK_SW) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	lua_pushboolean(L, 0);

	return 1;
}

/* RST
   .. attribute:: claimers

      (RO) An :class:`array` of players that have military influence over this
      field sorted by the amount of influence they have. Note that this does
      not necessarily mean that claimers[1] is also the owner of the field, as
      a field that houses a surrounded military building is owned by the
      surrounded player, but others have more military influence over it.

      Note: The one currently owning the field is in :attr:`owner`.
*/
int LuaField::get_claimers(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::Map& map = egbase.map();

	std::vector<PlrInfluence> claimers;

	iterate_players_existing(other_p, map.get_nrplayers(), egbase, plr) claimers.emplace_back(
	   plr->player_number(), plr->military_influence(map.get_index(coords_, map.get_width())));

	std::stable_sort(claimers.begin(), claimers.end(), sort_claimers);

	lua_createtable(L, 1, 0);  // We mostly expect one claimer per field.

	// Push the players with military influence
	uint32_t cidx = 1;
	for (const PlrInfluence& claimer : claimers) {
		if (claimer.second <= 0) {
			continue;
		}
		lua_pushuint32(L, cidx++);
		get_factory(L).push_player(L, claimer.first);
		lua_rawset(L, -3);
	}

	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaField::__eq(lua_State* L) {
	lua_pushboolean(L, static_cast<int>((*get_user_class<LuaField>(L, -1))->coords_ == coords_));
	return 1;
}

int LuaField::__tostring(lua_State* L) {  // NOLINT - can not be made const
	const std::string pushme = format("Field(%i,%i)", coords_.x, coords_.y);
	lua_pushstring(L, pushme);
	return 1;
}

/* RST
   .. function:: region(r1[, r2])

      Returns an :class:`array` of all Fields inside the given region. If one argument
      is given it defines the radius of the region. If both arguments are
      specified, the first one defines the outer radius and the second one the
      inner radius and a hollow region is returned, that is all fields in the
      outer radius region minus all fields in the inner radius region.

      A small example:

      .. code-block:: lua

         f:region(1)

      will return an :class:`array` with the following entries (Note: Ordering of the
      fields inside the :class:`array` is not guaranteed):

      .. code-block:: lua

         {f, f.rn, f.ln, f.brn, f.bln, f.tln, f.trn}

      :returns: The :class:`array` of the given fields.
      :rtype: :class:`array`
*/
int LuaField::region(lua_State* L) {
	uint32_t n = lua_gettop(L);

	if (n == 3) {
		uint32_t radius = luaL_checkuint32(L, -2);
		uint32_t inner_radius = luaL_checkuint32(L, -1);
		return hollow_region(L, radius, inner_radius);
	}

	uint32_t radius = luaL_checkuint32(L, -1);
	return region(L, radius);
}

/* RST
   .. method:: has_caps(capname)

      Returns :const:`true` if the field has this **capname** associated
      with it, otherwise returns :const:`false`.

      Note: Immovables will hide the caps. If you want to have the caps
      without immovables use has_max_caps instead

      :arg capname: Can be either of:
      :type capname: :class:`string`

      * :const:`"small"`: Can a small building be built here?
      * :const:`"medium"`: Can a medium building be built here?
      * :const:`"big"`: Can a big building be built here?
      * :const:`"mine"`: Can a mine be built here?
      * :const:`"port"`: Can a port be built here?
      * :const:`"flag"`: Can a flag be built here?
      * :const:`"walkable"`: Is this field passable for walking bobs?
      * :const:`"swimmable"`: Is this field passable for swimming bobs?


*/
int LuaField::has_caps(lua_State* L) {
	const Widelands::FCoords& f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);
	lua_pushboolean(
	   L, static_cast<int>(check_has_caps(L, query, f, f.field->nodecaps(), get_egbase(L).map())));
	return 1;
}

/* RST
   .. method:: has_max_caps(capname)

      Returns :const:`true` if the field has this maximum caps (not taking immovables into account)
      associated with it, otherwise returns :const:`false`.

      :arg capname: For possible values see :meth:`has_caps`
      :type capname: :class:`string`

*/
int LuaField::has_max_caps(lua_State* L) {
	const Widelands::FCoords& f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);
	lua_pushboolean(
	   L, static_cast<int>(check_has_caps(L, query, f, f.field->maxcaps(), get_egbase(L).map())));
	return 1;
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
/* R#S#T
   .. method:: indicate(on)

      Show/Hide an arrow that points to this field. You can only point to 1 field at the same time.

      :arg on: Whether to show or hide the arrow
      :type on: :class:`boolean`
*/
// UNTESTED
int LuaField::indicate(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Expected 1 boolean");
	}

	InteractivePlayer* ipl = dynamic_cast<InteractivePlayer*>(get_egbase(L).get_ibase());
	if (ipl == nullptr) {
		report_error(L, "This can only be called when there's an interactive player");
	}

	const bool on = luaL_checkboolean(L, -1);
	if (on) {
		ipl->set_training_wheel_indicator_field(fcoords(L));
	} else {
		ipl->set_training_wheel_indicator_field(
		   Widelands::FCoords(Widelands::FCoords::null(), nullptr));
	}
	return 2;
}
#endif

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
int LuaField::region(lua_State* L, uint32_t radius) {
	const Widelands::Map& map = get_egbase(L).map();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   map, Widelands::Area<Widelands::FCoords>(fcoords(L), radius));

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		const Widelands::FCoords& loc = mr.location();
		to_lua<LuaField>(L, new LuaField(loc.x, loc.y));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

int LuaField::hollow_region(lua_State* L, uint32_t radius, uint32_t inner_radius) {
	const Widelands::Map& map = get_egbase(L).map();
	Widelands::HollowArea<Widelands::Area<>> har(Widelands::Area<>(coords_, radius), inner_radius);

	Widelands::MapHollowRegion<Widelands::Area<>> mr(map, har);

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		to_lua<LuaField>(L, new LuaField(mr.location()));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

const Widelands::FCoords LuaField::fcoords(lua_State* L) {
	return get_egbase(L).map().get_fcoords(coords_);
}

}  // namespace LuaMaps
