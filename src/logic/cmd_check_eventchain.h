/*
* Copyright (C) 2002-2009 by the Widelands Development Team
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

#ifndef CMD_CHECK_EVENTCHAIN_H
#define CMD_CHECK_EVENTCHAIN_H

#include "cmd_queue.h"

#include "manager.h"

namespace Widelands {

struct EventChain;

struct Cmd_CheckEventChain : public GameLogicCommand {
	Cmd_CheckEventChain() : GameLogicCommand(0) {} // For savegame loading
	Cmd_CheckEventChain(int32_t const _duetime, uint16_t const _eventchain_id) :
		GameLogicCommand(_duetime), m_eventchain_id(_eventchain_id)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual uint8_t id() const {return QUEUE_CMD_CHECK_EVENTCHAIN;}

	virtual void execute(Game &);

private:
	Manager<EventChain>::Index m_eventchain_id;
};

}

#endif
