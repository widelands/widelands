/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#include "logic/map_objects/checkstep.h"

#include "economy/flag.h"
#include "economy/road.h"
#include "logic/map.h"
#include "logic/player.h"

namespace Widelands {

/**
 * The default constructor creates a functor that always returns \c false
 * on all checks.
 */
CheckStep::CheckStep() : capsule(always_false().capsule) {
}

struct CheckStepAlwaysFalse {
	bool allowed(Map&, const FCoords&, const FCoords&, int32_t, CheckStep::StepId) const {
		return false;
	}
	bool reachable_dest(Map&, const FCoords&) const {
		return false;
	}
};

const CheckStep& CheckStep::always_false() {
	static const CheckStep cstep = CheckStep(CheckStepAlwaysFalse());
	return cstep;
}

void CheckStepAnd::add(const CheckStep& sub) {
	subs.push_back(sub);
}

bool CheckStepAnd::allowed(Map& map,
                           const FCoords& start,
                           const FCoords& end,
                           int32_t const dir,
                           CheckStep::StepId const id) const {
	for (const CheckStep& checkstep : subs) {
		if (!checkstep.allowed(map, start, end, dir, id)) {
			return false;
		}
	}
	return true;
}

bool CheckStepAnd::reachable_dest(Map& map, const FCoords& dest) const {
	for (const CheckStep& checkstep : subs) {
		if (!checkstep.reachable_dest(map, dest)) {
			return false;
		}
	}
	return true;
}

/*
===============
CheckStepDefault
===============
*/
bool CheckStepDefault::allowed(
   Map&, const FCoords& start, const FCoords& end, int32_t, CheckStep::StepId) const {
	NodeCaps const endcaps = end.field->nodecaps();

	if (endcaps & movecaps_)
		return true;

	// Swimming bobs are allowed to move from a water field to a shore field
	NodeCaps const startcaps = start.field->nodecaps();

	if ((endcaps & MOVECAPS_WALK) && (startcaps & movecaps_ & MOVECAPS_SWIM))
		return true;

	return false;
}

bool CheckStepDefault::reachable_dest(Map& map, const FCoords& dest) const {
	NodeCaps const caps = dest.field->nodecaps();

	if (!(caps & movecaps_)) {
		if (!((movecaps_ & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
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
bool CheckStepWalkOn::allowed(
   Map&, const FCoords& start, const FCoords& end, int32_t, CheckStep::StepId const id) const {
	NodeCaps const startcaps = start.field->nodecaps();
	NodeCaps const endcaps = end.field->nodecaps();

	//  Make sure to not find paths where we walk onto an unwalkable node, then
	//  then back onto a walkable node.
	if (!onlyend_ && id != CheckStep::stepFirst && !(startcaps & movecaps_))
		return false;

	if (endcaps & movecaps_)
		return true;

	//  We can't move onto the node using normal rules.
	// If onlyend is true, exception rules only apply for the last step.
	if (onlyend_ && id != CheckStep::stepLast)
		return false;

	// If the previous field was walkable, we can move onto this one
	if (startcaps & movecaps_)
		return true;

	return false;
}

bool CheckStepWalkOn::reachable_dest(Map&, FCoords) const {
	// Don't bother solving this.
	return true;
}

bool CheckStepRoad::allowed(
   Map& map, const FCoords& start, const FCoords& end, int32_t, CheckStep::StepId const id) const {
	uint8_t const endcaps = player_.get_buildcaps(end);

	// Calculate cost and passability
	if (!(endcaps & movecaps_) &&
	    !((endcaps & MOVECAPS_WALK) && (player_.get_buildcaps(start) & movecaps_ & MOVECAPS_SWIM)))
		return false;

	// Check for blocking immovables
	if (BaseImmovable const* const imm = map.get_immovable(end))
		if (imm->get_size() >= BaseImmovable::SMALL) {
			if (id != CheckStep::stepLast)
				return false;

			return dynamic_cast<Flag const*>(imm) ||
			       (dynamic_cast<Road const*>(imm) && (endcaps & BUILDCAPS_FLAG));
		}

	return true;
}

bool CheckStepRoad::reachable_dest(Map& map, const FCoords& dest) const {
	NodeCaps const caps = dest.field->nodecaps();

	if (!(caps & movecaps_)) {
		if (!((movecaps_ & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map.can_reach_by_water(dest))
			return false;
	}

	return true;
}

bool CheckStepLimited::allowed(
   Map&, const FCoords&, const FCoords& end, int32_t, CheckStep::StepId) const {
	return allowed_locations_.find(end) != allowed_locations_.end();
}

bool CheckStepLimited::reachable_dest(Map&, FCoords) const {
	return true;
}
}
