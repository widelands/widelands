/*
 * Copyright (C) 2016 by the Widelands Development Team
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

#include "graphic/playercolor.h"

#include "graphic/texture.h"

Image* playercolor_image(const RGBColor* clr, const Image* image, const Image* color_mask) {
	int w = image->width();
	int h = image->height();
	Texture* rv = new Texture(w, h);
	rv->fill_rect(Rectf(0, 0, w, h), RGBAColor(0, 0, 0, 0));
	rv->blit_blended(Rectf(0, 0, w, h), *image, *color_mask, Rectf(0, 0, w, h), *clr);
	return rv;
}

Image* playercolor_image(int player_number, const Image* image, const Image* color_mask) {
	return playercolor_image(&kPlayerColors[player_number], image, color_mask);
}
