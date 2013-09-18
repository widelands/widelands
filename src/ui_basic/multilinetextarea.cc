/*
 * Copyright (C) 2002-2004, 2006-2009, 2011, 2013 by the Widelands Development Team
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

#include "ui_basic/multilinetextarea.h"

#include <boost/bind.hpp>

#include "constants.h"
#include "graphic/font_handler.h"
#include "graphic/richtext.h"
#include "graphic/wordwrap.h"

namespace UI {

static const uint32_t RICHTEXT_MARGIN = 2;

struct Multiline_Textarea::Impl {
	bool isrichtext;
	WordWrap ww;
	RichText rt;

	Impl() : isrichtext(false) {}
};

Multiline_Textarea::Multiline_Textarea
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const std::string & text,
	 const Align align,
	 const bool always_show_scrollbar)
	:
	Panel       (parent, x, y, w, h),
	m(new Impl),
	m_text      (text),
	m_scrollbar (this, get_w() - scrollbar_w(), 0, scrollbar_w(), h, false),
	m_scrollmode(ScrollNormal)
{
	assert(scrollbar_w() <= w);
	set_think(false);

	//  do not allow vertical alignment as it does not make sense
	m_align = static_cast<Align>(align & Align_Horizontal);

	m_scrollbar.moved.connect(boost::bind(&Multiline_Textarea::scrollpos_changed, this, _1));

	m_scrollbar.set_singlestepsize(g_fh->get_fontheight(UI_FONT_SMALL));
	m_scrollbar.set_pagesize(h - 2 * g_fh->get_fontheight(UI_FONT_BIG));
	m_scrollbar.set_steps(1);
	m_scrollbar.set_force_draw(always_show_scrollbar);

	set_font(UI_FONT_SMALL, UI_FONT_CLR_FG);

	update(0, 0, get_eff_w(), get_h());
}


/**
 * Free allocated resources
*/
Multiline_Textarea::~Multiline_Textarea()
{
}

/**
 * Change the font used for non-richtext text.
 */
void Multiline_Textarea::set_font(std::string name, int32_t size, RGBColor fg)
{
	m_fontname = name;
	m_fontsize = size;
	m_fcolor = fg;

	TextStyle style;
	style.font = Font::get(m_fontname, m_fontsize);
	style.fg = m_fcolor;
	style.bold = true; // for historic reasons

	m->ww.set_style(style);
	recompute();
}

/**
 * Replace the current text with a new one.
 * Fix up scrolling state if necessary.
 */
void Multiline_Textarea::set_text(const std::string & text)
{
	m_text = text;
	recompute();
}

/**
 * Recompute the word wrapping or rich-text layouting,
 * and adjust scrollbar settings accordingly.
 */
void Multiline_Textarea::recompute()
{
	uint32_t height;

	if (m_text.compare(0, 3, "<rt")) {
		m->isrichtext = false;
		m->ww.set_wrapwidth(get_eff_w());
		m->ww.wrap(m_text);
		height = m->ww.height();
	} else {
		m->isrichtext = true;
		m->rt.set_width(get_eff_w() - 2 * RICHTEXT_MARGIN);
		m->rt.parse(m_text);
		height = m->rt.height() + 2 * RICHTEXT_MARGIN;
	}

	bool setbottom = false;

	if (m_scrollmode == ScrollLog)
		if (m_scrollbar.get_scrollpos() >= m_scrollbar.get_steps() - 1)
			setbottom = true;

	m_scrollbar.set_steps(height - get_h());
	if (setbottom)
		m_scrollbar.set_scrollpos(height - get_h());

	update(0, 0, get_eff_w(), get_h());
}

/**
 * Callback from the scrollbar.
 */
void Multiline_Textarea::scrollpos_changed(int32_t const /* pixels */)
{
	update(0, 0, get_eff_w(), get_h());
}

/**
 * Change the scroll mode. This will not change the current scroll position;
 * it only affects the behaviour of set_text().
 */
void Multiline_Textarea::set_scrollmode(ScrollMode mode)
{
	m_scrollmode = mode;
}


/// Take care about scrollbar on resize
void Multiline_Textarea::layout()
{
	recompute();

	// Take care about the scrollbar
	m_scrollbar.set_pos(Point(get_w() - scrollbar_w(), 0));
	m_scrollbar.set_size(scrollbar_w(), get_h());
}

/**
 * Redraw the textarea
 */
void Multiline_Textarea::draw(RenderTarget & dst)
{
	if (m->isrichtext) {
		m->rt.draw(dst, Point(RICHTEXT_MARGIN, RICHTEXT_MARGIN - m_scrollbar.get_scrollpos()));
	} else {
		int32_t anchor = 0;

		switch (m_align & Align_Horizontal) {
		case Align_HCenter:
			anchor = get_eff_w() / 2;
			break;
		case Align_Right:
			anchor = get_eff_w();
			break;
		default:
			break;
		}
		m->ww.draw(dst, Point(anchor, -m_scrollbar.get_scrollpos()), m_align);
	}
}

bool Multiline_Textarea::handle_mousepress
	(Uint8 const btn, int32_t const x, int32_t const y)
{
	return
		btn == SDL_BUTTON_WHEELUP or btn == SDL_BUTTON_WHEELDOWN ?
		m_scrollbar.handle_mousepress(btn, x, y) : false;
}

} // namespace UI
