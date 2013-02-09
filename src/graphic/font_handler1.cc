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

#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>

#include "io/filesystem/filesystem.h"
#include "wexception.h"

#include "graphic.h"
#include "image.h"
#include "image_cache.h"
#include "rendertarget.h"
#include "surface.h"
#include "surface_cache.h"
#include "text/rt_errors.h"
#include "text/rt_render.h"
#include "text/sdl_ttf_font.h"

#include "font_handler1.h"

using namespace std;
using namespace boost;

namespace {

class RTImage : public Image {
public:
	RTImage
		(const string& hash, SurfaceCache* surface_cache, RT::IRenderer*
		 rt_renderer, const string& text, uint16_t width)
		: hash_(hash),
		surface_cache_(surface_cache),
		  rt_renderer_(rt_renderer),
		  text_(text), width_(width)
	{}

	// Implements Image.
	virtual uint16_t width() const {return surface()->width();}
	virtual uint16_t height() const {return surface()->height();}
	virtual const string& hash() const {return hash_;}
	virtual Surface* surface() const {
		Surface* surf = surface_cache_->get(hash_);
		if (surf)
			return surf;

		try {
			surf = rt_renderer_->render(text_, width_);
			surface_cache_->insert(hash_, surf);
		} catch (RT::Exception& e) {
			throw wexception("Richtext rendering error: %s", e.what());
		}
		return surf;
	}

private:
	const string hash_;
	SurfaceCache* surface_cache_;
	RT::IRenderer* rt_renderer_;
	const string text_;
	uint16_t width_;
};

}

namespace UI {

// NOCOM(#sirver): better docu, especially ownership
class Font_Handler1 : public IFont_Handler1 {
public:
	Font_Handler1(ImageCache* image_cache, SurfaceCache* surface_cache, RT::IRenderer* renderer) :
		surface_cache_(surface_cache), image_cache_(image_cache), renderer_(renderer) {};
	virtual ~Font_Handler1();

	void draw_text
		(RenderTarget & dst,
		 Point dstpoint,
		 const std::string & text,
		 uint16_t = 0,
		 Align = Align_TopLeft);

	const Image* render(const string& text, uint16_t w = 0);

private:
	SurfaceCache* const surface_cache_;  // not owned
	ImageCache* const image_cache_;  // not owned
	boost::scoped_ptr<RT::IRenderer> renderer_;
};

Font_Handler1::~Font_Handler1() {
}

const Image* Font_Handler1::render(const string& text, uint16_t w) {
	const string hash = boost::lexical_cast<string>(w) + text;

	if (image_cache_->has(hash))
		return image_cache_->get(hash);

	return image_cache_->insert(new RTImage(hash, surface_cache_, renderer_.get(), text, w));
}

void Font_Handler1::draw_text
		(RenderTarget & dst, Point dstpoint, const std::string & text, uint16_t w, Align align)
{
	if (text.empty())
		return;

	const Image* p = render(text, w);
	if (!p)
		return;

	if (align & Align_HCenter) dstpoint.x -= p->width() / 2;
	else if (align & Align_Right) dstpoint.x -= p->width();
	if (align & Align_VCenter) dstpoint.y -= p->height() / 2;
	else if (align & Align_Bottom) dstpoint.y -= p->height();

	dst.blit(Point(dstpoint.x, dstpoint.y), p);
}

IFont_Handler1 * create_fonthandler(Graphic* gr, FileSystem* fs) {
	return
		new Font_Handler1
		(&gr->imgcache(), &gr->surface_cache(),
		 RT::setup_renderer
		 (&gr->imgcache(), &gr->surface_cache(), RT::ttf_fontloader_from_filesystem(fs)));
}

IFont_Handler1 * g_fh1 = 0;

} // namespace UIae
