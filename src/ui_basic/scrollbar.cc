/*
 * Copyright (C) 2002, 2006-2011, 2013 by the Widelands Development Team
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

#include "ui_basic/scrollbar.h"

#include <algorithm>

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "ui_basic/mouse_constants.h"
#include "wlapplication.h"

namespace UI {

/*
==============================================================================

Scrollbar

==============================================================================
*/


/**
Initialize the scrollbar using default values.
*/
Scrollbar::Scrollbar
	(Panel * const parent,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 bool const horiz)
	:
	Panel           (parent, x, y, w, h),
	m_horizontal    (horiz),
	m_force_draw    (false),
	m_pos           (0),
	m_singlestepsize(1),
	m_pagesize      (5),
	m_steps         (100),
	m_pressed       (None),
	m_time_nextact  (0),
	m_knob_grabdelta(0),
	m_pic_minus
		(g_gr->images().get(horiz ? "pics/scrollbar_left.png"  : "pics/scrollbar_up.png")),
	m_pic_plus
		(g_gr->images().get(horiz ? "pics/scrollbar_right.png" : "pics/scrollbar_down.png")),
	m_pic_background
		(g_gr->images().get("pics/scrollbar_background.png")),
	m_pic_buttons   (g_gr->images().get("pics/but3.png"))
{
	set_think(true);
}


/**
 * Change the number of steps of the scrollbar.
*/
void Scrollbar::set_steps(int32_t steps)
{
	if (steps < 1)
		steps = 1;

	if (m_pos >= static_cast<uint32_t>(steps))
		set_scrollpos(steps - 1);

	m_steps = steps;
	update();
}


/**
 * \return the number of steps
 */
uint32_t Scrollbar::get_steps() const
{
	return m_steps;
}


/**
 * Change the number of steps one click on one of the arrow buttons will scroll.
 */
void Scrollbar::set_singlestepsize(uint32_t singlestepsize)
{
	if (singlestepsize < 1)
		singlestepsize = 1;

	m_singlestepsize = singlestepsize;
}


/**
 * Change the number of steps a pageup/down will scroll.
*/
void Scrollbar::set_pagesize(int32_t const pagesize)
{
	m_pagesize = pagesize < 1 ? 1 : pagesize;
}


/**
 * Change the current scrolling position.
 *
 * \param pos the new position, which will be snapped to the range [0,
 *            get_steps() - 1]
 */
void Scrollbar::set_scrollpos(int32_t pos)
{
	if (pos < 0)
		pos = 0;
	if (static_cast<uint32_t>(pos) >= m_steps)
		pos = m_steps - 1;

	if (m_pos == static_cast<uint32_t>(pos))
		return;

	m_pos = pos;
	moved(pos);

	update();
}


Scrollbar::Area Scrollbar::get_area_for_point(int32_t x, int32_t y)
{
	int32_t extent;

	// Out of panel
	if
		(x < 0 || x >= static_cast<int32_t>(get_w()) ||
		 y < 0 || y >= static_cast<int32_t>(get_h()))
		return None;

	// Normalize coordinates
	if (m_horizontal) {
		std::swap(x, y);
		extent = get_w();
	} else
		extent = get_h();

	// Determine the area
	int32_t knob = get_knob_pos();
	int32_t knobsize = get_knob_size();

	if (y < Size)
		return Minus;

	if (y < knob - knobsize / 2)
		return MinusPage;

	if (y < knob + knobsize / 2)
		return Knob;

	if (y < extent - Size)
		return PlusPage;

	return Plus;
}


/**
 * Return the center of the knob, in pixels, depending on the current position.
*/
uint32_t Scrollbar::get_knob_pos() {
	assert(0 != m_steps);
	uint32_t result = Size + get_knob_size() / 2;
	if (uint32_t const d = m_steps - 1)
		result += m_pos * ((m_horizontal ? get_w() : get_h()) - 2 * result) / d;
	return result;
}


/**
 * Change the position according to knob movement.
*/
void Scrollbar::set_knob_pos(int32_t pos)
{
	uint32_t knobsize = get_knob_size();
	int32_t extent = m_horizontal ? get_w() : get_h();

	extent -= 2 * Size + knobsize;
	pos -= Size + knobsize / 2;

	pos = (pos * static_cast<int32_t>(m_steps)) / extent;
	set_scrollpos(pos);
}


/**
 * Returns the desired size of the knob. The knob scales with the page size.
 * The returned knob size is always a multiple of 2 to avoid problems in
 * computations elsewhere.
 */
uint32_t Scrollbar::get_knob_size()
{
	uint32_t extent = m_horizontal ? get_w() : get_h();

	if (extent <= 3 * Size)
		return Size;

	uint32_t maxhalfsize = extent / 2 - Size;
	uint32_t halfsize = (maxhalfsize * get_pagesize()) /
		(m_steps + get_pagesize());
	uint32_t size = 2 * halfsize;
	if (size < Size)
		size = Size;
	return size;
}


