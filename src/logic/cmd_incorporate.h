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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef CMD_INCORPORATE_H
#define CMD_INCORPORATE_H

#include "logic/cmd_queue.h"
#include "logic/worker.h"

namespace Widelands {

#define CMD_INCORPORATE_VERSION 1

struct Cmd_Incorporate : public GameLogicCommand {
	Cmd_Incorporate() : GameLogicCommand(0) {} // For savegame loading
	Cmd_Incorporate (int32_t const t, Worker * const w)
		: GameLogicCommand(t), worker(w)
	{}

	void execute (Game & game) {worker->incorporate(game);}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual uint8_t id() const {return QUEUE_CMD_INCORPORATE;}

private:
	Worker * worker;
};

};

#endif
