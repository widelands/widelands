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

#include "ui_basic/checkbox.h"

#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"

constexpr int kPadding = 4;

namespace UI {
/**
 * Stateboxes start out enabled and unchecked.
 * If pic is non-zero, the given picture is used instead of the normal
 * checkbox graphics.
*/
Statebox::Statebox(Panel* const parent,
                   Vector2i const p,
                   const Image* pic,
                   const std::string& tooltip_text)
   : Panel(parent, p.x, p.y, kStateboxSize, kStateboxSize, tooltip_text),
     flags_(Is_Enabled),
     rendered_text_(nullptr) {
	uint16_t w = pic->width();
	uint16_t h = pic->height();
	set_desired_size(w, h);
	set_size(w, h);

	set_flags(Has_Custom_Picture, true);
	pic_graphics_ = pic;
}

Statebox::Statebox(Panel* const parent,
                   Vector2i const p,
                   const std::string& label_text,
                   const std::string& tooltip_text,
                   uint32_t width)
   : Panel(parent, p.x, p.y, kStateboxSize, kStateboxSize, tooltip_text),
     flags_(Is_Enabled),
     rendered_text_(label_text.empty() ? nullptr :
													  UI::g_fh1->render(as_uifont(label_text),
                                                           width > (kStateboxSize + kPadding) ?
                                                              width - kStateboxSize - kPadding :
                                                              0)) {
	pic_graphics_ = g_gr->images().get("images/ui_basic/checkbox_light.png");
	if (rendered_text_) {
		int w = rendered_text_->width() + kPadding + pic_graphics_->width() / 2;
		int h = std::max(rendered_text_->height(), pic_graphics_->height());
		set_desired_size(w, h);
		set_size(w, h);
	}
}

Statebox::~Statebox() {
}

/**
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked
 * and is somewhat darker to tell it apart from enabled ones.
 *
 * Args: enabled  true if the checkbox should be enabled, false otherwise
 */
void Statebox::set_enabled(bool const enabled) {
	if (((flags_ & Is_Enabled) > 1) && enabled)
		return;

	set_flags(Is_Enabled, enabled);

	if (!(flags_ & Has_Custom_Picture)) {
		pic_graphics_ = g_gr->images().get(enabled ? "images/ui_basic/checkbox_light.png" :
		                                             "images/ui_basic/checkbox.png");
		set_flags(Is_Highlighted, (flags_ & Is_Highlighted) && (flags_ & Is_Enabled));
	}
}

/**
 * Changes the state of the checkbox.
 *
 * Args: on  true if the checkbox should be checked
 */
void Statebox::set_state(bool const on) {
	if (on ^ ((flags_ & Is_Checked) > 1)) {
		set_flags(Is_Checked, on);
		changed();
		changedto(on);
	}
}

/**
 * Redraw the entire checkbox
*/
void Statebox::draw(RenderTarget& dst) {
	if (flags_ & Has_Custom_Picture) {
		// center picture
		const uint16_t w = pic_graphics_->width();
		const uint16_t h = pic_graphics_->height();

		dst.blit(Vector2f((get_inner_w() - w) / 2., (get_inner_h() - h) / 2.), pic_graphics_);

		if (flags_ & Is_Checked) {
			dst.draw_rect(Rectf(0.f, 0.f, get_w(), get_h()), RGBColor(229, 116, 2));
		} else if (flags_ & Is_Highlighted) {
			dst.draw_rect(Rectf(0.f, 0.f, get_w(), get_h()), RGBColor(100, 100, 80));
		}
	} else {
		static_assert(0 <= kStateboxSize, "assert(0 <= STATEBOX_WIDTH) failed.");
		static_assert(0 <= kStateboxSize, "assert(0 <= STATEBOX_HEIGHT) failed.");
		Vector2f image_anchor(0.f, 0.f);
		Vector2i text_anchor(kStateboxSize + kPadding, 0);

		if (rendered_text_) {
			if (UI::g_fh1->fontset()->is_rtl()) {
				text_anchor.x = 0;
				image_anchor.x = rendered_text_->width() + kPadding;
				image_anchor.y = (get_h() - kStateboxSize) / 2;
			}
			draw_text(dst, text_anchor, rendered_text_, UI::Align::kLeft);
		}

		dst.blitrect(
		   image_anchor, pic_graphics_,
		   Recti(Vector2i(flags_ & Is_Checked ? kStateboxSize : 0, 0), kStateboxSize, kStateboxSize));

		if (flags_ & Is_Highlighted)
			dst.draw_rect(
			   Rectf(image_anchor, kStateboxSize + 1, kStateboxSize + 1), RGBColor(100, 100, 80));
	}
}

/**
 * Highlight the checkbox when the mouse moves into it
 */
void Statebox::handle_mousein(bool const inside) {
	set_flags(Is_Highlighted, inside && (flags_ & Is_Enabled));
}

/**
 * Left-click: Toggle checkbox state
 */
bool Statebox::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && (flags_ & Is_Enabled)) {
		clicked();
		return true;
	}
	return false;
}
bool Statebox::handle_mouserelease(const uint8_t btn, int32_t, int32_t) {
	return btn == SDL_BUTTON_LEFT;
}

bool Statebox::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t) {
	return true;  // We handle this always by lighting up
}

/**
 * Toggle the checkbox state
 */
void Checkbox::clicked() {
	clickedto(!get_state());
	set_state(!get_state());
	play_click();
}
}
