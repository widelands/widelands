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

#include <SDL.h>
#include <SDL_image.h>

#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "wexception.h"

#include "surface.h"

#include "image_loader_impl.h"

using namespace std;

Surface* ImageLoaderImpl::load(const string& fname) const {
	FileRead fr;
	//log("Loading image %s.\n", fname.c_str());

	fr.fastOpen(*g_fs, fname.c_str());
	SDL_Surface* sdlsurf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);

	if (!sdlsurf)
		throw wexception("Could not open image %s: %s", fname.c_str(), IMG_GetError());

	return Surface::create(sdlsurf);
}

