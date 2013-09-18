/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef FONT_HANDLER1_H // TODO: rename
#define FONT_HANDLER1_H

#include <string>

#include <boost/noncopyable.hpp>

#include "align.h"
#include "point.h"

class FileSystem;
class Image;
class Graphic;

namespace UI {

/**
 * Main class for string rendering. Manages the cache of pre-rendered strings.
 */
class IFont_Handler1 : boost::noncopyable {
public:
	virtual ~IFont_Handler1() {};

	/*
	 * Renders the given text into an image. The image is cached and therefore
	 * ownership remains with this class. Will throw on error.
	 */
	virtual const Image* render(const std::string& text, uint16_t w = 0) = 0;
};

// Create a new Font_Handler1. Ownership for the objects is not taken.
IFont_Handler1 * create_fonthandler(Graphic* gr, FileSystem* fs);

extern IFont_Handler1 * g_fh1;

}

#endif
