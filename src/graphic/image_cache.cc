/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"

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
	images_.insert(std::make_pair(hash, std::move(image)));
	return return_value;
}

void ImageCache::fill_with_texture_atlases(
   std::vector<std::unique_ptr<Texture>> texture_atlases,
   std::map<std::string, std::unique_ptr<Texture>> textures_in_atlas) {
	texture_atlases_ = std::move(texture_atlases);
	for (auto& pair : textures_in_atlas) {
		images_.insert(std::move(pair));
	}
}

/** Lazy accees to _images via hash.
 *
 * In case hash is not not found it will we fetched via load_image().
 */
const Image* ImageCache::get(const std::string& hash) {
	auto it = images_.find(hash);
	if (it == images_.end()) {
		return images_.insert(std::make_pair(hash, load_image(hash))).first->second.get();
	}
	return it->second.get();
}
