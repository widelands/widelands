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
#include <vector>

#include <SDL.h>
#include <boost/algorithm/string/predicate.hpp>

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

	std::vector<std::unique_ptr<Texture>> images;
	std::unique_ptr<FileSystem> input_fs(&FileSystem::create(input_directory));
	std::vector<std::string> png_filenames;
	for (const std::string& filename : input_fs->list_directory("")) {
		if (boost::ends_with(filename, ".png")) {
			png_filenames.push_back(filename);
			images.emplace_back(load_image(filename, input_fs.get()));
		}
	}

	TextureAtlas atlas;
	for (auto& image : images) {
		atlas.add(*image);
	}
	std::vector<std::unique_ptr<Texture>> new_textures;
	auto packed_texture = atlas.pack(&new_textures);

	std::unique_ptr<FileSystem> output_fs(&FileSystem::create("."));
	std::unique_ptr<StreamWrite> sw(output_fs->open_stream_write("output.png"));
	save_to_png(packed_texture.get(), sw.get(), ColorType::RGBA);

	for (size_t i = 0; i < new_textures.size(); ++i) {
		log("%s:\n", FileSystem::fs_filename(png_filenames[i].c_str()));
		const FloatRect coords = new_textures[i]->texture_coordinates();
		log(" original   w:%d, h:%d\n", images[i]->width(), images[i]->height());
		log(" gl texture x:%.7f, y:%.7f, w:%.7f, h:%.7f\n", coords.x, coords.y, coords.w, coords.h);
		log(" as float   x:%.7f, y:%.7f, w:%.7f, h:%.7f\n", coords.x * packed_texture->width(),
		    coords.y * packed_texture->height(), coords.w * packed_texture->width(),
		    coords.h * packed_texture->height());
		log(" as int     x:%d, y:%d, w:%d, h:%d\n", static_cast<int>(coords.x * packed_texture->width()),
		    static_cast<int>(coords.y * packed_texture->height()),
		    static_cast<int>(coords.w * packed_texture->width()),
		    static_cast<int>(coords.h * packed_texture->height()));
	}

	SDL_Quit();
	return 0;
}
