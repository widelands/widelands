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

#include "graphic/font_handler1.h"

#include <functional>
#include <memory>

#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/rendertarget.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/rt_render.h"
#include "graphic/text/sdl_ttf_font.h"
#include "graphic/texture.h"
#include "graphic/texture_cache.h"
#include "io/filesystem/filesystem.h"


using namespace std;
using namespace boost;

namespace {

/// The size of the richtext surface cache in bytes. All work that the richtext
/// renderer does is / cached in this cache until it overflows.
const uint32_t RICHTEXT_SURFACE_CACHE = 160 << 20;   // shifting converts to MB

// An Image implementation that recreates a rich text texture when needed on
// the fly. It is meant to be saved into the ImageCache.
class RTImage : public Image {
public:
	RTImage(const string& ghash,
	        TextureCache* texture_cache,
	        std::function<RT::Renderer* ()> get_renderer,
	        const string& text,
	        int gwidth)
	   : hash_(ghash),
	     text_(text),
	     width_(gwidth),
		  get_renderer_(get_renderer),
	     texture_cache_(texture_cache) {
	}
	virtual ~RTImage() {}

	// Implements Image.
	int width() const override {return texture()->width();}
	int height() const override {return texture()->height();}

	int get_gl_texture() const override {
		return texture()->get_gl_texture();
	}

	const FloatRect& texture_coordinates() const override {
		return texture()->texture_coordinates();
	}

private:
	Texture* texture() const {
		Texture* surf = texture_cache_->get(hash_);
		if (surf)
			return surf;

		surf = get_renderer_()->render(text_, width_);
		texture_cache_->insert(hash_, surf, true);
		return surf;
	}

	const string hash_;
	const string text_;
	int width_;
	std::function<RT::Renderer* ()> get_renderer_;

	// Nothing owned.
	TextureCache* const texture_cache_;
};

}

namespace UI {

// Utility class to render a rich text string. The returned string is cached in
// the ImageCache, so repeated calls to render with the same arguments should not
// be a problem.
class FontHandler1 : public IFontHandler1 {
public:
	FontHandler1(ImageCache* image_cache)
	   : texture_cache_(create_texture_cache(RICHTEXT_SURFACE_CACHE)),
	     fontset_(new UI::FontSet(i18n::get_locale())),
	     rt_renderer_(new RT::Renderer(image_cache, texture_cache_.get(), fontset_.get())),
	     image_cache_(image_cache) {
	}
	virtual ~FontHandler1() {}

	const Image* render(const string& text, uint16_t w = 0) override {
		const string hash = boost::lexical_cast<string>(w) + text;

		if (image_cache_->has(hash))
			return image_cache_->get(hash);

		std::unique_ptr<RTImage> image(
		   new RTImage(hash, texture_cache_.get(), [this] {return rt_renderer_.get();}, text, w));
		image->width(); // force the rich text to get rendered in case there is an exception thrown.

		return image_cache_->insert(hash, std::move(image));
	}

	UI::FontSet& fontset() const override {return *fontset_.get();}

	void reinitialize_fontset() override {
		fontset_.reset(new UI::FontSet(i18n::get_locale()));
		texture_cache_.get()->flush();
		rt_renderer_.reset(new RT::Renderer(image_cache_, texture_cache_.get(), fontset_.get()));
	}

private:
	std::unique_ptr<TextureCache> texture_cache_;
	std::unique_ptr<UI::FontSet> fontset_; // The currently active FontSet
	UI::FontSets font_sets_; // All fontsets
	std::unique_ptr<RT::Renderer> rt_renderer_;
	ImageCache* const image_cache_;  // not owned
};

IFontHandler1 * create_fonthandler(Graphic* gr) {
	return new FontHandler1(&gr->images());
}

IFontHandler1 * g_fh1 = nullptr;

} // namespace UI
