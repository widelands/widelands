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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/build_texture_atlas.h"

#include <memory>

#include "base/log.h"
#include "base/string.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture_atlas.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"

namespace {

// This is chosen so that all graphics for tribes are still well inside this
// threshold, but not background pictures.
constexpr int kMaxAreaForTextureAtlas = 240 * 240;

// Returns true if 'filename' ends with an image extension.
bool is_image(const std::string& filename) {
	return ends_with(filename, ".png") || ends_with(filename, ".jpg");
}

// Recursively adds all images in 'directory' to 'ordered_images' and
// 'handled_images' for which 'predicate' returns true. We keep track of the
// images twice because we want to make sure that some end up in the same
// (first) texture atlas, so we add them first and we use the set to know that
// we already added an image.
void find_images(const std::string& directory,
                 std::unordered_set<std::string>* images,
                 std::vector<std::string>* ordered_images) {
	for (const std::string& filename : g_fs->list_directory(directory)) {
		if (g_fs->is_directory(filename)) {
			find_images(filename, images, ordered_images);
			continue;
		}
		if (is_image(filename) && !images->count(filename)) {
			images->insert(filename);
			ordered_images->push_back(filename);
		}
	}
}

// If 'filename' should end up in the texture atlas, will load it into 'image'
// and return true.
bool should_be_packed(const std::string& filename, std::unique_ptr<Texture>* image) {
	if (ends_with(filename, ".jpg")) {
		return false;
	}
	*image = load_image(filename, g_fs);
	const auto area = (*image)->width() * (*image)->height();
	return area <= kMaxAreaForTextureAtlas;
}

// Pack the images in 'filenames' into texture atlases.
std::vector<std::unique_ptr<Texture>>
pack_images(const std::vector<std::string>& filenames,
            const int max_size,
            std::map<std::string, std::unique_ptr<Texture>>* textures_in_atlas) {
	std::vector<std::pair<std::string, std::unique_ptr<Texture>>> to_be_packed;
	for (const auto& filename : filenames) {
		std::unique_ptr<Texture> image;
		if (should_be_packed(filename, &image)) {
			to_be_packed.push_back(std::make_pair(filename, std::move(image)));
		}
	}

	TextureAtlas atlas;
	for (auto& pair : to_be_packed) {
		atlas.add(*pair.second);
	}

	std::vector<std::unique_ptr<Texture>> texture_atlases;
	std::vector<TextureAtlas::PackedTexture> packed_textures;
	atlas.pack(max_size, &texture_atlases, &packed_textures);

	for (size_t i = 0; i < to_be_packed.size(); ++i) {
		textures_in_atlas->insert(
		   std::make_pair(to_be_packed[i].first, std::move(packed_textures[i].texture)));
	}
	return texture_atlases;
}

}  // namespace

std::vector<std::unique_ptr<Texture>>
build_texture_atlas(const int max_size,
                    std::map<std::string, std::unique_ptr<Texture>>* textures_in_atlas) {
	if (max_size < kMinimumSizeForTextures) {
		throw wexception("The texture atlas must use at least %d as size (%d was given)",
		                 kMinimumSizeForTextures, max_size);
	}
	std::vector<std::string> first_atlas_images;
	std::unordered_set<std::string> all_images;

	// For terrain textures.
	find_images("world/terrains", &all_images, &first_atlas_images);
	// For flags and roads.
	find_images("tribes/initialization", &all_images, &first_atlas_images);
	// For UI elements mostly, but we get more than we need really.
	find_images("images", &all_images, &first_atlas_images);

	// New terrains defined by world add-ons, and road images defined by tribe add-ons,
	// need to be in the same texture atlas as all other terrains and roads.
	// So we put all world images and all tribe-initialization images into this atlas.
	for (const std::string& dir : g_fs->list_directory(kAddOnDir)) {
		std::string file = dir;
		file += FileSystem::file_separator();
		file += kAddOnMainFile;
		try {
			Profile profile(file.c_str());
			const std::string category =
			   profile.get_safe_section("global").get_safe_string("category");
			if (category == "world") {
				find_images(dir, &all_images, &first_atlas_images);
			} else if (category == "tribes") {
				std::string tribes_dir = dir;
				tribes_dir += FileSystem::file_separator();
				tribes_dir += "tribes";
				if (g_fs->is_directory(tribes_dir)) {
					find_images(tribes_dir, &all_images, &first_atlas_images);
				}
			}
		} catch (const std::exception& e) {
			log_warn("Ignoring unreadable add-ons profile %s: %s", file.c_str(), e.what());
		}
	}

	auto first_texture_atlas = pack_images(first_atlas_images, max_size, textures_in_atlas);
	if (first_texture_atlas.size() != 1) {
		throw wexception("Not all images that should fit in the first texture atlas did actually "
		                 "fit. Widelands has now more images than before.");
	}
	return first_texture_atlas;
}
