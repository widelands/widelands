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
Statebox::Statebox
	(Panel             * const parent,
	 Point               const p,
	 const Image* pic,
	 const std::string &       tooltip_text)
	:
	Panel  (parent, p.x, p.y, kStateboxSize, kStateboxSize, tooltip_text),
	m_flags(Is_Enabled),
	rendered_text_(nullptr)
{
	uint16_t w = pic->width();
	uint16_t h = pic->height();
	set_desired_size(w, h);
	set_size(w, h);

	set_flags(Has_Custom_Picture, true);
	m_pic_graphics = pic;
}

Statebox::Statebox
	(Panel             * const parent,
	 Point               const p,
	 const std::string& label_text,
	 const std::string &       tooltip_text,
	 uint32_t width)
	:
	Panel  (parent, p.x, p.y, kStateboxSize, kStateboxSize, tooltip_text),
	m_flags(Is_Enabled),
	rendered_text_(
		label_text.empty() ?
			nullptr :
			UI::g_fh1->render(as_uifont(label_text),
									width > (kStateboxSize + kPadding) ? width - kStateboxSize - kPadding : 0))
{
	m_pic_graphics = g_gr->images().get("pics/checkbox_light.png");
	if (rendered_text_ ) {
		int w = rendered_text_->width() + kPadding + m_pic_graphics->width() / 2;
		int h = std::max(rendered_text_->height(), m_pic_graphics->height());
		set_desired_size(w, h);
		set_size(w, h);
	}
}

Statebox::~Statebox()
{
}


/**
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked
 * and is somewhat darker to tell it apart from enabled ones.
 *
 * Args: enabled  true if the checkbox should be enabled, false otherwise
 */
void Statebox::set_enabled(bool const enabled)
{
	if (((m_flags & Is_Enabled) > 1) && enabled)
		return;

	set_flags(Is_Enabled, enabled);

	if (!(m_flags & Has_Custom_Picture)) {
		m_pic_graphics = g_gr->images().get(enabled ? "pics/checkbox_light.png" : "pics/checkbox.png");
		set_flags
			(Is_Highlighted, (m_flags & Is_Highlighted) && (m_flags & Is_Enabled));
	}

	update();
}


/**
 * Changes the state of the checkbox.
 *
 * Args: on  true if the checkbox should be checked
 */
void Statebox::set_state(bool const on) {
	if (on ^ ((m_flags & Is_Checked) > 1)) {
		set_flags(Is_Checked, on);
		changed();
		changedto(on);
		update();
	}
}


/**
 * Redraw the entire checkbox
*/
void Statebox::draw(RenderTarget & dst)
{
	if (m_flags & Has_Custom_Picture) {
		// center picture
		const uint16_t w = m_pic_graphics->width();
		const uint16_t h = m_pic_graphics->height();

		dst.blit(Point((get_inner_w() - w) / 2, (get_inner_h() - h) / 2), m_pic_graphics);

		if (m_flags & Is_Checked) {
			dst.draw_rect
				(Rect(Point(0, 0), get_w(), get_h()), RGBColor(229, 116,   2));
		} else if (m_flags & Is_Highlighted) {
			dst.draw_rect
				(Rect(Point(0, 0), get_w(), get_h()), RGBColor(100, 100,  80));
		}
	} else {
		static_assert(0 <= kStateboxSize, "assert(0 <= STATEBOX_WIDTH) failed.");
		static_assert(0 <= kStateboxSize, "assert(0 <= STATEBOX_HEIGHT) failed.");
		Point image_anchor(0, 0);
		Point text_anchor(kStateboxSize + kPadding, 0);

		if (rendered_text_) {
			if (UI::g_fh1->fontset().is_rtl()) {
				text_anchor.x = 0;
				image_anchor.x = rendered_text_->width() + kPadding;
				image_anchor.y = (get_h() - kStateboxSize) / 2;
			}
			dst.blit(text_anchor, rendered_text_, BlendMode::UseAlpha, UI::Align::Align_Left);
		}

		dst.blitrect
			(image_anchor,
			 m_pic_graphics,
			 Rect
				(Point(m_flags & Is_Checked ? kStateboxSize : 0, 0),
				 kStateboxSize, kStateboxSize));

		if (m_flags & Is_Highlighted)
			dst.draw_rect
				(Rect(image_anchor, kStateboxSize + 1, kStateboxSize + 1), RGBColor(100, 100,  80));
	}
}


/**
 * Highlight the checkbox when the mouse moves into it
 */
void Statebox::handle_mousein(bool const inside) {
	bool oldhl = m_flags & Is_Highlighted;
	set_flags(Is_Highlighted, inside && (m_flags & Is_Enabled));

	if (oldhl != (m_flags & Is_Highlighted))
		update();
}


/**
 * Left-click: Toggle checkbox state
 */
bool Statebox::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && (m_flags & Is_Enabled)) {
		clicked();
		return true;
	} else
		return false;
}
bool Statebox::handle_mouserelease(const uint8_t btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT;
}

bool Statebox::handle_mousemove(const uint8_t, int32_t, int32_t, int32_t, int32_t) {
	return true; // We handle this always by lighting up
}

/**
 * Toggle the checkbox state
 */
void Checkbox::clicked()
{
	clickedto(!get_state());
	set_state(!get_state());
	play_click();
}

}
