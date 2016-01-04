/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GL_COORDINATE_CONVERSION_H
#define WL_GRAPHIC_GL_COORDINATE_CONVERSION_H

#include "base/rect.h"
#include "base/log.h" // NOCOM(#sirver): remove again

// Convert the 'rect' in pixel space into opengl space.
enum class ConversionMode {
	// Convert the rect as given.
	kLeftBottom,

	// Convert the rect so that the borders are in the center
	// of the pixels.
	kMidPoint,
};

// Converts the pixel (x, y) in a texture to a gl coordinate in [0, 1].
inline void pixel_to_gl_texture(const int width, const int height, float* x, float* y) {
	*x = (*x / width);
	*y = 1. - (*y / height);
}

// Converts the given pixel into an OpenGl point in the renderbuffer.
inline void pixel_to_gl_renderbuffer(const int width, const int height, float* x, float* y) {
	*x = (*x / width) * 2. - 1.;
	*y = 1. - (*y / height) * 2.;
}

// Converts 'rect' given on a screen of 'width' x 'height' pixels into a rect
// in opengl coordinates in a renderbuffer, i.e. in [-1, 1]. The edges The returned
// rectangle has positive width and height.
inline FloatRect
rect_to_gl_renderbuffer(const int width, const int height, const Rect& rect) {
	float left = rect.x;
	float top = rect.y;
	float right = rect.x + rect.w;
	float bottom = rect.y + rect.h;
	pixel_to_gl_renderbuffer(width, height, &left, &top);
	pixel_to_gl_renderbuffer(width, height, &right, &bottom);
	return FloatRect(left, bottom, right - left, top - bottom);
}

// Converts 'rect' given on a texture of 'width' x 'height' pixels into a rect
// in opengl coordinates in a texture, i.e. in [0, 1]. Texture pixels are sampled in their center.
// The returned rectangle has positive width and height.
inline FloatRect
rect_to_gl_texture(const int width, const int height, const FloatRect& rect) {
	log("#sirver width: %d,height: %d,rect.x: %f,rect.y: %f,rect.w: %f,rect.h: %f\n", width, height,
	    rect.x, rect.y, rect.w, rect.h);
	float left = rect.x;
	float top = rect.y;
	float right = rect.x + rect.w;
	float bottom = rect.y + rect.h;
	pixel_to_gl_texture(width, height, &left, &top);
	pixel_to_gl_texture(width, height, &right, &bottom);
	return FloatRect(left, bottom, right - left, top - bottom);
}

#endif  // end of include guard: WL_GRAPHIC_GL_COORDINATE_CONVERSION_H
