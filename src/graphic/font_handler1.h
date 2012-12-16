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

#include "ui_basic/align.h"
#include "point.h"

struct RenderTarget;
struct FileSystem;

namespace UI {

/**
 * Main class for string rendering. Manages the cache of pre-rendered strings.
 */
class IFont_Handler1 : boost::noncopyable {
public:
	virtual ~IFont_Handler1() {};

	virtual void draw_text
		(RenderTarget &,
		 Point dstpoint,
		 const std::string & text,
		 uint32_t w = 0,
		 Align = Align_TopLeft) = 0;

	/*
	 * Renders the given text into an image. Will return NULL on error or if the
	 * resulting image would have no size. The image is cached and therefore
	 * ownership remains with this class.
	 */
	virtual const IPicture* render(const std::string& text, uint32_t w = 0) = 0;
};

IFont_Handler1 * create_fonthandler(IGraphic& gr, FileSystem* lfs);

extern IFont_Handler1 * g_fh1;

}

#endif
