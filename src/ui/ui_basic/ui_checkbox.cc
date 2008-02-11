/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "ui_checkbox.h"

#include "graphic.h"
#include "rendertarget.h"

namespace UI {
/*
==============================================================================

Statebox

==============================================================================
*/


/**
Stateboxes start out enabled and unchecked.
If picid is non-zero, the given picture is used instead of the normal checkbox
graphics.
*/
Statebox::Statebox(Panel *parent, int32_t x, int32_t y, uint32_t picid)
	: Panel(parent, x, y, STATEBOX_WIDTH, STATEBOX_HEIGHT)
{
	if (picid)
	{
		uint32_t w, h;
		g_gr->get_picture_size(picid, w, h);
		set_size(w, h);

		m_custom_picture = true;
		m_pic_graphics = picid;
	}
	else
	{
		m_custom_picture = false;
		m_pic_graphics =
			g_gr->get_picture(PicMod_UI, "pics/checkbox_light_new.png");
	}

	m_highlighted = false;
	m_enabled = true;
	m_state = false;

	m_clr_highlight = RGBColor(100, 100, 80);
	m_clr_state     = RGBColor(229, 161,  2);

	m_id = -1;
}


Statebox::~Statebox() {}


/**
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked
 * and is somewhat darker to tell it apart from enabled ones.
 *
 * Args: enabled  true if the checkbox should be enabled, false otherwise
 */
void Statebox::set_enabled(bool enabled)
{
	m_enabled = enabled;

	if (!m_custom_picture) {
		m_pic_graphics = g_gr->get_picture
			(PicMod_UI,
			 m_enabled ? "pics/checkbox_light_new.png" : "pics/checkbox.png");
		m_highlighted &= m_enabled;
	}

	update(0, 0, get_w(), get_h());
}


/**
 * Changes the state of the checkbox.
 *
 * Args: on  true if the checkbox should be checked
 */
void Statebox::set_state(bool on)
{
	if (on == m_state)
		return;

	m_state = on;
	changed.call();
	changedto.call(on);
	if (m_id!=-1) changedtoid.call(m_id, on);
	update(0, 0, get_w(), get_h());
}


/**
Redraw the entire checkbox
*/
void Statebox::draw(RenderTarget* dst)
{
	if (m_custom_picture)
	{
		// center picture
		uint32_t w, h;
		g_gr->get_picture_size(m_pic_graphics, w, h);

		dst->blit
			(Point((get_inner_w() - w) / 2, (get_inner_h() - h) / 2),
			 m_pic_graphics);

		if (m_state)
			dst->draw_rect(Rect(Point(0, 0), get_w(), get_h()), m_clr_state);
		else if (m_highlighted)
			dst->draw_rect(Rect(Point(0, 0), get_w(), get_h()), m_clr_highlight);
	}
	else
	{
		compile_assert(0 <= STATEBOX_WIDTH);
		compile_assert(0 <= STATEBOX_HEIGHT);
		dst->blitrect
			(Point(0, 0),
			 m_pic_graphics,
			 Rect
			 (Point
			  (m_state ? STATEBOX_WIDTH : 0, 0), STATEBOX_WIDTH, STATEBOX_HEIGHT));

		if (m_highlighted)
			dst->draw_rect(Rect(Point(0, 0), get_w(), get_h()), m_clr_highlight);
	}
}


/**
 * Highlight the checkbox when the mouse moves into it
 */
void Statebox::handle_mousein(bool inside)
{
	m_highlighted = inside;
	update(0, 0, get_w(), get_h());
}


/**
 * Left-click: Toggle checkbox state
 */
bool Statebox::handle_mousepress(const Uint8 btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT) return false;

	if (m_enabled) clicked();

	return true;
}
bool Statebox::handle_mouserelease(const Uint8 btn, int32_t, int32_t)
{return btn == SDL_BUTTON_LEFT;}


/*
==============================================================================

Checkbox

==============================================================================
*/

/**
 * Toggle the checkbox state
 */
void Checkbox::clicked()
{
	set_state(!get_state());
	play_click();
}
};
