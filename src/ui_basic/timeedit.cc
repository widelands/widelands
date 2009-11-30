/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "timeedit.h"

#include "font_handler.h"

namespace UI {

inline TimeEdit::Crease_Button::Crease_Button
	(TimeEdit & parent, Point const p, PictureID const pic, bool const _enabled)
	:
	UI::Button
		(&parent,
		 p.x, p.y, 20, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 pic,
		 std::string(), _enabled)
{
	set_repeating(true);
}


TimeEdit::Increase_Button::Increase_Button
	(TimeEdit & parent, int32_t const x, bool const _enabled)
	:
	Crease_Button
		(parent, Point(x, 0),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 _enabled)
{}


TimeEdit::Decrease_Button::Decrease_Button
	(TimeEdit & parent, int32_t const x, bool const _enabled)
	:
	Crease_Button
		(parent, Point(x, 40),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 _enabled)
{}


TimeEdit::TimeEdit
	(Panel  * const parent,
	 Point    const p,
	 uint32_t       _time,
	 bool     const _enabled)
	:
	Panel(parent, p.x, p.y, 240, 60),
	m_value_msecs_third   (_time        % 10),
	m_value_msecs_secnd  ((_time /= 10) % 10),
	m_value_msecs_first  ((_time /= 10) % 10),
	m_value_seconds_secnd((_time /= 10) % 10),
	m_value_seconds_first((_time /= 10) %  6),
	m_value_minutes_secnd((_time /=  6) % 10),
	m_value_minutes_first((_time /= 10) %  6),
	m_value_hours_secnd  ((_time /=  6) % 10),
	m_value_hours_first  (std::min(_time / 10, 9U)),

	m_enabled(_enabled),

	m_hours_first_increase  (*this, _enabled),
	m_hours_first_decrease  (*this, _enabled),
	m_hours_secnd_increase  (*this, _enabled),
	m_hours_secnd_decrease  (*this, _enabled),
	m_minutes_first_increase(*this, _enabled),
	m_minutes_first_decrease(*this, _enabled),
	m_minutes_secnd_increase(*this, _enabled),
	m_minutes_secnd_decrease(*this, _enabled),
	m_seconds_first_increase(*this, _enabled),
	m_seconds_first_decrease(*this, _enabled),
	m_seconds_secnd_increase(*this, _enabled),
	m_seconds_secnd_decrease(*this, _enabled),
	m_msecs_first_increase  (*this, _enabled),
	m_msecs_first_decrease  (*this, _enabled),
	m_msecs_secnd_increase  (*this, _enabled),
	m_msecs_secnd_decrease  (*this, _enabled),
	m_msecs_third_increase  (*this, _enabled),
	m_msecs_third_decrease  (*this, _enabled)
{}


void TimeEdit::set_enabled(bool const on) {
	if (on != m_enabled) {
		m_enabled = on;
		m_hours_first_increase  .set_enabled(on and m_value_hours_first   < 9);
		m_hours_first_decrease  .set_enabled(on and m_value_hours_first);
		m_hours_secnd_increase  .set_enabled(on and m_value_hours_secnd   < 9);
		m_hours_secnd_decrease  .set_enabled(on and m_value_hours_secnd);
		m_minutes_first_increase.set_enabled(on and m_value_minutes_first < 5);
		m_minutes_first_decrease.set_enabled(on and m_value_minutes_first);
		m_minutes_secnd_increase.set_enabled(on and m_value_minutes_secnd < 9);
		m_minutes_secnd_decrease.set_enabled(on and m_value_minutes_secnd);
		m_seconds_first_increase.set_enabled(on and m_value_seconds_first < 5);
		m_seconds_first_decrease.set_enabled(on and m_value_seconds_first);
		m_seconds_secnd_increase.set_enabled(on and m_value_seconds_secnd < 9);
		m_seconds_secnd_decrease.set_enabled(on and m_value_seconds_secnd);
		m_msecs_first_increase  .set_enabled(on and m_value_msecs_first   < 9);
		m_msecs_first_decrease  .set_enabled(on and m_value_msecs_first);
		m_msecs_secnd_increase  .set_enabled(on and m_value_msecs_secnd   < 9);
		m_msecs_secnd_decrease  .set_enabled(on and m_value_msecs_secnd);
		m_msecs_third_increase  .set_enabled(on and m_value_msecs_third   < 9);
		m_msecs_third_decrease  .set_enabled(on and m_value_msecs_third);
	}
}


void TimeEdit::draw(RenderTarget & dst) {
	RGBColor const font_clr_fg =
		m_enabled ? UI_FONT_CLR_FG : UI_FONT_CLR_DISABLED;

#define DRAW_SEPARATOR(posx, separator)                                       \
   UI::g_fh->draw_string                                                      \
      (dst,                                                                   \
       UI_FONT_NAME, UI_FONT_SIZE_SMALL, font_clr_fg, UI_FONT_CLR_BG,         \
       Point(posx, 30),                                                       \
       separator, UI::Align_Center);

	DRAW_SEPARATOR (52, _(":"));
	DRAW_SEPARATOR(111, _(":"));
	DRAW_SEPARATOR(169, _("."));

#define DRAW_DIGIT(variable, posx, text_when_disabled_text) {                 \
   char buffer[2];                                                            \
   assert(variable <= 9);                                                     \
   sprintf(buffer, "%u", variable);                                           \
   UI::g_fh->draw_string                                                      \
      (dst,                                                                   \
       UI_FONT_NAME, UI_FONT_SIZE_SMALL, font_clr_fg, UI_FONT_CLR_BG,         \
       Point(posx, 30),                                                       \
       m_enabled ? buffer : text_when_disabled_text, UI::Align_Center);       \
}                                                                             \

	DRAW_DIGIT(m_value_hours_first,    10, _("h"));
	DRAW_DIGIT(m_value_hours_secnd,    32, _("h"));
	DRAW_DIGIT(m_value_minutes_first,  69, _("m"));
	DRAW_DIGIT(m_value_minutes_secnd,  91, _("m"));
	DRAW_DIGIT(m_value_seconds_first, 128, _("s"));
	DRAW_DIGIT(m_value_seconds_secnd, 150, _("s"));
	DRAW_DIGIT(m_value_msecs_first,   186, _("s"));
	DRAW_DIGIT(m_value_msecs_secnd,   208, _("s"));
	DRAW_DIGIT(m_value_msecs_third,   230, _("s"));
}

}
