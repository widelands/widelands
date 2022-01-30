/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_basic/textarea.h"

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/bidi.h"
#include "graphic/text_layout.h"

namespace UI {

Textarea::Textarea(Panel* parent,
                   PanelStyle s,
                   FontStyle style,
                   int32_t x,
                   int32_t y,
                   uint32_t w,
                   uint32_t h,
                   const std::string& text,
                   Align align,
                   LayoutMode layout_mode)
   : Panel(parent, s, x, y, w, h),
     layoutmode_(layout_mode),
     align_(align),
     text_(text),
     font_style_(style),
     font_style_override_(nullptr) {
	fixed_width_ = 0;
	set_handle_mouse(false);
	set_thinks(false);
	font_scale_ = 1.0f;
	update();
}

Textarea::Textarea(Panel* parent,
                   PanelStyle s,
                   FontStyle style,
                   int32_t x,
                   int32_t y,
                   uint32_t w,
                   uint32_t h,
                   const std::string& text,
                   Align align)
   : Textarea(parent, s, style, x, y, w, h, text, align, LayoutMode::AutoMove) {
}

Textarea::Textarea(
   Panel* parent, PanelStyle s, FontStyle style, const std::string& text, Align align)
   : Textarea(parent, s, style, 0, 0, 0, 0, text, align, LayoutMode::Layouted) {
}

inline const FontStyleInfo& Textarea::font_style() const {
	return font_style_override_ ? *font_style_override_ : g_style_manager->font_style(font_style_);
}

void Textarea::set_style(const FontStyle style) {
	font_style_ = style;
	font_style_override_ = nullptr;
	update();
}
void Textarea::set_style_override(const FontStyleInfo& style) {
	font_style_override_ = &style;
	update();
}

void Textarea::set_font_scale(float scale) {
	font_scale_ = scale;
	update();
}

void Textarea::update_template() {
	update();  // update rendered_text_
}

void Textarea::update() {
	if (layoutmode_ == LayoutMode::AutoMove) {
		collapse();  // collapse() implicitly updates the size and position
	}

	FontStyleInfo scaled_style(font_style());
	scaled_style.set_size(std::max(g_style_manager->minimum_font_size(),
	                               static_cast<int>(std::ceil(scaled_style.size() * font_scale_))));
	rendered_text_ = autofit_text(richtext_escape(text_), scaled_style, fixed_width_);

	if (layoutmode_ == LayoutMode::AutoMove) {
		expand();
	} else if (layoutmode_ == LayoutMode::Layouted) {
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
		Align alignment = mirror_alignment(align_, i18n::has_rtl_character(text_.c_str(), 20));
		Vector2i anchor((alignment == Align::kCenter)    ? get_w() / 2 :
		                (alignment == UI::Align::kRight) ? get_w() :
                                                         0,
		                0);
		rendered_text_->draw(dst, anchor, alignment);
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
			h = text_height(font_style(), font_scale_);
		}
	}
	set_desired_size(w, h);
}
}  // namespace UI
