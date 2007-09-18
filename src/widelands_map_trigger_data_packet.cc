/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "fileread.h"
#include "map.h"
#include "map_trigger_manager.h"
#include "trigger/trigger.h"
#include "trigger/trigger_factory.h"
#include "profile.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Trigger_Data_Packet::~Widelands_Map_Trigger_Data_Packet() {}


void Widelands_Map_Trigger_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   if (skip)
      return;

   // Skip, if no triggers saved
   FileRead fr;
   if (!fr.TryOpen(fs, "trigger"))
      return;

   Profile prof;
   prof.read("trigger", 0, fs);

   Section* s = prof.get_section("global");

	const int packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
		while ((s = prof.get_next_section(0))) {
         std::string name = s->get_name();
         std::string type = s->get_safe_string("type");
         bool set = s->get_safe_bool("set");
         Trigger* t = Trigger_Factory::get_correct_trigger(type.c_str());
         t->set_name(name.c_str());
         t->set_trigger(set);
         t->Read(s, egbase);

         egbase->get_map()->get_mtm().register_new_trigger(t);
		}
	} else
		throw wexception
			("Unknown version in Map Trigger Packet: %i", packet_version);
}

/*
 * Write Function
 */
void Widelands_Map_Trigger_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{

   Profile prof;
	prof.create_section("global")->set_int
		("packet_version", CURRENT_PACKET_VERSION);

   // Now write all the triggers
	const MapTriggerManager & mtm = egbase->get_map()->get_mtm();
	const MapTriggerManager::Index nr_triggers = mtm.get_nr_triggers();
	for (MapTriggerManager::Index i = 0; i < nr_triggers; ++i) {
		const Trigger & t = mtm.get_trigger_by_nr(i);
		Section & s = *prof.create_section(t.get_name());
		s.set_string("type", t.get_id());
		s.set_bool  ("set",  t.is_set());
		t.Write(s);
	}

   prof.write("trigger", false, fs);

   // done
}
