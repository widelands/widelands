/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#include "button.h"

#include "mouse_constants.h"

#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "wlapplication.h"
#include "log.h"


namespace UI {

Button::Button //  for textual buttons
	(Panel * const parent,
	 const std::string & name,
	 int32_t const x, int32_t const y, uint32_t const w, uint32_t const h,
	 PictureID background_picture_id,
	 std::string const & title_text,
	 std::string const & tooltip_text,
	 bool const _enabled, bool const flat,
	 std::string const & fontname,
	 uint32_t const      fontsize)
	:
	NamedPanel           (parent, name, x, y, w, h, tooltip_text),
	m_highlighted   (false),
	m_pressed       (false),
	m_permpressed   (false),
	m_enabled       (_enabled),
	m_repeating     (false),
	m_flat          (flat),
	m_needredraw    (true),
	m_cache_pic     (g_gr->get_no_picture()),
	m_title         (title_text),
	m_pic_background(background_picture_id),
	m_pic_custom    (g_gr->get_no_picture()),
	m_pic_custom_disabled(g_gr->get_no_picture()),
	m_fontname      (fontname),
	m_fontsize      (fontsize),
	m_clr_down      (229, 161, 2),
	m_draw_caret    (false)
{
	set_think(false);
}


Button::Button //  for pictorial buttons
	(Panel * const parent,
	 const std::string & name,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 PictureID background_picture_id,
	 const PictureID foreground_picture_id,
	 const std::string & tooltip_text,
	 bool const _enabled, bool const flat,
	 const std::string & fontname,
	 const uint32_t      fontsize)
	:
	NamedPanel      (parent, name, x, y, w, h, tooltip_text),
	m_highlighted   (false),
	m_pressed       (false),
	m_permpressed   (false),
	m_enabled       (_enabled),
	m_repeating     (false),
	m_flat          (flat),
	m_needredraw    (true),
	m_cache_pic     (g_gr->get_no_picture()),
	m_pic_background(background_picture_id),
	m_pic_custom    (foreground_picture_id),
	m_pic_custom_disabled(g_gr->create_grayed_out_pic(foreground_picture_id)),
	m_fontname      (fontname),
	m_fontsize      (fontsize),
	m_clr_down      (229, 161, 2),
	m_draw_caret    (false)
{
	set_think(false);
}


Button::~Button() {
	if (m_pic_custom_disabled != g_gr->get_no_picture())
		g_gr->free_picture_surface(m_pic_custom_disabled);
}


/**
 * Sets a new picture for the Button.
*/
void Button::set_pic(PictureID const picid)
{
	m_title.clear();

	if (m_pic_custom == picid)
		return;

	m_needredraw = true;

	m_pic_custom = picid;
	if (m_pic_custom_disabled != g_gr->get_no_picture())
		g_gr->free_picture_surface(m_pic_custom_disabled);
	m_pic_custom_disabled = g_gr->create_grayed_out_pic(picid);

	update();
}


/**
 * Set a text title for the Button
*/
void Button::set_title(std::string const & title) {
	if (m_title == title)
		return;

	m_pic_custom = g_gr->get_no_picture();
	m_title      = title;

	m_needredraw = true;
	update();
}


/**
 * Enable/Disable the button (disabled buttons can't be clicked).
 * Buttons are enabled by default
*/
void Button::set_enabled(bool const on)
{
	if (m_enabled == on)
		return;

	m_needredraw = true;

	// disabled buttons should look different...
	if (on)
		m_enabled = true;
	else {
		if (m_pressed) {
			m_pressed = false;
			set_think(false);
			grab_mouse(false);
		}
		m_enabled = false;
		m_highlighted = false;
	}
	update();
}


/**
 * Redraw the button
*/
void Button::draw(RenderTarget & odst)
{
	RenderTarget * dst = &odst;

	if (g_gr->caps().offscreen_rendering) {
		if (!m_needredraw)
		{
			odst.blit(Point(0, 0), m_cache_pic);
			return;
		} else {
			if (m_cache_pic == g_gr->get_no_picture())
				m_cache_pic =
					g_gr->create_picture_surface(get_w(), get_h(), m_flat);
			else if
				(m_cache_pic->rendertarget->get_w() != get_w() or
				 m_cache_pic->rendertarget->get_h() != get_h())
			{
				g_gr->free_picture_surface(m_cache_pic);
				m_cache_pic =
					g_gr->create_picture_surface(get_w(), get_h(), m_flat);
			}
			dst = (g_gr->get_surface_renderer(m_cache_pic));
		}
	}


	// Draw the background
	if (not m_flat) {
		assert(m_pic_background != g_gr->get_no_picture());
		dst->fill_rect
			(Rect(Point(0, 0), get_w(), get_h()), RGBAColor(0, 0, 0, 255));
		dst->tile
			(Rect(Point(0, 0), get_w(), get_h()),
			 m_pic_background,
			 Point(get_x(), get_y()));
	} else if (g_gr->caps().offscreen_rendering)
		dst->fill_rect
			(Rect(Point(0, 0), get_w(), get_h()), RGBAColor(0, 0, 0, 0));

	if (m_enabled and m_highlighted and not m_flat)
		dst->brighten_rect
			(Rect(Point(0, 0), get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);

	//  if we got a picture, draw it centered
	if (m_pic_custom != g_gr->get_no_picture()) {
		uint32_t cpw, cph;
		g_gr->get_picture_size(m_pic_custom, cpw, cph);

		//  ">> 1" is almost like "/ 2", but simpler for signed types (difference
		//  is that -1 >> 1 is -1 but -1 / 2 is 0).
		if (g_gr->caps().offscreen_rendering and m_flat)
			dst->blit_copy
				(Point
				 	((get_w() - static_cast<int32_t>(cpw)) >> 1,
				 	 (get_h() - static_cast<int32_t>(cph)) >> 1),
				 m_enabled ? m_pic_custom : m_pic_custom_disabled);
		else
			dst->blit
				(Point
				 	((get_w() - static_cast<int32_t>(cpw)) >> 1,
				 	 (get_h() - static_cast<int32_t>(cph)) >> 1),
				 m_enabled ? m_pic_custom : m_pic_custom_disabled);

	} else if (m_title.length()) //  otherwise draw title string centered
		UI::g_fh->draw_string
			(*dst,
			 m_fontname,
			 m_fontsize,
			 m_enabled ? UI_FONT_CLR_FG : UI_FONT_CLR_DISABLED, UI_FONT_CLR_BG,
			 Point(get_w() >> 1, get_h() >> 1),
			 m_title,
			 Align_Center,
			 std::numeric_limits<uint32_t>::max(),
			 Widget_Cache_None,
			 g_gr->get_no_picture(),
			 m_draw_caret ? m_title.length() :
			 std::numeric_limits<uint32_t>::max());

	//  draw border
	//  a pressed but not highlighted button occurs when the user has pressed
	//  the left mouse button and then left the area of the button or the button
	//  stays pressed when it is pressed once
	RGBAColor black(0, 0, 0, 255);

	// m_permpressed is true, we invert the behaviour on m_pressed
	bool draw_pressed = m_permpressed ? not (m_pressed and m_highlighted)
	                                  :     (m_pressed and m_highlighted);

	if (not m_flat) {
		assert(2 <= get_w());
		assert(2 <= get_h());
		//  button is a normal one, not flat
		if (not draw_pressed) {
			//  top edge
			dst->brighten_rect
				(Rect(Point(0, 0), get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  left edge
			dst->brighten_rect
				(Rect(Point(0, 2), 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
			//  bottom edge
			dst->fill_rect(Rect(Point(2, get_h() - 2), get_w() - 2, 1), black);
			dst->fill_rect(Rect(Point(1, get_h() - 1), get_w() - 1, 1), black);
			//  right edge
			dst->fill_rect(Rect(Point(get_w() - 2, 2), 1, get_h() - 2), black);
			dst->fill_rect(Rect(Point(get_w() - 1, 1), 1, get_h() - 1), black);
		} else {
			//  bottom edge
			dst->brighten_rect
				(Rect(Point(0, get_h() - 2), get_w(), 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);
			//  right edge
			dst->brighten_rect
				(Rect(Point(get_w() - 2, 0), 2, get_h() - 2),
				 BUTTON_EDGE_BRIGHT_FACTOR);
			//  top edge
			dst->fill_rect(Rect(Point(0, 0), get_w() - 1, 1), black);
			dst->fill_rect(Rect(Point(0, 1), get_w() - 2, 1), black);
			//  left edge
			dst->fill_rect(Rect(Point(0, 0), 1, get_h() - 1), black);
			dst->fill_rect(Rect(Point(1, 0), 1, get_h() - 2), black);
		}
	} else {
		//  Button is flat, do not draw borders, instead, if it is pressed, draw
		//  a box around it.
		if (m_enabled and m_highlighted)
		{
			RGBAColor shade(100, 100, 100, 80);
			dst->fill_rect(Rect(Point(0, 0), get_w(), 2), shade);
			dst->fill_rect(Rect(Point(0, 2), 2, get_h() - 2), shade);
			dst->fill_rect(Rect(Point(0, get_h() - 2), get_w(), get_h()), shade);
			dst->fill_rect(Rect(Point(get_w() - 2, 0), get_w(), get_h()), shade);
			//dst.draw_rect(Rect(Point(0, 0), get_w(), get_h()), m_clr_down);
		}
	}

	if (g_gr->caps().offscreen_rendering)
		odst.blit(Point(0, 0), m_cache_pic);
	m_needredraw = false;
}

void Button::think()
{
	assert(m_repeating);
	assert(m_pressed);
	Panel::think();

	if (m_highlighted) {
		int32_t const time = WLApplication::get()->get_time();
		if (m_time_nextact <= time) {
			m_time_nextact += MOUSE_BUTTON_AUTOREPEAT_TICK; //  schedule next tick
			if (m_time_nextact < time)
				m_time_nextact = time;
			play_click();
			clicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
}

/**
 * Update highlighted status
*/
void Button::handle_mousein(bool const inside)
{
	bool oldhl = m_highlighted;

	m_highlighted = inside && m_enabled;

	if (oldhl == m_highlighted)
		return;

	m_needredraw = true;
	update();
}


/**
 * Update the pressed status of the button
*/
bool Button::handle_mousepress(Uint8 const btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	if (m_enabled) {
		grab_mouse(true);
		if (!m_pressed)
			m_needredraw = true;
		m_pressed = true;
		if (m_repeating) {
			m_time_nextact =
				WLApplication::get()->get_time() + MOUSE_BUTTON_AUTOREPEAT_DELAY;
			set_think(true);
		}
	}
	update();

	return true;
}
bool Button::handle_mouserelease(Uint8 const btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT)
		return false;

	set_think(false);
	if (m_pressed) {
		if (m_pressed)
			m_needredraw = true;
		m_pressed = false;
		grab_mouse(false);
		update();
		if (m_highlighted && m_enabled) {
			play_click();
			clicked();
			//  The button may not exist at this point (for example if the button
			//  closed the dialog that it is part of). So member variables may no
			//  longer be accessed.
		}
	}
	return true;
}
bool Button::handle_mousemove(const Uint8, int32_t, int32_t, int32_t, int32_t) {
	return true; // We handle this always by lighting up
}

void Button::set_perm_pressed(bool state) {
	if (state != m_permpressed) {
		m_permpressed = state;
		m_needredraw = true;
		update();
	}
}

}
