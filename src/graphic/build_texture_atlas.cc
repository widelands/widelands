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

#include "graphic/build_texture_atlas.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include "build_info.h"
#include "config.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture_atlas.h"
#include "helper.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace {

// This is chosen so that all graphics for tribes are still well inside this
// threshold, but not background pictures.
constexpr int kMaxAreaForTextureAtlas = 240 * 240;

// A graphics card must at least support this size for texture for Widelands to
// run.
constexpr int kMinimumSizeForTextures = 2048;

// An image can either be Type::kPacked inside a texture atlas, in which case
// we need to keep track which one and where inside of that one. It can also be
// Type::kUnpacked if it is to be loaded from disk.
struct PackInfo {
	enum class Type {
		kUnpacked,
		kPacked,
	};

	Type type;
	int texture_atlas;
	Rect rect;
};

// Returns true if 'filename' is ends with a image extension.
bool is_image(const std::string& filename) {
	return boost::ends_with(filename, ".png") || boost::ends_with(filename, ".jpg");
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

void dump_result(const std::map<std::string, PackInfo>& pack_info,
                 std::vector<std::unique_ptr<Texture>>* texture_atlases,
                 FileSystem* fs) {
	// TODO(sirver): Maybe we want to use the cache directory for other things
	// in the future too, so nuking it here is probably rather extreme.
	if (fs->is_directory("cache")) {
		fs->fs_unlink("cache");
	}
	fs->ensure_directory_exists("cache");

	for (size_t i = 0; i < texture_atlases->size(); ++i) {
		std::unique_ptr<StreamWrite> sw(
		   fs->open_stream_write((boost::format("cache/texture_atlas_%02i.png") % i).str()));
		save_to_png(texture_atlases->at(i).get(), sw.get(), ColorType::RGBA);
	}

	{
		std::unique_ptr<StreamWrite> sw(fs->open_stream_write("cache/texture_atlas.lua"));
		sw->text("return {\n");
		sw->text((boost::format("   build_id = \"%s\",\n") % build_id()).str());
		for (const auto& pair : pack_info) {
			sw->text("   [\"");
			sw->text(pair.first);
			sw->text("\"] = {\n");

			switch (pair.second.type) {
			case PackInfo::Type::kPacked:
				sw->text("       type = \"packed\",\n");
				sw->text(
				   (boost::format("       texture_atlas = %d,\n") % pair.second.texture_atlas).str());
				sw->text((boost::format("       rect = { %d, %d, %d, %d },\n") % pair.second.rect.x %
				          pair.second.rect.y % pair.second.rect.w % pair.second.rect.h).str());
				break;

			case PackInfo::Type::kUnpacked:
				sw->text("       type = \"unpacked\",\n");
				break;
			}
			sw->text("   },\n");
		}
		sw->text("}\n");
	}
}

// If 'filename' should end up in the texture atlas, will load it into 'image'
// and return true.
bool should_be_packed(const std::string& filename, std::unique_ptr<Texture>* image) {
	// TODO(sirver): Maps should be self contained, i.e. come with everything
	// they need in their own directory. Unfortunately we reference the
	// background images in the elemental files by full path so that we can
	// reuse them for different campaigns. So we need to mention them in the
	// pack info file, but we do not want to have them in the texture atlas.
	if (boost::ends_with(filename, ".jpg") || boost::starts_with(filename, "campaigns/")) {
		return false;
	}
	*image = load_image(filename, g_fs);
	const auto area = (*image)->width() * (*image)->height();
	if (area > kMaxAreaForTextureAtlas) {
		return false;
	}
	return true;
}

// Pack the images in 'filenames' into texture atlases.
std::vector<std::unique_ptr<Texture>> pack_images(const std::vector<std::string>& filenames,
                                                  const int max_size,
                                                  std::map<std::string, PackInfo>* pack_info,
                                                  Texture* first_texture,
																  TextureAtlas::PackedTexture* first_atlas_packed_texture) {
	std::vector<std::pair<std::string, std::unique_ptr<Texture>>> to_be_packed;
	for (const auto& filename : filenames) {
		std::unique_ptr<Texture> image;
		if (should_be_packed(filename, &image)) {
			to_be_packed.push_back(std::make_pair(filename, std::move(image)));
		} else {
			pack_info->insert(std::make_pair(filename, PackInfo{
			                                              PackInfo::Type::kUnpacked, 0, Rect(),
			                                           }));
		}
	}

	TextureAtlas atlas;
	int packed_texture_index = 0;
	if (first_texture != nullptr) {
		atlas.add(*first_texture);
		packed_texture_index = 1;
	}
	for (auto& pair : to_be_packed) {
		atlas.add(*pair.second);
	}

	std::vector<std::unique_ptr<Texture>> texture_atlases;
	std::vector<TextureAtlas::PackedTexture> packed_textures;
	atlas.pack(max_size, &texture_atlases, &packed_textures);

	if (first_texture != nullptr) {
		assert(first_atlas_packed_texture != nullptr);
		*first_atlas_packed_texture = std::move(packed_textures[0]);
	}

	for (size_t i = 0; i < to_be_packed.size(); ++i) {
		const auto& packed_texture = packed_textures.at(packed_texture_index++);
		pack_info->insert(
		   std::make_pair(to_be_packed[i].first, PackInfo{PackInfo::Type::kPacked,
		                                                  packed_texture.texture_atlas,
		                                                  packed_texture.texture->blit_data().rect}));
	}
	return texture_atlases;
}

}  // namespace

