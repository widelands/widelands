/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include <algorithm>
#include "graphic.h"
#include "rendertarget.h"
#include "system.h"
#include "types.h"
#include "ui_scrollbar.h"

#define SCROLLBAR_AUTOREPEAT_DELAY	200
#define SCROLLBAR_AUTOREPEAT_TICK	50

/*
==============================================================================

Scrollbar

==============================================================================
*/


/**
Initialize the scrollbar using default values.
*/
UIScrollbar::UIScrollbar(UIPanel *parent, int x, int y, uint w, uint h, bool horiz)
	: UIPanel(parent, x, y, w, h)
{
	m_horizontal = horiz;

	m_pos = 0;
	m_pagesize = 5;
	m_steps = 100;

	m_pressed = None;

	if (m_horizontal)
	{
		m_pic_minus = g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png", RGBColor(0,0,255));
		m_pic_plus = g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png", RGBColor(0,0,255));
	}
	else
	{
		m_pic_minus = g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255));
		m_pic_plus = g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255));
	}

	m_pic_background = g_gr->get_picture(PicMod_UI, "pics/scrollbar_background.png");
	m_pic_buttons = g_gr->get_picture(PicMod_UI, "pics/but3.png");

	m_time_nextact = 0;

	set_think(true);
}


/**
Change the number of steps of the scrollbar.
*/
void UIScrollbar::set_steps(int steps)
{
	if (steps < 1)
		steps = 1;

	if (m_pos >= (uint)steps)
		set_pos(steps - 1);

	m_steps = steps;

	update(0, 0, get_w(), get_h());
}


/**
Change the number of steps a pageup/down will scroll.
*/
void UIScrollbar::set_pagesize(int pagesize)
{
	if (pagesize < 1)
		pagesize = 1;

	m_pagesize = pagesize;
}


/**
Change the current scrolling position.
*/
void UIScrollbar::set_pos(int pos)
{
	if (pos < 0)
		pos = 0;
	if ((uint)pos >= m_steps)
		pos = m_steps - 1;

	if (m_pos == (uint)pos)
		return;

	m_pos = pos;
	moved.call(pos);

	update(0, 0, get_w(), get_h());
}


/**
Scrollbar::get_area_for_point
*/
UIScrollbar::Area UIScrollbar::get_area_for_point(int x, int y)
{
	int extent;
	int knob;

	// Out of panel
	if (x < 0 || x >= get_w() || y < 0 || y >= get_h())
		return None;

	// Normalize coordinates
	if (m_horizontal)
	{
		std::swap(x, y);
		extent = get_w();
	}
	else
	{
		extent = get_h();
	}

	// Determine the area
	knob = get_knob_pos();

	if (y < Size)
		return Minus;

	if (y < knob - Size/2)
		return MinusPage;

	if (y < knob + Size/2)
		return Knob;

	if (y < extent - Size)
		return PlusPage;

	return Plus;
}


/**
Return the center of the knob, in pixels, depending on the current position.
*/
int UIScrollbar::get_knob_pos()
{
	int extent;

	if (m_horizontal)
		extent = get_w();
	else
		extent = get_h();

	extent -= 3 * Size; // plus button, minus button, knob size

	return (3 * Size / 2) + (m_pos * extent) / m_steps;
}


/**
Change the position according to knob movement.
*/
void UIScrollbar::set_knob_pos(int pos)
{
	int extent;

	if (m_horizontal)
		extent = get_w();
	else
		extent = get_h();

	extent -= 3 * Size;
	pos -= 3 * Size / 2;

	pos = (pos * (int)m_steps) / extent;
	set_pos(pos);
}


/**
Perform the action for klicking on the given area.
*/
void UIScrollbar::action(Area area)
{
	int diff;
	int pos;

	switch(area) {
	case Minus: diff = -1; break;
	case MinusPage: diff = -m_pagesize; break;
	case Plus: diff = 1; break;
	case PlusPage: diff = m_pagesize; break;
	default: return;
	}

	pos = (int)m_pos + diff;
	set_pos(pos);
}


