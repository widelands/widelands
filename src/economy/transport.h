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

#ifndef TRANSPORT_H
#define TRANSPORT_H

#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__ ((__format__ (__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif

#include "immovable.h"
#include "item_ware_descr.h"
#include "map.h"
#include "tattribute.h"
#include "trackptr.h"
#include "warelist.h"

#include "ui_unique_window.h"

#include <list>

#include "ware_instance.h"
#include "flag.h"
#include "road.h"
#include "route.h"
#include "transfer.h"

#include "supply.h"
#include "supply_list.h"

#include "wares_queue.h"

#include "economy.h"

namespace Widelands {


struct Cmd_Call_Economy_Balance : public GameLogicCommand {
	Cmd_Call_Economy_Balance () : GameLogicCommand (0) {} ///< for load and save

	Cmd_Call_Economy_Balance (int32_t starttime, Economy *, uint32_t timerid);

	void execute (Game &);

	virtual uint8_t id() const {return QUEUE_CMD_CALL_ECONOMY_BALANCE;}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

private:
	OPtr<Flag> m_flag;
	uint32_t m_timerid;
};

};

#endif
