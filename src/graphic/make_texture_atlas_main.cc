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

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <SDL.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#undef main // No, we do not want SDL_main

#include "base/log.h"
#include "config.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/texture_atlas.h"
#include "helper.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"

namespace {

// This is chosen so that all graphics for tribes are still well inside this
// threshold, but not background pictures.
constexpr int kMaxAreaForTextureAtlas = 240 * 240;

// An image can either be Type::kPacked inside a texture atlas, in which case
// we need to keep track which one and where inside of that one. It can also be
// Type::kUnpacked if it is to be loaded from disk.
struct PackInfo {
	enum class Type {
		kUnpacked,
		kPacked,
	};

	Type type;
	Rect rect;
};

// NOCOM(#sirver): needs modification
int parse_arguments(
   int argc, char** argv, std::string* input_directory)
{
	if (argc < 2) {
		std::cout << "Usage: wl_make_texture_atlas <input directory>" << std::endl << std::endl
		          << "Will write output.png in the current directory." << std::endl;
		return 1;
	}
	*input_directory = argv[1];
	return 0;
}

// Setup the static objects Widelands needs to operate and initializes systems.
void initialize() {
	SDL_Init(SDL_INIT_VIDEO);

	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));
	g_gr = new Graphic(1, 1, false);
}

// Returns true if 'filename' is ends with a image extension.
bool is_image(const std::string& filename) {
	return boost::ends_with(filename, ".png") || boost::ends_with(filename, ".jpg");
}

// Recursively adds all images in 'directory' to 'ordered_images' and
// 'handled_images'. We keep track of the images twice because we want to make
// sure that some end up in the same (first) texture atlas, so we add them
// first and we use the set to know that we already added an image.
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

void find_all_images(std::vector<std::string>* all_images,
                     std::vector<std::string>* images_that_must_be_in_first_atlas) {
	std::unordered_set<std::string> image_set;
	// The first texture atlas should contain all terrain textures and all road
	// textures, so we make sure we add them nice and early.
	find_images("world/terrains", &image_set, all_images);
	find_images("tribes/images", &image_set, all_images);

	// Make a copy, so we can check that they are indeed all in the first texture atlas.
	*images_that_must_be_in_first_atlas = *all_images;

	// Add all other images, we do not really cares about the order now.
	find_images("pics", &image_set, all_images);
	// NOCOM(#sirver): bring back
	// find_images("world", &image_set, all_images);
	// find_images("tribes", &image_set, all_images);
}

void dump_result(const std::map<std::string, PackInfo>& pack_info, Texture* packed_texture, FileSystem* fs) {
	{
		std::unique_ptr<StreamWrite> sw(fs->open_stream_write("output.png"));
		save_to_png(packed_texture, sw.get(), ColorType::RGBA);
	}

	{
		std::unique_ptr<StreamWrite> sw(fs->open_stream_write("output.lua"));
		sw->text("return {\n");
		for (const auto& pair : pack_info) {
			sw->text("   [\"");
			sw->text(pair.first);
			sw->text("\"] = {\n");

			switch (pair.second.type) {
			case PackInfo::Type::kPacked:
				sw->text("       type = \"packed\",\n");
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

}  // namespace

int main(int argc, char** argv) {
	std::string input_directory;
	if (parse_arguments(argc, argv, &input_directory))
		return 1;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDLInit did not succeed: " << SDL_GetError() << std::endl;
		return 1;
	}
	initialize();

	std::vector<std::string> all_images, images_that_must_be_in_first_atlas;
	find_all_images(&all_images, &images_that_must_be_in_first_atlas);

	std::map<std::string, PackInfo> pack_info;
	std::vector<std::pair<std::string, std::unique_ptr<Texture>>> to_be_packed;
	for (const auto& filename : all_images) {
		std::unique_ptr<Texture> image = load_image(filename, g_fs);
		const auto area = image->width() * image->height();
		if (area < kMaxAreaForTextureAtlas) {
			to_be_packed.push_back(std::make_pair(filename, std::move(image)));
		} else {
			pack_info[filename] = PackInfo{
			   PackInfo::Type::kUnpacked, Rect(),
			};
		}
	}

	TextureAtlas atlas;
	for (auto& pair : to_be_packed) {
		atlas.add(*pair.second);
	}

	// NOCOM(#sirver): figure out max size
	std::vector<std::unique_ptr<Texture>> new_textures;
	auto packed_texture = atlas.pack(&new_textures);
	for (size_t i = 0; i < new_textures.size(); ++i) {
		const BlitData& blit_data = new_textures[i]->blit_data();
		pack_info[to_be_packed[i].first] = PackInfo{PackInfo::Type::kPacked, blit_data.rect};
	}

	std::unique_ptr<FileSystem> output_fs(&FileSystem::create("."));
	dump_result(pack_info, packed_texture.get(), output_fs.get());

	SDL_Quit();
	return 0;
}
