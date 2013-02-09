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

#include <boost/utility.hpp>

#include "io/filesystem/filesystem.h"
#include "wexception.h"

#include "graphic.h"
#include "picture.h"
#include "rendertarget.h"
#include "text/rt_errors.h"
#include "text/rt_render.h"
#include "text/sdl_ttf_font.h"

#include "font_handler1.h"

using namespace std;
using namespace boost;

namespace UI {

// NOCOM(#sirver): this class should no longer be used. use ImageCache
// directly.
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

	const IPicture* render(const string& text, uint32_t w = 0);

private:
	boost::scoped_ptr<RT::IRenderer> renderer_;
};

Font_Handler1::Font_Handler1(IGraphic& gr, FileSystem* fs) {
}
Font_Handler1::~Font_Handler1() {
}

const IPicture* Font_Handler1::render(const string& text, uint32_t w) {
	return g_gr->imgcache().render_text(text, w);
}

// NOCOM(#sirver): get rid of this function first.
void Font_Handler1::draw_text
		(RenderTarget & dst, Point dstpoint, const std::string & text, uint32_t w, Align align)
{
	if (text.empty())
		return;

	const IPicture* p = render(text, w);
	if (!p)
		return;

	if (align & Align_HCenter) dstpoint.x -= p->width() / 2;
	else if (align & Align_Right) dstpoint.x -= p->width();
	if (align & Align_VCenter) dstpoint.y -= p->height() / 2;
	else if (align & Align_Bottom) dstpoint.y -= p->height();

	dst.blit(Point(dstpoint.x, dstpoint.y), p);
}

IFont_Handler1 * create_fonthandler(IGraphic& gr, FileSystem* lfs) {
	return new Font_Handler1(gr, lfs);
}

IFont_Handler1 * g_fh1 = 0;

} // namespace UIae
