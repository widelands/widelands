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

#ifndef WL_MAP_IO_MAP_IMAGES_H
#define WL_MAP_IO_MAP_IMAGES_H

#include "io/filesystem/filesystem.h"

namespace Widelands {

// Load all images from 'pics/*' in the map.
void load_map_images(FileSystem& filesystem);

// Copies all images from 'pics/*' in 'old_fs' into the 'new_fs'.
void save_map_images(FileSystem* new_fs, FileSystem* old_fs);

}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_IMAGES_H
