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

#include "graphic/align.h"

#include "graphic/font_handler1.h"
#include "graphic/text/font_set.h"

namespace UI {

// This mirrors the horizontal alignment for RTL languages.
Align mirror_alignment(Align alignment) {
	if (UI::g_fh1->fontset().is_rtl()) {
		switch (alignment) {
			case Align::kBottomLeft:
				alignment = Align::kBottomRight;
				break;
			case Align::kBottomRight:
				alignment = Align::kBottomLeft;
				break;
			case Align::kCenterLeft:
				alignment = Align::kCenterRight;
				break;
			case Align::kCenterRight:
				alignment = Align::kCenterLeft;
				break;
			case Align::kTopLeft:
				alignment = Align::kTopRight;
				break;
			case Align::kTopRight:
				alignment = Align::kTopLeft;
				break;
			default:
				break;
		}
	}
	return alignment;
}

void correct_for_align(Align align, uint32_t w, uint32_t h, Point* pt) {
	//Vertical Align
	if (static_cast<int>(align & (Align::kVCenter | Align::kBottom))) {
		if (static_cast<int>(align & Align::kVCenter))
			pt->y -= h / 2;
		else
			pt->y -= h;
	}

	//Horizontal Align
	if ((align & Align::kHorizontal) != Align::kLeft) {
		if (static_cast<int>(align & Align::kHCenter))
			pt->x -= w / 2;
		else if (static_cast<int>(align & Align::kRight))
			pt->x -= w;
	}
}

}  // namespace UI

