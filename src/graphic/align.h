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
 * Horizontal alignment.
 *
 * bits 0,1 values 0,1,2,3
 */
enum HAlign {
	kLeft       = 0x00,
	kHCenter    = 0x01,
	kRight      = 0x02,
};

/** Vertical alignment.
 *
 * bits 2,3 values 0,4,8,12
 */

enum VAlign {
	kTop        = 0x00,
	kVCenter    = 0x04,
	kBottom     = 0x08,
};

HAlign mirror_alignment(HAlign alignment);

void correct_for_align(HAlign, uint32_t w, Vector2f* pt);
}
#endif  // end of include guard: WL_GRAPHIC_ALIGN_H
