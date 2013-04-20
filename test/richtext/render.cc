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

#include <fstream>
#include <iostream>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <SDL.h>
#undef main // No, we do not want SDL_main
#include "lodepng.h"

#include "graphic/image_cache.h"
#include "graphic/surface_cache.h"
#include "graphic/render/sdl_surface.h"
#include "graphic/text/rt_parse.h"
#include "graphic/text/rt_render.h"
#include "graphic/text/sdl_ttf_font.h"

#include "from_file_image_loader.h"

using namespace std;

int save_png(const string& fn, const SDLSurface& surf) {
	// Save png data
	std::vector<unsigned char> png;

	lodepng::State st;

	st.encoder.auto_convert = LAC_NO;
	st.encoder.force_palette = LAC_NO;
	vector<unsigned char> out;
	int error = lodepng::encode
		(out, static_cast<const unsigned char*>(surf.get_pixels()), surf.width(), surf.height(), st);
	if (error) {
		std::cout << "PNG encoding error: " << lodepng_error_text(error) << std::endl;
		return 0;
	}

	lodepng::save_file(out, fn);

	return 0;
}

string read_stdin() {
	string txt;
	while (not cin.eof()) {
		string line;
		getline(cin, line);
		txt += line + "\n";
	}
	return txt;
}

string read_file(string fn) {
	string txt;
	ifstream f;
	f.open(fn.c_str());
	while (not f.eof()) {
		string line;
		getline(f, line);
		txt += line + "\n";
	}
	return txt;
}

int parse_arguments
	(int argc, char** argv, int32_t* w, string & outname, string & inname, set<string> & allowed_tags)
{
	if (argc < 4) {
		cout << "Usage: render <width in pixels> <outname> <inname> [allowed tag1] [allowed tags2] ... < "
			"input.txt" << endl << endl << "input.txt should contain a valid rich text formatting" << endl;
		return 1;
	}

	*w = strtol(argv[1], 0, 10);
	outname = argv[2];
	inname = argv[3];

	for (int i = 4; i < argc; i++)
		allowed_tags.insert(argv[i]);

	return 0;
}

int main(int argc, char** argv)
{
	int32_t w;
	set<string> allowed_tags;

	string outname, inname;
	if (parse_arguments(argc, argv, &w, outname, inname, allowed_tags))
		return 0;

	SDL_Init(SDL_INIT_VIDEO);

	string txt;
	if (inname == "-")
		txt = read_stdin();
	else
		txt = read_file(inname);

	boost::scoped_ptr<IImageLoader> image_loader(create_from_file_image_loader());
	boost::scoped_ptr<SurfaceCache> surface_cache(create_surface_cache(500 << 20));  // 500 MB
	boost::scoped_ptr<ImageCache> image_cache(create_image_cache(image_loader.get(), surface_cache.get()));
	boost::scoped_ptr<RT::IRenderer> renderer
		(RT::setup_renderer(image_cache.get(), surface_cache.get(), RT::ttf_fontloader_from_file("../../fonts")));

	try {
		SDLSurface& surf = *static_cast<SDLSurface*>
			(renderer->render(txt, w, allowed_tags));
		surf.lock(Surface::Lock_Normal);
		save_png(outname, surf);
		surf.unlock(Surface::Unlock_NoChange);
	} catch (RT::Exception & e) {
		cout << e.what() << endl;
	}

	SDL_Quit();

	return 0;
}
