/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#include "logic/map_objects/buildcost.h"

#include <map>
#include <memory>

#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"

namespace Widelands {

Buildcost::Buildcost() : std::map<DescriptionIndex, uint8_t>() {}

Buildcost::Buildcost(std::unique_ptr<LuaTable> table, const Tribes& tribes) :
	std::map<DescriptionIndex, uint8_t>() {
	for (const std::string& warename : table->keys<std::string>()) {
		int32_t value = INVALID_INDEX;
		try {
			DescriptionIndex const idx = tribes.safe_ware_index(warename);
			if (count(idx)) {
				throw GameDataError(
					"A buildcost item of this ware type has already been defined: %s", warename.c_str());
			}
			value = table->get_int(warename);
			const uint8_t ware_count = value;
			if (ware_count != value) {
				throw GameDataError("Ware count is out of range 1 .. 255");
			}
			insert(std::pair<DescriptionIndex, uint8_t>(idx, ware_count));
		} catch (const WException& e) {
			throw GameDataError("[buildcost] \"%s=%d\": %s", warename.c_str(), value, e.what());
		}
	}
}

/**
 * Compute the total buildcost.
 */
uint32_t Buildcost::total() const
{
	uint32_t sum = 0;
	for (const_iterator it = begin(); it != end(); ++it)
		sum += it->second;
	return sum;
}

void Buildcost::save(FileWrite& fw, const Widelands::TribeDescr& tribe) const {
	for (const_iterator it = begin(); it != end(); ++it) {
		fw.c_string(tribe.get_ware_descr(it->first)->name());
		fw.unsigned_8(it->second);
	}
	fw.c_string("");
}

void Buildcost::load(FileRead& fr, const Widelands::TribeDescr& tribe) {
	clear();

	for (;;) {
		std::string name = fr.c_string();
		if (name.empty())
			break;

		DescriptionIndex index = tribe.ware_index(name);
		if (!tribe.has_ware(index)) {
			log("buildcost: tribe %s does not define ware %s", tribe.name().c_str(), name.c_str());
			fr.unsigned_8();
		} else {
			(*this)[index] = fr.unsigned_8();
		}
	}
}

} // namespace Widelands
