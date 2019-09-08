/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_DRAW_TEXT_H
#define WL_LOGIC_MAP_OBJECTS_DRAW_TEXT_H

// Only declare powers of 2 and adjust the ~ operator below if you add any values to this enum
// class.
enum class TextToDraw {
	kNone = 0,
	kCensus = 1,
	kStatistics = 2,
};

inline TextToDraw operator|(TextToDraw a, TextToDraw b) {
	return static_cast<TextToDraw>(static_cast<int>(a) | static_cast<int>(b));
}
inline TextToDraw operator&(TextToDraw a, TextToDraw b) {
	return static_cast<TextToDraw>(static_cast<int>(a) & static_cast<int>(b));
}
inline TextToDraw removeFromTextToDraw(TextToDraw base, TextToDraw remove) {
	const int result = static_cast<int>(base) & ~static_cast<int>(remove);
	assert(result >= 0);
	assert(result <= 2);
	return static_cast<TextToDraw>(result);
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_DRAW_TEXT_H
