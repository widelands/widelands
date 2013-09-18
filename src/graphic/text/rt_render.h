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

#include <set>
#include <string>

#include <stdint.h>

#include "graphic/image.h"
#include "rgbcolor.h"

class SurfaceCache;
class ImageCache;

namespace RT {
/**
 * Wrapper object around a font.
 *
 * Fonts in our sense are defined by the general font shape (given by the font
 * name) and the size of the font. Note that Bold and Italic are special in the
 * regard that we expect that this is already handled by the Font File, so, the
 * font loader directly loads DejaVuSerifBoldItalic.ttf for example.
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

	virtual void dimensions(const std::string&, int, uint16_t *, uint16_t *) = 0;
	virtual const Surface& render(const std::string&, const RGBColor& clr, int, SurfaceCache*) = 0;

	virtual uint16_t ascent(int) const = 0;
};

/**
 * Loader class that can create Fonts from a name. This is the bridge
 * to the g_fs in Widelands but can be reimplemented for standalone programs/test
 * cases.
 */
class IFontLoader {
public:
	virtual ~IFontLoader() {};

	virtual IFont * load(const std::string& name, int ptsize) = 0;
};

/**
 * A map that maps pixels to a string. The string are the references which can be used
 * for hyperlink like constructions.
 */
class IRefMap {
public:
	virtual ~IRefMap() {};
	virtual std::string query(int16_t x, int16_t y) = 0;
};

/**
 * This is the rendering engine. The returned images are not owned by the
 * caller.
 */
typedef std::set<std::string> TagSet;
class IRenderer {
public:
	IRenderer() {};
	virtual ~IRenderer() {};

	// Render the given string in the given width. Restricts the allowed tags to
	// the ones in TagSet. The renderer does not do caching in the SurfaceCache
	// for its individual nodes, but the font render does.
	virtual Surface* render(const std::string&, uint16_t, const TagSet & = TagSet()) = 0;

	// Returns a reference map of the clickable hyperlinks in the image. This
	// will do no caching and needs to do all layouting, so do not call this too
	// often. The returned object must be freed.
	virtual IRefMap* make_reference_map(const std::string&, uint16_t, const TagSet & = TagSet()) = 0;
};

// Setup a renderer, takes ownership of fl but of nothing else.
IRenderer* setup_renderer(ImageCache* gr, SurfaceCache*, IFontLoader* fl);
};

#endif /* end of include guard: RT_RENDER_H */

