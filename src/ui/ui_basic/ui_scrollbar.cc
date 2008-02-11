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

#include "ui_scrollbar.h"

#include "graphic.h"
#include "rendertarget.h"
#include "wlapplication.h"

#include <algorithm>

namespace UI {
#define SCROLLBAR_AUTOREPEAT_DELAY 200
#define SCROLLBAR_AUTOREPEAT_TICK   50

/*
==============================================================================

Scrollbar

==============================================================================
*/


/**
Initialize the scrollbar using default values.
*/
Scrollbar::Scrollbar(Panel *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, bool horiz)
	: Panel(parent, x, y, w, h)
{
	m_horizontal = horiz;

	m_pos = 0;
	m_pagesize = 5;
	m_steps = 100;

	m_pressed = None;

	if (m_horizontal)
	{
		m_pic_minus = g_gr->get_picture(PicMod_UI,  "pics/scrollbar_left.png");
		m_pic_plus = g_gr->get_picture(PicMod_UI,  "pics/scrollbar_right.png");
	} else {
		m_pic_minus = g_gr->get_picture(PicMod_UI,  "pics/scrollbar_up.png");
		m_pic_plus = g_gr->get_picture(PicMod_UI,  "pics/scrollbar_down.png");
	}

	m_pic_background = g_gr->get_picture(PicMod_UI,  "pics/scrollbar_background.png");
	m_pic_buttons = g_gr->get_picture(PicMod_UI,  "pics/but3.png");

	m_time_nextact = 0;
	m_force_draw = false;

	set_think(true);
}


/**
Change the number of steps of the scrollbar.
*/
void Scrollbar::set_steps(int32_t steps)
{
	if (steps < 1)
		steps = 1;

	if (m_pos >= static_cast<uint32_t>(steps)) set_pos(steps - 1);

	m_steps = steps;

	update(0, 0, get_w(), get_h());
}


/**
Change the number of steps a pageup/down will scroll.
*/
void Scrollbar::set_pagesize(int32_t pagesize)
{
	if (pagesize < 1)
		pagesize = 1;

	m_pagesize = pagesize;
}


/**
Change the current scrolling position.
*/
void Scrollbar::set_pos(int32_t pos)
{
	if (pos < 0)
		pos = 0;
	if (static_cast<uint32_t>(pos) >= m_steps) pos = m_steps - 1;

	if (m_pos == static_cast<uint32_t>(pos)) return;

	m_pos = pos;
	moved.call(pos);

	update(0, 0, get_w(), get_h());
}


/**
Scrollbar::get_area_for_point
*/
Scrollbar::Area Scrollbar::get_area_for_point(int32_t x, int32_t y)
{
	int32_t extent;
	int32_t knob;

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
int32_t Scrollbar::get_knob_pos()
{
	int32_t extent;

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
void Scrollbar::set_knob_pos(int32_t pos)
{
	int32_t extent;

	if (m_horizontal)
		extent = get_w();
	else
		extent = get_h();

	extent -= 3 * Size;
	pos -= 3 * Size / 2;

	pos = (pos * static_cast<int32_t>(m_steps)) / extent;
	set_pos(pos);
}


/**
Perform the action for klicking on the given area.
*/
void Scrollbar::action(Area area)
{
	int32_t diff;
	int32_t pos;

	switch (area) {
	case Minus: diff = -1; break;
	case MinusPage: diff = -m_pagesize; break;
	case Plus: diff = 1; break;
	case PlusPage: diff = m_pagesize; break;
	default:
		return;
	}

	pos = static_cast<int32_t>(m_pos) + diff;
	set_pos(pos);
}


void Scrollbar::draw_button(RenderTarget & dst, const Area area, const Rect r) {
	uint32_t pic;

	dst.tile(r, m_pic_buttons, Point(get_x(), get_y()));

	// Draw the picture
	pic = 0;
	if (area == Minus)
		pic = m_pic_minus;
	else if (area == Plus)
		pic = m_pic_plus;

	if (pic) {
		uint32_t cpw, cph;
		g_gr->get_picture_size(pic, cpw, cph);

		dst.blit(r + Point((r.w - cpw) / 2, (r.h - cph) / 2), pic);
	}

	// Draw border
	RGBColor black(0, 0, 0);

	if (area != m_pressed)
	{
		// top edge
		dst.brighten_rect(Rect(r, r.w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dst.brighten_rect
			(Rect(r + Point(0, 2), 2, r.h - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// bottom edge
		dst.fill_rect(Rect(r + Point(2, r.h - 2), r.w - 2, 1), black);
		dst.fill_rect(Rect(r + Point(1, r.h - 1), r.w - 1, 1), black);
		// right edge
		dst.fill_rect(Rect(r + Point(r.w - 2, 2), 1, r.h - 2), black);
		dst.fill_rect(Rect(r + Point(r.w - 1, 1), 1, r.h - 1), black);
	}
	else
	{
		// bottom edge
		dst.brighten_rect
			(Rect(r + Point(0, r.h - 2), r.w, 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect
			(Rect(r + Point(r.w - 2, 0), 2, r.h - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Rect(r, r.w - 1, 1), black);
		dst.fill_rect(Rect(r + Point(0, 1), r.w - 2, 1), black);
		// left edge
		dst.fill_rect(Rect(r, 1, r.h - 1), black);
		dst.fill_rect(Rect(r + Point(1, 0), 1, r.h - 2), black);
	}
}


void Scrollbar::draw_area(RenderTarget & dst, const Area area, const Rect r) {
	dst.tile(r, m_pic_background, Point(get_x(), get_y()) + r);

	if (area == m_pressed) dst.brighten_rect(r, BUTTON_EDGE_BRIGHT_FACTOR);
}


/**
Draw the scrollbar.
*/
void Scrollbar::draw(RenderTarget* dst)
{
	uint32_t knobpos = get_knob_pos();

	if (m_steps == 1 && !m_force_draw)
		return; // don't draw a not doing scrollbar

	if (m_horizontal)
	{
		draw_button(*dst, Minus, Rect(Point(0, 0), Size, get_h()));
		draw_button(*dst, Plus, Rect(Point(get_w() - Size, 0), Size, get_h()));
		draw_button
			(*dst, Knob, Rect(Point(knobpos - Size / 2, 0), Size, get_h()));

		assert(3 * Size / 2 <= knobpos);
		draw_area
			(*dst,
			 MinusPage,
			 Rect(Point(Size, 0), knobpos - 3 * Size / 2, get_h()));
		assert(0 <= get_w());
		assert(knobpos + 3 * Size / 2 <= static_cast<uint32_t>(get_w()));
		draw_area
			(*dst,
			 PlusPage,
			 Rect
			 (Point(knobpos + Size / 2, 0),
			  get_w() - knobpos - 3 * Size / 2, get_h()));
	}
	else
	{
		draw_button(*dst, Minus, Rect(Point(0, 0), get_w(), Size));
		draw_button(*dst, Plus, Rect(Point(0, get_h() - Size), get_w(), Size));
		draw_button
			(*dst, Knob, Rect(Point(0, knobpos - Size / 2), get_w(), Size));

		assert(3 * Size / 2 <= knobpos);
		draw_area
			(*dst,
			 MinusPage,
			 Rect(Point(0, Size), get_w(), knobpos - 3 * Size / 2));
		assert(0 <= get_h());
		assert(knobpos + 3 * Size / 2 <= static_cast<uint32_t>(get_h()));
		draw_area
			(*dst,
			 PlusPage,
			 Rect
			 (Point(0, knobpos + Size / 2),
			  get_w(), get_h() - knobpos - 3 * Size / 2));
	}
}


/**
Check for possible auto-repeat scrolling.
*/
void Scrollbar::think()
{
	Panel::think();

	if (m_pressed == None || m_pressed == Knob)
		return;

	if (WLApplication::get()->get_time() - m_time_nextact < 0)
		return;

	action(m_pressed);

	// Schedule next tick
	m_time_nextact += SCROLLBAR_AUTOREPEAT_TICK;
	if (WLApplication::get()->get_time() - m_time_nextact >= 0)
		m_time_nextact = WLApplication::get()->get_time();
}


bool Scrollbar::handle_mousepress(const Uint8 btn, int32_t x, int32_t y) {
	bool result = false;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		m_pressed = get_area_for_point(x, y);
		if (m_pressed != None) {
			grab_mouse(true);
			if (m_pressed != Knob) {
				action(m_pressed);
				m_time_nextact =
					WLApplication::get()->get_time() + SCROLLBAR_AUTOREPEAT_DELAY;
			}
			else m_knob_grabdelta = (m_horizontal ? x : y) - get_knob_pos();
		}
		result = true;
		break;

	case SDL_BUTTON_WHEELUP:
		action(Minus);
		result = true;
		break;

	case SDL_BUTTON_WHEELDOWN:
		action(Plus);
		result = true;
		break;
	}

	update(0, 0, get_w(), get_h());
	return result;
}
bool Scrollbar::handle_mouserelease(const Uint8 btn, int32_t, int32_t) {
	bool result = false;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		if (m_pressed != None) {grab_mouse(false); m_pressed = None;}
		result = true;
		break;

	case SDL_BUTTON_WHEELUP:
		result = true;
		break;

	case SDL_BUTTON_WHEELDOWN:
		result = true;
		break;
	}

	update(0, 0, get_w(), get_h());
	return result;
}


/**
Move the knob while pressed.
*/
bool Scrollbar::handle_mousemove(const Uint8, int32_t mx, int32_t my, int32_t, int32_t) {
	if (m_pressed == Knob)
	{
		int32_t pos = m_horizontal ? mx : my;

		set_knob_pos(pos - m_knob_grabdelta);
	}
	return true;
}
};
