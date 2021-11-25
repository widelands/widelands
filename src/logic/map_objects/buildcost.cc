/*
 * Copyright (C) 2010-2021 by the Widelands Development Team
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

#include <memory>

#include "base/log.h"
#include "base/wexception.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/tribe_descr.h"

namespace Widelands {

Buildcost::Buildcost(std::unique_ptr<LuaTable> table, Widelands::Descriptions& descriptions) {
	for (const std::string& warename : table->keys<std::string>()) {
		DescriptionIndex const idx = descriptions.load_ware(warename);

		// Read value
		int32_t value = table->get_int(warename);
		if (value < 1) {
			throw GameDataError("Ware count needs to be > 0.\nEmpty buildcost "
			                    "tables are allowed if you wish to have an amount of 0.");
		}
		if (value > 255) {
			throw GameDataError("Ware count needs to be <= 255.");
		}

		// Add
		insert(std::pair<DescriptionIndex, uint8_t>(idx, value));
	}
}

/**
 * Compute the total buildcost.
 */
Widelands::Quantity Buildcost::total() const {
	Widelands::Quantity sum = 0;
	for (const auto& item : *this) {
		sum += item.second;
	}
	return sum;
}

void Buildcost::save(FileWrite& fw, const Widelands::TribeDescr& tribe) const {
	for (const auto& item : *this) {
		fw.c_string(tribe.get_ware_descr(item.first)->name());
		fw.unsigned_8(item.second);
	}
	fw.c_string("");
}

void Buildcost::load(FileRead& fr, const Widelands::TribeDescr& tribe) {
	clear();

	for (;;) {
		std::string name = fr.c_string();
		if (name.empty()) {
			break;
		}

		DescriptionIndex index = tribe.ware_index(name);
		if (!tribe.has_ware(index)) {
			log_warn(
			   "buildcost: tribe %s does not define ware %s", tribe.name().c_str(), name.c_str());
			fr.unsigned_8();
		} else {
			(*this)[index] = fr.unsigned_8();
		}
	}
}

}  // namespace Widelands
