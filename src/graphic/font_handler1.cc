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

#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "graphic/surface_cache.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/rt_render.h"
#include "graphic/text/sdl_ttf_font.h"
#include "io/filesystem/filesystem.h"


using namespace std;
using namespace boost;

namespace {

// An Image implementation that recreates a rich text surface when needed on
// the fly. It is meant to be saved into the ImageCache.
class RTImage : public Image {
public:
	RTImage
		(const string& ghash, SurfaceCache* surface_cache, RT::Renderer*
		 rt_renderer, const string& text, uint16_t gwidth)
		: hash_(ghash), text_(text), width_(gwidth), surface_cache_(surface_cache),
		  rt_renderer_(rt_renderer)
	{}
	virtual ~RTImage() {}

	// Implements Image.
	uint16_t width() const override {return surface()->width();}
	uint16_t height() const override {return surface()->height();}
	const string& hash() const override {return hash_;}
	Surface* surface() const override {
		Surface* surf = surface_cache_->get(hash_);
		if (surf)
			return surf;

		surf = rt_renderer_->render(text_, width_);
		surface_cache_->insert(hash_, surf, true);
		return surf;
	}

private:
	const string hash_;
	const string text_;
	uint16_t width_;

	// Nothing owned.
	SurfaceCache* const surface_cache_;
	RT::Renderer* const rt_renderer_;
};

}

namespace UI {

// Utility class to render a rich text string. The returned string is cached in
// the ImageCache, so repeated calls to render with the same arguments should not
// be a problem.
class Font_Handler1 : public IFont_Handler1 {
public:
	Font_Handler1(ImageCache* image_cache, SurfaceCache* surface_cache, RT::Renderer* renderer) :
		surface_cache_(surface_cache), image_cache_(image_cache), renderer_(renderer) {}
	virtual ~Font_Handler1() {}

	const Image* render(const string& text, uint16_t w = 0) override {
		const string hash = boost::lexical_cast<string>(w) + text;

		if (image_cache_->has(hash))
			return image_cache_->get(hash);

		std::unique_ptr<RTImage> image(new RTImage(hash, surface_cache_, renderer_.get(), text, w));
		image->surface(); // force the rich text to get rendered in case there is an exception thrown.

		return image_cache_->insert(image.release());
	}

private:
	SurfaceCache* const surface_cache_;  // not owned
	ImageCache* const image_cache_;  // not owned
	std::unique_ptr<RT::Renderer> renderer_;
};

IFont_Handler1 * create_fonthandler(Graphic* gr) {
	return new Font_Handler1(
	   &gr->images(), &gr->surfaces(), new RT::Renderer(&gr->images(), &gr->surfaces()));
}

IFont_Handler1 * g_fh1 = nullptr;

} // namespace UI
