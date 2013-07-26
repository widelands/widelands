/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

#include "ai/ai_help_structs.h"

#include "logic/player.h"
#include "upcast.h"


namespace Widelands {

// FindNodeWithFlagOrRoad

bool FindNodeWithFlagOrRoad::accept (const Map &, FCoords fc) const
{
	if (upcast(PlayerImmovable const, pimm, fc.field->get_immovable()))
		return
			pimm->get_economy() != economy
			and
			(dynamic_cast<Flag const *>(pimm)
			 or
			 (dynamic_cast<Road const *>(pimm) &&
			  (fc.field->nodecaps() & BUILDCAPS_FLAG)));
	return false;
}


// CheckStepRoadAI

bool CheckStepRoadAI::allowed
	(Map & map, FCoords, FCoords end, int32_t, CheckStep::StepId const id)
	const
{
	uint8_t endcaps = player->get_buildcaps(end);

	// Calculate cost and passability
	if (!(endcaps & movecaps))
		return false;

	// Check for blocking immovables
	if (BaseImmovable const * const imm = map.get_immovable(end))
		if (imm->get_size() >= BaseImmovable::SMALL) {
			if (id != CheckStep::stepLast && !openend)
				return false;

			if (dynamic_cast<Flag const *>(imm))
				return true;

			if (not dynamic_cast<Road const *>(imm) || !(endcaps & BUILDCAPS_FLAG))
				return false;
		}

	return true;
}

bool CheckStepRoadAI::reachabledest(Map & map, FCoords const dest) const
{
	NodeCaps const caps = dest.field->nodecaps();

	if (!(caps & movecaps)) {
		if (!((movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map.can_reach_by_water(dest))
			return false;
	}

	return true;
}

}
