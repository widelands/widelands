/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_IMAGE_H
#define WL_GRAPHIC_IMAGE_H

#include "base/macros.h"
#include "graphic/gl/blit_data.h"

/**
 * Interface to a bitmap that can act as the source of a rendering
 * operation.
 */
class Image {
public:
	Image() = default;
	virtual ~Image() {
	}

	// Dimensions of this Image in pixels.
	virtual int width() const = 0;
	virtual int height() const = 0;

	// OpenGL texture and texture coordinates backing this Image. This can
	// change at any time, so do not hold one to this value for more than one
	// frame.
	virtual const BlitData& blit_data() const = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(Image);
};

#endif  // end of include guard: WL_GRAPHIC_IMAGE_H
