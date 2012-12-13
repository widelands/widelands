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

class Font_Handler1 : public IFont_Handler1 {
public:
	Font_Handler1(IGraphic& gr, FileSystem* fs);
	virtual ~Font_Handler1();

	void draw_text
		(RenderTarget & dst,
		 Point dstpoint,
		 const std::string & text,
		 uint32_t = 0,
		 Align = Align_TopLeft);

private:
	RT::IRenderer* m_renderer;
};

Font_Handler1::Font_Handler1(IGraphic& gr, FileSystem* fs) {
	RT::IFontLoader * floader = RT::ttf_fontloader_from_filesystem(fs);
	m_renderer = RT::setup_renderer(gr, floader);
}
Font_Handler1::~Font_Handler1() {
	delete m_renderer;
}

void Font_Handler1::draw_text(RenderTarget & dst, Point dstpoint, const std::string & text, uint32_t w, Align align) {
	log("text: %s\n", text.c_str());
	const IPicture* p = 0;
	// TODO(sirver): ownage is not clear
	try {
		p = m_renderer->render(text, w);
	} catch (RT::Exception & e) {
		log("Text rendering error: %s", e.what()); // TODO(sirver): Should throw
	}
	if (!p)
		return;

	if (align & Align_HCenter) dstpoint.x -= p->get_w() / 2;
	else if (align & Align_Right) dstpoint.x -= p->get_w();
	if (align & Align_VCenter) dstpoint.y -= p->get_h() / 2;
	else if (align & Align_Bottom) dstpoint.y -= p->get_h();

	dst.blit(Point(dstpoint.x, dstpoint.y), p);
}

IFont_Handler1 * create_fonthandler(IGraphic& gr, FileSystem* lfs) {
	return new Font_Handler1(gr, lfs);
}

IFont_Handler1 * g_fh1 = 0;

} // namespace UI
