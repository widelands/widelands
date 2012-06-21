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

#ifndef RT_RENDER_H
#define RT_RENDER_H

#include <SDL.h>

#include <stdint.h>
#include <string>

namespace RT {
/**
 * Wrapper object around a font.
 *
 * Fonts in our sense are defined by the general font shape (given by the font
 * name) and the size of the font.
 */
class IFont {
public:
	enum {
		DEFAULT = 0,
		BOLD = 1,
		ITALIC = 2,
		UNDERLINE = 4,
		SHADOW = 8,
	};
	virtual ~IFont() {};

	virtual void dimensions(std::string, int, uint32_t *, uint32_t *) = 0;
	virtual SDL_Surface * render(std::string, SDL_Color clr, int) = 0;

	virtual uint32_t ascent(int) const = 0;
};

/**
 * Loader class that can create Fonts from a name. This is the bridge
 * to the g_fs in Widelands but can be reimplemented for standalone programs/test
 * cases.
 */
class IFontLoader {
public:
	virtual ~IFontLoader() {};

	virtual IFont * load(std::string name, int ptsize) = 0;
};

/**
 * Loader class that can create Images from a name. This is the bridge
 * to the g_fs in Widelands but can be reimplemented for standalone programs/test
 * cases.
 *
 * Note that we do not take ownership of the loaded SDL_Surface. If you want it
 * Freed, you have to do it yourself.
 */
class IImageLoader {
public:
	virtual ~IImageLoader() {};

	virtual SDL_Surface * load(std::string name) = 0;
};

/**
 * A map that maps pixels to a string. The string are the references which can be used
 * for hyperlink like constructions
 */
class IRefMap {
public:
	virtual ~IRefMap() {};
	virtual std::string query(int16_t x, int16_t y) = 0;
};

/**
 * This is the rendering engine. It does not do any caching at all and
 * it works entirely inside the SDL Framework.
 */
class IRenderer {
public:
	IRenderer(IFontLoader *) {};
	virtual ~IRenderer() {};

	virtual SDL_Surface * render(std::string, uint32_t, IRefMap ** = 0) = 0;
};

IRenderer * setup_renderer(IFontLoader * fl, IImageLoader * imgl);
};

#endif /* end of include guard: RT_RENDER_H */

