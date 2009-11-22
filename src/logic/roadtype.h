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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef S__ROADTYPE_H
#define S__ROADTYPE_H

namespace Widelands {

/** \todo: This are two enums in one. They do not have a true logical
 * connection.
 *
 * The first 5 enums define what kind of roads this are, the other three in
 * which direction they point for a field. Still, the first few are needed for
 * the drawing code to keep around what kind of road it should draw. I guess
 * that is the reason why it was combined in the first place. Still it is ugly.
 *
 * SirVer
 */
enum RoadType {
	Road_None = 0,
	Road_Normal = 1,
	Road_Busy = 2,
	Road_Water = 3,
	Road_Mask = 3,

	Road_East      = 0, //  shift values
	Road_SouthEast = 2,
	Road_SouthWest = 4,
};

}

#endif


