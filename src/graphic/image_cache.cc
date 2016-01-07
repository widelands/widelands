/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include "graphic/image_cache.h"

#include <cassert>
#include <memory>
#include <set>
#include <string>

#include <SDL.h>
#include <boost/format.hpp>

#include "graphic/image.h"
#include "base/log.h" // NOCOM(#sirver): remove again
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "io/fileread.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_table.h"
#include "scripting/lua_interface.h"

static bool _initialized = false;

ImageCache::ImageCache() {
}

ImageCache::~ImageCache() {
}

bool ImageCache::has(const std::string& hash) const {
	return images_.count(hash);
}

const Image* ImageCache::insert(const std::string& hash, std::unique_ptr<const Image> image) {
	assert(!has(hash));
	const Image* return_value = image.get();
	images_.emplace(hash, std::move(image));
	return return_value;
}

const Image* ImageCache::get(const std::string& image_hash) {
	// NOCOM(#sirver): ugly hack.
	if (!_initialized) {
		_initialized = true;
		LuaInterface lua;

		for (int i = 0; i < 100; ++i) {
			const auto filename = (boost::format("cache/output_%02d.png") % i).str();
			if (!g_fs->file_exists(filename)) {
				break;
			}
			texture_atlases_.emplace_back(load_image(filename));
		}

		// NOCOM(#sirver): output is a rather stupid name.
		auto config = lua.run_script("cache/output.lua");
		for (const auto& hash : config->keys<std::string>()) {
			auto image_config = config->get_table(hash);
			if (image_config->get_string("type") == "unpacked") {
				images_.emplace(hash, load_image(hash));
			} else {
				int texture_atlas_index = image_config->get_int("texture_atlas");
				const auto& parent = texture_atlases_[texture_atlas_index]->blit_data();
				auto rect_config = image_config->get_table("rect");
				const Rect subrect(rect_config->get_int(1), rect_config->get_int(2),
						rect_config->get_int(3), rect_config->get_int(4));
				images_.emplace(hash, std::unique_ptr<Texture>(new Texture(
								parent.texture_id, subrect, parent.rect.w, parent.rect.h)));
			}
		}
	}
	auto it = images_.find(image_hash);
	if (it == images_.end()) {
		log("#sirver not found :(hash: %s\n", image_hash.c_str());
		images_.emplace(image_hash, std::move(load_image(image_hash)));
		return get(image_hash);
	}
	return it->second.get();
}
