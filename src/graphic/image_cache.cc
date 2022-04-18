/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "graphic/image_cache.h"

#include <cassert>
#include <memory>

#include "base/multithreading.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"

ImageCache* g_image_cache;

bool ImageCache::has(const std::string& hash) const {
	return images_.count(hash) != 0u;
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
	// TODO(Nordfriese): There may still be pointers to existing images and
	// textures, so we need to keep them around but no longer return any new
	// pointers to them. It would be better to fetch all existing pointers afresh.
	// This will require some non-trivial refactoring to all UI widgets that store
	// `Image` pointers or references – either pass ownership of the respective
	// textures to them, or let them store only the string key and call `get()`
	// every time they need to draw or query the texture.

	for (auto& t : texture_atlases_) {
		outdated_texture_atlases_.push_back(std::move(t));
	}
	texture_atlases_ = std::move(texture_atlases);

	for (auto& pair : images_) {
		outdated_images_.push_back(std::move(pair.second));
	}
	images_.clear();
	for (auto& pair : textures_in_atlas) {
		images_.insert(std::move(pair));
	}
}

/** Lazy access to images_ via hash.
 *
 * In case hash is not not found it will we fetched via load_image().
 */
const Image* ImageCache::get(const std::string& hash) {
	auto it = images_.find(hash);
	if (it == images_.end()) {
		NoteThreadSafeFunction::instantiate(
		   [this, hash]() { images_.insert(std::make_pair(hash, load_image(hash))); }, true);
		it = images_.find(hash);
		assert(it != images_.end());
	}
	return it->second.get();
}
