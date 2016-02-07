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
		layoutmode_(AutoMove),
		align_    (align)
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
		layoutmode_(AutoMove),
		align_    (align)
{
	init();
}

Textarea:: Textarea
	(Panel * parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 const std::string & text, Align align)
	:
		Panel      (parent, x, y, w, h),
		layoutmode_(AutoMove),
		align_    (align)
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
layoutmode_(Layouted),
align_(align)
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
	if (textstyle_ == style)
		return;

	if (layoutmode_ == AutoMove)
		collapse();
	textstyle_ = style;
	rendered_text_ = UI::g_fh1->render(
									as_uifont(text_,
												 textstyle_.font->size() - UI::g_fh1->fontset().size_offset(),
												 textstyle_.fg));

	if (layoutmode_ == AutoMove)
		expand();
	else if (layoutmode_ == Layouted)
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
	if (text_ == text)
		return;

	if (layoutmode_ == AutoMove)
		collapse(); // collapse() implicitly updates

	text_ = text;
	rendered_text_ = UI::g_fh1->render(
									as_uifont(text_,
												 textstyle_.font->size() - UI::g_fh1->fontset().size_offset(),
												 textstyle_.fg));
	if (layoutmode_ == AutoMove)
		expand();
	else if (layoutmode_ == Layouted)
		update_desired_size();
}

const std::string& Textarea::get_text()
{
	return text_;
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
	if (!text_.empty()) {
		Point anchor
			(static_cast<int>(align_ & UI::Align::kHCenter) ?
			 get_w() / 2 : static_cast<int>(align_ & UI::Align::kRight)  ? get_w() : 0,
			 static_cast<int>(align_ & UI::Align::kVCenter) ?
			 get_h() / 2 : static_cast<int>(align_ & UI::Align::kBottom) ? get_h() : 0);

		dst.blit(anchor, rendered_text_, BlendMode::UseAlpha, align_);
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

	if (static_cast<int>(align_ & UI::Align::kHCenter))
		x += w >> 1;
	else if (static_cast<int>(align_ & UI::Align::kRight))
		x += w;

	if (static_cast<int>(align_ & UI::Align::kVCenter))
		y += h >> 1;
	else if (static_cast<int>(align_ & UI::Align::kBottom))
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
	int w, h;
	get_desired_size(&w, &h);

	if      (static_cast<int>(align_ & UI::Align::kHCenter))
		x -= w >> 1;
	else if (static_cast<int>(align_ & UI::Align::kRight))
		x -= w;

	if      (static_cast<int>(align_ & UI::Align::kVCenter))
		y -= h >> 1;
	else if (static_cast<int>(align_ & UI::Align::kBottom))
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
		if (text_.empty()) {
			h = UI::g_fh1->render(
					 as_uifont(".",
								  textstyle_.font->size() - UI::g_fh1->fontset().size_offset(),
								  textstyle_.fg))->height();
		}
	}
	set_desired_size(w, h);
}

}
