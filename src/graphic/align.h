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
 *
 * TODO klaus.halfmann as this is not a real enum all compiler warnings about
 *      incomplete usage are useless.
 */
enum class Align {
	kLeft       = 0x00,
	kHCenter    = 0x01,
	kRight      = 0x02,
	kHorizontal = 0x03,

	kTop        = 0x00,
	kVCenter    = 0x04,
	kBottom     = 0x08,
	kVertical   = 0x0C,

	kTopLeft        = kLeft | kTop,
	kCenterLeft     = kLeft | kVCenter,
	kBottomLeft     = kLeft | kBottom,

    kTopCenter      = kHCenter | kTop,
	kCenter         = kHCenter | kVCenter,
	kBottomCenter   = kHCenter | kBottom,

	kTopRight       = kRight | kTop,
	kCenterRight    = kRight | kVCenter,
	kBottomRight    = kRight | kBottom,
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
