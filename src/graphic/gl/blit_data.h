/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_GL_BLIT_DATA_H
#define WL_GRAPHIC_GL_BLIT_DATA_H

#include <cstdint>

#include "base/rect.h"

// Information of the internal OpenGL data needed to properly blit this image.
struct BlitData {
	// The OpenGl name or id of the parent texture. The parent texture is either
	// - the packed texture of the texture atlas, if this texture is part of a texture atlas.
	// - the texture itself if it is a standalone texture.
	uint32_t texture_id;

	// Dimension of the parent texture, For stand alone textures this is the
	// dimensions of the texture itself and therefore equal to
	// rect.[w|h].
	int parent_width;
	int parent_height;

	// The subrect in the parent texture.
	Rectf rect;
};

#endif  // end of include guard: WL_GRAPHIC_GL_BLIT_DATA_H
