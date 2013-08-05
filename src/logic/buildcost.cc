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

#include "logic/tribe.h"
#include "profile/profile.h"
#include "wexception.h"

namespace Widelands {

void Buildcost::parse(const Tribe_Descr & tribe, Section & buildcost_s)
{
	while (Section::Value const * const val = buildcost_s.get_next_val())
		try {
			if (Ware_Index const idx = tribe.ware_index(val->get_name())) {
				if (count(idx))
					throw wexception
						("a buildcost item of this ware type has already been "
						 "defined");
				int32_t const value = val->get_int();
				if (value < 1 or 255 < value)
					throw wexception("count is out of range 1 .. 255");
				insert(std::pair<Ware_Index, uint8_t>(idx, value));
			} else
				throw wexception
					("tribe does not define a ware type with this name");
		} catch (const _wexception & e) {
			throw wexception
				("[buildcost] \"%s=%s\": %s",
				 val->get_name(), val->get_string(), e.what());
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

void Buildcost::save
	(Widelands::FileWrite & fw, const Widelands::Tribe_Descr & tribe) const
{
	for (const_iterator it = begin(); it != end(); ++it) {
		fw.CString(tribe.get_ware_descr(it->first)->name());
		fw.Unsigned8(it->second);
	}
	fw.CString("");
}

void Buildcost::load
	(Widelands::FileRead & fr, const Widelands::Tribe_Descr & tribe)
{
	clear();

	for (;;) {
		std::string name = fr.CString();
		if (name.empty())
			break;

		Ware_Index index = tribe.ware_index(name);
		if (!index) {
			log("buildcost: tribe %s does not define ware %s", tribe.name().c_str(), name.c_str());
			fr.Unsigned8();
		} else {
			(*this)[index] = fr.Unsigned8();
		}
	}
}

} // namespace Widelands
