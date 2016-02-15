/*
 * Copyright (C) 2008-2010, 2013 by the Widelands Development Team
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

#include "logic/findnode.h"

#include "base/wexception.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/map_objects/immovable.h"

namespace Widelands {


FindNodeAnd::Subfunctor::Subfunctor(const FindNode & _ff, bool const _negate)
	: negate(_negate), findfield(_ff)
{}

void FindNodeAnd::add(const FindNode & findfield, bool const negate)
{
	subfunctors.push_back(Subfunctor(findfield, negate));
}

bool FindNodeAnd::accept(const Map & map, const FCoords & coord) const {
	for (const Subfunctor& subfunctor : subfunctors) {
		if (subfunctor.findfield.accept(map, coord) == subfunctor.negate) {
			return false;
		}
	}
	return true;
}


bool FindNodeCaps::accept(const Map &, const FCoords & coord) const {
	NodeCaps nodecaps = coord.field->nodecaps();

	if ((nodecaps & BUILDCAPS_SIZEMASK) < (mincaps & BUILDCAPS_SIZEMASK))
		return false;

	if ((mincaps & ~BUILDCAPS_SIZEMASK) & ~(nodecaps & ~BUILDCAPS_SIZEMASK))
		return false;

	return true;
}

bool FindNodeSize::accept(const Map &, const FCoords & coord) const {
	if (BaseImmovable const * const immovable = coord.field->get_immovable())
		if (immovable->get_size() > BaseImmovable::NONE)
			return false;
	NodeCaps const nodecaps = coord.field->nodecaps();

	switch (size) {
	case sizeBuild:
		return
			nodecaps & (BUILDCAPS_SIZEMASK | BUILDCAPS_FLAG | BUILDCAPS_MINE);
	case sizeMine:
		return nodecaps & BUILDCAPS_MINE;
	case sizePort:
		return nodecaps & BUILDCAPS_PORT;
	case sizeSmall:
		return (nodecaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_SMALL;
	case sizeMedium:
		return (nodecaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_MEDIUM;
	case sizeBig:
		return (nodecaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_BIG;
	case sizeAny:
		return true;
	}
	NEVER_HERE();
}

bool FindNodeImmovableSize::accept(const Map &, const FCoords & coord) const {
	int32_t size = BaseImmovable::NONE;

	if (BaseImmovable * const imm = coord.field->get_immovable())
		size = imm->get_size();

	switch (size) {
	case BaseImmovable::NONE:   return sizes & sizeNone;
	case BaseImmovable::SMALL:  return sizes & sizeSmall;
	case BaseImmovable::MEDIUM: return sizes & sizeMedium;
	case BaseImmovable::BIG:    return sizes & sizeBig;
	default:
		throw wexception("FindNodeImmovableSize: bad size = %i", size);
	}
}


bool FindNodeImmovableAttribute::accept
	(const Map &, const FCoords & coord) const
{
	if (BaseImmovable * const imm = coord.field->get_immovable())
		return imm->has_attribute(attribute);
	return false;
}


bool FindNodeResource::accept(const Map &, const FCoords & coord) const {
	return
		resource == coord.field->get_resources() &&
		coord.field->get_resources_amount();
}


bool FindNodeResourceBreedable::accept
	(const Map & map, const FCoords & coord) const
{
	// Accept a tile that is full only if a neighbor also matches resource and
	// is not full.
	if (resource != coord.field->get_resources()) {
		return false;
	}
	if (coord.field->get_resources_amount() < coord.field->get_initial_res_amount()) {
		return true;
	}
	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		const FCoords neighb = map.get_neighbour(coord, dir);
		if
			(resource == neighb.field->get_resources()
			 &&
			 neighb.field->get_resources_amount() < neighb.field->get_initial_res_amount())
		{
			return true;
		}
	}
	return false;
}

bool FindNodeShore::accept(const Map & map, const FCoords & coord) const
{
	if (!(coord.field->nodecaps() & MOVECAPS_WALK))
		return false;

	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		FCoords neighb = map.get_neighbour(coord, dir);
		if (neighb.field->nodecaps() & MOVECAPS_SWIM)
			return true;
	}

	return false;
}

}
