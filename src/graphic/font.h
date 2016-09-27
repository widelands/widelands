/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_FONT_H
#define WL_GRAPHIC_FONT_H

#include <SDL_ttf.h>

#include "graphic/color.h"
#include "io/fileread.h"

namespace UI {

/**
 * Margin around text that is kept to make space for the caret.
 */
#define LINE_MARGIN 1

/**
 * Wrapper object around a font.
 *
 * Fonts in our sense are defined by the general font shape (given by the font
 * name) and the size of the font.
 */
struct Font {
	friend struct TextStyle;

	static void shutdown();
	static Font* get(const std::string& name, int size);

	uint32_t size() const;
	uint32_t ascent() const;
	uint32_t height() const;
	uint32_t lineskip() const;

	TTF_Font* get_ttf_font() const {
		return font_;
	}

private:
	Font(const std::string& name, int size);
	~Font();

	FileRead fontfile_;
	TTF_Font* font_;

	/**
	 * Work around weird fonts with very large lineskip, to get something
	 * that makes more sense as the default skip in Latin scripts.
	 */
	int32_t computed_typical_miny_;
	int32_t computed_typical_maxy_;

	int size_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_FONT_H
