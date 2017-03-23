/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_ROADTYPE_H
#define WL_LOGIC_ROADTYPE_H

namespace Widelands {

// TODO(SirVer): This are two enums in one. They do not have a true logical
// connection.
//
// The first 5 enums define what kind of roads this are, the other three in
// which direction they point for a field. Still, the first few are needed for
// the drawing code to keep around what kind of road it should draw. I guess
// that is the reason why it was combined in the first place. Still it is ugly.
enum RoadType {
	kNone = 0,
	kNormal = 1,
	kBusy = 2,
	kWater = 3,
	kMask = 3,

	kEast = 0,  //  shift values
	kSouthEast = 2,
	kSouthWest = 4,
};
}

#endif  // end of include guard: WL_LOGIC_ROADTYPE_H
