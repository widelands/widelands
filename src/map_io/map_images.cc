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

#include "map_io/map_images.h"

#include "graphic/image_cache.h"
#include "graphic/image_io.h"

namespace Widelands {

void load_map_images(FileSystem& fs) {
	// Read all pics.
	if (!fs.file_exists("pics") || !fs.is_directory("pics")) {
		return;
	}
	for (const std::string& pname : fs.list_directory("pics")) {
		if (fs.is_directory(pname)) {
			continue;
		}
		const std::string hash = std::string("map:") + FileSystem::fs_filename(pname.c_str());
		if (!g_image_cache->has(hash)) {
			g_image_cache->insert(hash, load_image(pname, &fs));
		}
	}
}

void save_map_images(FileSystem* new_fs, FileSystem* map_fs) {
	if (!map_fs || !map_fs->file_exists("pics") || !map_fs->is_directory("pics")) {
		return;
	}
	new_fs->ensure_directory_exists("pics");
	for (const std::string& picture : map_fs->list_directory("pics")) {
		size_t length;
		void* input_data = map_fs->load(picture, length);
		new_fs->write(picture, input_data, length);
		free(input_data);
	}
}
}  // namespace Widelands
