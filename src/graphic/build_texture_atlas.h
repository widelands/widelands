/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_BUILD_TEXTURE_ATLAS_H
#define WL_GRAPHIC_BUILD_TEXTURE_ATLAS_H

// Builds a texture atlas where no texture is bigger than 'max_size' x
// 'max_size' using the images that ship with Widelands and dump them into a
// 'cache' directory in the writable path.
void make_texture_atlas(int max_size);

// Returns true if there is a texture atlas on disk and it has the same
// build_id() as the running binary.
bool is_texture_atlas_current();

#endif  // end of include guard: WL_GRAPHIC_BUILD_TEXTURE_ATLAS_H
