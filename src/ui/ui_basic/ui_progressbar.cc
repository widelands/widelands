/*
 * Copyright (C) 2004 by Widelands Development Team
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



/**
Initialize the progress bar.
*/
UIProgress_Bar::UIProgress_Bar(UIPanel* parent, int x, int y, int w, int h, uint orientation)
	: UIPanel(parent, x, y, w, h)
{
	m_orientation = orientation;

	m_state = 0;
	m_total = 100;
}


/**
Set the current state of progress.
*/
void UIProgress_Bar::set_state(uint state)
{
	m_state = state;

	update(0, 0, get_w(), get_h());
}


/**
Set the maximum state
*/
void UIProgress_Bar::set_total(uint total)
{
	m_total = total;

	update(0, 0, get_w(), get_h());
}


/**
Draw the progressbar.
*/
void UIProgress_Bar::draw(RenderTarget* dst)
{
	RGBColor color;
	float percent = (float)m_state / m_total;

	if (percent < 0)
		percent = 0;
	else if (percent > 1)
		percent = 1;

	if (percent <= 0.15)
		color = RGBColor(255, 0, 0);
	else if (percent <= 0.5)
		color = RGBColor(255, 255, 0);
	else
		color = RGBColor(0, 255, 0);

	// Draw the actual bar
	if (m_orientation == Horizontal)
	{
		int w = (int)(get_w() * percent);

		dst->fill_rect(0, 0, w, get_h(), color);
		dst->fill_rect(w, 0, get_w() - w, get_h(), RGBColor(0, 0, 0));
	}
	else
	{
		int h = (int)(get_h() * (1.0 - percent));

		dst->fill_rect(0, 0, get_w(), h, RGBColor(0, 0, 0));
		dst->fill_rect(0, h, get_w(), get_h() - h, color);
	}

	// Print the state in percent
	char buf[30];

	snprintf(buf, sizeof(buf), "%u%%", (uint)(percent * 100));

	g_font->draw_string(dst, get_w() / 2, get_h() / 2, buf, Align_Center);
}

