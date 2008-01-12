/*
* Copyright (C) 2002-2008 by the Widelands Development Team
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

#ifndef __S__CMD_CHECK_EVENTCHAIN_H
#define __S__CMD_CHECK_EVENTCHAIN_H

#include "cmd_queue.h"
#include "map_eventchain_manager.h"

namespace Widelands {

class Cmd_CheckEventChain : public GameLogicCommand {
private:
	MapEventChainManager::Index m_eventchain_id;

public:
	Cmd_CheckEventChain() : GameLogicCommand(0) {} // For savegame loading
	Cmd_CheckEventChain(int32_t, int32_t);

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_CHECK_EVENTCHAIN;} // Get this command id

	virtual void execute(Game *);
};

};

#endif
