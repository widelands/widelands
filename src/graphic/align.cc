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
			case Align::Align_BottomLeft:
				alignment = Align::Align_BottomRight;
				break;
			case Align::Align_BottomRight:
				alignment = Align::Align_BottomLeft;
				break;
			case Align::Align_CenterLeft:
				alignment = Align::Align_CenterRight;
				break;
			case Align::Align_CenterRight:
				alignment = Align::Align_CenterLeft;
				break;
			case Align::Align_TopLeft:
				alignment = Align::Align_TopRight;
				break;
			case Align::Align_TopRight:
				alignment = Align::Align_TopLeft;
				break;
			default:
				break;
		}
	}
	return alignment;
}

void correct_for_align(Align align, uint32_t w, uint32_t h, Point* pt) {
	//Vertical Align
	if (static_cast<int>(align & (Align::Align_VCenter | Align::Align_Bottom))) {
		if (static_cast<int>(align & Align::Align_VCenter))
			pt->y -= h / 2;
		else
			pt->y -= h;
	}

	//Horizontal Align
	if ((align & Align::Align_Horizontal) != Align::Align_Left) {
		if (static_cast<int>(align & Align::Align_HCenter))
			pt->x -= w / 2;
		else if (static_cast<int>(align & Align::Align_Right))
			pt->x -= w;
	}
}

}  // namespace UI

