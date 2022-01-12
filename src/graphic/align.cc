/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

namespace UI {

/**
 * Align pt horizontally to match align based on width w.
 *
 * When correcting for align, we never move from pixel boundaries to
 * sub-pixels, because this might lead from pixel-perfect rendering to
 * subsampled rendering - this can lead to blurry texts. That is why we
 * never do float divisions in this function.
 */
void correct_for_align(Align align, uint32_t w, Vector2i* pt) {
	if (align == Align::kCenter) {
		pt->x -= w / 2;
	} else if (align == Align::kRight) {
		pt->x -= w;
	}
}

/**
 * Adjust the y coordinate in 'point 'pt' to vertically center an element with height 'h'.
 */
void center_vertically(uint32_t h, Vector2i* pt) {
	pt->y -= h / 2;
}

/**
 * This mirrors the horizontal alignment for RTL languages.
 *
 * Do not store this value as it is based on the global font setting.
 */
Align mirror_alignment(Align alignment, bool is_rtl) {
	if (is_rtl) {
		switch (alignment) {
		case Align::kLeft:
			alignment = Align::kRight;
			break;
		case Align::kRight:
			alignment = Align::kLeft;
			break;
		case Align::kCenter:
			break;
		}
	}
	return alignment;
}

Rectf fit_image(float img_w, float img_h, float available_w, float available_h, bool crop) {
	if (crop ? (img_w < available_w || img_h < available_h) :
              (img_w < available_w && img_h < available_h)) {
		return Rectf((available_w - img_w) / 2.f, (available_h - img_h) / 2.f, img_w, img_h);
	}

	const float new_h = img_h * available_w / img_w;
	if (crop ? (new_h < available_h) : (new_h > available_h)) {
		const float new_w = img_w * available_h / img_h;
		return Rectf((available_w - new_w) / 2.f, 0, new_w, available_h);
	}

	return Rectf(0, (available_h - new_h) / 2.f, available_w, new_h);
}
}  // namespace UI