/// Perform the action for clicking on the given area.
void Scrollbar::action(Area const area)
{
	int32_t diff;
	int32_t pos;

	switch (area) {
	case Minus: diff = -m_singlestepsize; break;
	case MinusPage: diff = -m_pagesize; break;
	case Plus: diff = m_singlestepsize; break;
	case PlusPage: diff = m_pagesize; break;
	default:
		return;
	}

	pos = static_cast<int32_t>(m_pos) + diff;
	set_scrollpos(pos);
}


void Scrollbar::draw_button(RenderTarget & dst, const Area area, const Rect r) {

	dst.tile(r, m_pic_buttons, Point(get_x(), get_y()));

	// Draw the picture
	const Image* pic = nullptr;
	if (area == Minus)
		pic = m_pic_minus;
	else if (area == Plus)
		pic = m_pic_plus;

	if (pic) {
		uint16_t cpw = pic->width();
		uint16_t cph = pic->height();

		dst.blit(r + Point((r.w - cpw) / 2, (r.h - cph) / 2), pic);
	}

	// Draw border
	RGBColor black(0, 0, 0);

	if (area != m_pressed) {
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
	} else {
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

	if (area == m_pressed)
		dst.brighten_rect(r, BUTTON_EDGE_BRIGHT_FACTOR);
}


/**
 * Draw the scrollbar.
*/
void Scrollbar::draw(RenderTarget & dst)
{
	uint32_t knobpos = get_knob_pos();
	uint32_t knobsize = get_knob_size();

	if (m_steps == 1 && !m_force_draw)
		return; // don't draw a not doing scrollbar

	if (m_horizontal) {
		if ((2 * Size + knobsize) > static_cast<uint32_t>(get_w())) {
			// Our owner obviously allocated too little space - draw something
			// stupid
			draw_button(dst, Minus, Rect(Point(0, 0), get_w(), get_h()));
			return;
		}

		draw_button(dst, Minus, Rect(Point(0, 0), Size, get_h()));
		draw_button(dst, Plus, Rect(Point(get_w() - Size, 0), Size, get_h()));
		draw_button
			(dst, Knob, Rect(Point(knobpos - knobsize / 2, 0), knobsize, get_h()));

		assert(Size + knobsize / 2 <= knobpos);
		draw_area
			(dst,
			 MinusPage,
			 Rect(Point(Size, 0), knobpos - Size - knobsize / 2, get_h()));
		assert(knobpos + knobsize / 2 + Size <= static_cast<uint32_t>(get_w()));
		draw_area
			(dst,
			 PlusPage,
			 Rect
			 	(Point(knobpos + knobsize / 2, 0),
			 	 get_w() - knobpos - knobsize / 2 - Size, get_h()));
	} else {
		if ((2 * Size + knobsize) > static_cast<uint32_t>(get_h())) {
			// Our owner obviously allocated too little space - draw something
			// stupid
			draw_button(dst, Minus, Rect(Point(0, 0), get_w(), get_h()));
			return;
		}

		draw_button(dst, Minus, Rect(Point(0, 0), get_w(), Size));
		draw_button(dst, Plus, Rect(Point(0, get_h() - Size), get_w(), Size));
		draw_button
			(dst, Knob, Rect(Point(0, knobpos - knobsize / 2), get_w(), knobsize));

		assert(Size + knobsize / 2 <= knobpos);
		draw_area
			(dst,
			 MinusPage,
			 Rect(Point(0, Size), get_w(), knobpos - Size - knobsize / 2));
		assert
			(knobpos + knobsize / 2 + Size <=
			 static_cast<uint32_t>(get_h()));
		draw_area
			(dst,
			 PlusPage,
			 Rect
			 	(Point(0, knobpos + knobsize / 2),
			 	 get_w(), get_h() - knobpos - knobsize / 2 - Size));
	}
}


/**
 * Check for possible auto-repeat scrolling.
*/
void Scrollbar::think()
{
	Panel::think();

	if (m_pressed == None || m_pressed == Knob)
		return;

	int32_t const time = WLApplication::get()->get_time();
	if (time < m_time_nextact)
		return;

	action(m_pressed);

	// Schedule next tick
	m_time_nextact += MOUSE_BUTTON_AUTOREPEAT_TICK;
	if (m_time_nextact < time)
		m_time_nextact = time;
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
					WLApplication::get()->get_time() +
					MOUSE_BUTTON_AUTOREPEAT_DELAY;
			} else
				m_knob_grabdelta = (m_horizontal ? x : y) - get_knob_pos();
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
	default:
		break;
	}
	update();
	return result;
}
bool Scrollbar::handle_mouserelease(const Uint8 btn, int32_t, int32_t) {
	bool result = false;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		if (m_pressed != None) {
			grab_mouse(false);
			m_pressed = None;
		}
		result = true;
		break;

	case SDL_BUTTON_WHEELUP:
		result = true;
		break;

	case SDL_BUTTON_WHEELDOWN:
		result = true;
		break;
	default:
		break;
	}
	update();
	return result;
}


/**
 * Move the knob while pressed.
 */
bool Scrollbar::handle_mousemove
	(Uint8, int32_t const mx, int32_t const my, int32_t, int32_t)
{
	if (m_pressed == Knob)
		set_knob_pos((m_horizontal ? mx : my) - m_knob_grabdelta);
	return true;
}

}
