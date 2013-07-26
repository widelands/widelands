/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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
 */

#include "ui_basic/slider.h"

#include <cmath>

#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "ui_basic/mouse_constants.h"

namespace UI {

/**
 * \brief Initialize a Slider.
 *
 * \param parent The parent panel.
 * \param x The X position.
 * \param y The Y position.
 * \param w The widget width.
 * \param h The widget height.
 * \param min_value The minimal value accepted by the slider.
 * \param max_value The maximal value accepted by the slider.
 * \param value The initial value.
 * \param background_picture_id Identifier of the picture used for the
 * cursor background.
 * \param tooltip_text An optional help message.
 * \param cursor_size The size of the cursor, moving direction.
 * \param enabled The widget accessibility.
 * \param x_gap size between the cursor and the vertical border.
 * \param y_gap size between the cursor and the horizontal border.
 * \param bar_size Length of the cursor move.
 */
Slider::Slider
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const int32_t min_value, const int32_t max_value, const int32_t value,
	 const Image* background_picture_id,
	 const std::string & tooltip_text,
	 const uint32_t cursor_size,
	 const bool enabled,
	 const int32_t x_gap, const int32_t y_gap, const int32_t bar_size)
	:
	Panel            (parent, x, y, w, h, tooltip_text),
	m_min_value      (min_value),
	m_max_value      (max_value),
	m_value          (value),
	m_highlighted    (false),
	m_pressed        (false),
	m_enabled        (enabled),
	m_pic_background (background_picture_id),
	m_x_gap          (x_gap),
	m_y_gap          (y_gap),
	m_bar_size       (bar_size),
	m_cursor_size (cursor_size)
{
	set_think(false);
	calc_cursor_pos();
}

void Slider::set_value(int32_t new_value)
{
	new_value = std::max(m_min_value, std::min(new_value, m_max_value));

	if (new_value != m_value) {
		m_value = new_value;
		calc_cursor_pos();
		send_value_changed();
		update();
	}
}

void Slider::calc_cursor_pos() {
	if (m_max_value == m_min_value) {
		m_cursor_pos = m_min_value;
	} else if (m_value == m_min_value) {
		m_cursor_pos = 0;
	} else if (m_value == m_max_value) {
		m_cursor_pos = get_bar_size();
	} else {
		m_cursor_pos =
			(m_value - m_min_value) * get_bar_size()
			/
			(m_max_value - m_min_value);
	}
}

void Slider::layout() {
	Panel::layout();
	calc_cursor_pos();
}

/**
 * \brief Sets max value.
 *
 * \param new_max The new max value.
 */
void Slider::set_max_value(int32_t new_max) {
	assert(m_min_value <= new_max);
	if (m_max_value != new_max)
		update();
	m_max_value = new_max;
	set_value(m_value);
}

/**
 * \brief Sets min value.
 *
 * \param new_min The new min value.
 */
void Slider::set_min_value(int32_t new_min) {
	assert(m_max_value >= new_min);
	if (m_min_value != new_min)
		update();
	m_min_value = new_min;
	set_value(m_value);
}

/**
 * \brief Draw the cursor.
 *
 * \param dst Graphic target.
 * \param x The cursor x position.
 * \param y The cursor y position.
 * \param w The cursor width.
 * \param h The cursor height.
 */
