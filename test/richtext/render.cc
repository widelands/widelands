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

#include <iostream>
#include <fstream>
#include <string>

#include <SDL.h>
#undef main // No, we do not want SDL_main
#include "lodepng.h"

#include "rt_parse.h"
#include "rt_render.h"
#include "sdl_ttf_font.h"
#include "lodepng_image_loader.h"

using namespace std;

int save_png(string fn, SDL_Surface * surf) {
	std::vector<unsigned char> image;
	image.resize(surf->w*surf->h*4);

	// Copy pixel data
	for (uint32_t y = 0; y < surf->h; ++y) {
		for (uint32_t x = 0; x < surf->w; ++x) {
			for (int i = 0; i < 4; ++i)
				image[y*surf->w+x+i] = static_cast<unsigned char*>(surf->pixels)[y*surf->pitch + x + i];
		}
	}

	// Save png data
	std::vector<unsigned char> png;

	lodepng::State st;

	st.encoder.auto_convert = LAC_NO;
	st.encoder.force_palette = LAC_NO;
	vector<unsigned char> out;
	int error = lodepng::encode
		(out, static_cast<const unsigned char*>(surf->pixels), surf->w, surf->h, st);
	if(error) {
		std::cout << "PNG encoding error: " << lodepng_error_text(error) << std::endl;
		return 0;
	}

	lodepng::save_file(out, fn);

	return 0;
}

string read_stdin(void) {
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

int parse_arguments(int argc, char** argv, int32_t * w, string & outname, string & inname) {
	if (argc < 2) {
		cout << "Usage: render <width in pixels> [outname] [inname] < input.txt" << endl << endl <<
			"input.txt should contain a valid rich text formatting" << endl;
		return 1;
	}

	*w = strtol(argv[1], 0, 10);
	if (argc > 2)
		outname = argv[2];
	else
		outname = "out.png";

	if (argc > 3)
		inname = argv[3];
	else
		inname = "-";

	return 0;
}


int main(int argc, char *argv[])
{
	int32_t w;

	string outname, inname;
	if (parse_arguments(argc, argv, &w, outname, inname))
		return 0;

	SDL_Init(SDL_INIT_VIDEO);

	string txt;
	if (inname == "-")
		txt = read_stdin();
	else
		txt = read_file(inname);

	RT::IFontLoader * floader = RT::ttf_fontloader_from_file();
	RT::IImageLoader * imgl = setup_lodepng_img_loader();
	RT::IRenderer * renderer = RT::setup_renderer(floader, imgl);

	SDL_Surface * surface = 0;
	try {
		surface = renderer->render(txt, w);
	} catch(RT::Exception & e) {
			cout << e.what() << endl;
	}

	if (surface) {
		SDL_LockSurface(surface);
		save_png(outname, surface);
		SDL_UnlockSurface(surface);
		SDL_FreeSurface(surface);
	}

	delete renderer;

	SDL_Quit();

	return 0;
}
