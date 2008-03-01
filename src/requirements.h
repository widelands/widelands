/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

#include <vector>

#include "tattribute.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"

namespace Widelands {

class Map_Object;
class Editor_Game_Base;
class Map_Map_Object_Loader;
class Map_Map_Object_Saver;


/**
 * Requirements can be attached to Requests.
 *
 * Requirements are matched to a \ref Map_Object 's \ref tAttribute as
 * returned by \ref get_tattribute .
 */
struct Requirements {
	Requirements ();

	void clear();
	void set(tAttribute at, int32_t min, int32_t max);

	/**
	 * \return \c true if the object satisfies the requirements.
	 */
	bool check(Map_Object* obj) const;

	// For Save/Load Games
	void Read(FileRead  *, Editor_Game_Base *, Map_Map_Object_Loader *);
	void Write(FileWrite *, Editor_Game_Base *, Map_Map_Object_Saver  *);

private:
	struct MinMax {
		tAttribute at;
		int32_t min;
		int32_t max;

		MinMax(tAttribute _at, int32_t _min, int32_t _max)
			: at(_at), min(_min), max(_max) {}
	};

	std::vector<MinMax> r;
};

}

#endif // REQUIREMENTS_H
