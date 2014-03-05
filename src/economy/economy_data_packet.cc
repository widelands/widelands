/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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

#include "economy/economy_data_packet.h"

#include "economy/economy.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"

#define CURRENT_ECONOMY_VERSION 3

namespace Widelands {

void EconomyDataPacket::Read(FileRead & fr)
{
	uint16_t const version = fr.Unsigned16();

	try {
		if (1 <= version and version <= CURRENT_ECONOMY_VERSION) {
			if (2 <= version)
				try {
					const Tribe_Descr & tribe = m_eco->owner().tribe();
					while (Time const last_modified = fr.Unsigned32()) {
						char const * const type_name = fr.CString();
						uint32_t const permanent = fr.Unsigned32();
						if (version <= 2)
							fr.Unsigned32();
						if (Ware_Index i = tribe.ware_index(type_name)) {
							if
								(tribe.get_ware_descr(i)->default_target_quantity()
								 ==
								 std::numeric_limits<uint32_t>::max())
								log
									("WARNING: target quantity configured for %s, "
									 "which should not have target quantity, "
									 "ignoring\n",
									 type_name);
							else {
								Economy::Target_Quantity & tq =
									m_eco->m_ware_target_quantities[i.value()];
								if (tq.last_modified)
									throw game_data_error
										("duplicated entry for %s", type_name);
								tq.permanent         = permanent;
								tq.last_modified     = last_modified;
							}
						} else if ((i = tribe.worker_index(type_name))) {
							if
								(tribe.get_worker_descr(i)->default_target_quantity()
								 ==
								 std::numeric_limits<uint32_t>::max())
								log
									("WARNING: target quantity configured for %s, "
									 "which should not have target quantity, "
									 "ignoring\n",
									 type_name);
							else {
								Economy::Target_Quantity & tq =
									m_eco->m_worker_target_quantities[i.value()];
								if (tq.last_modified)
									throw game_data_error
										("duplicated entry for %s", type_name);
								tq.permanent         = permanent;
								tq.last_modified     = last_modified;
							}
						} else
							log
								("WARNING: target quantity configured for \"%s\", "
								 "which is not a ware or worker type defined in tribe "
								 "%s, ignoring\n",
								 type_name, tribe.name().c_str());
					}
				} catch (const _wexception & e) {
					throw game_data_error("target quantities: %s", e.what());
				}
			m_eco->m_request_timerid = fr.Unsigned32();
		} else {
			throw game_data_error("unknown version %u", version);
		}
	} catch (const std::exception & e) {
		throw game_data_error("economy: %s", e.what());
	}
}

void EconomyDataPacket::Write(FileWrite & fw)
{
	fw.Unsigned16(CURRENT_ECONOMY_VERSION);
	const Tribe_Descr & tribe = m_eco->owner().tribe();
	for (Ware_Index i = tribe.get_nrwares(); i.value();) {
		--i;
		const Economy::Target_Quantity & tq =
			m_eco->m_ware_target_quantities[i.value()];
		if (Time const last_modified = tq.last_modified) {
			fw.Unsigned32(last_modified);
			fw.CString(tribe.get_ware_descr(i)->name());
			fw.Unsigned32(tq.permanent);
		}
	}
	for (Ware_Index i = tribe.get_nrworkers(); i.value();) {
		--i;
		const Economy::Target_Quantity & tq =
			m_eco->m_worker_target_quantities[i.value()];
		if (Time const last_modified = tq.last_modified) {
			fw.Unsigned32(last_modified);
			fw.CString(tribe.get_worker_descr(i)->name());
			fw.Unsigned32(tq.permanent);
		}
	}
	fw.Unsigned32(0); //  terminator
	fw.Unsigned32(m_eco->m_request_timerid);
}

}
