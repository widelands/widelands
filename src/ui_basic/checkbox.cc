/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "checkbox.h"

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"

#include "compile_assert.h"

namespace UI {
/**
 * Stateboxes start out enabled and unchecked.
 * If picid is non-zero, the given picture is used instead of the normal
 * checkbox graphics.
*/
Statebox::Statebox
	(Panel             * const parent,
	 Point               const p,
	 PictureID           const picid,
	 std::string const &       tooltip_text)
	:
	Panel  (parent, p.x, p.y, STATEBOX_WIDTH, STATEBOX_HEIGHT, tooltip_text),
	m_flags(Is_Enabled)
{
	if (picid != g_gr->get_no_picture()) {
		uint32_t w, h;
		g_gr->get_picture_size(picid, w, h);
		set_size(w, h);

		set_flags(Has_Custom_Picture, true);
		m_pic_graphics = picid;
	} else
		m_pic_graphics =
			g_gr->get_picture(PicMod_UI, "pics/checkbox_light_new.png");

	m_id = -1;
}


Statebox::~Statebox() {
	if (m_flags & Owns_Custom_Picture) {
		assert(m_flags & Has_Custom_Picture);
		g_gr->free_surface(m_pic_graphics);
	}
}


/**
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked
 * and is somewhat darker to tell it apart from enabled ones.
 *
 * Args: enabled  true if the checkbox should be enabled, false otherwise
 */
void Statebox::set_enabled(bool const enabled)
{
	set_flags(Is_Enabled, enabled);

	if (not (m_flags & Has_Custom_Picture)) {
		m_pic_graphics = g_gr->get_picture
			(PicMod_UI,
			 enabled ? "pics/checkbox_light_new.png" : "pics/checkbox.png");
		set_flags
			(Is_Highlighted, m_flags & Is_Highlighted and m_flags & Is_Enabled);
	}

	update();
}


/**
 * Changes the state of the checkbox.
 *
 * Args: on  true if the checkbox should be checked
 */
void Statebox::set_state(bool const on) {
	if (on xor (m_flags & Is_Checked)) {
		set_flags(Is_Checked, on);
		changed    .call        ();
		changedto  .call      (on);
		changedtoid.call(m_id, on);
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
		uint32_t w, h;
		g_gr->get_picture_size(m_pic_graphics, w, h);

		dst.blit
			(Point((get_inner_w() - w) / 2, (get_inner_h() - h) / 2),
			 m_pic_graphics);

		if (m_flags & Is_Checked)
			dst.draw_rect
				(Rect(Point(0, 0), get_w(), get_h()), RGBColor(229, 116,   2));
		else if (m_flags & Is_Highlighted)
			dst.draw_rect
				(Rect(Point(0, 0), get_w(), get_h()), RGBColor(100, 100,  80));
	} else {
		compile_assert(0 <= STATEBOX_WIDTH);
		compile_assert(0 <= STATEBOX_HEIGHT);
		dst.blitrect
			(Point(0, 0),
			 m_pic_graphics,
			 Rect
			 	(Point(m_flags & Is_Checked ? STATEBOX_WIDTH : 0, 0),
			 	 STATEBOX_WIDTH, STATEBOX_HEIGHT));

		if (m_flags & Is_Highlighted)
			dst.draw_rect
				(Rect(Point(0, 0), get_w(), get_h()), RGBColor(100, 100,  80));
	}
}


/**
 * Highlight the checkbox when the mouse moves into it
 */
void Statebox::handle_mousein(bool const inside) {
	set_flags(Is_Highlighted, inside);
	update();
}


/**
 * Left-click: Toggle checkbox state
 */
bool Statebox::handle_mousepress(const Uint8 btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT and m_flags & Is_Enabled) {
		clicked();
		return true;
	} else
		return false;
}
bool Statebox::handle_mouserelease(const Uint8 btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT;
}


/**
 * Toggle the checkbox state
 */
void Checkbox::clicked()
{
	clickedto.call(!get_state());
	set_state(!get_state());
	play_click();
}

}
