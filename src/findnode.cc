/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#include "container_iterate.h"

namespace Widelands {


FindNodeAnd::Subfunctor::Subfunctor(FindNode const & _ff, bool const _negate)
	: negate(_negate), findfield(_ff)
{}

void FindNodeAnd::add(FindNode const & findfield, bool const negate)
{
	m_subfunctors.push_back(Subfunctor(findfield, negate));
}

bool FindNodeAnd::accept(Map const & map, FCoords const & coord) const {
	container_iterate_const(std::vector<Subfunctor>, m_subfunctors, i)
		if (i.current->findfield.accept(map, coord) == i.current->negate)
			return false;

	return true;
}


bool FindNodeCaps::accept(Map const &, FCoords const & coord) const {
	uint8_t fieldcaps = coord.field->get_caps();

	if ((fieldcaps & BUILDCAPS_SIZEMASK) < (m_mincaps & BUILDCAPS_SIZEMASK))
		return false;

	if ((m_mincaps & ~BUILDCAPS_SIZEMASK) & ~(fieldcaps & ~BUILDCAPS_SIZEMASK))
		return false;

	return true;
}

bool FindNodeSize::accept(Map const &, FCoords const & coord) const {
	if (BaseImmovable const * const immovable = coord.field->get_immovable())
		if (immovable->get_size() > BaseImmovable::NONE)
			return false;
	uint8_t const fieldcaps = coord.field->get_caps();

	switch (m_size) {
	case sizeBuild:
		return
			fieldcaps & (BUILDCAPS_SIZEMASK | BUILDCAPS_FLAG | BUILDCAPS_MINE);
	case sizeMine:
		return fieldcaps & BUILDCAPS_MINE;
	case sizePort:
		return fieldcaps & BUILDCAPS_PORT;
	case sizeSmall:
		return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_SMALL;
	case sizeMedium:
		return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_MEDIUM;
	case sizeBig:
		return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_BIG;
	case sizeAny:
	default:
		return true;
	}
}

bool FindNodeImmovableSize::accept(Map const &, FCoords const & coord) const {
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


bool FindNodeImmovableAttribute::accept
	(Map const &, FCoords const & coord) const
{
	if (BaseImmovable * const imm = coord.field->get_immovable())
		return imm->has_attribute(m_attribute);
	return false;
}


bool FindNodeResource::accept(Map const &, FCoords const & coord) const {
	return
		m_resource == coord.field->get_resources() &&
		coord.field->get_resources_amount();
}


bool FindNodeResourceBreedable::accept
	(Map const &, FCoords const & coord) const
{
	return
		m_resource == coord.field->get_resources() &&
		coord.field->get_resources_amount   ()
		<
		coord.field->get_starting_res_amount();
}

}
