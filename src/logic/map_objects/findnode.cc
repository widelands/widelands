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

#include "logic/map_objects/findnode.h"

#include "base/wexception.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/world/terrain_description.h"

namespace Widelands {

FindNodeAnd::Subfunctor::Subfunctor(const FindNode& init_findfield, bool const init_negate)
   : negate(init_negate), findfield(init_findfield) {
}

void FindNodeAnd::add(const FindNode& findfield, bool const negate) {
	subfunctors.emplace_back(findfield, negate);
}

bool FindNodeAnd::accept(const EditorGameBase& egbase, const FCoords& coord) const {
	for (const Subfunctor& subfunctor : subfunctors) {
		if (subfunctor.findfield.accept(egbase, coord) == subfunctor.negate) {
			return false;
		}
	}
	return true;
}

bool FindNodeCaps::accept(const EditorGameBase& /* egbase */, const FCoords& coord) const {
	NodeCaps nodecaps = coord.field->nodecaps();

	if ((nodecaps & BUILDCAPS_SIZEMASK) < (mincaps & BUILDCAPS_SIZEMASK)) {
		return false;
	}
	if (((mincaps & ~BUILDCAPS_SIZEMASK) & ~(nodecaps & ~BUILDCAPS_SIZEMASK)) != 0) {
		return false;
	}
	return true;
}

bool FindNodeSize::accept(const EditorGameBase& egbase, const FCoords& coord) const {
	if (BaseImmovable const* const immovable = coord.field->get_immovable()) {
		if (immovable->get_size() > BaseImmovable::NONE) {
			return false;
		}
	}
	NodeCaps const nodecaps = coord.field->nodecaps();
	const Map& map = egbase.map();

	switch (size) {
	case sizeBuild:
		return (nodecaps & (BUILDCAPS_SIZEMASK | BUILDCAPS_FLAG | BUILDCAPS_MINE)) != 0;
	case sizeMine:
		return (nodecaps & BUILDCAPS_MINE) != 0;
	case sizePort:
		return (nodecaps & BUILDCAPS_PORT) != 0;
	case sizeSmall:
		return (nodecaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_SMALL;
	case sizeMedium:
		return (nodecaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_MEDIUM;
	case sizeBig:
		return (nodecaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_BIG;
	case sizeSwim: {
		const Descriptions& world = egbase.descriptions();
		return ((world.get_terrain_descr(coord.field->terrain_d())->get_is() &
		         TerrainDescription::Is::kWater) != 0) ||
		       ((world.get_terrain_descr(coord.field->terrain_r())->get_is() &
		         TerrainDescription::Is::kWater) != 0) ||
		       ((world.get_terrain_descr(map.tl_n(coord).field->terrain_d())->get_is() &
		         TerrainDescription::Is::kWater) != 0) ||
		       ((world.get_terrain_descr(map.tl_n(coord).field->terrain_r())->get_is() &
		         TerrainDescription::Is::kWater) != 0) ||
		       ((world.get_terrain_descr(map.tr_n(coord).field->terrain_d())->get_is() &
		         TerrainDescription::Is::kWater) != 0) ||
		       ((world.get_terrain_descr(map.l_n(coord).field->terrain_r())->get_is() &
		         TerrainDescription::Is::kWater) != 0);
	}
	case sizeAny:
		return true;
	}
	NEVER_HERE();
}

bool FindNodeTerraform::accept(const EditorGameBase& egbase, const FCoords& coord) const {
	const Map& map = egbase.map();
	const Descriptions& descriptions = egbase.descriptions();
	return !(
	   descriptions.get_terrain_descr(coord.field->terrain_d())->enhancement(category_).empty() &&
	   descriptions.get_terrain_descr(coord.field->terrain_r())->enhancement(category_).empty() &&
	   descriptions.get_terrain_descr(map.tl_n(coord).field->terrain_d())
	      ->enhancement(category_)
	      .empty() &&
	   descriptions.get_terrain_descr(map.tl_n(coord).field->terrain_r())
	      ->enhancement(category_)
	      .empty() &&
	   descriptions.get_terrain_descr(map.tr_n(coord).field->terrain_d())
	      ->enhancement(category_)
	      .empty() &&
	   descriptions.get_terrain_descr(map.l_n(coord).field->terrain_r())
	      ->enhancement(category_)
	      .empty());
}

bool FindNodeImmovableSize::accept(const EditorGameBase& /* egbase */, const FCoords& coord) const {
	int32_t size = BaseImmovable::NONE;

	if (BaseImmovable* const imm = coord.field->get_immovable()) {
		size = imm->get_size();
	}

	switch (size) {
	case BaseImmovable::NONE:
		return (sizes & sizeNone) != 0u;
	case BaseImmovable::SMALL:
		return (sizes & sizeSmall) != 0u;
	case BaseImmovable::MEDIUM:
		return (sizes & sizeMedium) != 0u;
	case BaseImmovable::BIG:
		return (sizes & sizeBig) != 0u;
	default:
		throw wexception("FindNodeImmovableSize: bad size = %i", size);
	}
}

bool FindNodeImmovableAttribute::accept(const EditorGameBase& /* egbase */,
                                        const FCoords& coord) const {
	if (BaseImmovable* const imm = coord.field->get_immovable()) {
		return imm->has_attribute(attribute);
	}
	return false;
}

bool FindNodeResource::accept(const EditorGameBase& /* egbase */, const FCoords& coord) const {
	return resource == coord.field->get_resources() && (coord.field->get_resources_amount() != 0u);
}

bool FindNodeResourceBreedable::accept(const EditorGameBase& egbase, const FCoords& coord) const {
	// Accept a tile that is full only if a neighbor also matches resource and
	// is not full.
	if (resource != coord.field->get_resources()) {
		return false;
	}
	switch (strictness) {
	case AnimalBreedable::kDefault:
		// We test actual breedability
		if (coord.field->get_resources_amount() < coord.field->get_initial_res_amount()) {
			return true;
		}
		break;
	case AnimalBreedable::kAnimalFull:
		// We test potential breedability, just resources are now full
		if (coord.field->get_resources_amount() == coord.field->get_initial_res_amount()) {
			return true;
		}
		break;
	}
	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		const FCoords neighb = egbase.map().get_neighbour(coord, dir);
		if (resource == neighb.field->get_resources()) {
			switch (strictness) {
			case AnimalBreedable::kDefault:
				if (neighb.field->get_resources_amount() < neighb.field->get_initial_res_amount()) {
					return true;
				}
				break;
			case AnimalBreedable::kAnimalFull:
				if (neighb.field->get_resources_amount() == neighb.field->get_initial_res_amount()) {
					return true;
				}
				break;
			}
		}
	}
	return false;
}

bool FindNodeShore::accept(const EditorGameBase& egbase, const FCoords& coords) const {
	if ((coords.field->nodecaps() & MOVECAPS_WALK) == 0) {
		return false;
	}

	// Vector of fields whose neighbours are to be checked, starting with current one
	std::vector<FCoords> nodes_to_process = {coords};
	// Set of nodes that that are swimmable & and achievable by swimming
	// We use hashes here
	std::set<uint32_t> accepted_nodes;
	// just not to check the same node twice
	std::set<uint32_t> rejected_nodes;

	// Continue untill all nodes to process are processed, or we found sufficient number of nodes
	while (!nodes_to_process.empty() && accepted_nodes.size() < min_fields) {
		FCoords cur = nodes_to_process.back();
		nodes_to_process.pop_back();

		// Now test all neighbours
		for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			FCoords neighb = egbase.map().get_neighbour(cur, dir);
			if (accepted_nodes.count(neighb.hash()) > 0 || rejected_nodes.count(neighb.hash()) > 0) {
				// We already processed this node
				continue;
			}

			if ((neighb.field->nodecaps() & MOVECAPS_SWIM) != 0) {
				// This is new node, that is swimmable
				accepted_nodes.insert(neighb.hash());
				// But also neighbours must be processed in next iterations
				nodes_to_process.push_back(neighb);
			} else {
				rejected_nodes.insert(neighb.hash());
			}
		}
	}

	// We iterated over all reachanble fields or we found sufficient number of swimmable nodes
	return accepted_nodes.size() >= min_fields;
}
}  // namespace Widelands
