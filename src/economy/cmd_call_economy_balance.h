/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#ifndef S__CMD_CALL_ECONOMY_BALANCE_H
#define S__CMD_CALL_ECONOMY_BALANCE_H

#include "economy/flag.h"
#include "logic/cmd_queue.h"
#include "logic/instances.h"

namespace Widelands {
class Economy;
class Game;
class Map_Map_Object_Loader;


struct Cmd_Call_Economy_Balance : public GameLogicCommand {
	Cmd_Call_Economy_Balance () : GameLogicCommand(0), m_timerid(0) {} ///< for load and save

	Cmd_Call_Economy_Balance (int32_t starttime, Economy *, uint32_t timerid);

	void execute (Game &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_CALL_ECONOMY_BALANCE;}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

private:
	OPtr<Flag> m_flag;
	uint32_t m_timerid;
};

}

#endif
