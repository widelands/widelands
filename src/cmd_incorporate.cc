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

#include "cmd_incorporate.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "wexception.h"


void Cmd_Incorporate::Read
(WidelandsFileRead               & fr,
 Editor_Game_Base                & egbase,
 Widelands_Map_Map_Object_Loader & mol)
{
	const Uint16 packet_version = fr.Unsigned16();
	if (packet_version == CMD_INCORPORATE_VERSION) {
		// Read Base Commands
		GameLogicCommand::Read(fr, egbase, mol);

		// Serial of worker
		const Uint32 fileserial = fr.Unsigned32();
		assert(mol.is_object_known(fileserial)); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
		worker = dynamic_cast<Worker *>(mol.get_object_by_file_index(fileserial)); //  FIXME check
	} else
		throw wexception
			("Unknown version in Cmd_Incorporate::Read: %u", packet_version);
}


void Cmd_Incorporate::Write
(WidelandsFileWrite             & fw,
 Editor_Game_Base               & egbase,
 Widelands_Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(CMD_INCORPORATE_VERSION);

	// Write base classes
	GameLogicCommand::Write(fw, egbase, mos);

	// Now serial
	assert(mos.is_object_known(worker));
	fw.Unsigned32(mos.get_object_file_index(worker));
}