void make_texture_atlas(const int max_size) {
	if (max_size < kMinimumSizeForTextures) {
		throw wexception("The texture atlas must use at least %d as size (%d was given)",
		                 kMinimumSizeForTextures, max_size);
	}
	// For performance reasons, we need to have some images in the first texture
	// atlas, so that OpenGL texture switches do not happen during (for example)
	// terrain or road rendering. To ensure this, we separate all images into
	// two disjunct sets. We than pack all images that should go into the first
	// texture atlas into a texture atlas. Than, we pack all remaining textures
	// into a texture atlas, but including the first texture atlas as a singular
	// image (which will probaby be the biggest we allow).
	//
	// We have to adjust the sub rectangle rendering for the images in the first
	// texture atlas in 'pack_info' later, before dumping the results.
	std::vector<std::string> other_images, images_that_must_be_in_first_atlas;
	std::unordered_set<std::string> all_images;

	// For terrain textures.
	find_images("world/terrains", &all_images, &images_that_must_be_in_first_atlas);
	// For flags and roads.
	find_images("tribes/images", &all_images, &images_that_must_be_in_first_atlas);
	// For UI elements mostly, but we get more than we need really.
	find_images("pics", &all_images, &images_that_must_be_in_first_atlas);

	// Add all other images, we do not really cares about the order for these.
	// TODO(sirver): having all assets in an assets/ sub directory would future
	// proof this a bit better.
	find_images("campaigns", &all_images, &other_images);
	find_images("pics", &all_images, &other_images);
	find_images("tribes", &all_images, &other_images);
	find_images("world", &all_images, &other_images);


	assert(images_that_must_be_in_first_atlas.size() + other_images.size() == all_images.size());

	std::map<std::string, PackInfo> first_texture_atlas_pack_info;
	auto first_texture_atlas = pack_images(images_that_must_be_in_first_atlas, max_size,
	                                       &first_texture_atlas_pack_info, nullptr, nullptr);
	if (first_texture_atlas.size() != 1) {
		throw wexception("Not all images that should fit in the first texture atlas did actually "
		                 "fit. Widelands has now more images than before.");
	}

	std::map<std::string, PackInfo> pack_info;
	TextureAtlas::PackedTexture first_atlas_packed_texture;
	auto texture_atlases = pack_images(other_images, max_size, &pack_info,
	                                   first_texture_atlas[0].get(), &first_atlas_packed_texture);

	const auto& blit_data = first_atlas_packed_texture.texture->blit_data();
	for (const auto& pair : first_texture_atlas_pack_info) {
		assert(pack_info.count(pair.first) == 0);
		pack_info.insert(std::make_pair(pair.first, PackInfo{
		                                               pair.second.type,
		                                               first_atlas_packed_texture.texture_atlas,
		                                               Rect(blit_data.rect.x + pair.second.rect.x,
		                                                    blit_data.rect.y + pair.second.rect.y,
		                                                    pair.second.rect.w, pair.second.rect.h),
		                                            }));
	}

	// Make sure we have all images.
	assert(all_images.size() == pack_info.size());
	dump_result(pack_info, &texture_atlases, g_fs);
}

bool is_texture_atlas_current() {
	if (!g_fs->file_exists("cache/texture_atlas.lua")) {
		return false;
	}

	LuaInterface lua;
	auto config = lua.run_script("cache/texture_atlas.lua");
	config->do_not_warn_about_unaccessed_keys();
	return config->get_string("build_id") == build_id();
}
