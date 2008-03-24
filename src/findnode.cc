/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "findnode.h"

#include "field.h"
#include "immovable.h"
#include "wexception.h"

namespace Widelands {


FindNodeAnd::Subfunctor::Subfunctor(const FindNode& _ff, bool _negate)
	: negate(_negate), findfield(_ff)
{
}

void FindNodeAnd::add(const FindNode& findfield, bool negate)
{
	m_subfunctors.push_back(Subfunctor(findfield, negate));
}

bool FindNodeAnd::accept(const Map & map, const FCoords& coord) const {
	for
		(std::vector<Subfunctor>::const_iterator it = m_subfunctors.begin();
		 it != m_subfunctors.end();
		 ++it)
	{
		if (it->findfield.accept(map, coord) == it->negate)
			return false;
	}

	return true;
}


bool FindNodeCaps::accept(const Map &, const FCoords& coord) const {
	uint8_t fieldcaps = coord.field->get_caps();

	if ((fieldcaps & BUILDCAPS_SIZEMASK) < (m_mincaps & BUILDCAPS_SIZEMASK))
		return false;

	if ((m_mincaps & ~BUILDCAPS_SIZEMASK) & ~(fieldcaps & ~BUILDCAPS_SIZEMASK))
		return false;

	return true;
}

bool FindNodeSize::accept(const Map &, const FCoords& coord) const {
	if (BaseImmovable const * const immovable = coord.field->get_immovable())
		if (immovable->get_size() > BaseImmovable::NONE)
			return false;
	uint8_t fieldcaps = coord.field->get_caps();

	switch (m_size) {
	case sizeBuild:  return (fieldcaps & (BUILDCAPS_SIZEMASK | BUILDCAPS_FLAG | BUILDCAPS_MINE));
	case sizeMine:   return (fieldcaps & BUILDCAPS_MINE);
	case sizePort:   return (fieldcaps & BUILDCAPS_PORT);
	case sizeSmall:  return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_SMALL;
	case sizeMedium: return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_MEDIUM;
	case sizeBig:    return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_BIG;
	case sizeAny:
	default:
		return true;
	}
}

bool FindNodeImmovableSize::accept(const Map &, const FCoords& coord) const {
	int32_t size = BaseImmovable::NONE;

	if (BaseImmovable * const imm = coord.field->get_immovable())
		size = imm->get_size();

	switch (size) {
	case BaseImmovable::NONE:   return m_sizes & sizeNone;
	case BaseImmovable::SMALL:  return m_sizes & sizeSmall;
	case BaseImmovable::MEDIUM: return m_sizes & sizeMedium;
	case BaseImmovable::BIG:    return m_sizes & sizeBig;
	default:
		throw wexception("FindNodeImmovableSize: bad size = %i", size);
	}
}


bool FindNodeImmovableAttribute::accept(const Map &, const FCoords& coord) const
{
	BaseImmovable* imm = coord.field->get_immovable();

	if (!imm)
		return false;

	return imm->has_attribute(m_attribute);
}


bool FindNodeResource::accept(const Map &, const FCoords& coord) const {
	return
		((m_resource == coord.field->get_resources()) &&
		 coord.field->get_resources_amount());
}


bool FindNodeResourceEmpty::accept(const Map &, const FCoords& coord) const {
	return
		((m_resource == coord.field->get_resources()) &&
		 ! coord.field->get_resources_amount());
}


}
