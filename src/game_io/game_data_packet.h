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

#ifndef GAME_DATA_PACKET_H
#define GAME_DATA_PACKET_H

#include "wexception.h"

class FileSystem;

namespace Widelands {

class Game;
class Map_Map_Object_Loader;
struct Map_Map_Object_Saver;

/*
========================================

This class represents a data packet in a widelands
saved game file. it is an abstract base class

========================================
*/
struct Game_Data_Packet {
	virtual ~Game_Data_Packet() {}
	virtual void Read (FileSystem &, Game &, Map_Map_Object_Loader * = nullptr) = 0;
	virtual void Write(FileSystem &, Game &, Map_Map_Object_Saver  * = nullptr) = 0;
};

}

#endif
