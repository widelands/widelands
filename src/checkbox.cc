/*
 * Copyright (C) 2002 Widelands Development Team
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

#include "widelands.h"
#include "ui.h"

/*
==============================================================================

Statebox

==============================================================================
*/


/*
===============
Statebox::Statebox

Stateboxes start out enabled and unchecked.
If picid is non-zero, the given picture is used instead of the normal checkbox
graphics.
===============
*/
Statebox::Statebox(Panel *parent, int x, int y, uint picid)
	: Panel(parent, x, y, STATEBOX_WIDTH, STATEBOX_HEIGHT)
{
	if (picid)
	{
		int w, h;

		g_gr->get_picture_size(picid, &w, &h);
		set_size(w, h);

		m_custom_picture = true;
		m_pic_graphics = picid;
	}
	else
	{
		m_custom_picture = false;
		m_pic_graphics = g_gr->get_picture(PicMod_UI, "pics/checkbox.png");
	}

	m_highlighted = false;
	m_enabled = true;
	m_state = false;

	m_clr_highlight.set(100, 100, 80);
	m_clr_state.set(229, 161, 2);
}


/*
===============
Statebox::~Statebox

Clean up resources
===============
*/
Statebox::~Statebox()
{
}


/** Statebox::set_enabled(bool enabled)
 *
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked.
 *
 * Args: enabled	true if the checkbox should be disabled
 */
void Statebox::set_enabled(bool enabled)
{
	m_enabled = enabled;
	if (!m_enabled)
		m_highlighted = false;
	update(0, 0, get_w(), get_h());
}


/** Statebox::set_state(bool on)
 *
 * Changes the state of the checkbox.
 *
 * Args: on		true if the checkbox should be checked
 */
void Statebox::set_state(bool on)
{
	if (on == m_state)
		return;

	m_state = on;
	changed.call();
	changedto.call(on);
	update(0, 0, get_w(), get_h());
}


/*
===============
Statebox::draw

Redraw the entire checkbox
===============
*/
void Statebox::draw(RenderTarget* dst)
{
	if (m_custom_picture)
	{
		dst->blit(0, 0, m_pic_graphics);

		if (m_state)
			dst->draw_rect(0, 0, get_w(), get_h(), m_clr_state);
		else if (m_highlighted)
			dst->draw_rect(0, 0, get_w(), get_h(), m_clr_highlight);
	}
	else
	{
		int x;

		if (m_state)
			x = STATEBOX_WIDTH;
		else
			x = 0;
		dst->blitrect(0, 0, m_pic_graphics, x, 0, STATEBOX_WIDTH, STATEBOX_HEIGHT);

		if (m_highlighted)
			dst->draw_rect(0, 0, get_w(), get_h(), m_clr_highlight);
	}
}


/** Statebox::handle_mousein(bool inside)
 *
 * Highlight the checkbox when the mouse moves into it
 */
void Statebox::handle_mousein(bool inside)
{
	m_highlighted = inside;
	update(0, 0, get_w(), get_h());
}


/** Statebox::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Left-click: Toggle checkbox state
 */
bool Statebox::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0)
		return false;

	if (down) {
		if (m_enabled)
			clicked();
	}

	return true;
}


/*
==============================================================================

Checkbox

==============================================================================
*/

/*
A checkbox only differs from a Statebox in that clicking on it toggles the
state
*/

/** Checkbox::clicked()
 *
 * Toggle the checkbox state
 */
void Checkbox::clicked()
{
	set_state(!get_state());
}
