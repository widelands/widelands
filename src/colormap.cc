/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "colormap.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create a new Colormap, taking the palette as a parameter.
 * It automatically creates the colormap for shading.
 */
Colormap::Colormap (const SDL_Color & pal, const SDL_PixelFormat & format) {
	int32_t i, j, r, g, b;

	memcpy(palette, &pal, sizeof(palette));

	assert(format.BytesPerPixel == 2 or format.BytesPerPixel == 4);
	colormap = malloc(format.BytesPerPixel * 65536);

	for (i=0;i<256;i++)
		for (j=0;j<256;j++) {
			int32_t shade=(j<128)?j:(j-256);
			shade=256+2*shade;

			r = (palette[i].r*shade)>>8;
			g = (palette[i].g*shade)>>8;
			b = (palette[i].b*shade)>>8;

			if (r>255) r=255;
			if (g>255) g=255;
			if (b>255) b=255;

			const Uint32 value =
					SDL_MapRGB(&const_cast<SDL_PixelFormat &>(format), r, g, b);

			if (format.BytesPerPixel == 2)
				static_cast<Uint16 *>(colormap)[(j << 8) | i] = value;
			else
				static_cast<Uint32 *>(colormap)[(j << 8) | i] = value;
		}
}

/**
 * Clean up.
 */
Colormap::~Colormap ()
{
	free(colormap);
}
