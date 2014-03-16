/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "logic/cmd_incorporate.h"

#include "i18n.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "wexception.h"

namespace Widelands {

void Cmd_Incorporate::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_INCORPORATE_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			uint32_t const worker_serial = fr.Unsigned32();
			try {
				worker = &mol.get<Worker>(worker_serial);
			} catch (const _wexception & e) {
				throw wexception("worker %u: %s", worker_serial, e.what());
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw wexception("incorporate: %s", e.what());
	}
}


void Cmd_Incorporate::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_INCORPORATE_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	assert(mos.is_object_known(*worker));
	fw.Unsigned32(mos.get_object_file_index(*worker));
}

}
