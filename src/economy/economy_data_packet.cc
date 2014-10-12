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
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

constexpr uint16_t kCurrentPacketVersion = 3;

namespace Widelands {

void EconomyDataPacket::read(FileRead & fr)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			try {
				const TribeDescr & tribe = m_eco->owner().tribe();
				while (Time const last_modified = fr.unsigned_32()) {
					char const * const type_name = fr.c_string();
					uint32_t const permanent = fr.unsigned_32();
					WareIndex i = tribe.ware_index(type_name);
					if (i != INVALID_INDEX) {
						if (tribe.get_ware_descr(i)->default_target_quantity() ==
							 std::numeric_limits<uint32_t>::max())
							log("WARNING: target quantity configured for %s, "
								 "which should not have target quantity, "
								 "ignoring\n",
								 type_name);
						else {
							Economy::TargetQuantity & tq =
								m_eco->m_ware_target_quantities[i];
							if (tq.last_modified)
								throw GameDataError
									("duplicated entry for %s", type_name);
							tq.permanent         = permanent;
							tq.last_modified     = last_modified;
						}
					} else if ((i = tribe.worker_index(type_name)) != INVALID_INDEX) {
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
							Economy::TargetQuantity & tq =
								m_eco->m_worker_target_quantities[i];
							if (tq.last_modified)
								throw GameDataError
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
			} catch (const WException & e) {
				throw GameDataError("target quantities: %s", e.what());
			}
		m_eco->m_request_timerid = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception & e) {
		throw GameDataError("economy: %s", e.what());
	}
}

void EconomyDataPacket::write(FileWrite & fw)
{
	fw.unsigned_16(kCurrentPacketVersion);
	const TribeDescr & tribe = m_eco->owner().tribe();
	for (WareIndex i = tribe.get_nrwares(); i;) {
		--i;
		const Economy::TargetQuantity & tq =
			m_eco->m_ware_target_quantities[i];
		if (Time const last_modified = tq.last_modified) {
			fw.unsigned_32(last_modified);
			fw.c_string(tribe.get_ware_descr(i)->name());
			fw.unsigned_32(tq.permanent);
		}
	}
	for (WareIndex i = tribe.get_nrworkers(); i;) {
		--i;
		const Economy::TargetQuantity & tq =
			m_eco->m_worker_target_quantities[i];
		if (Time const last_modified = tq.last_modified) {
			fw.unsigned_32(last_modified);
			fw.c_string(tribe.get_worker_descr(i)->name());
			fw.unsigned_32(tq.permanent);
		}
	}
	fw.unsigned_32(0); //  terminator
	fw.unsigned_32(m_eco->m_request_timerid);
}

}
