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

#include "logic/buildcost.h"

#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/tribes/tribe_descr.h"

namespace Widelands {

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

		WareIndex index = tribe.ware_index(name);
		if (!tribe.has_ware(index)) {
			log("buildcost: tribe %s does not define ware %s", tribe.name().c_str(), name.c_str());
			fr.unsigned_8();
		} else {
			(*this)[index] = fr.unsigned_8();
		}
	}
}

} // namespace Widelands
