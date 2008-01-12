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

#ifndef __S__WIDELANDS_MAP_BOBDATA_DATA_PACKET_H
#define __S__WIDELANDS_MAP_BOBDATA_DATA_PACKET_H

#include "widelands_fileread.h"
#include "widelands_map_data_packet.h"

namespace Widelands {

class Worker;
class Critter_Bob;

/*
 * This cares for the data of bobs
 */
struct Map_Bobdata_Data_Packet : public Map_Data_Packet {
	virtual void Read
		(FileSystem &,
		 Editor_Game_Base*,
		 const bool,
		 Map_Map_Object_Loader * = 0)
		throw (_wexception);
	void Write(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * = 0)
		throw (_wexception);

private:
	void write_critter_bob
		(FileWrite *, Editor_Game_Base *, Map_Map_Object_Saver *, Critter_Bob *);
	void read_critter_bob
		(FileRead *, Editor_Game_Base *, Map_Map_Object_Loader *, Critter_Bob *);
	void write_worker_bob
		(FileWrite *, Editor_Game_Base *, Map_Map_Object_Saver  *, Worker *);
	void read_worker_bob
		(FileRead  *, Editor_Game_Base *, Map_Map_Object_Loader *, Worker *);
};

};

#endif
