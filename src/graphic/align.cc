/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

/**
 * This mirrors the horizontal alignment for RTL languages.
 *
 * Do not store this value as it is based on the global font setting.
 */
HAlign mirror_alignment(HAlign alignment) {
	if (UI::g_fh1->fontset()->is_rtl()) {
		switch (alignment) {
		case HAlign::kLeft:
			alignment = HAlign::kRight;
			break;
		case HAlign::kRight:
			alignment = HAlign::kLeft;
			break;
		case HAlign::kHCenter:
			break;
		}
	}
	return alignment;
}


/**
 * Align pt horizontally to match align based on width w and height h.
 *
 * When correcting for align, we never move from pixel boundaries to
 * sub-pixels, because this might lead from pixel-perfect rendering to
 * subsampled rendering - this can lead to blurry texts. That is why we
 * never do float divisions in this function.
 */
void correct_for_align(HAlign align, uint32_t w, Vector2f* pt) {

	if (align == HAlign::kHCenter)
		pt->x -= w / 2;
	else if (align == HAlign::kRight)
		pt->x -= w;
}

}  // namespace UI
