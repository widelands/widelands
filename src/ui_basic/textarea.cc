/*
 * Copyright (C) 2002, 2006-2011 by the Widelands Development Team
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

#include "ui_basic/textarea.h"

#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"

namespace UI {

Textarea::Textarea
	(Panel * parent,
	 int32_t x, int32_t y,
	 const std::string & text, Align align)
	:
		Panel      (parent, x, y, 0, 0),
		m_layoutmode(AutoMove),
		m_align    (align)
{
	init();
	set_text(text);
}

Textarea::Textarea
	(Panel *  parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 Align align)
	:
		Panel      (parent, x, y, w, h),
		m_layoutmode(AutoMove),
		m_align    (align)
{
	init();
}

Textarea:: Textarea
	(Panel * parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 const std::string & text, Align align)
	:
		Panel      (parent, x, y, w, h),
		m_layoutmode(AutoMove),
		m_align    (align)
{
	init();
	set_text(text);
}

Textarea::Textarea
	(Panel * parent,
	 const std::string & text,
	 Align align)
:
Panel(parent, 0, 0, 0, 0),
m_layoutmode(Layouted),
m_align(align)
{
	init();
	set_text(text);
}

/**
 * Initialization tasks that are common to all constructors.
 */
void Textarea::init()
{
	fixed_width_ = 0;
	set_handle_mouse(false);
	set_thinks(false);
	set_textstyle(UI::TextStyle::ui_small());
}

/**
 * Set the font of the textarea.
 */
void Textarea::set_textstyle(const TextStyle & style)
{
	if (m_textstyle == style)
		return;

	if (m_layoutmode == AutoMove)
		collapse();
	m_textstyle = style;
	rendered_text_ = UI::g_fh1->render(
									as_uifont(m_text,
												 m_textstyle.font->size() - UI::g_fh1->fontset().size_offset(),
												 m_textstyle.fg));

	if (m_layoutmode == AutoMove)
		expand();
	else if (m_layoutmode == Layouted)
		update_desired_size();
}

/**
 * @deprecated
 */
void Textarea::set_font(const std::string & name, int size, RGBColor clr)
{
	set_textstyle(TextStyle::makebold(Font::get(name, size), clr));
}

/**
 * Set the text of the Textarea. Size (or desired size) is automatically
 * adjusted depending on the Textarea mode.
 */
void Textarea::set_text(const std::string & text)
{
	if (m_text == text)
		return;

	if (m_layoutmode == AutoMove)
		collapse(); // collapse() implicitly updates

	m_text = text;
	rendered_text_ = UI::g_fh1->render(
									as_uifont(m_text,
												 m_textstyle.font->size() - UI::g_fh1->fontset().size_offset(),
												 m_textstyle.fg));
	if (m_layoutmode == AutoMove)
		expand();
	else if (m_layoutmode == Layouted)
		update_desired_size();

	update();
}

const std::string& Textarea::get_text()
{
	return m_text;
}


/**
 * Set the fixed width. The Textarea will still collapse, but then restore this width when expand() is called.
 */
void Textarea::set_fixed_width(uint32_t w) {
	fixed_width_ = w;
}


/**
 * Redraw the Textarea
 */
void Textarea::draw(RenderTarget & dst)
{
	if (!m_text.empty()) {
		Point anchor
			(static_cast<int>(m_align & UI::Align::kHCenter) ?
			 get_w() / 2 : static_cast<int>(m_align & UI::Align::kRight)  ? get_w() : 0,
			 static_cast<int>(m_align & UI::Align::kVCenter) ?
			 get_h() / 2 : static_cast<int>(m_align & UI::Align::kBottom) ? get_h() : 0);

		dst.blit(anchor, rendered_text_, BlendMode::UseAlpha, m_align);
	}
}


/**
 * Reduce the Textarea to size 0x0 without messing up the alignment
 */
void Textarea::collapse()
{
	int32_t x = get_x();
	int32_t y = get_y();
	int32_t w = get_w();
	int32_t h = get_h();

	if (static_cast<int>(m_align & UI::Align::kHCenter))
		x += w >> 1;
	else if (static_cast<int>(m_align & UI::Align::kRight))
		x += w;

	if (static_cast<int>(m_align & UI::Align::kVCenter))
		y += h >> 1;
	else if (static_cast<int>(m_align & UI::Align::kBottom))
		y += h;

	set_pos(Point(x, y));
	set_size(0, 0);
}


/**
 * Expand the size of the Textarea until it fits the size of the text
 */
void Textarea::expand()
{
	int32_t x = get_x();
	int32_t y = get_y();

	update_desired_size();
	uint32_t w, h;
	get_desired_size(w, h);

	if      (static_cast<int>(m_align & UI::Align::kHCenter))
		x -= w >> 1;
	else if (static_cast<int>(m_align & UI::Align::kRight))
		x -= w;

	if      (static_cast<int>(m_align & UI::Align::kVCenter))
		y -= h >> 1;
	else if (static_cast<int>(m_align & UI::Align::kBottom))
		y -= h;

	set_pos(Point(x, y));
	set_size(w, h);
}

/**
 * Recompute the desired size based on the size of the text.
 */
void Textarea::update_desired_size()
{
	uint32_t w = 0;
	uint16_t h = 0;

	if (rendered_text_) {
		w = fixed_width_ > 0 ? fixed_width_ : rendered_text_->width();
		h = rendered_text_->height();
		// We want empty textareas to have height
		if (m_text.empty()) {
			h = UI::g_fh1->render(
					 as_uifont(".",
								  m_textstyle.font->size() - UI::g_fh1->fontset().size_offset(),
								  m_textstyle.fg))->height();
		}
	}
	set_desired_size(w, h);
}

}
