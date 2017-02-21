/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_ALIGN_H
#define WL_GRAPHIC_ALIGN_H

#include "base/vector.h"

namespace UI {

/**
 * This Enum is a binary mix of one-dimensional and two-dimensional alignments.
 *
 * bits 0,1 values 0,1,2,3  are horizontal
 * bits 2,3 values 0,4,8,12 are vertical
 *
 * mixed aligenments are results of a binary | operation.
 */

// TODO(klaus.halfmann): as this is not a real enum all compiler warnings about
// incomplete usage are useless.

enum class Align {
	kLeft = 0,
	kHCenter = 1,
	kRight = 2,
	kHorizontal = 3,

	kTop = 0,
	kVCenter = 4,
	kBottom = 8,
	kVertical = 12,

	kTopLeft = 0,
	kCenterLeft = Align::kVCenter,
	kBottomLeft = Align::kBottom,

	kTopCenter = Align::kHCenter,
	kCenter = Align::kHCenter | Align::kVCenter,
	kBottomCenter = Align::kHCenter | Align::kBottom,

	kTopRight = Align::kRight,
	kCenterRight = Align::kRight | Align::kVCenter,

	kBottomRight = Align::kRight | Align::kBottom,
};

inline Align operator&(Align a, Align b) {
	return static_cast<Align>(static_cast<int>(a) & static_cast<int>(b));
}

inline Align operator|(Align a, Align b) {
	return static_cast<Align>(static_cast<int>(a) | static_cast<int>(b));
}

Align mirror_alignment(Align alignment);
void correct_for_align(Align, uint32_t w, uint32_t h, Vector2f* pt);
}
#endif  // end of include guard: WL_GRAPHIC_ALIGN_H
