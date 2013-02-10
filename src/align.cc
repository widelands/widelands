/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include "align.h"

namespace UI {

void correct_for_align(Align align, uint32_t w, uint32_t h, Point* pt) {
	//Vertical Align
	if (align & (Align_VCenter | Align_Bottom)) {
		if (align & Align_VCenter)
			pt->y -= h / 2;
		else
			pt->y -= h;
	}

	//Horizontal Align
	if ((align & Align_Horizontal) != Align_Left) {
		if (align & Align_HCenter)
			pt->x -= w / 2;
		else if (align & Align_Right)
			pt->x -= w;
	}
}

}  // namespace UI

