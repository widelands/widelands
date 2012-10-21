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

// TODO: kill font.h
// TODO: kill graphic/richtext.cc
// TODO: kill wordwrap.h
// TODO: kill text_parser

#include <boost/format.hpp>

#include "log.h" // TODO: kill this

#include "io/filesystem/filesystem.h"

#include "graphic.h"
#include "text/rt_render.h"
#include "text/rt_errors.h"
#include "text/sdl_ttf_font.h"
#include "rendertarget.h"
#include "picture.h"

#include "font_handler1.h"


using namespace std;
using namespace boost;

namespace UI {

// Private Stuff {{{
class ImageLoader : public RT::IImageLoader {
public:
	SDL_Surface * load(std::string s) {
		// TODO
		// unsigned w, h;
		// unsigned char * image;


		// throw RT::BadImage
			// ((format("Problem loading image %s: %s\n") % s % lodepng_error_text(error)).str());

		// Uint32 rmask, gmask, bmask, amask;
		// [> SDL interprets each pixel as a 32-bit number, so our masks must depend
			// on the endianness (byte order) of the machine */
// #if SDL_BYTEORDER == SDL_BIG_ENDIAN
		// rmask = 0xff000000;
		// gmask = 0x00ff0000;
		// bmask = 0x0000ff00;
		// amask = 0x000000ff;
// #else
		// rmask = 0x000000ff;
		// gmask = 0x0000ff00;
		// bmask = 0x00ff0000;
		// amask = 0xff000000;
// #endif

		// SDL_Surface * sur = SDL_CreateRGBSurfaceFrom(image, w, h, 32, w*4, rmask, gmask, bmask, amask);
		// if (!sur)
			// throw RT::BadImage
				// ((format("Problem creating surface for image %s: %s\n") % s % SDL_GetError()).str());

		// m_imgcache[s] = sur;
		// return sur;
		return 0;
	}
};
// End: Private Stuff }}}

class Font_Handler1 : public IFont_Handler1 {
public:
	Font_Handler1(LayeredFileSystem & fs);
	virtual ~Font_Handler1();

	void draw_text
		(RenderTarget & dst,
		 Point dstpoint,
		 const std::string & text,
		 uint32_t = 0,
		 Align = Align_TopLeft);

private:
	LayeredFileSystem & m_fs;
	RT::IRenderer * m_renderer;
};

Font_Handler1::Font_Handler1(LayeredFileSystem & fs) :
	m_fs(fs)
{
	RT::IFontLoader * floader = RT::ttf_fontloader_from_file();
	RT::IImageLoader * imgl = new ImageLoader();
	m_renderer = RT::setup_renderer(floader, imgl);
}
Font_Handler1::~Font_Handler1() {
	delete m_renderer;
}

void Font_Handler1::draw_text(RenderTarget & dst, Point dstpoint, const std::string & text, uint32_t w, Align align) {
	log("text: %s\n", text.c_str());
	SDL_Surface * text_surface = 0;
	try {
		text_surface = m_renderer->render(text, w);
	} catch (RT::Exception & e) {
		log((format("Text rendering error: %s") % e.what()).str().c_str()); // TODO(sirver): Should throw
	}
	if (!text_surface)
		return;

	PictureID p = g_gr->convert_sdl_surface_to_picture(text_surface, true);

	if (align & Align_HCenter) dstpoint.x -= p->get_w() / 2;
	else if (align & Align_Right) dstpoint.x -= p->get_w();
	if (align & Align_VCenter) dstpoint.y -= p->get_h() / 2;
	else if (align & Align_Bottom) dstpoint.y -= p->get_h();

	dst.blit(Point(dstpoint.x, dstpoint.y), p);
}

IFont_Handler1 * create_fonthandler(LayeredFileSystem & lfs) {
	return new Font_Handler1(lfs);
}

IFont_Handler1 * g_fh1 = 0;

} // namespace UI
