/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "widelands_map_trigger_data_packet.h"

#include "logic/editor_game_base.h"
#include "map.h"
#include "trigger/trigger.h"
#include "trigger/trigger_factory.h"
#include "profile/profile.h"
#include "widelands_fileread.h"
#include "world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Trigger_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	if (!fr.TryOpen(fs, "trigger")) //  Skip, if no triggers saved.
		return;

	Profile prof;
	prof.read("trigger", 0, fs);

	try {
		uint32_t const packet_version =
			prof.get_safe_section("global").get_safe_positive("packet_version");
		if (packet_version <= CURRENT_PACKET_VERSION) {
			Manager<Trigger> & mtm = egbase.map().mtm();
			while (Section * const s = prof.get_next_section(0)) {
				char const * const name = s->get_name();
				try {
					bool         const set  = s->get_bool("set", false);
					char const * const type_name = s->get_safe_string("type");
					Trigger & trigger =
						Trigger_Factory::create(type_name, name, set);
					try {
						mtm.register_new(trigger);
					} catch (Manager<Trigger>::Already_Exists) {
						throw wexception("duplicated");
					}
					trigger.Read(*s, egbase);
				} catch (std::exception const & e) {
					throw wexception("%s: %s", name, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
		prof.check_used();
	} catch (_wexception const & e) {
		throw wexception("Triggers: %s", e.what());
	}
}


void Map_Trigger_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver * const)
throw (_wexception)
{

	Profile prof;
	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	Manager<Trigger> const & mtm = egbase.map().mtm();
	Manager<Trigger>::Index const nr_triggers = mtm.size();
	for (Manager<Trigger>::Index i = 0; i < nr_triggers; ++i) {
		const Trigger & trigger = mtm[i];
		Section & s = prof.create_section(trigger.name().c_str());
		if (trigger.is_set())
			s.set_bool("set", true);
		trigger.Write(s, egbase);
	}

	prof.write("trigger", false, fs);
}

}
