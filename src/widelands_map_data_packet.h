/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef __S__WIDELANDS_MAP_DATA_PACKET_H
#define __S__WIDELANDS_MAP_DATA_PACKET_H

#include "wexception.h"
#include "widelands_filewrite.h"

class FileSystem;

namespace Widelands {

class Editor_Game_Base;
class Map_Map_Object_Loader;
class Map_Map_Object_Saver;

/** This class represents a data packet in a widelands map file. it is an
 * abstract base class
*/
struct Map_Data_Packet {
	virtual ~Map_Data_Packet() {m_skip = false;}

	virtual void Read
		(FileSystem &,
		 Editor_Game_Base*,
		 const bool,
		 Map_Map_Object_Loader * = 0)
		throw (_wexception)
		= 0;
	virtual void Write
		(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * = 0)
		throw (_wexception)
		= 0;

private:
      bool m_skip;
};

};

#endif
