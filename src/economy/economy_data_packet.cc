/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#include "economy_data_packet.h"

#include "economy.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "map_io/widelands_map_map_object_loader.h"

#include "tribe.h"
#include "logic/player.h"


#define CURRENT_ECONOMY_VERSION 2

namespace Widelands {

void EconomyDataPacket::Read(FileRead & fr)
{
	uint16_t const version = fr.Unsigned16();

	try {
		if (1 <= version and version <= CURRENT_ECONOMY_VERSION) {
			if (2 <= version)
				try {
					Tribe_Descr const & tribe = m_eco->owner().tribe();
					while (Time const last_modified = fr.Unsigned32()) {
						char const * const ware_type_name = fr.CString();
						uint32_t const permanent = fr.Unsigned32();
						uint32_t const temporary = fr.Unsigned32();
						Ware_Index const ware_type =
							tribe.ware_index(ware_type_name);
						if (not ware_type)
							log
								("WARNING: target quantity configured for \"%s\", "
								 "which is not a ware type defined in tribe %s, "
								 "ignoring\n",
								 ware_type_name, tribe.name().c_str());
						else if
							(tribe.get_ware_descr(ware_type)->default_target_quantity()
							 ==
							 std::numeric_limits<uint32_t>::max())
							log
								("WARNING: target quantity configured for %s, which "
								 "should not have target quantity, ignoring\n",
								 ware_type_name);
						else {
							Economy::Target_Quantity & tq =
								m_eco->m_target_quantities[ware_type.value()];
							if (tq.last_modified)
								throw wexception
									("duplicated entry for %s", ware_type_name);
							tq.permanent         = permanent;
							tq.temporary         = temporary;
							tq.last_modified     = last_modified;
						}
					}
				} catch (_wexception const & e) {
					throw wexception("target quantities: %s", e.what());
				}
			m_eco->m_request_timerid = fr.Unsigned32();
		} else {
			throw wexception("unknown version %u", version);
		}
	} catch (std::exception const & e) {
		throw wexception("economy: %s", e.what());
	}
}

void EconomyDataPacket::Write(FileWrite & fw)
{
	fw.Unsigned16(CURRENT_ECONOMY_VERSION);
	Tribe_Descr const & tribe = m_eco->owner().tribe();
	for (Ware_Index i = tribe.get_nrwares(); i.value();) {
		--i;
		Economy::Target_Quantity const & tq =
			m_eco->m_target_quantities[i.value()];
		if (Time const last_modified = tq.last_modified) {
			fw.Unsigned32(last_modified);
			fw.CString(tribe.get_ware_descr(i)->name());
			fw.Unsigned32(tq.permanent);
			fw.Unsigned32(tq.temporary);
		}
	}
	fw.Unsigned32(0); //  terminator
	fw.Unsigned32(m_eco->m_request_timerid);
}

}
