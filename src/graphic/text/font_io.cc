/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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

#include "graphic/text/font_io.h"

#include <memory>

#include <boost/format.hpp>

#include "graphic/text/rt_errors.h"
#include "graphic/text/sdl_ttf_font.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

namespace RT {

IFont* load_font(const std::string& face, int ptsize) {
	std::string filename = "i18n/fonts/";
	filename += face;

	// Some older versions of sdl_ttf seem to rely on this block of memory to
	// remain intact, therefore, we keep it around till the program exits and
	// this class is destroyed.
	std::unique_ptr<std::string> memory;
	{
		std::unique_ptr<FileRead> fr(new FileRead());
		fr->open(*g_fs, filename);
		memory.reset(new std::string(fr->data(0), fr->get_size()));
	}

	SDL_RWops* ops = SDL_RWFromConstMem(memory->data(), memory->size());
	if (!ops) {
		throw BadFont("could not load font!: RWops Pointer invalid");
	}

	TTF_Font* font = TTF_OpenFontIndexRW(ops, true, ptsize, 0);
	if (!font) {
		throw BadFont(
		   (boost::format("Font loading error for %s, %i pts: %s") % face % ptsize % TTF_GetError())
		      .str());
	}

	return new SdlTtfFont(font, face, ptsize, memory.release());
}
}  // namespace RT
