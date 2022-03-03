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

#include "logic/map_objects/findimmovable.h"

#include "base/macros.h"
#include "economy/flag.h"
#include "logic/map.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/militarysite.h"

namespace Widelands {

struct FindImmovableAlwaysTrueImpl {
	bool accept(const BaseImmovable& /* immovable */) const {
		return true;
	}
};

const FindImmovable& find_immovable_always_true() {
	static FindImmovable alwaystrue = FindImmovableAlwaysTrueImpl();
	return alwaystrue;
}

bool FindImmovableSize::accept(const BaseImmovable& imm) const {
	int32_t const size = imm.get_size();
	return min <= size && size <= max;
}

bool FindImmovableType::accept(const BaseImmovable& imm) const {
	return type == imm.descr().type();
}

bool FindImmovableAttribute::accept(const BaseImmovable& imm) const {
	return imm.has_attribute(attrib);
}

bool FindImmovablePlayerImmovable::accept(const BaseImmovable& imm) const {
	return dynamic_cast<PlayerImmovable const*>(&imm);
}

bool FindImmovablePlayerMilitarySite::accept(const BaseImmovable& imm) const {
	if (upcast(MilitarySite const, ms, &imm)) {
		return &ms->owner() == &player;
	}
	return false;
}

bool FindImmovableAttackTarget::accept(const BaseImmovable& imm) const {
	if (upcast(Building const, b, &imm)) {
		return b->attack_target() != nullptr;
	}
	return false;
}

// Enemy military sites that cannot be attacked (for scout)
// Note that this also select ally's military sites. This is okay
// for scout, since those are stripped away later: Allied foreign
// msites are visible.

bool FindForeignMilitarysite::accept(const BaseImmovable& imm) const {
	if (upcast(MilitarySite const, ms, &imm)) {
		return &ms->owner() != &player;
	}
	return false;
}

bool FindImmovableByDescr::accept(const BaseImmovable& baseimm) const {
	if (upcast(const Immovable, imm, &baseimm)) {
		if (&imm->descr() == &descr) {
			return true;
		}
	}
	return false;
}

bool FindFlagOf::accept(const BaseImmovable& baseimm) const {
	if (upcast(const Flag, flag, &baseimm)) {
		if (Building* building = flag->get_building()) {
			if (finder.accept(*building)) {
				return true;
			}
		}
	}
	return false;
}

}  // namespace Widelands
