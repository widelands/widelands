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

#ifndef WL_GRAPHIC_IMAGE_IO_H
#define WL_GRAPHIC_IMAGE_IO_H

#include <memory>

#include "base/wexception.h"

class FileSystem;
class Texture;
class StreamWrite;
struct SDL_Surface;

class ImageNotFound : public WException {
public:
	explicit ImageNotFound(const std::string& fn) : wexception("Image not found: %s", fn.c_str()) {
	}
};

class ImageLoadingError : public WException {
public:
	ImageLoadingError(const std::string& fn, const std::string& reason)
	   : wexception("Error loading %s: %s", fn.c_str(), reason.c_str()) {
	}
};

/// Loads the image 'fn' from 'fs'.
std::unique_ptr<Texture> load_image(const std::string& fn, FileSystem* fs = nullptr);

/// Loads the image 'fn' from 'fs' into an SDL_Surface. Caller must SDL_FreeSurface() the returned
/// value.
SDL_Surface* load_image_as_sdl_surface(const std::string& fn, FileSystem* fs = nullptr);

/// Saves the 'texture' to 'sw' as a PNG.
enum class ColorType { RGB, RGBA };
bool save_to_png(Texture* texture, StreamWrite* sw, ColorType color_type);

#endif  // end of include guard: WL_GRAPHIC_IMAGE_IO_H
