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

#ifndef WIDELANDS_MAP_SAVER_H
#define WIDELANDS_MAP_SAVER_H

#include "wexception.h"

struct FileSystem;

namespace Widelands {

struct Editor_Game_Base;
struct Map_Map_Object_Saver;

/*
===========================

This class saves a widelands map into a file system

throws wexecption on failure

NOTE: The widelands map format is mostly binary, but some stuff is
ascii. The binary is done for speeds sake, and the ASCII for easier
debugability

===========================
*/
struct Map_Saver {
	Map_Saver(FileSystem &, Editor_Game_Base*);
	~Map_Saver();

	void save() throw (_wexception);
	Map_Map_Object_Saver * get_map_object_saver() {return m_mos;}

private:
	Editor_Game_Base               * m_egbase;
	FileSystem & m_fs;
	Map_Map_Object_Saver * m_mos;
};

};

#endif
