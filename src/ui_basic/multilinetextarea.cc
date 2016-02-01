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

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"

namespace UI {

static const uint32_t RICHTEXT_MARGIN = 2;

MultilineTextarea::MultilineTextarea
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const std::string& text,
	 const Align align,
	 MultilineTextarea::ScrollMode scroll_mode)
	:
	Panel       (parent, x, y, w, h),
	m_text      (text),
	m_style(UI::TextStyle::ui_small()),
	isrichtext(false),
	m_scrollbar (this, get_w() - scrollbar_w(), 0, scrollbar_w(), h, false),
	m_scrollmode(scroll_mode)
{
	assert(scrollbar_w() <= w);
	set_thinks(false);

	//  do not allow vertical alignment as it does not make sense
	m_align = align & UI::Align::kHorizontal;

	m_scrollbar.moved.connect(boost::bind(&MultilineTextarea::scrollpos_changed, this, _1));

	m_scrollbar.set_singlestepsize(UI::g_fh1->render(as_uifont(".", UI_FONT_SIZE_SMALL))->height());
	m_scrollbar.set_pagesize(h - 2 * UI::g_fh1->render(as_uifont(".", UI_FONT_SIZE_BIG))->height());
	m_scrollbar.set_steps(1);
	m_scrollbar.set_force_draw(m_scrollmode == ScrollMode::kScrollNormalForced || m_scrollmode == ScrollMode::kScrollLogForced);

	recompute();
}


/**
 * Replace the current text with a new one.
 * Fix up scrolling state if necessary.
 */
void MultilineTextarea::set_text(const std::string& text)
{
	m_text = text;
	recompute();
}

/**
 * Recompute the text rendering or rich-text layouting,
 * and adjust scrollbar settings accordingly.
 */
void MultilineTextarea::recompute()
{
	uint32_t height;

	// We wrap the text twice. We need to do this to account for the presence/absence of the scollbar.
	bool scrollbar_was_enabled = m_scrollbar.is_enabled();
	for (int i = 0; i < 2; ++i) {
		if (m_text.compare(0, 3, "<rt")) {
			isrichtext = false;
			std::string text_to_render = richtext_escape(m_text);
			boost::replace_all(text_to_render, "\n", "<br>");
			const Image* text_im = UI::g_fh1->render(as_uifont(text_to_render, m_style.font->size(), m_style.fg),
																  get_eff_w() - 2 * RICHTEXT_MARGIN);
			height = text_im->height();
		} else {
			isrichtext = true;
			rt.set_width(get_eff_w() - 2 * RICHTEXT_MARGIN);
			rt.parse(m_text);
			height = rt.height() + 2 * RICHTEXT_MARGIN;
		}

		bool setbottom = false;

		if (m_scrollmode == ScrollMode::kScrollLog || m_scrollmode == ScrollMode::kScrollLogForced) {
			if (m_scrollbar.get_scrollpos() >= m_scrollbar.get_steps() - 1)
				setbottom = true;
		} else if (m_scrollmode == ScrollMode::kNoScrolling) {
			m_scrollbar.set_scrollpos(0);
			m_scrollbar.set_steps(1);
			set_desired_size(get_w(), height);
			set_size(get_w(), height);
		}

		m_scrollbar.set_steps(height - get_h());
		if (setbottom)
			m_scrollbar.set_scrollpos(height - get_h());

		if (m_scrollbar.is_enabled() == scrollbar_was_enabled) {
			break; // No need to wrap twice.
		}
	}
}

/**
 * Callback from the scrollbar.
 */
void MultilineTextarea::scrollpos_changed(int32_t const /* pixels */)
{
}

/// Take care about scrollbar on resize
void MultilineTextarea::layout()
{
	recompute();

	// Take care about the scrollbar
	m_scrollbar.set_pos(Point(get_w() - scrollbar_w(), 0));
	m_scrollbar.set_size(scrollbar_w(), get_h());
}

/**
 * Redraw the textarea
 */
void MultilineTextarea::draw(RenderTarget& dst)
{
	if (isrichtext) {
		rt.draw(dst, Point(RICHTEXT_MARGIN, RICHTEXT_MARGIN - m_scrollbar.get_scrollpos()));
	} else {
		std::string text_to_render = richtext_escape(m_text);
		boost::replace_all(text_to_render, "\n", "<br>");
		const Image* text_im =
				UI::g_fh1->render(as_aligned(text_to_render, m_align, m_style.font->size(), m_style.fg),
										get_eff_w() - 2 * RICHTEXT_MARGIN);

		uint32_t blit_width = std::min(text_im->width(), static_cast<int>(get_eff_w()));
		uint32_t blit_height = std::min(text_im->height(), static_cast<int>(get_inner_h()));

		if (blit_width > 0 && blit_height > 0) {
			int32_t anchor = 0;
			Align alignment = mirror_alignment(m_align);
			switch (alignment & UI::Align::kHorizontal) {
			case UI::Align::kHCenter:
				anchor = (get_eff_w() - blit_width) / 2;
				break;
			case UI::Align::kRight:
				anchor = get_eff_w() - blit_width - RICHTEXT_MARGIN;
				break;
			default:
				anchor = RICHTEXT_MARGIN;
				break;
			}

			dst.blitrect_scale(
				Rect(anchor, 0, blit_width, blit_height),
				text_im,
				Rect(0, m_scrollbar.get_scrollpos(), blit_width, blit_height),
				1.,
				BlendMode::UseAlpha);
		}
	}
}


bool MultilineTextarea::handle_mousewheel(uint32_t which, int32_t x, int32_t y) {
	return m_scrollbar.handle_mousewheel(which, x, y);
}

void MultilineTextarea::scroll_to_top() {
	m_scrollbar.set_scrollpos(0);
}

} // namespace UI