void Slider::draw_cursor
	(RenderTarget & dst,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h)
{

	RGBColor black(0, 0, 0);

	dst.tile //  background
		(Rect(Point(x, y), w, h), m_pic_background, Point(get_x(), get_y()));

	if (m_highlighted)
		dst.brighten_rect(Rect(Point(x, y), w, h), MOUSE_OVER_BRIGHT_FACTOR);

	if (m_pressed) { //  draw border
		dst.brighten_rect //  bottom edge
			(Rect(Point(x, y + h - 2), w,     2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect //  right edge
			(Rect(Point(x + w - 2, y), 2, h - 2), BUTTON_EDGE_BRIGHT_FACTOR);

		//  top edge
		dst.fill_rect(Rect(Point(x, y),     w - 1, 1), black);
		dst.fill_rect(Rect(Point(x, y + 1), w - 2, 1), black);

		//  left edge
		dst.fill_rect(Rect(Point(x,     y), 1, h - 1), black);
		dst.fill_rect(Rect(Point(x + 1, y), 1, h - 2), black);
	} else {
		dst.brighten_rect //  top edge
			(Rect(Point(x, y),     w,     2), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect //  left edge
			(Rect(Point(x, y + 2), 2, h - 2), BUTTON_EDGE_BRIGHT_FACTOR);

		//  bottom edge
		dst.fill_rect(Rect(Point(x + 2, y + h - 2), w - 2, 1), black);
		dst.fill_rect(Rect(Point(x + 1, y + h - 1), w - 1, 1), black);

		//  right edge
		dst.fill_rect(Rect(Point(x + w - 2, y + 2), 1, h - 2), black);
		dst.fill_rect(Rect(Point(x + w - 1, y + 1), 1, h - 1), black);
	}

}


/**
 * \brief Send an event when the slider is moved by used.
 */
void Slider::send_value_changed()
{
	changed();
	changedto(m_value);
}


/**
 * \brief Enable/Disable the slider.
 *
 * Disabled slider can't be clicked. Sliders are enabled by default.
 */
void Slider::set_enabled(const bool enabled)
{
	//  TODO: disabled should look different...
	if (m_enabled == enabled)
		return;

	m_enabled = enabled;
	if (not enabled) {
		m_pressed = false;
		m_highlighted = false;
		grab_mouse(false);
	}
	update();
}


/**
 * Set whether the sliding button should be highlighted,
 * and trigger a draw update when necessary.
 */
void Slider::set_highlighted(bool highlighted)
{
	if (m_highlighted == highlighted)
		return;

	m_highlighted = highlighted;
	update();
}


/**
 * \brief Mouse entered and exited events.
 *
 * Change the cursor style.
 */
void Slider::handle_mousein(bool inside)
{
	if (not inside)
		set_highlighted(false);
}


/**
 * \brief Mouse released event.
 *
 * Update pressed status.
 */
bool Slider::handle_mouserelease(const Uint8 btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;
	if (m_pressed) {
		grab_mouse(false);
		m_pressed = false;

		//  cursor position: align to integer value
		calc_cursor_pos();

		update();
	}
	return true;
}


/**
 * \brief Update the value when the cursor is moved.
 *
 * \param pointer The relative position of the mouse pointer.
 * \param x The x position of the mouse pointer.
 * \param y The y position of the mouse pointer.
 */
void Slider::cursor_moved(int32_t pointer, int32_t x, int32_t y) {
	int32_t o_cursor_pos = m_cursor_pos;

	if (not m_enabled)
		return;

	set_highlighted
		(pointer >= m_cursor_pos and  pointer <= m_cursor_pos + m_cursor_size
		 and y >= 0 and y < get_h() and x >= 0 and x < get_w());

	if (not m_pressed)
		return;

	m_cursor_pos = pointer - m_relative_move;
	if (m_cursor_pos < 0)
		m_cursor_pos = 0;
	if (m_cursor_pos > get_bar_size())
		m_cursor_pos = get_bar_size();

	//  absolute value
	int32_t new_value =
		static_cast<int32_t>
			(rint
			 	(static_cast<double>((m_max_value - m_min_value) * m_cursor_pos)
			 	 /
			 	 get_bar_size()));

	//  relative value in bounds
	new_value += m_min_value;
	if (new_value < m_min_value)
		new_value = m_min_value;
	if (new_value > m_max_value)
		new_value = m_max_value;

	//  updating
	if (new_value != m_value) {
		m_value = new_value;
		send_value_changed();
	}

	if (o_cursor_pos != m_cursor_pos)
		update();
}


/**
 * \brief Event when the cursor is presed.
 *
 * \param pointer The relative position of the mouse pointer.
 */
void Slider::cursor_pressed(int32_t pointer) {
	if (not m_enabled)
		return;

	grab_mouse(true);
	m_pressed = true;
	m_highlighted = true;
	m_relative_move = pointer - m_cursor_pos;

	play_click();

	update();
}


/**
 * \brief Event when the bar is pressed.
 *
 * \param pointer The relative position of the mouse pointer.
 * \param ofs The cursor offset.
 */
void Slider::bar_pressed(int32_t pointer, int32_t ofs) {
	if (not m_enabled)
		return;

	grab_mouse(true);
	m_pressed = true;

	m_cursor_pos = pointer - ofs;

	//  absolute value
	if (get_bar_size() == 0) {
		m_value = 0;
	} else {
		m_value =
			static_cast<int32_t>
				(rint
					(static_cast<double>((m_max_value - m_min_value) * m_cursor_pos)
					 /
					 get_bar_size()));
	}

	//  relative value in bounds
	if (m_value < m_min_value)
		m_value = m_min_value;
	if (m_value > m_max_value)
		m_value = m_max_value;

	play_click();
	send_value_changed();

	m_relative_move = ofs;
	update();
}

void VerticalSlider::layout() {
	m_x_gap = get_w() / 2 - 2;
	m_bar_size = get_h() - m_cursor_size;
	Slider::layout();
}


////////////////////////////////////////////////////////////////////////////////
//                               HORIZONTAL                                   //
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Redraw the slide bar. The horizontal bar is painted.
 *
 * \param dst The graphic destination.
 */
void HorizontalSlider::draw(RenderTarget & dst)
{
	RGBAColor black(0, 0, 0, 255);

	if (get_bar_size() > 0) {
		dst.brighten_rect //  bottom edge
			(Rect(Point(get_x_gap(), get_h() / 2), get_bar_size(), 2),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect //  right edge
			(Rect(Point(get_x_gap() + get_bar_size() - 2, get_y_gap()), 2, 2),
			 BUTTON_EDGE_BRIGHT_FACTOR);

		//  top edge
		dst.fill_rect
			(Rect(Point(get_x_gap(), get_y_gap()),     get_bar_size() - 1, 1), black);
		dst.fill_rect
			(Rect(Point(get_x_gap(), get_y_gap() + 1), get_bar_size() - 2, 1), black);
	}

	//  left edge
	dst.fill_rect(Rect(Point(get_x_gap(),     get_y_gap()), 1, 4), black);
	dst.fill_rect(Rect(Point(get_x_gap() + 1, get_y_gap()), 1, 3), black);

	draw_cursor(dst, m_cursor_pos, 0, m_cursor_size, get_h());
}


/**
 * \brief Mouse move event
 *
 * \param btn The new stat of the mouse buttons (unused)
 * \param x The new X position of mouse pointer.
 * \param y The new Y position of mouse pointer.
 */
bool HorizontalSlider::handle_mousemove
	(Uint8, int32_t const x, int32_t const y, int32_t, int32_t)
{
	cursor_moved(x, x, y);
	return true;
}


/**
 * \brief Mouse pressed event.
 *
 * \param btn The new stat of the mouse buttons.
 * \param x The X position of mouse pointer.
 * \param y The Y position of mouse pointer.
 */
bool HorizontalSlider::handle_mousepress
		(const Uint8 btn, int32_t x, int32_t y)
{
	if (btn != SDL_BUTTON_LEFT)
		return false;


	if (x >= m_cursor_pos and x <= m_cursor_pos + m_cursor_size) {
		//  click on cursor
		cursor_pressed(x);
		return true;
	} else if
		(y >= get_y_gap() - 2 and
		 y <= static_cast<int32_t>(get_h()) - get_y_gap() + 2 and
		 x >= get_x_gap() and
		 x <  static_cast<int32_t>(get_w()) - get_x_gap())
	{ //  click on bar
		bar_pressed(x, get_x_gap());
		return true;
	} else return false;
}

void HorizontalSlider::layout() {
	m_y_gap = get_h() / 2 - 2;
	m_bar_size = get_w() - m_cursor_size;
	Slider::layout();
}

////////////////////////////////////////////////////////////////////////////////
//                               VERTICAL                                     //
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Redraw the slide bar. The vertical bar is painted.
 *
 * \param dst The graphic destination.
 */
void VerticalSlider::draw(RenderTarget & dst)
{
	RGBAColor black(0, 0, 0, 255);

	dst.brighten_rect //  right edge
		(Rect(Point(get_w() / 2, get_y_gap()), 2, get_bar_size()),
		 BUTTON_EDGE_BRIGHT_FACTOR);
	dst.brighten_rect //  bottom edge
		(Rect(Point(get_x_gap(), get_y_gap() + get_bar_size() - 2), 2, 2),
		 BUTTON_EDGE_BRIGHT_FACTOR);

	//  left edge
	dst.fill_rect
		(Rect(Point(get_x_gap(),     get_y_gap()), 1, get_bar_size() - 1), black);
	dst.fill_rect
		(Rect(Point(get_x_gap() + 1, get_y_gap()), 1, get_bar_size() - 2), black);

	//  top edge
	dst.fill_rect(Rect(Point(get_x_gap(), get_y_gap()),     4, 1), black);
	dst.fill_rect(Rect(Point(get_x_gap(), get_y_gap() + 1), 3, 1), black);

	draw_cursor(dst, 0, m_cursor_pos, get_w(), m_cursor_size);
}


/**
 * \brief Mouse move event
 *
 * \param btn The new stat of the mouse buttons (unused)
 * \param x The new X position of mouse pointer.
 * \param y The new Y position of mouse pointer.
 */
bool VerticalSlider::handle_mousemove
	(Uint8, int32_t const x, int32_t const y, int32_t, int32_t)
{
	cursor_moved(y, x, y);
	return true;
}


/**
 * \brief Mouse pressed event.
 *
 * \param btn The new stat of the mouse buttons (unused)
 * \param x The X position of mouse pointer.
 * \param y The Y position of mouse pointer.
 */
bool VerticalSlider::handle_mousepress(const Uint8 btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	if (y >= m_cursor_pos and y <= m_cursor_pos + m_cursor_size) {
		//  click on cursor
		cursor_pressed(y);
		return true;
	} else if
		(y >= get_y_gap() and
		 y <= static_cast<int32_t>(get_h()) - get_y_gap() and
		 x >= get_x_gap() - 2 and
		 x < static_cast<int32_t>(get_w()) - get_x_gap() + 2)
	{ //  click on bar
		bar_pressed(y, get_y_gap());
		return true;
	} else return false;
}

////////////////////////////////////////////////////////////////////////////////
//                               DISCRETE                                     //
////////////////////////////////////////////////////////////////////////////////

DiscreteSlider::DiscreteSlider
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const std::vector<std::string> labels_in,
	 uint32_t m_value,
	 const Image* background_picture_id,
	 const std::string & tooltip_text,
	 const uint32_t cursor_size,
	 const bool enabled)
	:
	Panel (parent, x, y, w, h, tooltip_text),
	slider
		(this,
		 // here, we take into account the h_gap introduced by HorizontalSlider
		 w / (2 * labels_in.size()) - cursor_size / 2, 0,
		 w - (w / labels_in.size()) + cursor_size,
		 h - UI::Font::ui_small()->lineskip() - 2,
		 0, labels_in.size() - 1, m_value,
		 background_picture_id,
		 tooltip_text,
		 cursor_size,
		 enabled),
	labels(labels_in)
{
	slider.changed.connect(changed);
	slider.changedto.connect(changedto);
}

/**
 * \brief Redraw the slide bar. The discrete horizontal bar is painted.
 *
 * \param dst The graphic destination.
 */
void DiscreteSlider::draw(RenderTarget & dst)
{
	Panel::draw(dst);

	UI::TextStyle ts = UI::TextStyle::ui_small();

	uint32_t gap_1 = get_w() / (2 * labels.size());
	uint32_t gap_n = get_w() / labels.size();

	for (uint32_t i = 0; i < labels.size(); i++) {
		UI::g_fh->draw_text
			(dst, ts,
			 Point(gap_1 + i * gap_n, get_h() + 2),
			 labels[i], UI::Align_BottomCenter);
	}

}

void DiscreteSlider::set_labels(const std::vector<std::string> labels_in) {
	labels = labels_in;
	slider.set_max_value(labels_in.size() - 1);
	layout();
}

void DiscreteSlider::layout() {
	uint32_t w = get_w();
	uint32_t h = get_h();
	assert(labels.size());
	slider.set_pos(Point(w / (2 * labels.size()) - slider.m_cursor_size / 2, 0));
	slider.set_size
		(w - (w / labels.size()) + slider.m_cursor_size,
		 h - UI::Font::ui_small()->lineskip() - 2);
	Panel::layout();
}

}