void UIScrollbar::draw_button(RenderTarget* dst, Area area, int x, int y, int w, int h)
{
	uint pic;

	dst->tile(x, y, w, h, m_pic_buttons, get_x(), get_y());

	// Draw the picture
	pic = 0;
	if (area == Minus)
		pic = m_pic_minus;
	else if (area == Plus)
		pic = m_pic_plus;

	if (pic) {
		int cpw, cph;

		g_gr->get_picture_size(pic, &cpw, &cph);

		dst->blit(x + (w - cpw)/2, y + (h - cph)/2, pic);
	}

	// Draw border
   RGBColor black(0,0,0);

	if (area != m_pressed)
	{
		// top edge
		dst->brighten_rect(x, y, w, 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dst->brighten_rect(x, y+2, 2, h-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// bottom edge
		dst->fill_rect(x+2, y+h-2, w-2, 1, black);
		dst->fill_rect(x+1, y+h-1, w-1, 1, black);
		// right edge
		dst->fill_rect(x+w-2, y+2, 1, h-2, black);
		dst->fill_rect(x+w-1, y+1, 1, h-1, black);
	}
	else
	{
		// bottom edge
		dst->brighten_rect(x, y+h-2, w, 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst->brighten_rect(x+w-2, y, 2, h-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst->fill_rect(x, y, w-1, 1, black);
		dst->fill_rect(x, y+1, w-2, 1, black);
		// left edge
		dst->fill_rect(x, y, 1, h-1, black);
		dst->fill_rect(x+1, y, 1, h-2, black);
	}
}


void UIScrollbar::draw_area(RenderTarget* dst, Area area, int x, int y, int w, int h)
{
	dst->tile(x, y, w, h, m_pic_background, get_x()+x, get_y()+y);

	if (area == m_pressed)
		dst->brighten_rect(x, y, w, h, BUTTON_EDGE_BRIGHT_FACTOR);
}


/**
Draw the scrollbar.
*/
void UIScrollbar::draw(RenderTarget* dst)
{
	uint knobpos = get_knob_pos();

	if (m_horizontal)
	{
		draw_button(dst, Minus, 0, 0, Size, get_h());
		draw_button(dst, Plus, get_w() - Size, 0, Size, get_h());
		draw_button(dst, Knob, knobpos - Size/2, 0, Size, get_h());

		draw_area(dst, MinusPage, Size, 0, knobpos - 3*Size/2, get_h());
		draw_area(dst, PlusPage, knobpos + Size/2, 0, get_w() - knobpos - 3*Size/2, get_h());
	}
	else
	{
		draw_button(dst, Minus, 0, 0, get_w(), Size);
		draw_button(dst, Plus, 0, get_h() - Size, get_w(), Size);
		draw_button(dst, Knob, 0, knobpos - Size/2, get_w(), Size);

		draw_area(dst, MinusPage, 0, Size, get_w(), knobpos - 3*Size/2);
		draw_area(dst, PlusPage, 0, knobpos + Size/2, get_w(), get_h() - knobpos - 3*Size/2);
	}
}


/**
Check for possible auto-repeat scrolling.
*/
void UIScrollbar::think()
{
	UIPanel::think();

	if (m_pressed == None || m_pressed == Knob)
		return;

	if (Sys_GetTime() - m_time_nextact < 0)
		return;

	action(m_pressed);

	// Schedule next tick
	m_time_nextact += SCROLLBAR_AUTOREPEAT_TICK;
	if (Sys_GetTime() - m_time_nextact >= 0)
		m_time_nextact = Sys_GetTime();
}


bool UIScrollbar::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0) // only react on left button
		return false;

	if (down)
	{
		m_pressed = get_area_for_point(x, y);
		if (m_pressed != None) {
			grab_mouse(true);

			if (m_pressed != Knob)
			{
				action(m_pressed);
				m_time_nextact = Sys_GetTime() + SCROLLBAR_AUTOREPEAT_DELAY;
			}
			else
			{
				int grabpos = m_horizontal ? x : y;

				m_knob_grabdelta = grabpos - get_knob_pos();
			}
		}
	}
	else
	{
		if (m_pressed != None) {
         grab_mouse(false);
			m_pressed = None;
      }
   }

	update(0, 0, get_w(), get_h());

	return true;
}


/**
Move the knob while pressed.
*/
void UIScrollbar::handle_mousemove(int mx, int my, int xdiff, int ydiff, uint btns)
{
	if (m_pressed == Knob)
	{
		int pos = m_horizontal ? mx : my;

		set_knob_pos(pos - m_knob_grabdelta);
	}
}
