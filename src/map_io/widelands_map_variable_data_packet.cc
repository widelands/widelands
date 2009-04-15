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

#include "widelands_map_variable_data_packet.h"

#include "editor_game_base.h"
#include "map.h"

#include "profile.h"

#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Variable_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	Profile prof;
	try {prof.read("variable", 0, fs);} catch (...) {return;}
	Manager<Variable> & mvm = egbase.map().mvm();

	try {
		int32_t const packet_version =
			prof.get_safe_section("global").get_safe_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			while (Section * const s = prof.get_next_section(0)) {
				char const * const      name = s->get_name();
				try {
					char const * const type_name = s->get_safe_string("type");
					bool const delete_protected =
						s->get_safe_bool("delete_protected");
					Variable * variable;
					if      (not strcmp(type_name, "int")) {
						Variable_Int    & v = *new Variable_Int   (delete_protected);
						v.set_value(s->get_safe_int("value"));
						variable = &v;
					} else if (not strcmp(type_name, "string")) {
						Variable_String & v = *new Variable_String(delete_protected);
						v.set_value(s->get_safe_string("value"));
						variable = &v;
					} else
						throw wexception("invalid type \"%s\"", type_name);
					variable->set_name(name);
					try {
						mvm.register_new(*variable);
					} catch (Manager<Variable>::Already_Exists) {
						throw wexception("duplicated");
					}
				} catch (_wexception const & e) {
					throw wexception("variable %s: %s", name, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("variables: %s", e.what());
	}
}


void Map_Variable_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver * const)
throw (_wexception)
{
	Profile prof;
	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	//  Now, all positions in order, first x, then y.
	Manager<Variable> const & mvm = egbase.map().mvm();
	Manager<Variable>::Index const nr_variables = mvm.size();
	for (Manager<Variable>::Index i = 0; i < nr_variables; ++i) {
		Variable const & variable = mvm[i];
		Section & s = prof.create_section(variable.name().c_str());
		s.set_bool("delete_protected", variable.is_delete_protected());
		if        (upcast(Variable_Int    const, ivar, &variable)) {
			s.set_string("type", "int");
			s.set_int   ("value", ivar->get_value());
		} else if (upcast(Variable_String const, svar, &variable)) {
			s.set_string("type", "string");
			s.set_string("value", svar->get_value());
		} else
			throw wexception("Unknown Variable type in Map_Variable_Data_Packet");
	}

	prof.write("variable", false, fs);
}

};
