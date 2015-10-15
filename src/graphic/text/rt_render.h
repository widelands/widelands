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

#ifndef WL_GRAPHIC_TEXT_RT_RENDER_H
#define WL_GRAPHIC_TEXT_RT_RENDER_H

#include <memory>
#include <set>
#include <string>

#include <stdint.h>

#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/text/font_set.h"

class Texture;
class ImageCache;
class TextureCache;

namespace RT {

class FontCache;
class Parser;
class RenderNode;

struct RendererStyle {
	RendererStyle(const std::string& font_face_, uint8_t font_size_,
						  uint16_t remaining_width_, uint16_t overall_width_) :
		font_face(font_face_), font_size(font_size_),
		remaining_width(remaining_width_), overall_width(overall_width_) {}

	std::string font_face;
	uint8_t font_size;
	uint16_t remaining_width;
	uint16_t overall_width;
};

/**
 * Wrapper object around a font.
 *
 * Fonts in our sense are defined by the general font shape (given by the font
 * name) and the size of the font. Note that Bold and Italic are special in the
 * regard that we expect that this is already handled by the Font File, so, the
 * font loader directly loads DejaVuSerif-Bold.ttf for example.
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
	virtual ~IFont() {}

	virtual void dimensions(const std::string&, int, uint16_t *, uint16_t *) = 0;
	virtual const Texture& render(const std::string&, const RGBColor& clr, int, TextureCache*) = 0;

	virtual uint16_t ascent(int) const = 0;
};

/**
 * A map that maps pixels to a string. The string are the references which can be used
 * for hyperlink like constructions.
 */
class IRefMap {
public:
	virtual ~IRefMap() {}
	virtual std::string query(int16_t x, int16_t y) = 0;
};

/**
 * This is the rendering engine. The returned images are not owned by the
 * caller.
 */
using TagSet = std::set<std::string>;
class Renderer {
public:
	// Ownership is not taken.
	Renderer(ImageCache* image_cache, TextureCache* texture_cache, const UI::FontSets& fontsets);
	~Renderer();

	// Render the given string in the given width. Restricts the allowed tags to
	// the ones in TagSet. The renderer does not do caching in the TextureCache
	// for its individual nodes, but the font render does.
	Texture* render(const std::string&, uint16_t width, const TagSet& tagset = TagSet());

	// Returns a reference map of the clickable hyperlinks in the image. This
	// will do no caching and needs to do all layouting, so do not call this too
	// often. The returned object must be freed.
	IRefMap* make_reference_map(const std::string&, uint16_t, const TagSet& = TagSet());

private:
	RenderNode* layout_(const std::string& text, uint16_t width, const TagSet& allowed_tags);

	std::unique_ptr<FontCache> font_cache_;
	std::unique_ptr<Parser> parser_;
	ImageCache* const image_cache_;  // Not owned.
	TextureCache* const texture_cache_;  // Not owned.
	const UI::FontSets& fontsets_; // All fontsets
	RendererStyle renderer_style_; // Properties that all render nodes need to know about
};

}

#endif  // end of include guard: WL_GRAPHIC_TEXT_RT_RENDER_H
