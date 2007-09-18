/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "widelands_map_variable_data_packet.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "map.h"
#include "map_variable_manager.h"
#include "profile.h"


#define CURRENT_PACKET_VERSION 1

Widelands_Map_Variable_Data_Packet::~Widelands_Map_Variable_Data_Packet() {}


void Widelands_Map_Variable_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   if (skip)
      return;

   Profile prof;
	try {prof.read("variable", 0, fs);} catch (...) {return;}
   MapVariableManager & mvm = egbase->get_map()->get_mvm();

   Section* s = prof.get_section("global");

   const int packet_version = s->get_int("packet_version");
	if (packet_version == CURRENT_PACKET_VERSION) {
		while ((s = prof.get_next_section(0))) {
         std::string type = s->get_safe_string("type");
			if (type == "int") {
            Int_MapVariable* v = new Int_MapVariable(s->get_safe_bool("delete_protected"));
            v->set_name(s->get_name());
            v->set_value(s->get_safe_int("value"));
            mvm.register_new_variable(v);
			} else if (type == "string") {
            String_MapVariable* v = new String_MapVariable(s->get_safe_bool("delete_protected"));
            v->set_name(s->get_name());
            v->set_value(s->get_safe_string("value"));
            mvm.register_new_variable(v);
			} else
            throw wexception("Unknown Map Variable type %s", type.c_str());

		}
	} else
		throw wexception
			("Unknown version in Map Variable Data Packet: %i", packet_version);
}


/*
 * Write Function
 */
void Widelands_Map_Variable_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
	prof.create_section("global")->set_int
		("packet_version", CURRENT_PACKET_VERSION);

   // Now, all positions in order, first x, then y
	const MapVariableManager & mvm = egbase->get_map()->get_mvm();
	const MapVariableManager::Index nr_variables =
		mvm.get_nr_variables();
	for (MapVariableManager::Index i = 0; i < nr_variables; ++i) {
		const MapVariable & v = mvm.get_variable_by_nr(i);
      Section & s = *prof.create_section(v.get_name());
      s.set_bool("delete_protected", v.is_delete_protected());
		switch (v.get_type()) {
		case MapVariable::MVT_INT:
			s.set_string("type", "int");
			s.set_int("value", static_cast<const Int_MapVariable &>(v).get_value());
         break;

		case MapVariable::MVT_STRING:
			s.set_string("type", "string");
			s.set_string("value", static_cast<const String_MapVariable &>(v).get_value());
         break;

		default:
			throw wexception
				("Unknown Variable type in Widelands_Map_Variable_Data_Packet: %i",
				 v.get_type());
		}
	}

   prof.write("variable", false, fs);
}
