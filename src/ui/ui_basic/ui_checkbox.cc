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
#include "ui_basic.h"

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
UIStatebox::UIStatebox(UIPanel *parent, int x, int y, uint picid)
	: UIPanel(parent, x, y, STATEBOX_WIDTH, STATEBOX_HEIGHT)
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

   m_id=-1;
}


/**
Clean up resources
*/
UIStatebox::~UIStatebox()
{
}


/** 
 * Set the enabled state of the checkbox. A disabled checkbox cannot be clicked.
 *
 * Args: enabled	true if the checkbox should be disabled
 */
void UIStatebox::set_enabled(bool enabled)
{
	m_enabled = enabled;
	if (!m_enabled)
		m_highlighted = false;
	update(0, 0, get_w(), get_h());
}


/** 
 * Changes the state of the checkbox.
 *
 * Args: on		true if the checkbox should be checked
 */
void UIStatebox::set_state(bool on)
{
	if (on == m_state)
		return;

	m_state = on;
	changed.call();
	changedto.call(on);
	if(m_id!=-1) changedtoid.call(m_id,on);
   update(0, 0, get_w(), get_h());
}


/**
Redraw the entire checkbox
*/
void UIStatebox::draw(RenderTarget* dst)
{
	if (m_custom_picture)
	{
		// center picture
      int xpos, ypos;
      int w,h;
     
      g_gr->get_picture_size(m_pic_graphics, &w, &h);
      
      xpos=(get_inner_w()-w)/2;
      ypos=(get_inner_h()-h)/2;
      dst->blit(xpos, ypos, m_pic_graphics);

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


/**
 * Highlight the checkbox when the mouse moves into it
 */
void UIStatebox::handle_mousein(bool inside)
{
	m_highlighted = inside;
	update(0, 0, get_w(), get_h());
}


/** 
 * Left-click: Toggle checkbox state
 */
bool UIStatebox::handle_mouseclick(uint btn, bool down, int x, int y)
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

/**
 * Toggle the checkbox state
 */
void UICheckbox::clicked()
{
	set_state(!get_state());
}
