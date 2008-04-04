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

#include "widelands_map_immovabledata_data_packet.h"

#include "editor_game_base.h"
#include "immovable.h"
#include "immovable_program.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "log.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Immovabledata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      *,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {
		fr.Open(fs, "binary/immovable_data");
	} catch (...) {
		// not there, so skip
		return;
	}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			for (;;) {
				Serial const serial = fr.Unsigned32();
				//  FIXME Just test EndOfFile instead in the next packet version.
				if (serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw wexception
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Immovable & imm = ol->get<Immovable>(serial);
					Immovable_Descr const & descr = imm.descr();

					// Animation
					char const * const animname = fr.CString();
					try {
						imm.m_anim = descr.get_animation(animname);
					} catch (Map_Object_Descr::Animation_Nonexistent) {
						imm.m_anim = descr.main_animation();
						log
							("WARNING: Animation \"%s\" not found, using animation "
							 "%s).\n",
							 animname, descr.get_animation_name(imm.m_anim).c_str());
					}
					imm.m_animstart = fr.Signed32();

					//  program
					if (fr.Unsigned8())
						imm.m_program = descr.get_program(fr.CString());
					else
						imm.m_program = 0;
					imm.m_program_ptr = fr.Unsigned32();
					if (!imm.m_program) {
						imm.m_program_ptr = 0;
					} else {
						if (imm.m_program->get_size() <= imm.m_program_ptr) {
							//  Try to not fail if the program of some immovable has
							//  changed significantly. Note that in some cases, the
							//  immovable may end up broken despite the fixup, but
							//  there is not really anything we can do against that.
							log
								("Warning: Immovable '%s', size of program '%s' seems "
								 "to have changed.\n",
								 descr.name().c_str(),
								 imm.m_program->get_name().c_str());
							imm.m_program_ptr = 0;
						}
					}
					imm.m_program_step = fr.Signed32();

					ol->mark_object_as_loaded(&imm);
				} catch (_wexception const & e) {
					throw wexception("immovable %u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("immovable data: %s", e.what());
	}
}


void Map_Immovabledata_Data_Packet::Write
	(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Immovable_Data_Packet is obsolete");
}

};
