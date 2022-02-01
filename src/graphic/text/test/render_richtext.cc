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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>

#undef main  // No, we do not want SDL_main

#include "config.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/rendertarget.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/test/render.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/streamwrite.h"

namespace {

std::string read_stdin() {
	std::string txt;
	while (!std::cin.eof()) {
		std::string line;
		getline(std::cin, line);
		txt += line + "\n";
	}
	return txt;
}

std::string read_file(std::string fn) {
	std::string txt;
	std::ifstream f;
	f.open(fn.c_str());
	if (!f.good()) {
		std::cerr << "Could not open " << fn << std::endl;
		return "";
	}

	while (f.good()) {
		std::string line;
		getline(f, line);
		txt += line + "\n";
	}
	return txt;
}

int parse_arguments(int argc,
                    char** argv,
                    int32_t* w,
                    std::string& outname,
                    std::string& inname,
                    std::set<std::string>& allowed_tags) {
	if (argc < 4) {
		std::cout << "Usage: render <width in pixels> <outname> <inname> [allowed tag1] [allowed "
		             "tags2] ... < "
		             "input.txt"
		          << std::endl
		          << std::endl
		          << "input.txt should contain a valid rich text formatting" << std::endl;
		return 1;
	}

	*w = strtol(argv[1], 0, 10);
	outname = argv[2];
	inname = argv[3];

	for (int i = 4; i < argc; i++)
		allowed_tags.insert(argv[i]);

	return 0;
}

// Setup the static objects Widelands needs to operate and initializes systems.
void initialize() {
	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));

	g_gr = new Graphic();
	g_gr->initialize(Graphic::TraceGl::kNo, 1, 1, false);
}

}  // namespace

int main(int argc, char** argv) {
	int32_t w;
	std::set<std::string> allowed_tags;

	std::string outname, inname;
	if (parse_arguments(argc, argv, &w, outname, inname, allowed_tags))
		return 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDLInit did not succeed: " << SDL_GetError() << std::endl;
		return 1;
	}

	if (TTF_Init() == -1) {
		std::cerr << "True Type library did not initialize: " << TTF_GetError() << std::endl;
		return 1;
	}

	std::string txt;
	if (inname == "-")
		txt = read_stdin();
	else
		txt = read_file(inname);
	if (txt.empty()) {
		return 1;
	}

	initialize();

	StandaloneRenderer standalone_renderer;

	try {
		std::shared_ptr<const UI::RenderedText> rendered_text =
		   standalone_renderer.renderer()->render(txt, w, allowed_tags);
		std::unique_ptr<Texture> texture(
		   new Texture(rendered_text->width(), rendered_text->height()));
		std::unique_ptr<RenderTarget> dst(new RenderTarget(texture.get()));
		rendered_text->draw(*dst, Vector2i::zero());

		std::unique_ptr<FileSystem> fs(&FileSystem::create("."));
		std::unique_ptr<StreamWrite> sw(fs->open_stream_write(outname));

		if (!save_to_png(texture.get(), sw.get(), ColorType::RGBA)) {
			std::cout << "Could not encode PNG." << std::endl;
		}
	} catch (RT::Exception& e) {
		std::cout << e.what() << std::endl;
	}

	SDL_Quit();

	return 0;
}
