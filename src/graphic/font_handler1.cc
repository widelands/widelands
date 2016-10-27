/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

// The size of the richtext surface cache in bytes. All work that the richtext
// renderer does is / cached in this cache until it overflows. The idea is that
// this is big enough to cache the text that is used on a typical screen - so
// that we do not need to lay out text every frame. Last benchmarked at r7712,
// 30 MB was enough to cache texts for many frames (> 1000), while it is
// quickly overflowing in the map selection menu.
// This might need reevaluation is the new font handler is used for more stuff.
const uint32_t RICHTEXT_TEXTURE_CACHE = 30 << 20;  // shifting converts to MB

// An Image implementation that recreates a rich text texture when needed on
// the fly. It is meant to be saved into the ImageCache.
class RTImage : public Image {
public:
	RTImage(const string& ghash,
	        TextureCache* texture_cache,
	        std::function<RT::Renderer*()> get_renderer,
	        const string& text,
	        int gwidth)
	   : hash_(ghash),
	     text_(text),
	     width_(gwidth),
	     get_renderer_(get_renderer),
	     texture_cache_(texture_cache) {
	}
	virtual ~RTImage() {
	}

	// Implements Image.
	int width() const override {
		return texture()->width();
	}
	int height() const override {
		return texture()->height();
	}

	const BlitData& blit_data() const override {
		return texture()->blit_data();
	}

private:
	Texture* texture() const {
		Texture* surf = texture_cache_->get(hash_);
		if (surf != nullptr) {
			return surf;
		}
		return texture_cache_->insert(
		   hash_, std::unique_ptr<Texture>(get_renderer_()->render(text_, width_)));
	}

	const string hash_;
	const string text_;
	int width_;
	std::function<RT::Renderer*()> get_renderer_;

	// Nothing owned.
	TextureCache* const texture_cache_;
};
}

namespace UI {

int RenderedText::width() const {
	int result = 0;
	for (const auto& rect : texts) {
		result = std::max(result, rect->point.x + rect->image->width());
	}
	return result;
}
int RenderedText::height() const {
	int result = 0;
	for (const auto& rect : texts) {
		result = std::max(result, rect->point.y + rect->image->height());
	}
	return result;
}

// Utility class to render a rich text string. The returned string is cached in
// the ImageCache, so repeated calls to render with the same arguments should not
// be a problem.
class FontHandler1 : public IFontHandler1 {
public:
	FontHandler1(ImageCache* image_cache)
	   : texture_cache_(new TextureCache(RICHTEXT_TEXTURE_CACHE)),
	     fontsets_(),
	     fontset_(fontsets_.get_fontset(i18n::get_locale())),
	     rt_renderer_(new RT::Renderer(image_cache, texture_cache_.get(), fontsets_)),
	     image_cache_(image_cache) {
	}
	virtual ~FontHandler1() {
		for (auto& render_result : render_results_) {
			delete render_result.second;
		}
		render_results_.clear();
	}

	const Image* render(const string& text, uint16_t w = 0) override {
		const string hash = boost::lexical_cast<string>(w) + text;
		if (render_results_.count(hash) != 1) {
			std::unique_ptr<RTImage> image(new RTImage(
			   hash, texture_cache_.get(), [this] { return rt_renderer_.get(); }, text, w));
			// force the rich text to get rendered in case there is an exception thrown.
			image->width();

			RenderedText* rendered_text = new RenderedText();
			rendered_text->texts.push_back(std::unique_ptr<RenderedRect>(new RenderedRect(Vector2i(0, 0), image_cache_->insert(hash, std::move(image)))));
			render_results_.insert(std::make_pair(hash, rendered_text));
		} else {
			assert(image_cache_->has(hash));
		}
		return image_cache_->get(hash);
	}

	// NOCOM(GunChleoc): This will become the normal render function when we're done.
	const RenderedText* render_multi(const string& text, uint16_t w = 0) override {
		const string hash = boost::lexical_cast<string>(w) + text;
		if (render_results_.count(hash) != 1) {
			std::unique_ptr<RTImage> image(new RTImage(
			   hash, texture_cache_.get(), [this] { return rt_renderer_.get(); }, text, w));
			// force the rich text to get rendered in case there is an exception thrown.
			image->width();

			RenderedText* rendered_text = new RenderedText();
			// NOCOM only 1 texture so far, this needs to be implemented. We need to get the interface
			// to work first though.
			rendered_text->texts.push_back(std::unique_ptr<RenderedRect>(new RenderedRect(Vector2i(0, 0), image_cache_->insert(hash, std::move(image)))));
			render_results_.insert(std::make_pair(hash, rendered_text));
		} else {
			assert(image_cache_->has(hash));
		}
		return (*render_results_.find(hash)).second;
	}

	UI::FontSet const* fontset() const override {
		return fontset_;
	}

	void reinitialize_fontset() override {
		fontset_ = fontsets_.get_fontset(i18n::get_locale());
		texture_cache_.get()->flush();
		rt_renderer_.reset(new RT::Renderer(image_cache_, texture_cache_.get(), fontsets_));
	}

private:
	std::unique_ptr<TextureCache> texture_cache_;
	UI::FontSets fontsets_;       // All fontsets
	UI::FontSet const* fontset_;  // The currently active FontSet
	std::unique_ptr<RT::Renderer> rt_renderer_;
	ImageCache* const image_cache_;  // not owned
	std::unordered_map<std::string, RenderedText*> render_results_;
};

IFontHandler1* create_fonthandler(ImageCache* image_cache) {
	return new FontHandler1(image_cache);
}

IFontHandler1* g_fh1 = nullptr;

}  // namespace UI
