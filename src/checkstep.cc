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

#include "checkstep.h"

#include "map.h"
#include "logic/player.h"
#include "economy/flag.h"
#include "economy/road.h"

namespace Widelands {


/**
 * The default constructor creates a functor that always returns \c false
 * on all checks.
 */
CheckStep::CheckStep()
	: capsule(alwaysfalse().capsule)
{
}

struct CheckStepAlwaysFalse {
	bool allowed
		(Map &, FCoords const &, FCoords const &, int32_t, CheckStep::StepId)
		const
	{
		return false;
	}
	bool reachabledest(Map &, FCoords const &) const {return false;}
};

CheckStep const & CheckStep::alwaysfalse()
{
	static const CheckStep cstep = CheckStep(CheckStepAlwaysFalse());
	return cstep;
}


void CheckStepAnd::add(CheckStep const & sub)
{
	subs.push_back(sub);
}

bool CheckStepAnd::allowed
	(Map & map, FCoords const start, FCoords const end, int32_t const dir,
	 CheckStep::StepId const id)
	const
{
	container_iterate_const(std::vector<CheckStep>, subs, i)
		if (!i.current->allowed(map, start, end, dir, id))
			return false;

	return true;
}

bool CheckStepAnd::reachabledest(Map & map, FCoords const dest) const
{
	container_iterate_const(std::vector<CheckStep>, subs, i)
		if (!i.current->reachabledest(map, dest))
			return false;

	return true;
}

/*
===============
CheckStepDefault
===============
*/
bool CheckStepDefault::allowed
	(Map &, FCoords start, FCoords end, int32_t, CheckStep::StepId) const
{
	uint8_t endcaps = end.field->get_caps();

	if (endcaps & m_movecaps)
		return true;

	// Swimming bobs are allowed to move from a water field to a shore field
	uint8_t startcaps = start.field->get_caps();

	if ((endcaps & MOVECAPS_WALK) && (startcaps & m_movecaps & MOVECAPS_SWIM))
		return true;

	return false;
}

bool CheckStepDefault::reachabledest(Map & map, FCoords const dest) const
{
	uint8_t caps = dest.field->get_caps();

	if (!(caps & m_movecaps)) {
		if (!((m_movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map.can_reach_by_water(dest))
			return false;
	}

	return true;
}


/*
===============
CheckStepWalkOn
===============
*/
bool CheckStepWalkOn::allowed
	(Map &, FCoords start, FCoords end, int32_t, CheckStep::StepId id) const
{
	uint8_t startcaps = start.field->get_caps();
	uint8_t endcaps = end.field->get_caps();

	// Make sure that we don't find paths where we walk onto an unwalkable field,
	// then move back onto a walkable field.
	if (!m_onlyend && id != CheckStep::stepFirst && !(startcaps & m_movecaps))
		return false;

	if (endcaps & m_movecaps)
		return true;

	//  We can't move onto the node using normal rules.
	// If onlyend is true, exception rules only apply for the last step.
	if (m_onlyend && id != CheckStep::stepLast)
		return false;

	// If the previous field was walkable, we can move onto this one
	if (startcaps & m_movecaps)
		return true;

	return false;
}

bool CheckStepWalkOn::reachabledest(Map &, FCoords) const {
	// Don't bother solving this.
	return true;
}


bool CheckStepRoad::allowed
	(Map & map, FCoords const start, FCoords const end, int32_t,
	 CheckStep::StepId const id)
	const
{
	uint8_t const endcaps = m_player.get_buildcaps(end);

	// Calculate cost and passability
	if
		(not (endcaps & m_movecaps)
		 and
		 not
		 ((endcaps & MOVECAPS_WALK)
		  and
		  (m_player.get_buildcaps(start) & m_movecaps & MOVECAPS_SWIM)))
		return false;

	// Check for blocking immovables
	if (BaseImmovable const * const imm = map.get_immovable(end))
		if (imm->get_size() >= BaseImmovable::SMALL) {
			if (id != CheckStep::stepLast)
				return false;

			return
				dynamic_cast<Flag const *>(imm)
				or
				(dynamic_cast<Road const *>(imm) and endcaps & BUILDCAPS_FLAG);
		}

	return true;
}

bool CheckStepRoad::reachabledest(Map & map, FCoords const dest) const
{
	uint8_t caps = dest.field->get_caps();

	if (!(caps & m_movecaps)) {
		if (!((m_movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map.can_reach_by_water(dest))
			return false;
	}

	return true;
}


bool CheckStepLimited::allowed
	(Map &, FCoords, FCoords const end, int32_t, CheckStep::StepId) const
{
	return m_allowed_locations.find(end) != m_allowed_locations.end();
}

bool CheckStepLimited::reachabledest(Map &, FCoords) const {
	return true;
}

}
