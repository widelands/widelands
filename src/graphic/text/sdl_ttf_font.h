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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_TEXT_SDL_TTF_FONT_H
#define WL_GRAPHIC_TEXT_SDL_TTF_FONT_H

#include <memory>

#include <SDL_ttf.h>

#include "graphic/text/texture_cache.h"
#include "graphic/texture.h"

namespace RT {

/**
 * Wrapper object around a font.
 *
 * Fonts in our sense are defined by the general font shape (given by the font
 * name) and the size of the font. Note that Bold and Italic are special in the
 * regard that we expect that this is already handled by the Font File, so, the
 * font loader directly loads DejaVuSans-Bold.ttf for example.
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
	virtual ~IFont() {
	}

	virtual void dimensions(const std::string&, int, uint16_t*, uint16_t*) = 0;
	virtual std::shared_ptr<const Image>
	render(const std::string&, const RGBColor& clr, int, TextureCache*) = 0;

	virtual uint16_t ascent(int) const = 0;
	virtual TTF_Font* get_ttf_font() const = 0;
};

// Implementation of a Font object using SDL_ttf.
class SdlTtfFont : public IFont {
public:
	SdlTtfFont(TTF_Font* ttf, const std::string& face, int ptsize, std::string* ttf_memory_block);
	~SdlTtfFont() override;

	void dimensions(const std::string&, int, uint16_t* w, uint16_t* h) override;
	std::shared_ptr<const Image>
	render(const std::string&, const RGBColor& clr, int, TextureCache*) override;
	uint16_t ascent(int) const override;
	TTF_Font* get_ttf_font() const override {
		return font_;
	}

private:
	void set_style(int);

	TTF_Font* font_;
	int style_;
	const std::string font_name_;
	const int ptsize_;
	// Old version of SDLTtf seem to need to keep this around.
	std::unique_ptr<std::string> ttf_file_memory_block_;
};

}  // namespace RT

#endif  // end of include guard:
