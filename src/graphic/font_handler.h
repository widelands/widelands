/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_FONT_HANDLER_H
#define WL_GRAPHIC_FONT_HANDLER_H

#include <limits>
#include <memory>
#include <string>

#include "base/vector.h"
#include "graphic/align.h"

class RenderTarget;

namespace UI {

struct TextStyle;

/**
 * Main class for string rendering. Manages the cache of pre-rendered strings.
 */
struct FontHandler {
	FontHandler();
	~FontHandler();

	void draw_text(RenderTarget&,
	               const TextStyle&,
	               Vector2i dstpoint,
	               const std::string& text,
	               Align align = UI::Align::kLeft,
	               uint32_t caret = std::numeric_limits<uint32_t>::max());
	uint32_t
	draw_text_raw(RenderTarget&, const TextStyle&, Vector2i dstpoint, const std::string& text);

	void get_size(const TextStyle&,
	              const std::string& text,
	              uint32_t& w,
	              uint32_t& h,
	              uint32_t wrap = std::numeric_limits<uint32_t>::max());
	void get_size(const std::string& fontname,
	              int32_t size,
	              const std::string& text,
	              uint32_t& w,
	              uint32_t& h,
	              uint32_t wrap = std::numeric_limits<uint32_t>::max());

	// Delete the whole cache.
	void flush();

private:
	struct Data;
	std::unique_ptr<Data> d;
};

extern FontHandler* g_fh;
}

#endif  // end of include guard: WL_GRAPHIC_FONT_HANDLER_H
