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

#ifndef FONT_HANDLER_H
#define FONT_HANDLER_H

#include "ui_basic/align.h"
#include "point.h"
#include "rgbcolor.h"
#include "graphic.h"
#include "picture_id.h"

#include <SDL_ttf.h>

#include <boost/scoped_ptr.hpp>

#include <list>
#include <cstring>
#include <vector>

struct RenderTarget;

namespace UI {

struct TextStyle;

/**
 * Main class for string rendering. Manages the cache of pre-rendered strings.
 */
struct Font_Handler {
	Font_Handler();
	~Font_Handler();

	void draw_text
		(RenderTarget &,
		 const TextStyle &,
		 Point dstpoint,
		 const std::string & text,
		 Align align = Align_CenterLeft,
		 uint32_t caret = std::numeric_limits<uint32_t>::max());
	void draw_text_shadow
		(RenderTarget &,
		 const TextStyle &,
		 Point dstpoint,
		 const std::string & text,
		 Align align = Align_CenterLeft,
		 uint32_t caret = std::numeric_limits<uint32_t>::max());
	uint32_t draw_text_raw(RenderTarget &, const TextStyle &, Point dstpoint, const std::string & text);
	void draw_multiline
		(RenderTarget &,
		 const TextStyle &,
		 Point dstpoint,
		 const std::string & text,
		 Align align = Align_CenterLeft,
		 uint32_t wrap = std::numeric_limits<uint32_t>::max(),
		 uint32_t caret = std::numeric_limits<uint32_t>::max());

	void get_size
		(const TextStyle &,
		 const std::string & text,
		 uint32_t & w, uint32_t & h,
		 uint32_t wrap = std::numeric_limits<uint32_t>::max());
	void get_size
		(std::string const & fontname, int32_t size,
		 const std::string & text,
		 uint32_t & w, uint32_t & h,
		 uint32_t wrap = std::numeric_limits<uint32_t>::max());
	uint32_t get_fontheight(std::string const & name, int32_t size);
	void do_align
		(Align, int32_t & dstx, int32_t & dsty, int32_t w, int32_t h);
	// This deletes all cached pictures, it is called
	// from the graphics code before the graphics are flushed,
	// to make sure that everything is forgotten
	void flush_cache();

private:
	struct Data;
	boost::scoped_ptr<Data> d;

private:
	void draw_caret
		(RenderTarget &,
		 const TextStyle &,
		 Point dstpoint,
		 const std::string & text,
		 uint32_t caret);
};

extern Font_Handler * g_fh;

}

#endif
