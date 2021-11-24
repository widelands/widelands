/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <SDL_mouse.h>

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"

constexpr int kPadding = 4;

namespace {
int text_width(int available_width, int pic_width) {
	return available_width > (pic_width + kPadding) ? available_width - pic_width - kPadding : 0;
}
}  // namespace

namespace UI {
/**
 * Stateboxes start out enabled and unchecked.
 * If pic is non-zero, the given picture is used instead of the normal
 * checkbox graphics.
 */
Statebox::Statebox(Panel* const parent,
                   PanelStyle s,
                   Vector2i const p,
                   const Image* pic,
                   const std::string& tooltip_text)
   : Panel(parent, s, p.x, p.y, kStateboxSize, kStateboxSize, tooltip_text),
     flags_(Is_Enabled),
     pic_graphics_(pic),
     rendered_text_(nullptr) {
	uint16_t w = pic->width();
	uint16_t h = pic->height();
	set_desired_size(w, h);
	set_size(w, h);
	set_flags(Has_Custom_Picture, true);
	set_can_focus(true);
}

static inline std::string get_checkbox_graphics(const PanelStyle& s) {
	return s == PanelStyle::kWui ? "images/ui_basic/checkbox_light.png" :
                                  "images/ui_basic/checkbox_dark.png";
}

Statebox::Statebox(Panel* const parent,
                   PanelStyle s,
                   Vector2i const p,
                   const std::string& label_text,
                   const std::string& tooltip_text,
                   int width)
   : Panel(parent, s, p.x, p.y, std::max(width, kStateboxSize), kStateboxSize, tooltip_text),
     flags_(Is_Enabled),
     pic_graphics_(g_image_cache->get(get_checkbox_graphics(panel_style_))),
     rendered_text_(nullptr),
     label_text_(label_text) {
	set_flags(Has_Text, !label_text_.empty());
	set_can_focus(true);
	layout();
}

void Statebox::update_template() {
	layout();  // update rendered_text_
}

void Statebox::layout() {
	// We only need to relayout if we have text and the available width changed
	if ((flags_ & Has_Text)) {
		int w = get_w();
		int h = kStateboxSize;
		int pic_width = kStateboxSize;
		if (pic_graphics_) {
			w = std::max(pic_graphics_->width(), w);
			h = pic_graphics_->height();
			pic_width = pic_graphics_->width();
		}
		rendered_text_ = label_text_.empty() ?
                          nullptr :
                          UI::g_fh->render(as_richtext_paragraph(
		                                        label_text_, panel_style_ == PanelStyle::kFsMenu ?
                                                              UI::FontStyle::kFsMenuLabel :
                                                              UI::FontStyle::kWuiLabel),
		                                     text_width(get_w(), pic_width));
		if (rendered_text_) {
			w = std::max(rendered_text_->width() + kPadding + pic_width, w);
			h = std::max(rendered_text_->height(), h);
		}
		set_desired_size(w, h);
	}
}

/**
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked
 * and is somewhat darker to tell it apart from enabled ones.
 *
 * Args: enabled  true if the checkbox should be enabled, false otherwise
 */
void Statebox::set_enabled(bool const enabled) {
	if (((flags_ & Is_Enabled) > 1) && enabled) {
		return;
	}

	set_flags(Is_Enabled, enabled);
	set_can_focus(enabled);

	if (!(flags_ & Has_Custom_Picture)) {
		pic_graphics_ = g_image_cache->get(enabled ? get_checkbox_graphics(panel_style_) :
                                                   "images/ui_basic/checkbox.png");
		set_flags(Is_Highlighted, (flags_ & Is_Highlighted) && (flags_ & Is_Enabled));
	}
}

/**
 * Changes the state of the checkbox.
 *
 * Args: on  true if the checkbox should be checked
 */
void Statebox::set_state(bool const on, const bool send_signal) {
	if (on ^ ((flags_ & Is_Checked) > 1)) {
		set_flags(Is_Checked, on);
		if (send_signal) {
			changed();
			changedto(on);
		}
	}
}

std::vector<Recti> Statebox::focus_overlay_rects() {
	return (flags_ & Has_Custom_Picture) ? Panel::focus_overlay_rects(1, 1, -1) :
                                          Panel::focus_overlay_rects();
}

/**
 * Redraw the entire checkbox
 */
void Statebox::draw_overlay(RenderTarget& dst) {
	Panel::draw_overlay(dst);
	if (flags_ & Has_Custom_Picture) {
		// TODO(Nordfriese): Move colours to style manager
		if (flags_ & Is_Checked) {
			dst.draw_rect(Recti(0, 0, get_w(), get_h()), RGBColor(226, 200, 6));
		} else if (flags_ & Is_Highlighted) {
			dst.draw_rect(Recti(0, 0, get_w(), get_h()), RGBColor(100, 100, 80));
		}
	}
}
void Statebox::draw(RenderTarget& dst) {
	if (flags_ & Has_Custom_Picture) {
		if (flags_ & Is_Checked) {
			dst.brighten_rect(Recti(0, 0, get_w(), get_h()), -24);
		}

		// center picture
		const uint16_t w = pic_graphics_->width();
		const uint16_t h = pic_graphics_->height();

		dst.blit(Vector2i((get_inner_w() - w) / 2, (get_inner_h() - h) / 2), pic_graphics_);
	} else {
		static_assert(0 <= kStateboxSize, "assert(0 <= kStateboxSize) failed.");
		Vector2i image_anchor = Vector2i::zero();
		Vector2i text_anchor(kStateboxSize + kPadding, 0);

		if (rendered_text_) {
			if (UI::g_fh->fontset()->is_rtl()) {
				text_anchor.x = 0;
				image_anchor.x = rendered_text_->width() + kPadding;
				image_anchor.y = (get_h() - kStateboxSize) / 2;
			}
			rendered_text_->draw(dst, text_anchor);
		}

		dst.blitrect(image_anchor, pic_graphics_,
		             Recti(Vector2i((flags_ & Is_Checked) ? kStateboxSize : 0, 0), kStateboxSize,
		                   kStateboxSize));

		if (flags_ & Is_Highlighted) {
			dst.draw_rect(
			   Recti(image_anchor, kStateboxSize + 1, kStateboxSize + 1), RGBColor(100, 100, 80));
		}
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
		button_clicked();
		return true;
	}
	return false;
}

bool Statebox::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t) {
	return true;  // We handle this always by lighting up
}

bool Statebox::handle_key(bool down, SDL_Keysym code) {
	if (down && code.sym == SDLK_SPACE) {
		button_clicked();
		return true;
	}
	return Panel::handle_key(down, code);
}

/**
 * Toggle the checkbox state
 */
void Checkbox::button_clicked() {
	clickedto(!get_state());
	set_state(!get_state());
	play_click();
}
}  // namespace UI
