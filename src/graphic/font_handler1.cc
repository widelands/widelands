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

#include <memory>

#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>

#include "base/i18n.h"
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

// An Image implementation that recreates a rich text texture when needed on
// the fly. It is meant to be saved into the ImageCache.
class RTImage : public Image {
public:
	RTImage
		(const string& ghash, TextureCache* texture_cache, RT::Renderer*
		 rt_renderer, const string& text, uint16_t gwidth)
		: hash_(ghash), text_(text), width_(gwidth), texture_cache_(texture_cache),
		  rt_renderer_(rt_renderer)
	{}
	virtual ~RTImage() {}

	// Implements Image.
	uint16_t width() const override {return texture()->width();}
	uint16_t height() const override {return texture()->height();}
	Texture* texture() const override {
		Texture* surf = texture_cache_->get(hash_);
		if (surf)
			return surf;

		surf = rt_renderer_->render(text_, width_);
		texture_cache_->insert(hash_, surf, true);
		return surf;
	}

private:
	const string hash_;
	const string text_;
	uint16_t width_;

	// Nothing owned.
	TextureCache* const texture_cache_;
	RT::Renderer* const rt_renderer_;
};

}

namespace UI {

// Utility class to render a rich text string. The returned string is cached in
// the ImageCache, so repeated calls to render with the same arguments should not
// be a problem.
class FontHandler1 : public IFontHandler1 {
public:
	FontHandler1(ImageCache* image_cache, TextureCache* texture_cache, RT::Renderer* renderer) :
		texture_cache_(texture_cache), image_cache_(image_cache),
		fontset_(new UI::FontSet(i18n::get_locale())), renderer_(renderer) {}

	virtual ~FontHandler1() {}

	const Image* render(const string& text, uint16_t w = 0) override {
		const string hash = boost::lexical_cast<string>(w) + text;

		if (image_cache_->has(hash))
			return image_cache_->get(hash);

		std::unique_ptr<RTImage> image(new RTImage(hash, texture_cache_, renderer_.get(), text, w));
		image->texture(); // force the rich text to get rendered in case there is an exception thrown.

		return image_cache_->insert(hash, std::move(image));
	}

	UI::FontSet& fontset() const {return *fontset_.get();}

	void reinitialize_fontset() {
		fontset_.reset(new UI::FontSet(i18n::get_locale()));
		renderer_.reset(new RT::Renderer(image_cache_, texture_cache_, fontset_.get()));
	}

private:
	TextureCache* const texture_cache_;  // not owned
	ImageCache* const image_cache_;  // not owned
	unique_ptr<UI::FontSet> fontset_; // The currently active FontSet
	std::unique_ptr<RT::Renderer> renderer_;
};

IFontHandler1 * create_fonthandler(Graphic* gr) {

	return new FontHandler1(
		&gr->images(), &gr->textures(), new RT::Renderer(&gr->images(), &gr->textures(),
																		 new UI::FontSet(i18n::get_locale())));
}

IFontHandler1 * g_fh1 = nullptr;

} // namespace UI
