/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include "game.h"
#include "game_data_error.h"

#include "cmd_calculate_statistics.h"


namespace Widelands {

void Cmd_CalculateStatistics::execute (Game & game) {
	game.sample_statistics();
	game.enqueue_command
		(new Cmd_CalculateStatistics
		 (game.get_gametime() + STATISTICS_SAMPLE_TIME));
}

#define CMD_CALCULATE_STATISTICS_VERSION 1
void Cmd_CalculateStatistics::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CMD_CALCULATE_STATISTICS_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("calculate statistics function: %s"), e.what());
	}
}
void Cmd_CalculateStatistics::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CMD_CALCULATE_STATISTICS_VERSION);
	GameLogicCommand::Write(fw, egbase, mos);

}

}

