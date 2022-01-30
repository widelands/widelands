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

#include "logic/map.h"

#include <cstdlib>
#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "base/scoped_timer.h"
#include "base/string.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/roadbase.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/filesystem_constants.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/mapfringeregion.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/note_map_options.h"
#include "logic/objective.h"
#include "logic/pathfield.h"
#include "map_io/s2map.h"
#include "map_io/widelands_map_loader.h"
#include "notifications/notifications.h"

namespace Widelands {

const std::vector<Map::OldWorldInfo> Map::kOldWorldNames = {
   /** TRANSLATORS: A world name for the random map generator in the editor */
   {"summer", "greenland", []() { return _("Summer"); }},
   /** TRANSLATORS: A world name for the random map generator in the editor */
   {"winter", "winterland", []() { return _("Winter"); }},
   /** TRANSLATORS: A world name for the random map generator in the editor */
   {"wasteland", "blackland", []() { return _("Wasteland"); }},
   /** TRANSLATORS: A world name for the random map generator in the editor */
   {"desert", "desert", []() { return _("Desert"); }}};
const Map::OldWorldInfo& Map::get_old_world_info_by_old_name(const std::string& old_name) {
	for (const OldWorldInfo& owi : kOldWorldNames) {
		if (owi.old_name == old_name) {
			return owi;
		}
	}
	NEVER_HERE();
}
const Map::OldWorldInfo& Map::get_old_world_info_by_new_name(const std::string& new_name) {
	for (const OldWorldInfo& owi : kOldWorldNames) {
		if (owi.name == new_name) {
			return owi;
		}
	}
	NEVER_HERE();
}

FieldData::FieldData(const Field& field)
   : height(field.get_height()),
     resources(field.get_resources()),
     resource_amount(field.get_initial_res_amount()),
     terrains(field.get_terrains()) {
	if (const BaseImmovable* imm = field.get_immovable()) {
		immovable = imm->descr().name();
	} else {
		immovable = "";
	}
	for (Bob* bob = field.get_first_bob(); bob; bob = bob->get_next_bob()) {
		bobs.push_back(bob->descr().name());
	}
}

// static
FindCritterByClass::Class FindCritterByClass::classof(const CritterDescr& cd) {
	return cd.is_herbivore() ? cd.is_carnivore() ? Class::Neither : Class::Herbivore :
	       cd.is_carnivore() ? Class::Carnivore :
                              Class::Neither;
}
bool FindCritterByClass::accept(Bob* b) const {
	if (upcast(const Critter, c, b)) {
		return classof(c->descr()) == class_;
	}
	return false;
}
bool FindCarnivores::accept(Bob* b) const {
	if (upcast(const Critter, c, b)) {
		return c->descr().is_carnivore();
	}
	return false;
}
bool FindCritter::accept(Bob* b) const {
	return b && b->descr().type() == MapObjectType::CRITTER;
}
bool FindBobByName::accept(Bob* b) const {
	assert(b);
	return b->descr().name() == name_;
}

/*
==============================================================================

Map IMPLEMENTATION

==============================================================================
*/

/** class Map
 *
 * This really identifies a map like it is in the game
 */

Map::Map()
   : nrplayers_(0),
     scenario_types_(NO_SCENARIO),
     width_(0),
     height_(0),
     localize_author_(false),
     pathfieldmgr_(new PathfieldManager),
     allows_seafaring_(false),
     waterway_max_length_(0) {
}

Map::~Map() {
	cleanup();
}

void Map::recalc_border(const FCoords& fc) {
	if (const PlayerNumber owner = fc.field->get_owned_by()) {
		//  A node that is owned by a player and has a neighbour that is not owned
		//  by that player is a border node.
		for (uint8_t i = 1; i <= 6; ++i) {
			FCoords neighbour;
			get_neighbour(fc, i, &neighbour);
			if (neighbour.field->get_owned_by() != owner) {
				fc.field->set_border(true);
				return;  //  Do not calculate further if there is a border.
			}
		}
	}
	fc.field->set_border(false);
}

/*
===============
Call this function whenever the field at fx/fy has changed in one of the ways:
 - height has changed
 - robust MapObject has been added or removed

This performs the steps outlined in the comment above Map::recalc_brightness()
and recalcs the interactive player's overlay.
===============
*/
void Map::recalc_for_field_area(const EditorGameBase& egbase, const Area<FCoords> area) {
	assert(0 <= area.x);
	assert(area.x < width_);
	assert(0 <= area.y);
	assert(area.y < height_);
	assert(fields_.get() <= area.field);
	assert(area.field < fields_.get() + max_index());

	{  //  First pass.
		MapRegion<Area<FCoords>> mr(*this, area);
		do {
			recalc_brightness(mr.location());
			recalc_border(mr.location());
			recalc_nodecaps_pass1(egbase, mr.location());
		} while (mr.advance(*this));
	}

	{  //  Second pass.
		MapRegion<Area<FCoords>> mr(*this, area);
		do {
			recalc_nodecaps_pass2(egbase, mr.location());
		} while (mr.advance(*this));
	}
}

/*
===========

this recalculates all data that needs to be recalculated.
This is only needed when all fields have change, this means
a map has been loaded or newly created or in the editor that
the overlays have completely changed.
===========
*/
void Map::recalc_whole_map(const EditorGameBase& egbase) {
	//  Post process the map in the necessary two passes to calculate
	//  brightness and building caps
	FCoords f;

	for (int16_t y = 0; y < height_; ++y) {
		for (int16_t x = 0; x < width_; ++x) {
			f = get_fcoords(Coords(x, y));
			uint32_t radius = 0;
			check_neighbour_heights(f, radius);
			recalc_brightness(f);
			recalc_border(f);
			recalc_nodecaps_pass1(egbase, f);
		}
	}

	for (int16_t y = 0; y < height_; ++y) {
		for (int16_t x = 0; x < width_; ++x) {
			f = get_fcoords(Coords(x, y));
			recalc_nodecaps_pass2(egbase, f);
		}
	}
	recalculate_allows_seafaring();
}

void Map::recalc_default_resources(const Descriptions& descriptions) {
	for (int16_t y = 0; y < height_; ++y) {
		for (int16_t x = 0; x < width_; ++x) {
			FCoords f, f1;
			f = get_fcoords(Coords(x, y));
			//  only on unset nodes
			if (f.field->get_resources() != Widelands::kNoResource ||
			    f.field->get_resources_amount()) {
				continue;
			}
			std::map<int32_t, int32_t> m;
			ResourceAmount amount = 0;

			//  this node
			{
				const TerrainDescription* terr = descriptions.get_terrain_descr(f.field->terrain_r());
				++m[terr->get_default_resource()];
				amount += terr->get_default_resource_amount();
			}
			{
				const TerrainDescription* terd = descriptions.get_terrain_descr(f.field->terrain_d());
				++m[terd->get_default_resource()];
				amount += terd->get_default_resource_amount();
			}

			//  If one of the neighbours is unwalkable, count its resource
			//  stronger
			//  top left neigbour
			get_neighbour(f, WALK_NW, &f1);
			{
				const TerrainDescription* terr = descriptions.get_terrain_descr(f1.field->terrain_r());
				const DescriptionIndex resr = terr->get_default_resource();
				const ResourceAmount default_amount = terr->get_default_resource_amount();
				if ((terr->get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0) {
					m[resr] += 3;
				} else {
					++m[resr];
				}
				amount += default_amount;
			}
			{
				const TerrainDescription* terd = descriptions.get_terrain_descr(f1.field->terrain_d());
				const DescriptionIndex resd = terd->get_default_resource();
				const ResourceAmount default_amount = terd->get_default_resource_amount();
				if ((terd->get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0) {
					m[resd] += 3;
				} else {
					++m[resd];
				}
				amount += default_amount;
			}

			//  top right neigbour
			get_neighbour(f, WALK_NE, &f1);
			{
				const TerrainDescription* terd = descriptions.get_terrain_descr(f1.field->terrain_d());
				const DescriptionIndex resd = terd->get_default_resource();
				const ResourceAmount default_amount = terd->get_default_resource_amount();
				if ((terd->get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0) {
					m[resd] += 3;
				} else {
					++m[resd];
				}
				amount += default_amount;
			}

			//  left neighbour
			get_neighbour(f, WALK_W, &f1);
			{
				const TerrainDescription* terr = descriptions.get_terrain_descr(f1.field->terrain_r());
				const DescriptionIndex resr = terr->get_default_resource();
				const ResourceAmount default_amount = terr->get_default_resource_amount();
				if ((terr->get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0) {
					m[resr] += 3;
				} else {
					++m[resr];
				}
				amount += default_amount;
			}

			int32_t lv = 0;
			int32_t res = 0;
			std::map<int32_t, int32_t>::iterator i = m.begin();
			while (i != m.end()) {
				if (i->second > lv) {
					lv = i->second;
					res = i->first;
				}
				++i;
			}
			amount /= 6;

			if (res == -1 || res == INVALID_INDEX || res == Widelands::kNoResource || !amount) {
				clear_resources(f);
			} else {
				initialize_resources(f, res, amount);
			}
		}
	}
}

size_t Map::count_all_conquerable_fields() {
	if (!valuable_fields_.empty()) {
		// Already calculated
		return valuable_fields_.size();
	}

	std::set<FCoords> coords_to_check;

	verb_log_info("Collecting valuable fields ... ");
	ScopedTimer timer(" → took %ums", true);

	// If we don't have the given coordinates yet, walk the map and collect conquerable fields,
	// initialized with the given radius around the coordinates
	const auto walk_starting_coords = [this, &coords_to_check](const Coords& coords, int radius) {
		FCoords fcoords = get_fcoords(coords);

		// We already have these coordinates
		if (valuable_fields_.count(fcoords) == 1) {
			return;
		}

		// Add starting field
		valuable_fields_.insert(fcoords);

		// Add outer land coordinates around the starting field for the given radius
		std::unique_ptr<Widelands::HollowArea<>> hollow_area(
		   new Widelands::HollowArea<>(Widelands::Area<>(fcoords, radius), 2));
		std::unique_ptr<Widelands::MapHollowRegion<>> map_region(
		   new Widelands::MapHollowRegion<>(*this, *hollow_area));
		do {
			coords_to_check.insert(get_fcoords(map_region->location()));
		} while (map_region->advance(*this));

		// Walk the map
		while (!coords_to_check.empty()) {
			// Get some coordinates to check
			const auto coords_it = coords_to_check.begin();
			fcoords = *coords_it;

			// Get region according to buildcaps
			radius = 0;
			int inner_radius = 2;
			if ((fcoords.field->maxcaps() & BUILDCAPS_BIG) == BUILDCAPS_BIG) {
				radius = 9;
				inner_radius = 7;
			} else if (fcoords.field->maxcaps() & BUILDCAPS_MEDIUM) {
				radius = 7;
				inner_radius = 5;
			} else if (fcoords.field->maxcaps() & BUILDCAPS_SMALL) {
				radius = 5;
			}

			// Check region and add walkable fields
			if (radius > 0) {
				hollow_area.reset(
				   new Widelands::HollowArea<>(Widelands::Area<>(fcoords, radius), inner_radius));
				map_region.reset(new Widelands::MapHollowRegion<>(*this, *hollow_area));
				do {
					fcoords = get_fcoords(map_region->location());

					// We do the caps check first, because the comparison is faster than the container
					// check
					if ((fcoords.field->maxcaps() & MOVECAPS_WALK) &&
					    (valuable_fields_.count(fcoords) == 0)) {
						valuable_fields_.insert(fcoords);
						coords_to_check.insert(fcoords);
					}
				} while (map_region->advance(*this));
			}

			// These coordinates are done. We do not keep track of visited coordinates that didn't make
			// the result, because the container insert operations are more expensive than the checks
			coords_to_check.erase(coords_it);
		}
	};

	// Walk the map from the starting field of each player
	for (const Coords& coords : starting_pos_) {
		walk_starting_coords(coords, 9);
	}

	// Walk the map from port spaces
	if (allows_seafaring()) {
		for (const Coords& coords : get_port_spaces()) {
			walk_starting_coords(coords, 5);
		}
	}

	verb_log_info("%" PRIuS " found ... ", valuable_fields_.size());
	return valuable_fields_.size();
}

size_t Map::count_all_fields_excluding_caps(NodeCaps caps) {
	if (!valuable_fields_.empty()) {
		// Already calculated
		return valuable_fields_.size();
	}

	verb_log_info("Collecting valuable fields ... ");
	ScopedTimer timer(" → took %ums", true);

	for (MapIndex i = 0; i < max_index(); ++i) {
		Field& field = fields_[i];
		if (!(field.nodecaps() & caps)) {
			valuable_fields_.insert(get_fcoords(field));
		}
	}

	verb_log_info("%" PRIuS " found ... ", valuable_fields_.size());
	return valuable_fields_.size();
}

std::map<PlayerNumber, size_t>
Map::count_owned_valuable_fields(const std::string& immovable_attribute) const {
	std::map<PlayerNumber, size_t> result;
	const bool use_attribute = !immovable_attribute.empty();
	const uint32_t attribute_id =
	   use_attribute ? MapObjectDescr::get_attribute_id(immovable_attribute) : 0U;
	for (const FCoords& fcoords : valuable_fields_) {
		if (use_attribute) {
			const BaseImmovable* imm = fcoords.field->get_immovable();
			if (imm != nullptr && imm->has_attribute(attribute_id)) {
				++result[fcoords.field->get_owned_by()];
			}
		} else {
			++result[fcoords.field->get_owned_by()];
		}
	}
	return result;
}

/*
===============
remove your world, remove your data
go back to your initial state
===============
*/
void Map::cleanup() {
	nrplayers_ = 0;
	width_ = height_ = 0;

	fields_.reset();

	starting_pos_.clear();
	scenario_tribes_.clear();
	scenario_names_.clear();
	scenario_ais_.clear();
	scenario_closeables_.clear();

	tags_.clear();
	hint_ = std::string();
	background_ = std::string();
	background_theme_ = std::string();

	objectives_.clear();
	port_spaces_.clear();
	allows_seafaring_ = false;

	// TODO(meitis): should be done here ... but WidelandsMapLoader::preload_map calls
	// this cleanup AFTER assigning filesystem_ in WidelandsMapLoader::WidelandsMapLoader
	// ... so we can't do it here :/
	// filesystem_.reset(nullptr);
}

/*
===========
creates an empty map without name with
the given data
===========
*/
void Map::create_empty_map(const EditorGameBase& egbase,
                           uint32_t const w,
                           uint32_t const h,
                           const Widelands::DescriptionIndex default_terrain,
                           const std::string& name,
                           const std::string& author,
                           const std::string& description) {
	set_size(w, h);
	set_name(name);
	set_author(author);
	set_localize_author(false);  // no author i18n markup by default
	set_description(description);
	set_nrplayers(0);
	{
		Field::Terrains default_terrains;
		default_terrains.d = default_terrain;
		default_terrains.r = default_terrain;
		for (int16_t y = 0; y < height_; ++y) {
			for (int16_t x = 0; x < width_; ++x) {
				auto f = get_fcoords(Coords(x, y));
				f.field->set_height(10);
				f.field->set_terrains(default_terrains);
				clear_resources(f);
			}
		}
	}
	recalc_whole_map(egbase);

	filesystem_.reset(nullptr);
}

void Map::set_origin(const Coords& new_origin) {
	assert(0 <= new_origin.x);
	assert(new_origin.x < width_);
	assert(0 <= new_origin.y);
	assert(new_origin.y < height_);

	const size_t field_size = width_ * height_;

	for (uint8_t i = get_nrplayers(); i;) {
		starting_pos_[--i].reorigin(new_origin, extent());
	}

	std::unique_ptr<Field[]> new_field_order(new Field[field_size]());

	// Rearrange The fields
	// NOTE because of the triangle design, we have to take special care of cases
	// NOTE where y is changed by an odd number
	bool yisodd = (new_origin.y % 2) != 0;
	for (Coords c(Coords(0, 0)); c.y < height_; ++c.y) {
		bool cyisodd = (c.y % 2) != 0;
		for (c.x = 0; c.x < width_; ++c.x) {
			Coords temp;
			if (yisodd && cyisodd) {
				temp = Coords(c.x + new_origin.x + 1, c.y + new_origin.y);
			} else {
				temp = Coords(c.x + new_origin.x, c.y + new_origin.y);
			}
			normalize_coords(temp);
			new_field_order[get_index(c, width_)] = operator[](temp);
		}
	}
	// Now that we restructured the fields, we just overwrite the old order
	fields_ = std::move(new_field_order);

	//  Inform immovables and bobs about their new coordinates.
	for (FCoords c(Coords(0, 0), fields_.get()); c.y < height_; ++c.y) {
		for (c.x = 0; c.x < width_; ++c.x, ++c.field) {
			assert(c.field == &operator[](c));
			if (upcast(Immovable, immovable, c.field->get_immovable())) {
				immovable->position_ = c;
			}
			bool is_first_bob = true;
			for (Bob* bob = c.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
				bob->position_.x = c.x;
				bob->position_.y = c.y;
				bob->position_.field = c.field;
				if (is_first_bob) {
					bob->linkpprev_ = &c.field->bobs;
					is_first_bob = false;
				}
			}
		}
	}

	// Take care of port spaces
	PortSpacesSet new_port_spaces;
	for (const Coords& space : port_spaces_) {
		Coords temp;
		if (yisodd && ((space.y % 2) == 0)) {
			temp = Coords(space.x - new_origin.x - 1, space.y - new_origin.y);
		} else {
			temp = Coords(space.x - new_origin.x, space.y - new_origin.y);
		}
		normalize_coords(temp);
		new_port_spaces.insert(temp);
	}
	port_spaces_ = new_port_spaces;
	verb_log_info("Map origin was shifted by (%d, %d)\n", new_origin.x, new_origin.y);
}

// Helper function for resize()
constexpr int32_t kInvalidCoords = -1;
static inline int32_t resize_coordinates_conversion(const int32_t old_coord,
                                                    const int32_t split_point,
                                                    const int32_t old_dimension,
                                                    const int32_t new_dimension) {
	if (new_dimension == old_dimension) {
		// trivial
		return old_coord;
	}
	if (new_dimension > old_dimension) {
		// enlarge
		return old_coord > split_point ? old_coord + new_dimension - old_dimension : old_coord;
	}
	if (split_point > new_dimension) {
		// shrink, origin deleted
		return (old_coord >= split_point || old_coord < split_point - new_dimension) ?
                kInvalidCoords :
                old_coord - split_point + new_dimension;
	}
	// shrink, origin preserved
	return old_coord < split_point ? old_coord :
	       old_coord < split_point + old_dimension - new_dimension ?
                                    kInvalidCoords :
                                    old_coord + new_dimension - old_dimension;
}

void Map::resize(EditorGameBase& egbase, const Coords split, const int32_t w, const int32_t h) {
	if (w == width_ && h == height_) {
		return;
	}

	// Generate the new fields. Does not modify the actual map yet.

	std::unique_ptr<Field[]> new_fields(new Field[w * h]());
	std::unique_ptr<bool[]> was_preserved(new bool[width_ * height_]());
	std::unique_ptr<bool[]> was_created(new bool[w * h]());

	for (int16_t x = 0; x < width_; ++x) {
		for (int16_t y = 0; y < height_; ++y) {
			const int16_t new_x = resize_coordinates_conversion(x, split.x, width_, w);
			const int16_t new_y = resize_coordinates_conversion(y, split.y, height_, h);
			assert((new_x >= 0 && new_x < w) || new_x == kInvalidCoords);
			assert((new_y >= 0 && new_y < h) || new_y == kInvalidCoords);
			if (new_x != kInvalidCoords && new_y != kInvalidCoords) {
				Coords old_coords(x, y);
				Coords new_coords(new_x, new_y);
				const MapIndex old_index = get_index(old_coords);
				const MapIndex new_index = get_index(new_coords, w);

				assert(!was_preserved[old_index]);
				was_preserved[old_index] = true;

				assert(!was_created[new_index]);
				was_created[new_index] = true;

				new_fields[new_index] = (*this)[old_coords];
			}
		}
	}
	Field::Terrains default_terrains;
	default_terrains.r = 0;
	default_terrains.d = 0;
	for (MapIndex index = w * h; index; --index) {
		if (!was_created[index - 1]) {
			Field& field = new_fields[index - 1];
			field.set_height(10);
			field.set_terrains(default_terrains);
		}
	}

	// Now modify our starting positions and port spaces

	for (Coords& c : starting_pos_) {
		if (c) {  // only if set (c != Coords::null())
			const int16_t x = resize_coordinates_conversion(c.x, split.x, width_, w);
			const int16_t y = resize_coordinates_conversion(c.y, split.y, height_, h);
			assert((x >= 0 && x < w) || x == kInvalidCoords);
			assert((y >= 0 && y < h) || y == kInvalidCoords);
			c = ((x == kInvalidCoords || y == kInvalidCoords) ? Coords::null() : Coords(x, y));
		}
	}

	{
		PortSpacesSet new_port_spaces;
		for (const Coords& c : port_spaces_) {
			const int16_t x = resize_coordinates_conversion(c.x, split.x, width_, w);
			const int16_t y = resize_coordinates_conversion(c.y, split.y, height_, h);
			assert((x >= 0 && x < w) || x == kInvalidCoords);
			assert((y >= 0 && y < h) || y == kInvalidCoords);
			if (x != kInvalidCoords && y != kInvalidCoords) {
				new_port_spaces.insert(Coords(x, y));
			}
		}
		port_spaces_ = new_port_spaces;
	}

	// Delete map objects whose position will be deleted.
	for (int16_t x = 0; x < width_; ++x) {
		for (int16_t y = 0; y < height_; ++y) {
			Coords c(x, y);
			if (!was_preserved[get_index(c)]) {
				Field& f = (*this)[c];
				if (upcast(Immovable, i, f.get_immovable())) {
					i->remove(egbase);
				}
				while (Bob* b = f.get_first_bob()) {
					b->remove(egbase);
				}
			}
		}
	}

	// Now replace all existing fields with the new values.

	width_ = w;
	height_ = h;
	fields_ = std::move(new_fields);

	// Always call allocate_player_maps() while changing the map's size.
	// Forgetting to do so will result in random crashes.
	egbase.allocate_player_maps();
	// Recalculate nodecaps etc
	recalc_whole_map(egbase);

	// MapObjects keep pointers to the Field they are located on. These pointers
	// need updating because the memory addresses of all fields changed now.
	for (int16_t x = 0; x < width_; ++x) {
		for (int16_t y = 0; y < height_; ++y) {
			Field& f = (*this)[Coords(x, y)];
			FCoords fc(Coords(x, y), &f);
			if (upcast(Immovable, imm, f.get_immovable())) {
				imm->position_ = fc;
			}
			if (Bob* b = f.get_first_bob()) {
				b->linkpprev_ = &f.bobs;
			}
			for (Bob* b = f.get_first_bob(); b; b = b->get_next_bob()) {
				b->position_ = fc;
			}
		}
	}

	verb_log_info("Map was resized to %d×%d\n", width_, height_);
}

ResizeHistory Map::dump_state(const EditorGameBase&) const {
	ResizeHistory rh;
	rh.size.w = width_;
	rh.size.h = height_;
	rh.port_spaces = port_spaces_;
	rh.starting_positions = starting_pos_;
	for (MapIndex i = max_index(); i; --i) {
		rh.fields.push_back(FieldData(operator[](i - 1)));
	}
	return rh;
}

void Map::set_to(EditorGameBase& egbase, ResizeHistory rh) {
	std::list<FieldData> backup = rh.fields;

	// Delete all map objects
	for (int16_t x = 0; x < width_; ++x) {
		for (int16_t y = 0; y < height_; ++y) {
			Field& f = operator[](Coords(x, y));
			if (upcast(Immovable, i, f.get_immovable())) {
				i->remove(egbase);
			}
			while (Bob* b = f.get_first_bob()) {
				b->remove(egbase);
			}
		}
	}

	// Reset the fields to blank
	width_ = rh.size.w;
	height_ = rh.size.h;
	fields_.reset(new Field[width_ * height_]);
	egbase.allocate_player_maps();

	// Overwrite starting locations and port spaces
	port_spaces_ = rh.port_spaces;
	starting_pos_ = rh.starting_positions;

	// First pass: Initialize all fields with the saved basic data
	for (MapIndex i = max_index(); i; --i) {
		const FieldData& fd = rh.fields.front();
		Field& f = fields_[i - 1];
		f.set_terrains(fd.terrains);
		f.set_height(fd.height);
		f.resources = fd.resources;
		f.initial_res_amount = fd.resource_amount;
		f.res_amount = fd.resource_amount;
		rh.fields.pop_front();
	}
	// Calculate nodecaps and stuff
	recalc_whole_map(egbase);

	// Second pass: Re-create desired map objects
	for (MapIndex i = max_index(); i; --i) {
		const FieldData& fd = backup.front();
		FCoords fc = get_fcoords(operator[](i - 1));
		if (!fd.immovable.empty()) {
			egbase.create_immovable_with_name(fc, fd.immovable, nullptr, nullptr);
		}
		for (const std::string& bob : fd.bobs) {
			egbase.create_critter(fc, bob);
		}
		backup.pop_front();
	}
}

/*
===============
Set the size of the map. This should only happen once during initial load.
===============
*/
void Map::set_size(const uint32_t w, const uint32_t h) {
	assert(!fields_);

	width_ = w;
	height_ = h;

	const uint32_t field_size = w * h;

	fields_.reset(new Field[field_size]());

	pathfieldmgr_->set_size(field_size);
}

const std::string& Map::minimum_required_widelands_version() const {
	return map_version_.minimum_required_widelands_version;
}

void Map::calculate_minimum_required_widelands_version(bool is_post_one_world) {
	if (map_version_.minimum_required_widelands_version.empty()) {
		if (nrplayers_ > 8) {
			// We introduced support for 16 players after Build 19
			map_version_.minimum_required_widelands_version = "build 20";
		} else if (is_post_one_world) {
			// We merged the worlds in the engine after Build 18
			map_version_.minimum_required_widelands_version = "build 19";
		}
	}
}

/*
 * Getter and setter for the highest permitted length of a waterway on this map.
 * A value of 0 or 1 means no waterways can be built.
 */
uint32_t Map::get_waterway_max_length() const {
	return waterway_max_length_;
}

void Map::set_waterway_max_length(uint32_t max_length) {
	waterway_max_length_ = max_length;
}

/*
 * The scenario get/set functions
 */
const std::string& Map::get_scenario_player_tribe(const PlayerNumber p) const {
	assert(scenario_tribes_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_tribes_[p - 1];
}

const std::string& Map::get_scenario_player_name(const PlayerNumber p) const {
	assert(scenario_names_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_names_[p - 1];
}

const std::string& Map::get_scenario_player_ai(const PlayerNumber p) const {
	assert(scenario_ais_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_ais_[p - 1];
}

bool Map::get_scenario_player_closeable(const PlayerNumber p) const {
	assert(scenario_closeables_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_closeables_[p - 1];
}

void Map::swap_filesystem(std::unique_ptr<FileSystem>& fs) {
	filesystem_.swap(fs);
}

void Map::reset_filesystem() {
	filesystem_.reset();
}

FileSystem* Map::filesystem() const {
	return filesystem_.get();
}

void Map::set_scenario_player_tribe(PlayerNumber const p, const std::string& tribename) {
	assert(p);
	assert(p <= get_nrplayers());
	scenario_tribes_.resize(get_nrplayers());
	scenario_tribes_[p - 1] = tribename;
}

void Map::set_scenario_player_name(PlayerNumber const p, const std::string& playername) {
	assert(p);
	assert(p <= get_nrplayers());
	scenario_names_.resize(get_nrplayers());
	scenario_names_[p - 1] = playername;
}

void Map::set_scenario_player_ai(PlayerNumber const p, const std::string& ainame) {
	assert(p);
	assert(p <= get_nrplayers());
	scenario_ais_.resize(get_nrplayers());
	scenario_ais_[p - 1] = ainame;
}

void Map::set_scenario_player_closeable(PlayerNumber const p, bool closeable) {
	assert(p);
	assert(p <= get_nrplayers());
	scenario_closeables_.resize(get_nrplayers());
	scenario_closeables_[p - 1] = closeable;
}

/*
===============
Change the number of players the map supports.
Could happen multiple times in the map editor.
===============
*/
void Map::set_nrplayers(PlayerNumber const nrplayers) {
	if (!nrplayers) {
		nrplayers_ = 0;
		return;
	}

	starting_pos_.resize(nrplayers, Coords::null());
	scenario_tribes_.resize(nrplayers);
	scenario_ais_.resize(nrplayers);
	scenario_closeables_.resize(nrplayers);
	scenario_names_.resize(nrplayers);
	scenario_tribes_.resize(nrplayers);

	nrplayers_ = nrplayers;  // in case the number players got less
}

/*
===============
Set the starting coordinates of a player
===============
*/
void Map::set_starting_pos(PlayerNumber const plnum, const Coords& c) {
	assert(1 <= plnum && plnum <= get_nrplayers());
	starting_pos_[plnum - 1] = c;
}

void Map::set_filename(const std::string& filename) {
	filename_ = filename;
}

void Map::set_author(const std::string& author) {
	author_ = author;
}

void Map::set_localize_author(const bool l) {
	localize_author_ = l;
}

void Map::set_name(const std::string& name) {
	name_ = name;
}

void Map::set_description(const std::string& description) {
	description_ = description;
}

void Map::set_hint(const std::string& hint) {
	hint_ = hint;
}

void Map::set_background(const std::string& image_path) {
	if (image_path.empty()) {
		background_.clear();
	} else {
		background_ = image_path;
	}
}

void Map::set_background_theme(const std::string& bt) {
	background_theme_ = bt;
}

void Map::add_tag(const std::string& tag) {
	tags_.insert(tag);
}

void Map::delete_tag(const std::string& tag) {
	if (has_tag(tag)) {
		tags_.erase(tags_.find(tag));
	}
}

NodeCaps Map::get_max_nodecaps(const EditorGameBase& egbase, const FCoords& fc) const {
	NodeCaps max_caps = calc_nodecaps_pass1(egbase, fc, false);
	max_caps = calc_nodecaps_pass2(egbase, fc, false, max_caps);
	return static_cast<NodeCaps>(max_caps);
}

/// \returns the immovable at the given coordinate
BaseImmovable* Map::get_immovable(const Coords& coord) const {
	return operator[](coord).get_immovable();
}

/*
===============
Call the functor for every field that can be reached from coord without moving
outside the given radius.

Functor is of the form: functor(Map*, FCoords)
===============
*/
template <typename functorT>
void Map::find_reachable(const EditorGameBase& egbase,
                         const Area<FCoords>& area,
                         const CheckStep& checkstep,
                         functorT& functor) const {
	std::vector<Coords> queue;
	std::shared_ptr<Pathfields> pathfields = pathfieldmgr_->allocate();

	queue.push_back(area);

	while (!queue.empty()) {
		// Pop the last ware from the queue
		FCoords const cur = get_fcoords(*queue.rbegin());
		queue.pop_back();
		Pathfield& curpf = pathfields->fields[cur.field - fields_.get()];

		//  handle this node
		functor(egbase, cur);
		curpf.cycle = pathfields->cycle;

		// Get neighbours
		for (Direction dir = 1; dir <= 6; ++dir) {
			FCoords neighb;

			get_neighbour(cur, dir, &neighb);

			if  //  node not already handled?
			   (pathfields->fields[neighb.field - fields_.get()].cycle != pathfields->cycle &&
			    //  node within the radius?
			    calc_distance(area, neighb) <= area.radius &&
			    //  allowed to move onto this node?
			    checkstep.allowed(*this, cur, neighb, dir,
			                      cur == area ? CheckStep::stepFirst : CheckStep::stepNormal)) {
				queue.push_back(neighb);
			}
		}
	}
}

/*
===============
Call the functor for every field within the given radius.

Functor is of the form: functor(Map &, FCoords)
===============
*/
template <typename functorT>
void Map::find(const EditorGameBase& egbase, const Area<FCoords>& area, functorT& functor) const {
	MapRegion<Area<FCoords>> mr(*this, area);
	do {
		functor(egbase, mr.location());
	} while (mr.advance(*this));
}

/*
===============
FindBobsCallback

The actual logic behind find_bobs and find_reachable_bobs.
===============
*/
struct FindBobsCallback {
	FindBobsCallback(std::vector<Bob*>* const list, const FindBob& functor)
	   : list_(list), functor_(functor), found_(0) {
	}

	void operator()(const EditorGameBase&, const FCoords& cur) {
		for (Bob* bob = cur.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
			if (list_ && std::find(list_->begin(), list_->end(), bob) != list_->end()) {
				continue;
			}

			if (functor_.accept(bob)) {
				if (list_) {
					list_->push_back(bob);
				}
				++found_;
			}
		}
	}

	std::vector<Bob*>* list_;
	const FindBob& functor_;
	uint32_t found_;
};

/*
===============
Find Bobs in the given area. Only finds objects for which
functor.accept() returns true (the default functor always returns true)
If list is non-zero, pointers to the relevant objects will be stored in
the list.

Returns the number of objects found.
===============
*/
uint32_t Map::find_bobs(const EditorGameBase& egbase,
                        Area<FCoords> const area,
                        std::vector<Bob*>* const list,
                        const FindBob& functor) const {
	FindBobsCallback cb(list, functor);

	find(egbase, area, cb);

	return cb.found_;
}

/*
===============
Find Bobs that are reachable by moving within the given radius (also see
find_reachable()).
Only finds objects for which functor.accept() returns true (the default functor
always returns true).
If list is non-zero, pointers to the relevant objects will be stored in
the list.

Returns the number of objects found.
===============
*/
uint32_t Map::find_reachable_bobs(const EditorGameBase& egbase,
                                  Area<FCoords> const area,
                                  std::vector<Bob*>* const list,
                                  const CheckStep& checkstep,
                                  const FindBob& functor) const {
	FindBobsCallback cb(list, functor);

	find_reachable(egbase, area, checkstep, cb);

	return cb.found_;
}

/*
===============
FindImmovablesCallback

The actual logic behind find_immovables and find_reachable_immovables.
===============
*/
struct FindImmovablesCallback {
	FindImmovablesCallback(std::vector<ImmovableFound>* const list, const FindImmovable& functor)
	   : list_(list), functor_(functor), found_(0) {
	}

	void operator()(const EditorGameBase&, const FCoords& cur) {
		BaseImmovable* const imm = cur.field->get_immovable();

		if (!imm) {
			return;
		}

		if (functor_.accept(*imm)) {
			if (list_) {
				ImmovableFound imf;
				imf.object = imm;
				imf.coords = cur;
				list_->push_back(imf);
			}

			++found_;
		}
	}

	std::vector<ImmovableFound>* list_;
	const FindImmovable& functor_;
	uint32_t found_;
};

/*
===============
Find all immovables in the given area for which functor returns true
(the default functor always returns true).
Returns true if an immovable has been found.
If list is not 0, found immovables are stored in list.
===============
*/
uint32_t Map::find_immovables(const EditorGameBase& egbase,
                              Area<FCoords> const area,
                              std::vector<ImmovableFound>* const list,
                              const FindImmovable& functor) const {
	FindImmovablesCallback cb(list, functor);

	find(egbase, area, cb);

	return cb.found_;
}

/*
===============
Find all immovables reachable by moving in the given radius (see
find_reachable()).
Return immovables for which functor returns true (the default functor
always returns true).
If list is not 0, found immovables are stored in list.
Returns the number of immovables we found.
===============
*/
uint32_t Map::find_reachable_immovables(const EditorGameBase& egbase,
                                        Area<FCoords> const area,
                                        std::vector<ImmovableFound>* const list,
                                        const CheckStep& checkstep,
                                        const FindImmovable& functor) const {
	FindImmovablesCallback cb(list, functor);

	find_reachable(egbase, area, checkstep, cb);

	return cb.found_;
}

/**
 * Find all immovables that are reachable without moving out of the
 * given area with the additional constraints given by checkstep,
 * and store them uniquely in a list.
 *
 * \return the number of immovables found.
 */
uint32_t Map::find_reachable_immovables_unique(const EditorGameBase& egbase,
                                               const Area<FCoords> area,
                                               std::vector<BaseImmovable*>& list,
                                               const CheckStep& checkstep,
                                               const FindImmovable& functor) const {
	std::vector<ImmovableFound> duplist;
	FindImmovablesCallback cb(&duplist, find_immovable_always_true());

	find_reachable(egbase, area, checkstep, cb);

	for (ImmovableFound& imm_found : duplist) {
		BaseImmovable& obj = *imm_found.object;
		if (std::find(list.begin(), list.end(), &obj) == list.end()) {
			if (functor.accept(obj)) {
				list.push_back(&obj);
			}
		}
	}

	return list.size();
}

/*
===============
FindNodesCallback

The actual logic behind find_fields and find_reachable_fields.
===============
*/
struct FindNodesCallback {
	FindNodesCallback(std::vector<Coords>* const list, const FindNode& functor)
	   : list_(list), functor_(functor), found_(0) {
	}

	void operator()(const EditorGameBase& egbase, const FCoords& cur) {
		if (functor_.accept(egbase, cur)) {
			if (list_) {
				list_->push_back(cur);
			}
			++found_;
		}
	}

	std::vector<Coords>* list_;
	const FindNode& functor_;
	uint32_t found_;
};

/*
===============
Fills in a list of coordinates of fields within the given area that functor
accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
uint32_t Map::find_fields(const EditorGameBase& egbase,
                          Area<FCoords> const area,
                          std::vector<Coords>* list,
                          const FindNode& functor) const {
	FindNodesCallback cb(list, functor);

	find(egbase, area, cb);

	return cb.found_;
}

/*
===============
Fills in a list of coordinates of fields reachable by walking within the given
radius that functor accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
uint32_t Map::find_reachable_fields(const EditorGameBase& egbase,
                                    Area<FCoords> const area,
                                    std::vector<Coords>* list,
                                    const CheckStep& checkstep,
                                    const FindNode& functor) const {
	FindNodesCallback cb(list, functor);

	find_reachable(egbase, area, checkstep, cb);

	return cb.found_;
}

/*
Node attribute recalculation passes
------------------------------------

Some events can change the map in a way that run-time calculated attributes
(Field::brightness and Field::caps) need to be recalculated.

These events include:
- change of height (e.g. by planing)
- change of terrain (in the editor)
- insertion of a "robust" MapObject
- removal of a "robust" MapObject

All these events can change the passability, buildability, etc. of fields
with a radius of two fields. This means that you must build a list of the
directly affected field and all fields that can be reached in two steps.

You must then perform the following operations:
1. Call recalc_brightness() and recalc_nodecaps_pass1() on all nodes
2. Call recalc_nodecaps_pass2() on all fields

Note: it is possible to leave out recalc_brightness() unless the height has
been changed.

The Field::caps calculation is split into more passes because of inter-field
dependencies.
*/

/*
===============
Fetch the slopes to neighbours and call the actual logic in Field
===============
*/
void Map::recalc_brightness(const FCoords& f) {
	int32_t left, right, top_left, top_right, bottom_left, bottom_right;
	Field::Height const height = f.field->get_height();

	{
		FCoords neighbour;
		get_ln(f, &neighbour);
		left = height - neighbour.field->get_height();
	}
	{
		FCoords neighbour;
		get_rn(f, &neighbour);
		right = height - neighbour.field->get_height();
	}
	{
		FCoords neighbour;
		get_tln(f, &neighbour);
		top_left = height - neighbour.field->get_height();
		get_rn(neighbour, &neighbour);
		top_right = height - neighbour.field->get_height();
	}
	{
		FCoords neighbour;
		get_bln(f, &neighbour);
		bottom_left = height - neighbour.field->get_height();
		get_rn(neighbour, &neighbour);
		bottom_right = height - neighbour.field->get_height();
	}
	f.field->set_brightness(left, right, top_left, top_right, bottom_left, bottom_right);
}

/*
===============
Recalculate the caps for the given node.
 - Check terrain types for passability and flag buildability

I hope this is understandable and maintainable.

Note: due to inter-field dependencies, nodecaps calculations are split up
into two passes. You should always perform both passes. See the comment
above recalc_brightness.
===============
*/
void Map::recalc_nodecaps_pass1(const EditorGameBase& egbase, const FCoords& f) {
	f.field->caps = calc_nodecaps_pass1(egbase, f, true);
	f.field->max_caps = calc_nodecaps_pass1(egbase, f, false);
}

NodeCaps
Map::calc_nodecaps_pass1(const EditorGameBase& egbase, const FCoords& f, bool consider_mobs) const {
	uint8_t caps = CAPS_NONE;
	const Descriptions& descriptions = egbase.descriptions();

	// 1a) Get all the neighbours to make life easier
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords l = l_n(f);

	const TerrainDescription::Is tr_d_terrain_is =
	   descriptions.get_terrain_descr(tr.field->terrain_d())->get_is();
	const TerrainDescription::Is tl_r_terrain_is =
	   descriptions.get_terrain_descr(tl.field->terrain_r())->get_is();
	const TerrainDescription::Is tl_d_terrain_is =
	   descriptions.get_terrain_descr(tl.field->terrain_d())->get_is();
	const TerrainDescription::Is l_r_terrain_is =
	   descriptions.get_terrain_descr(l.field->terrain_r())->get_is();
	const TerrainDescription::Is f_d_terrain_is =
	   descriptions.get_terrain_descr(f.field->terrain_d())->get_is();
	const TerrainDescription::Is f_r_terrain_is =
	   descriptions.get_terrain_descr(f.field->terrain_r())->get_is();

	//  1b) Collect some information about the neighbours
	uint8_t cnt_unwalkable = 0;
	uint8_t cnt_water = 0;
	uint8_t cnt_unreachable = 0;

	if (tr_d_terrain_is & TerrainDescription::Is::kUnwalkable) {
		++cnt_unwalkable;
	}
	if (tl_r_terrain_is & TerrainDescription::Is::kUnwalkable) {
		++cnt_unwalkable;
	}
	if (tl_d_terrain_is & TerrainDescription::Is::kUnwalkable) {
		++cnt_unwalkable;
	}
	if (l_r_terrain_is & TerrainDescription::Is::kUnwalkable) {
		++cnt_unwalkable;
	}
	if (f_d_terrain_is & TerrainDescription::Is::kUnwalkable) {
		++cnt_unwalkable;
	}
	if (f_r_terrain_is & TerrainDescription::Is::kUnwalkable) {
		++cnt_unwalkable;
	}

	if (tr_d_terrain_is & TerrainDescription::Is::kWater) {
		++cnt_water;
	}
	if (tl_r_terrain_is & TerrainDescription::Is::kWater) {
		++cnt_water;
	}
	if (tl_d_terrain_is & TerrainDescription::Is::kWater) {
		++cnt_water;
	}
	if (l_r_terrain_is & TerrainDescription::Is::kWater) {
		++cnt_water;
	}
	if (f_d_terrain_is & TerrainDescription::Is::kWater) {
		++cnt_water;
	}
	if (f_r_terrain_is & TerrainDescription::Is::kWater) {
		++cnt_water;
	}

	if (tr_d_terrain_is & TerrainDescription::Is::kUnreachable) {
		++cnt_unreachable;
	}
	if (tl_r_terrain_is & TerrainDescription::Is::kUnreachable) {
		++cnt_unreachable;
	}
	if (tl_d_terrain_is & TerrainDescription::Is::kUnreachable) {
		++cnt_unreachable;
	}
	if (l_r_terrain_is & TerrainDescription::Is::kUnreachable) {
		++cnt_unreachable;
	}
	if (f_d_terrain_is & TerrainDescription::Is::kUnreachable) {
		++cnt_unreachable;
	}
	if (f_r_terrain_is & TerrainDescription::Is::kUnreachable) {
		++cnt_unreachable;
	}

	//  2) Passability

	//  2a) If any of the neigbouring triangles is walkable this node is
	//  walkable.
	if (cnt_unwalkable < 6) {
		caps |= MOVECAPS_WALK;
	}

	//  2b) If all neighbouring triangles are water, the node is swimmable.
	if (cnt_water == 6) {
		caps |= MOVECAPS_SWIM;
	}

	// 2c) [OVERRIDE] If any of the neighbouring triangles is really "bad" (such
	// as lava), we can neither walk nor swim to this node.
	if (cnt_unreachable) {
		caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
	}

	//  === everything below is used to check buildability ===

	// if we are interested in the maximum theoretically available NodeCaps, this is not run
	if (consider_mobs) {
		//  3) General buildability check: if a "robust" MapObject is on this node
		//  we cannot build anything on it. Exception: we can build flags on roads and waterways.
		if (BaseImmovable* const imm = get_immovable(f)) {
			if (!dynamic_cast<RoadBase const*>(imm) && imm->get_size() >= BaseImmovable::SMALL) {
				// 3b) [OVERRIDE] check for "unwalkable" MapObjects
				if (!imm->get_passable()) {
					caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
				}
				return static_cast<NodeCaps>(caps);
			}
		}
	}

	//  4) Flags
	//  We can build flags on anything that's walkable and buildable, with some
	//  restrictions
	if (caps & MOVECAPS_WALK) {
		//  4b) Flags must be at least 2 edges apart
		if (consider_mobs && find_immovables(egbase, Area<FCoords>(f, 1), nullptr,
		                                     FindImmovableType(MapObjectType::FLAG))) {
			return static_cast<NodeCaps>(caps);
		}
		caps |= BUILDCAPS_FLAG;
	}
	return static_cast<NodeCaps>(caps);
}

/*
===============
Second pass of nodecaps. Determine which kind of building (if any) can be built
on this Field.

Important: flag buildability has already been checked in the first pass.
===============
*/
void Map::recalc_nodecaps_pass2(const EditorGameBase& egbase, const FCoords& f) {
	f.field->caps = calc_nodecaps_pass2(egbase, f, true);
	f.field->max_caps =
	   calc_nodecaps_pass2(egbase, f, false, static_cast<NodeCaps>(f.field->max_caps));
}

NodeCaps Map::calc_nodecaps_pass2(const EditorGameBase& egbase,
                                  const FCoords& f,
                                  bool consider_mobs,
                                  NodeCaps initcaps) const {
	uint8_t caps = consider_mobs ? f.field->caps : static_cast<uint8_t>(initcaps);

	// NOTE  This dependency on the bottom-right neighbour is the reason
	// NOTE  why the caps calculation is split into two passes
	// NOTE  However this dependency has to be recalculated in case we are interested in the
	// NOTE  maximum possible NodeCaps for this FCoord
	const FCoords br = br_n(f);
	if (consider_mobs) {
		if (!(br.field->caps & BUILDCAPS_FLAG) &&
		    (!br.field->get_immovable() ||
		     br.field->get_immovable()->descr().type() != MapObjectType::FLAG)) {
			return static_cast<NodeCaps>(caps);
		}
	} else {
		if (!(calc_nodecaps_pass1(egbase, br, false) & BUILDCAPS_FLAG)) {
			return static_cast<NodeCaps>(caps);
		}
	}

	bool mine;
	uint8_t buildsize = calc_buildsize(egbase, f, true, &mine, consider_mobs, initcaps);
	if (buildsize < BaseImmovable::SMALL) {
		return static_cast<NodeCaps>(caps);
	}
	assert(buildsize >= BaseImmovable::SMALL && buildsize <= BaseImmovable::BIG);

	if (buildsize == BaseImmovable::BIG) {
		if (calc_buildsize(egbase, l_n(f), false, nullptr, consider_mobs, initcaps) <
		       BaseImmovable::BIG ||
		    calc_buildsize(egbase, tl_n(f), false, nullptr, consider_mobs, initcaps) <
		       BaseImmovable::BIG ||
		    calc_buildsize(egbase, tr_n(f), false, nullptr, consider_mobs, initcaps) <
		       BaseImmovable::BIG) {
			buildsize = BaseImmovable::MEDIUM;
		}
	}

	// Reduce building size if it would block connectivity
	if (buildsize == BaseImmovable::BIG) {
		if (!is_cycle_connected(br, {WALK_NE, WALK_NE, WALK_NW, WALK_W, WALK_W, WALK_SW, WALK_SW,
		                             WALK_SE, WALK_E, WALK_E})) {
			buildsize = BUILDCAPS_MEDIUM;
		}
	}
	if (buildsize < BaseImmovable::BIG) {
		if (!is_cycle_connected(br, {WALK_NE, WALK_NW, WALK_W, WALK_SW, WALK_SE, WALK_E})) {
			return static_cast<NodeCaps>(caps);
		}
	}

	if (mine) {
		if (static_cast<int32_t>(br.field->get_height()) - f.field->get_height() < 4) {
			caps |= BUILDCAPS_MINE;
		}
	} else {
		Field::Height const f_height = f.field->get_height();

		// Reduce building size based on slope of direct neighbours:
		//  - slope >= 4: can't build anything here -> return
		//  - slope >= 3: maximum size is small
		{
			MapFringeRegion<Area<FCoords>> mr(*this, Area<FCoords>(f, 1));
			do {
				uint16_t const slope = abs(mr.location().field->get_height() - f_height);
				if (slope >= 4) {
					return static_cast<NodeCaps>(caps);
				}
				if (slope >= 3) {
					buildsize = BaseImmovable::SMALL;
				}
			} while (mr.advance(*this));
		}
		if (abs(br.field->get_height() - f_height) >= 2) {
			return static_cast<NodeCaps>(caps);
		}

		// Reduce building size based on height diff. of second order
		// neighbours  If height difference between this field and second
		// order neighbour is >= 3, we can only build a small house here.
		// Additionally, we can potentially build a port on this field
		// if one of the second order neighbours is swimmable.
		if (buildsize >= BaseImmovable::MEDIUM) {
			MapFringeRegion<Area<FCoords>> mr(*this, Area<FCoords>(f, 2));

			do {
				if (abs(mr.location().field->get_height() - f_height) >= 3) {
					buildsize = BaseImmovable::SMALL;
					break;
				}
			} while (mr.advance(*this));
		}

		if ((buildsize == BaseImmovable::BIG) && is_port_space(f) &&
		    !find_portdock(f, false).empty()) {
			caps |= BUILDCAPS_PORT;
		}

		caps |= buildsize;
	}
	return static_cast<NodeCaps>(caps);
}

/**
 * Return the size of immovable that is supposed to be buildable on \p f,
 * based on immovables on \p f and its neighbours.
 * Sets \p ismine depending on whether the field is on mountaineous terrain
 * or not.
 * \p consider_mobs defines, whether mapobjects currently on \p f or neighbour fields should be
 * considered
 * for the calculation. If not (calculation of maximum theoretical possible buildsize) initcaps must
 * be set.
 */
int Map::calc_buildsize(const EditorGameBase& egbase,
                        const FCoords& f,
                        bool avoidnature,
                        bool* ismine,
                        bool consider_mobs,
                        NodeCaps initcaps) const {
	if (consider_mobs) {
		if (!(f.field->get_caps() & MOVECAPS_WALK)) {
			return BaseImmovable::NONE;
		}
		if (BaseImmovable const* const immovable = get_immovable(f)) {
			if (immovable->get_size() >= BaseImmovable::SMALL) {
				return BaseImmovable::NONE;
			}
		}
	} else if (!(initcaps & MOVECAPS_WALK)) {
		return BaseImmovable::NONE;
	}

	// Get all relevant neighbours and count terrain triangle types.
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords l = l_n(f);

	const Descriptions& descriptions = egbase.descriptions();
	const TerrainDescription::Is terrains[6] = {
	   descriptions.get_terrain_descr(tr.field->terrain_d())->get_is(),
	   descriptions.get_terrain_descr(tl.field->terrain_r())->get_is(),
	   descriptions.get_terrain_descr(tl.field->terrain_d())->get_is(),
	   descriptions.get_terrain_descr(l.field->terrain_r())->get_is(),
	   descriptions.get_terrain_descr(f.field->terrain_d())->get_is(),
	   descriptions.get_terrain_descr(f.field->terrain_r())->get_is()};

	uint32_t cnt_mineable = 0;
	uint32_t cnt_walkable = 0;
	for (const TerrainDescription::Is& is : terrains) {
		if (is & TerrainDescription::Is::kWater || is & TerrainDescription::Is::kUnwalkable) {
			return BaseImmovable::NONE;
		}
		if (is & TerrainDescription::Is::kMineable) {
			++cnt_mineable;
		}
		if (is & TerrainDescription::Is::kWalkable) {
			++cnt_walkable;
		}
	}

	if (ismine) {
		*ismine = (cnt_mineable == 6);
	}
	if ((cnt_mineable && cnt_mineable < 6) || cnt_walkable) {
		return BaseImmovable::NONE;
	}

	// Adjust size based on neighbouring immovables
	int buildsize = BaseImmovable::BIG;
	if (consider_mobs) {
		std::vector<ImmovableFound> objectlist;
		find_immovables(egbase, Area<FCoords>(f, 1), &objectlist,
		                FindImmovableSize(BaseImmovable::SMALL, BaseImmovable::BIG));
		for (const ImmovableFound& immfound : objectlist) {
			const BaseImmovable* obj = immfound.object;
			int objsize = obj->get_size();
			if (objsize == BaseImmovable::NONE) {
				continue;
			}
			if (avoidnature && obj->descr().type() == MapObjectType::IMMOVABLE) {
				++objsize;
			}
			if (objsize + buildsize > BaseImmovable::BIG) {
				buildsize = BaseImmovable::BIG - objsize + 1;
			}
		}
	}

	return buildsize;
}

/**
 * We call a cycle on the map simply 'connected' if it can be walked on.
 *
 * The cycle is described as a \p start point plus
 * a description of the directions in which to walk from the starting point.
 */
bool Map::is_cycle_connected(const FCoords& start, const std::vector<WalkingDir>& dirs) const {
	if (!(start.field->maxcaps() & MOVECAPS_WALK)) {
		return false;
	}
	FCoords f = start;
	for (const WalkingDir& dir : dirs) {
		f = get_neighbour(f, dir);
		if (!(f.field->maxcaps() & MOVECAPS_WALK)) {
			return false;
		}
	}
	assert(start == f);
	return true;
}

/**
 * Returns a list of portdock fields (if any) that a port built at \p c should have.
 */
std::vector<Coords> Map::find_portdock(const Coords& c, bool force) const {
	static const WalkingDir cycledirs[16] = {WALK_NE, WALK_NE, WALK_NE, WALK_NW, WALK_NW, WALK_W,
	                                         WALK_W,  WALK_W,  WALK_SW, WALK_SW, WALK_SW, WALK_SE,
	                                         WALK_SE, WALK_E,  WALK_E,  WALK_E};
	const FCoords start = br_n(br_n(get_fcoords(c)));
	const Widelands::PlayerNumber owner = start.field->get_owned_by();
	FCoords f = start;
	std::vector<Coords> portdock;
	for (uint32_t i = 0; i < 16; ++i) {
		if (force) {
			if (f.field->maxcaps() & MOVECAPS_SWIM) {
				return {f};
			}
		} else {
			bool is_good_water =
			   (f.field->get_caps() & (MOVECAPS_SWIM | MOVECAPS_WALK)) == MOVECAPS_SWIM;

			// Any immovable here? (especially another portdock)
			if (is_good_water && f.field->get_immovable()) {
				is_good_water = false;
			}

			// If starting point is owned we make sure this field has the same owner
			if (is_good_water && owner != neutral() && f.field->get_owned_by() != owner) {
				is_good_water = false;
			}

			// ... and is not on a border
			if (is_good_water && owner != neutral() && f.field->is_border()) {
				is_good_water = false;
			}

			if (is_good_water) {
				portdock.push_back(f);
				// Occupy 2 fields maximum in order not to block space for other ports that
				// might be built in the vicinity.
				if (portdock.size() == 2) {
					return portdock;
				}
			}
		}

		if (i < 15) {
			f = get_neighbour(f, cycledirs[i]);
		}
	}

	return portdock;
}

bool Map::is_port_space_allowed(const EditorGameBase& egbase, const FCoords& fc) const {
	return (get_max_nodecaps(egbase, fc) & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG &&
	       !find_portdock(fc, false).empty();
}

/// \returns true, if Coordinates are in port space list
bool Map::is_port_space(const Coords& c) const {
	return port_spaces_.count(c);
}

bool Map::set_port_space(const EditorGameBase& egbase,
                         const Coords& c,
                         bool set,
                         bool force,
                         bool recalculate_seafaring) {
	bool success = false;
	if (set) {
		success = force || is_port_space_allowed(egbase, get_fcoords(c));
		if (success) {
			port_spaces_.insert(c);
			recalculate_seafaring &= !allows_seafaring();
		}
	} else {
		recalculate_seafaring &= allows_seafaring();
		port_spaces_.erase(c);
		success = true;
	}
	if (recalculate_seafaring) {
		recalculate_allows_seafaring();
	}
	return success;
}

/**
 * Calculate the (Manhattan) distance from a to b
 * a and b are expected to be normalized!
 */
uint32_t Map::calc_distance(const Coords& a, const Coords& b) const {
	uint32_t dist;
	int32_t dy;

	// do we fly up or down?
	dy = b.y - a.y;
	if (dy > static_cast<int32_t>(height_ >> 1)) {  //  wrap-around!
		dy -= height_;
	} else if (dy < -static_cast<int32_t>(height_ >> 1)) {
		dy += height_;
	}

	dist = abs(dy);

	if (static_cast<int16_t>(dist) >= width_) {
		// no need to worry about x movement at all
		return dist;
	}

	// [lx..rx] is the x-range we can cover simply by walking vertically
	// towards b
	// Hint: (~a.y & 1) is 1 for even rows, 0 for odd rows.
	// This means we round UP for even rows, and we round DOWN for odd rows.
	int32_t lx, rx;

	lx = a.x - ((dist + (~a.y & 1)) >> 1);  // div 2
	rx = lx + dist;

	// Allow for wrap-around
	// Yes, the second is an else if; see the above if (dist >= width_)
	if (lx < 0) {
		lx += width_;
	} else if (rx >= static_cast<int32_t>(width_)) {
		rx -= width_;
	}

	// Normal, non-wrapping case
	if (lx <= rx) {
		if (b.x < lx) {
			int32_t dx1 = lx - b.x;
			int32_t dx2 = b.x - (rx - width_);
			dist += std::min(dx1, dx2);
		} else if (b.x > rx) {
			int32_t dx1 = b.x - rx;
			int32_t dx2 = (lx + width_) - b.x;
			dist += std::min(dx1, dx2);
		}
	} else {
		// Reverse case
		if (b.x > rx && b.x < lx) {
			int32_t dx1 = b.x - rx;
			int32_t dx2 = lx - b.x;
			dist += std::min(dx1, dx2);
		}
	}

	return dist;
}

#define BASE_COST_PER_FIELD 1800
#define SLOPE_COST_DIVISOR 50
#define SLOPE_COST_STEPS 8

/*
===============
Calculates the cost estimate between the two points.
This function is used mainly for the path-finding estimate.
===============
*/
int32_t Map::calc_cost_estimate(const Coords& a, const Coords& b) const {
	return calc_distance(a, b) * BASE_COST_PER_FIELD;
}

/**
 * \return a lower bound on the time required to walk from \p a to \p b
 */
int32_t Map::calc_cost_lowerbound(const Coords& a, const Coords& b) const {
	return calc_distance(a, b) * calc_cost(-SLOPE_COST_STEPS);
}

/*
===============
Calculate the hard cost of walking the given slope (positive means up,
negative means down).
The cost is in milliseconds it takes to walk.

The time is calculated as BASE_COST_PER_FIELD * f, where

f = 1.0 + d(Slope) - d(0)
d = (Slope + SLOPE_COST_STEPS) * (Slope + SLOPE_COST_STEPS - 1)
       / (2 * SLOPE_COST_DIVISOR)

Note that the actual calculations multiply through by (2 * SLOPE_COST_DIVISOR)
to avoid using floating point numbers in game logic code.

Slope is limited to the range [ -SLOPE_COST_STEPS; +oo [
===============
*/
#define CALC_COST_D(slope) (((slope) + SLOPE_COST_STEPS) * ((slope) + SLOPE_COST_STEPS - 1))

static int32_t calc_cost_d(int32_t slope) {
	if (slope < -SLOPE_COST_STEPS) {
		slope = -SLOPE_COST_STEPS;
	}

	return CALC_COST_D(slope);
}

int32_t Map::calc_cost(int32_t const slope) const {
	return BASE_COST_PER_FIELD * (2 * SLOPE_COST_DIVISOR + calc_cost_d(slope) - CALC_COST_D(0)) /
	       (2 * SLOPE_COST_DIVISOR);
}

/*
===============
Return the time it takes to walk the given step from coords in the given
direction, in milliseconds.
===============
*/
int32_t Map::calc_cost(const Coords& coords, const int32_t dir) const {
	FCoords f;
	int32_t startheight;
	int32_t delta;

	// Calculate the height delta
	f = get_fcoords(coords);
	startheight = f.field->get_height();

	get_neighbour(f, dir, &f);
	delta = f.field->get_height() - startheight;

	return calc_cost(delta);
}

/*
===============
Calculate the average cost of walking the given step in both directions.
===============
*/
int32_t Map::calc_bidi_cost(const Coords& coords, const int32_t dir) const {
	FCoords f;
	int32_t startheight;
	int32_t delta;

	// Calculate the height delta
	f = get_fcoords(coords);
	startheight = f.field->get_height();

	get_neighbour(f, dir, &f);
	delta = f.field->get_height() - startheight;

	return (calc_cost(delta) + calc_cost(-delta)) / 2;
}

/*
===============
Calculate the cost of walking the given path.
If either of the forward or backward pointers is set, it will be filled in
with the cost of walking in said direction.
===============
*/
void Map::calc_cost(const Path& path, int32_t* const forward, int32_t* const backward) const {
	Coords coords = path.get_start();

	if (forward) {
		*forward = 0;
	}
	if (backward) {
		*backward = 0;
	}

	const Path::StepVector::size_type nr_steps = path.get_nsteps();
	for (Path::StepVector::size_type i = 0; i < nr_steps; ++i) {
		const Direction dir = path[i];

		if (forward) {
			*forward += calc_cost(coords, dir);
		}
		get_neighbour(coords, dir, &coords);
		if (backward) {
			*backward += calc_cost(coords, get_reverse_dir(dir));
		}
	}
}

/// Get a node's neighbour by direction.
void Map::get_neighbour(const Coords& f, Direction const dir, Coords* const o) const {
	switch (dir) {
	case WALK_NW:
		get_tln(f, o);
		break;
	case WALK_NE:
		get_trn(f, o);
		break;
	case WALK_E:
		get_rn(f, o);
		break;
	case WALK_SE:
		get_brn(f, o);
		break;
	case WALK_SW:
		get_bln(f, o);
		break;
	case WALK_W:
		get_ln(f, o);
		break;
	default:
		NEVER_HERE();
	}
}

void Map::get_neighbour(const FCoords& f, Direction const dir, FCoords* const o) const {
	switch (dir) {
	case WALK_NW:
		get_tln(f, o);
		break;
	case WALK_NE:
		get_trn(f, o);
		break;
	case WALK_E:
		get_rn(f, o);
		break;
	case WALK_SE:
		get_brn(f, o);
		break;
	case WALK_SW:
		get_bln(f, o);
		break;
	case WALK_W:
		get_ln(f, o);
		break;
	default:
		NEVER_HERE();
	}
}

std::unique_ptr<MapLoader> Map::get_correct_loader(const std::string& filename) {
	std::unique_ptr<MapLoader> result;

	const std::string lower_filename = to_lower(filename);

	try {
		if (ends_with(lower_filename, kWidelandsMapExtension)) {
			result.reset(new WidelandsMapLoader(g_fs->make_sub_file_system(filename), this));
		} else if (ends_with(lower_filename, kSavegameExtension)) {
			std::unique_ptr<FileSystem> sub_fs(g_fs->make_sub_file_system(filename));
			result.reset(new WidelandsMapLoader(sub_fs->make_sub_file_system("map"), this));
		} else if (ends_with(lower_filename, kS2MapExtension1) ||
		           ends_with(lower_filename, kS2MapExtension2)) {
			result.reset(new S2MapLoader(filename, *this));
		}
	} catch (const FileError& e) {
		// file might not have existed
		log_err("Map::get_correct_loader: File error: %s\n", e.what());
	} catch (std::exception& e) {
		log_err("Map::get_correct_loader: Unknown error: %s\n", e.what());
	}
	return result;
}

/**
 * Finds a path from start to end for a MapObject with the given movecaps.
 *
 * The path is stored in \p path, as a series of MapObject::WalkingDir entries.
 *
 * \param persist tells the function how hard it should try to find a path:
 * If \p persist is \c 0, the function will never give up early. Otherwise, the
 * function gives up when it becomes clear that the path takes longer than
 * persist * bird's distance of flat terrain.
 * Note that if the terrain contains steep hills, the cost calculation will
 * cause the search to terminate earlier than you may think. If persist==1,
 * findpath() can only find a path if the terrain is completely flat.
 *
 * \param checkstep findpath() calls this checkstep functor-like to determine
 * whether moving from one field to another is legal.
 *
 * \param instart starting point of the search
 * \param inend end point of the search
 * \param path will receive the found path if successful
 * \param flags UNDOCUMENTED
 * \param type whether to find a path for a ware or a worker
 *
 * \return the cost of the path (in milliseconds of normal walking
 * speed) or -1 if no path has been found.
 */
// TODO(unknown): Document parameters instart, inend, path, flags
int32_t Map::findpath(Coords instart,
                      Coords inend,
                      int32_t const persist,
                      Path& path,
                      const CheckStep& checkstep,
                      uint32_t const flags,
                      uint32_t const caps_sensitivity,
                      WareWorker type) const {
	FCoords start;
	FCoords end;
	int32_t upper_cost_limit;
	FCoords cur;

	normalize_coords(instart);
	normalize_coords(inend);

	start = FCoords(instart, &operator[](instart));
	end = FCoords(inend, &operator[](inend));

	path.path_.clear();

	// Some stupid cases...
	if (start == end) {
		path.start_ = start;
		path.end_ = end;
		return 0;  // duh...
	}

	if (!checkstep.reachable_dest(*this, end)) {
		return -1;
	}

	if (!persist) {
		upper_cost_limit = 0;
	} else {
		// assume flat terrain
		upper_cost_limit = persist * calc_cost_estimate(start, end);
	}

	// Actual pathfinding
	std::shared_ptr<Pathfields> pathfields = pathfieldmgr_->allocate();
	Pathfield::Queue Open(type);
	Pathfield* curpf = &pathfields->fields[start.field - fields_.get()];
	curpf->cycle = pathfields->cycle;
	curpf->real_cost = 0;
	curpf->estim_cost = calc_cost_lowerbound(start, end);
	curpf->backlink = IDLE;

	Open.push(curpf);

	for (;;) {
		if (Open.empty()) {  // there simply is no path
			return -1;
		}
		curpf = Open.top();
		Open.pop(curpf);

		cur.field = fields_.get() + (curpf - pathfields->fields.get());
		get_coords(*cur.field, cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit) {
			break;  // upper cost limit reached, give up
		}
		if (cur == end) {
			break;  // found our target
		}

		// avoid bias by using different orders when pathfinding
		static const int8_t order1[] = {WALK_NW, WALK_NE, WALK_E, WALK_SE, WALK_SW, WALK_W};
		static const int8_t order2[] = {WALK_NW, WALK_W, WALK_SW, WALK_SE, WALK_E, WALK_NE};
		int8_t const* direction = ((cur.x + cur.y) & 1) ? order1 : order2;

		// Check all the 6 neighbours
		for (uint32_t i = 6; i; i--, direction++) {
			FCoords neighb;
			int32_t cost;

			get_neighbour(cur, *direction, &neighb);
			Pathfield& neighbpf = pathfields->fields[neighb.field - fields_.get()];

			// Is the field Closed already?
			if (neighbpf.cycle == pathfields->cycle && !neighbpf.heap_cookie.is_active()) {
				continue;
			}

			// Check passability
			if (!checkstep.allowed(*this, cur, neighb, *direction,
			                       neighb == end ? CheckStep::stepLast :
			                       cur == start  ? CheckStep::stepFirst :
                                                CheckStep::stepNormal)) {
				continue;
			}

			// Calculate cost
			cost = curpf->real_cost + ((flags & fpBidiCost) ? calc_bidi_cost(cur, *direction) :
                                                           calc_cost(cur, *direction));

			// If required (indicated by caps_sensitivity) we increase the path costs
			// if the path is just crossing a field with building capabilities
			if (caps_sensitivity > 0) {
				int32_t buildcaps_score = neighb.field->get_caps() & BUILDCAPS_SIZEMASK;
				buildcaps_score += (neighb.field->get_caps() & BUILDCAPS_MINE) ? 1 : 0;
				buildcaps_score += (neighb.field->get_caps() & BUILDCAPS_PORT) ? 9 : 0;
				cost += buildcaps_score * caps_sensitivity;
			}

			if (neighbpf.cycle != pathfields->cycle) {
				// add to open list
				neighbpf.cycle = pathfields->cycle;
				neighbpf.real_cost = cost;
				neighbpf.estim_cost = calc_cost_lowerbound(neighb, end);
				neighbpf.backlink = *direction;
				Open.push(&neighbpf);
			} else if (neighbpf.cost(type) > cost + neighbpf.estim_cost) {
				// found a better path to a field that's already Open
				neighbpf.real_cost = cost;
				neighbpf.backlink = *direction;
				Open.decrease_key(&neighbpf);
			}
		}
	}

	// Now unwind the taken route (even if we couldn't find a complete one!)
	int32_t const result = cur == end ? curpf->real_cost : -1;

	path.start_ = start;
	path.end_ = cur;

	path.path_.clear();

	while (curpf->backlink != IDLE) {
		path.path_.push_back(curpf->backlink);

		// Reverse logic! (WALK_NW needs to find the SE neighbour)
		get_neighbour(cur, get_reverse_dir(curpf->backlink), &cur);
		curpf = &pathfields->fields[cur.field - fields_.get()];
	}

	return result;
}

bool Map::can_reach_by_water(const Coords& field) const {
	FCoords fc = get_fcoords(field);

	if (fc.field->nodecaps() & MOVECAPS_SWIM) {
		return true;
	}
	if (!(fc.field->nodecaps() & MOVECAPS_WALK)) {
		return false;
	}

	FCoords neighb;

	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		if (get_neighbour(fc, dir).field->nodecaps() & MOVECAPS_SWIM) {
			return true;
		}
	}

	return false;
}

int32_t Map::change_terrain(const EditorGameBase& egbase,
                            TCoords<FCoords> const c,
                            DescriptionIndex const terrain) {
	c.node.field->set_terrain(c.t, terrain);

	// remove invalid resources if necessary
	// check vertex to which the triangle belongs
	if (!is_resource_valid(egbase.descriptions(), c.node, c.node.field->get_resources())) {
		clear_resources(c.node);
	}

	// always check south-east vertex
	Widelands::FCoords f_se(c.node);
	get_neighbour(f_se, Widelands::WALK_SE, &f_se);
	if (!is_resource_valid(egbase.descriptions(), f_se, f_se.field->get_resources())) {
		clear_resources(f_se);
	}

	// check south-west vertex if d-Triangle is changed, check east vertex if r-Triangle is changed
	Widelands::FCoords f_sw_e(c.node);
	get_neighbour(f_sw_e, c.t == TriangleIndex::D ? Widelands::WALK_SW : Widelands::WALK_E, &f_sw_e);
	if (!is_resource_valid(egbase.descriptions(), f_sw_e, f_sw_e.field->get_resources())) {
		clear_resources(f_sw_e);
	}

	Notifications::publish(
	   NoteFieldTerrainChanged{c.node, static_cast<MapIndex>(c.node.field - &fields_[0])});

	// Changing the terrain can affect ports, which can be up to 3 fields away.
	constexpr int kPotentiallyAffectedNeighbors = 3;
	recalc_for_field_area(egbase, Area<FCoords>(c.node, kPotentiallyAffectedNeighbors));
	return kPotentiallyAffectedNeighbors;
}

bool Map::is_resource_valid(const Widelands::Descriptions& descriptions,
                            const Widelands::FCoords& c,
                            DescriptionIndex curres) const {
	if (curres == Widelands::kNoResource) {
		return true;
	}

	Widelands::FCoords f1;

	int32_t count = 0;

	//  this field
	count += descriptions.get_terrain_descr(c.field->terrain_r())->is_resource_valid(curres);
	count += descriptions.get_terrain_descr(c.field->terrain_d())->is_resource_valid(curres);

	//  If one of the neighbours is impassable, count its resource stronger.
	//  top left neigbour
	get_neighbour(c, Widelands::WALK_NW, &f1);
	count += descriptions.get_terrain_descr(f1.field->terrain_r())->is_resource_valid(curres);
	count += descriptions.get_terrain_descr(f1.field->terrain_d())->is_resource_valid(curres);

	//  top right neigbour
	get_neighbour(c, Widelands::WALK_NE, &f1);
	count += descriptions.get_terrain_descr(f1.field->terrain_d())->is_resource_valid(curres);

	//  left neighbour
	get_neighbour(c, Widelands::WALK_W, &f1);
	count += descriptions.get_terrain_descr(f1.field->terrain_r())->is_resource_valid(curres);

	return count > 1;
}

void Map::ensure_resource_consistency(const Descriptions& descriptions) {
	for (MapIndex i = 0; i < max_index(); ++i) {
		auto fcords = get_fcoords(fields_[i]);
		if (!is_resource_valid(descriptions, fcords, fcords.field->get_resources())) {
			clear_resources(fcords);
		}
	}
}

void Map::initialize_resources(const FCoords& c,
                               const DescriptionIndex resource_type,
                               ResourceAmount amount) {
	// You cannot have an amount of nothing.
	if (resource_type == Widelands::kNoResource) {
		amount = 0;
	}
	c.field->resources = resource_type;
	c.field->initial_res_amount = amount;
	c.field->res_amount = amount;
}

void Map::set_resources(const FCoords& c, ResourceAmount amount) {
	// You cannot change the amount of resources on a field without resources.
	if (c.field->resources == Widelands::kNoResource) {
		return;
	}
	c.field->res_amount = amount;
}

void Map::clear_resources(const FCoords& c) {
	initialize_resources(c, Widelands::kNoResource, 0);
}

uint32_t Map::set_height(const EditorGameBase& egbase, const FCoords fc, uint8_t const new_value) {
	assert(new_value <= MAX_FIELD_HEIGHT);
	assert(fields_.get() <= fc.field);
	assert(fc.field < fields_.get() + max_index());
	fc.field->height = new_value;
	uint32_t radius = 2;
	check_neighbour_heights(fc, radius);
	recalc_for_field_area(egbase, Area<FCoords>(fc, radius));
	return radius;
}

uint32_t
Map::change_height(const EditorGameBase& egbase, Area<FCoords> area, int16_t const difference) {
	{
		MapRegion<Area<FCoords>> mr(*this, area);
		do {
			if (difference < 0 && mr.location().field->height < static_cast<uint8_t>(-difference)) {
				mr.location().field->height = 0;
			} else if (static_cast<int16_t>(MAX_FIELD_HEIGHT) - difference <
			           static_cast<int16_t>(mr.location().field->height)) {
				mr.location().field->height = MAX_FIELD_HEIGHT;
			} else {
				mr.location().field->height += difference;
			}
		} while (mr.advance(*this));
	}
	uint32_t regional_radius = 0;
	MapFringeRegion<Area<FCoords>> mr(*this, area);
	do {
		uint32_t local_radius = 0;
		check_neighbour_heights(mr.location(), local_radius);
		regional_radius = std::max(regional_radius, local_radius);
	} while (mr.advance(*this));
	area.radius += regional_radius + 2;
	recalc_for_field_area(egbase, area);
	return area.radius;
}

uint32_t
Map::set_height(const EditorGameBase& egbase, Area<FCoords> area, HeightInterval height_interval) {
	assert(height_interval.valid());
	assert(height_interval.max <= MAX_FIELD_HEIGHT);
	{
		MapRegion<Area<FCoords>> mr(*this, area);
		do {
			if (mr.location().field->height < height_interval.min) {
				mr.location().field->height = height_interval.min;
			} else if (height_interval.max < mr.location().field->height) {
				mr.location().field->height = height_interval.max;
			}
		} while (mr.advance(*this));
	}
	++area.radius;
	{
		MapFringeRegion<Area<FCoords>> mr(*this, area);
		bool changed;
		do {
			changed = false;
			height_interval.min = height_interval.min < MAX_FIELD_HEIGHT_DIFF ?
                                  0 :
                                  height_interval.min - MAX_FIELD_HEIGHT_DIFF;
			height_interval.max = height_interval.max < MAX_FIELD_HEIGHT - MAX_FIELD_HEIGHT_DIFF ?
                                  height_interval.max + MAX_FIELD_HEIGHT_DIFF :
                                  MAX_FIELD_HEIGHT;
			do {
				if (mr.location().field->height < height_interval.min) {
					mr.location().field->height = height_interval.min;
					changed = true;
				} else if (height_interval.max < mr.location().field->height) {
					mr.location().field->height = height_interval.max;
					changed = true;
				}
			} while (mr.advance(*this));
			mr.extend(*this);
		} while (changed);
		area.radius = mr.radius();
	}
	recalc_for_field_area(egbase, area);
	return area.radius;
}

/*
===========
Map::check_neighbour_heights()

This private functions checks all neighbours of a field
if they are in valid boundaries, if not, they are reheighted
accordingly.
The radius of modified fields is stored in area.
=============
*/
void Map::check_neighbour_heights(FCoords coords, uint32_t& area) {
	assert(fields_.get() <= coords.field);
	assert(coords.field < fields_.get() + max_index());

	int32_t height = coords.field->get_height();
	bool check[] = {false, false, false, false, false, false};

	const FCoords n[] = {
	   tl_n(coords), tr_n(coords), l_n(coords), r_n(coords), bl_n(coords), br_n(coords)};

	for (uint8_t i = 0; i < 6; ++i) {
		Field& f = *n[i].field;
		const int32_t diff = height - f.get_height();
		if (diff > MAX_FIELD_HEIGHT_DIFF) {
			++area;
			f.set_height(height - MAX_FIELD_HEIGHT_DIFF);
			check[i] = true;
		}
		if (diff < -MAX_FIELD_HEIGHT_DIFF) {
			++area;
			f.set_height(height + MAX_FIELD_HEIGHT_DIFF);
			check[i] = true;
		}
	}

	for (uint8_t i = 0; i < 6; ++i) {
		if (check[i]) {
			check_neighbour_heights(n[i], area);
		}
	}
}

bool Map::allows_seafaring() const {
	return allows_seafaring_;
}

// This check can become very expensive, so we only recalculate this on relevant map changes.
void Map::recalculate_allows_seafaring() {

	// There need to be at least 2 port spaces for seafaring to make sense
	if (get_port_spaces().size() < 2) {
		allows_seafaring_ = false;
		Notifications::publish(NoteMapOptions());
		return;
	}

	std::set<Coords> reachable_from_previous_ports;

	for (const Coords& c : get_port_spaces()) {
		std::queue<Coords> positions_to_check;
		std::set<Coords> reachable_from_current_port;
		FCoords fc = get_fcoords(c);

		// Get portdock slots for this port
		for (const Coords& portdock : find_portdock(fc, true)) {
			reachable_from_current_port.insert(portdock);
			positions_to_check.push(portdock);
		}

		// Pick up all positions that can be reached from the current port
		while (!positions_to_check.empty()) {
			// Take a copy, because we'll pop it
			const Coords current_position = positions_to_check.front();
			positions_to_check.pop();

			// Found one
			if (reachable_from_previous_ports.count(current_position) > 0) {
				allows_seafaring_ = true;
				Notifications::publish(NoteMapOptions());
				return;
			}

			// Adding the neighbors to the list
			for (uint8_t i = 1; i <= 6; ++i) {
				FCoords neighbour;
				get_neighbour(get_fcoords(current_position), i, &neighbour);
				if ((neighbour.field->get_caps() & (MOVECAPS_SWIM | MOVECAPS_WALK)) == MOVECAPS_SWIM) {
					auto insert = reachable_from_current_port.insert(neighbour);
					if (insert.second) {
						positions_to_check.push(neighbour);
					}
				}
			}
		}

		// Couldn't connect to another port, so we add our reachable nodes to the list
		for (const Coords& reachable_coord : reachable_from_current_port) {
			reachable_from_previous_ports.insert(reachable_coord);
		}
	}
	allows_seafaring_ = false;
	Notifications::publish(NoteMapOptions());
}

void Map::cleanup_port_spaces(const EditorGameBase& egbase) {
	// Temporary set to avoid problems with concurrent container operations
	PortSpacesSet clean_me_up;
	for (const Coords& c : get_port_spaces()) {
		if (!is_port_space_allowed(egbase, get_fcoords(c))) {
			clean_me_up.insert(c);
		}
	}
	for (const Coords& c : clean_me_up) {
		set_port_space(egbase, c, false);
	}
	recalculate_allows_seafaring();
}

bool Map::has_artifacts() {
	for (MapIndex i = 0; i < max_index(); ++i) {
		if (upcast(Immovable, immovable, fields_[i].get_immovable())) {
			if (immovable->descr().has_attribute(MapObjectDescr::get_attribute_id("artifact"))) {
				return true;
			}
		}
	}
	return false;
}

#define MAX_RADIUS 32
MilitaryInfluence Map::calc_influence(Coords const a, Area<> const area) const {
	const int16_t w = get_width();
	const int16_t h = get_height();
	MilitaryInfluence influence =
	   std::max(std::min(std::min(abs(a.x - area.x), abs(a.x - area.x + w)), abs(a.x - area.x - w)),
	            std::min(std::min(abs(a.y - area.y), abs(a.y - area.y + h)), abs(a.y - area.y - h)));

	influence = influence > area.radius ? 0 : influence == 0 ? MAX_RADIUS : MAX_RADIUS - influence;
	influence *= influence;

	return influence;
}

std::set<Coords> Map::to_set(Area<Coords> area) const {
	std::set<Coords> result;
	MapRegion<Area<Coords>> mr(*this, area);
	do {
		result.insert(mr.location());
	} while (mr.advance(*this));
	return result;
}

// Returns all triangles whose corners are all in the given area
std::set<TCoords<Coords>> Map::triangles_in_region(std::set<Coords> area) const {
	std::set<TCoords<Coords>> result;
	for (const Coords& c : area) {
		if (!area.count(br_n(c))) {
			continue;
		}
		if (area.count(r_n(c))) {
			result.insert(TCoords<Coords>(c, TriangleIndex::R));
		}
		if (area.count(bl_n(c))) {
			result.insert(TCoords<Coords>(c, TriangleIndex::D));
		}
	}
	return result;
}

}  // namespace Widelands
