/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

Textarea::Textarea(Panel* parent, int32_t x, int32_t y, const std::string& text, Align align)
   : Panel(parent, x, y, 0, 0), layoutmode_(AutoMove), align_(align) {
	init();
	set_text(text);
}

Textarea::Textarea(Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, Align align)
   : Panel(parent, x, y, w, h), layoutmode_(AutoMove), align_(align) {
	init();
}

Textarea::Textarea(Panel* parent,
                   int32_t x,
                   int32_t y,
                   uint32_t w,
                   uint32_t h,
                   const std::string& text,
                   Align align)
   : Panel(parent, x, y, w, h), layoutmode_(AutoMove), align_(align) {
	init();
	set_text(text);
}

Textarea::Textarea(Panel* parent, const std::string& text, Align align)
   : Panel(parent, 0, 0, 0, 0), layoutmode_(Layouted), align_(align) {
	init();
	set_text(text);
}

/**
 * Initialization tasks that are common to all constructors.
 */
void Textarea::init() {
	fixed_width_ = 0;
	set_handle_mouse(false);
	set_thinks(false);
	color_ = UI_FONT_CLR_FG;
	fontsize_ = UI_FONT_SIZE_SMALL;
	update();
}

void Textarea::set_color(RGBColor color) {
	if (color_ != color) {
		color_ = color;
		update();
	}
}

void Textarea::set_fontsize(int fontsize) {
	if (fontsize_ != fontsize) {
		fontsize_ = fontsize;
		update();
	}
}

void Textarea::update() {
	if (layoutmode_ == AutoMove) {
		collapse();  // collapse() implicitly updates the size and position
	}

	rendered_text_ = autofit_ui_text(text_, fixed_width_, color_, fontsize_);

	if (layoutmode_ == AutoMove) {
		expand();
	} else if (layoutmode_ == Layouted) {
		update_desired_size();
	}
}

/**
 * Set the text of the Textarea. Size (or desired size) is automatically
 * adjusted depending on the Textarea mode.
 */
void Textarea::set_text(const std::string& text) {
	if (text_ != text) {
		text_ = text;
		update();
	}
}

const std::string& Textarea::get_text() {
	return text_;
}

/**
 * Set the fixed width. The Textarea will still collapse, but then restore this width when expand()
 * is called.
 * If this is set, text will also autoshrink to fit the width.
 */
void Textarea::set_fixed_width(int w) {
	if (fixed_width_ != w) {
		fixed_width_ = w;
		update();
	}
}

/**
 * Redraw the Textarea
 */
void Textarea::draw(RenderTarget& dst) {
	if (!text_.empty()) {
		Vector2i anchor(
		   (align_ == Align::kCenter) ? get_w() / 2 : (align_ == UI::Align::kRight) ? get_w() : 0, 0);
		UI::correct_for_align(align_, rendered_text_->width(), &anchor);
		dst.blit(anchor, rendered_text_, BlendMode::UseAlpha);
	}
}

/**
 * Reduce the Textarea to size 0x0 without messing up the alignment
 */
void Textarea::collapse() {
	int32_t x = get_x();
	int32_t y = get_y();
	int32_t w = get_w();

	switch (align_) {
	case UI::Align::kCenter:
		x += w >> 1;
		break;
	case UI::Align::kRight:
		x += w;
		break;
	case UI::Align::kLeft:
		break;
	}

	set_pos(Vector2i(x, y));
	set_size(0, 0);
}

/**
 * Expand the size of the Textarea until it fits the size of the text
 */
void Textarea::expand() {
	int32_t x = get_x();
	int32_t y = get_y();

	update_desired_size();
	int w, h = 0;
	get_desired_size(&w, &h);

	switch (align_) {
	case UI::Align::kCenter:
		x -= w >> 1;
		break;
	case UI::Align::kRight:
		x -= w;
		break;
	case UI::Align::kLeft:
		break;
	}

	set_pos(Vector2i(x, y));
	set_size(w, h);
}

/**
 * Recompute the desired size based on the size of the text.
 */
void Textarea::update_desired_size() {
	uint32_t w = 0;
	uint16_t h = 0;

	if (rendered_text_) {
		w = fixed_width_ > 0 ? fixed_width_ : rendered_text_->width();
		h = rendered_text_->height();
		// We want empty textareas to have height
		if (text_.empty()) {
			h = UI::g_fh1->render(
			                as_uifont(UI::g_fh1->fontset()->representative_character(), fontsize_))
			       ->height();
		}
	}
	set_desired_size(w, h);
}
}
