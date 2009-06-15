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

#include "multilineeditbox.h"

#include "scrollbar.h"
#include "constants.h"
#include "font_handler.h"
#include "graphic/rendertarget.h"
#include "wlapplication.h"

#include <SDL_keysym.h>

namespace UI {
/**
 * Initialize an editbox that supports multiline strings.
*/
Multiline_Editbox::Multiline_Editbox
	(Panel * parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 const char * text)
	:
	Multiline_Textarea(parent, x, y, w, h, text, Align_Left, true),
	m_cur_pos         (get_text().size()),
	m_maxchars        (0xffff),
	m_needs_update    (false)
{
	set_scrollmode(ScrollNormal);
	set_handle_mouse(true);
	set_can_focus(true);
	set_think(false);
}


/**
 * A key event must be handled
*/
bool Multiline_Editbox::handle_key(bool down, SDL_keysym code) {
	char c = code.unicode & 0xff80 ? '\0' : code.unicode;

	m_needs_update = true;

	if (down) {
		std::string txt =
			g_fh->word_wrap_text(m_fontname, m_fontsize, get_text(), get_eff_w());
		switch (code.sym) {
		case SDLK_BACKSPACE:
			if (txt.size() and m_cur_pos)
				--m_cur_pos;
			else
				break;
			//  fallthrough

		case SDLK_DELETE:
			if (txt.size() and m_cur_pos < txt.size()) {
				txt.erase(txt.begin() + m_cur_pos);
				set_text(txt.c_str());
			}
			break;

		case SDLK_LEFT:
			if (0 < m_cur_pos) {
				--m_cur_pos;
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_cur_pos = m_cur_pos;; m_cur_pos = new_cur_pos)
						if (0 == new_cur_pos or isspace(txt[--new_cur_pos]))
							break;
			}
			break;

		case SDLK_RIGHT:
			if (m_cur_pos < txt.size()) {
				++m_cur_pos;
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_cur_pos = m_cur_pos;; ++new_cur_pos)
						if
							(new_cur_pos == txt.size()
							 or
							 isspace(txt[new_cur_pos - 1]))
						{
							m_cur_pos = new_cur_pos;
							break;
						}
			}
			break;

		case SDLK_DOWN:
			if (m_cur_pos < txt.size() - 1) {
				uint32_t begin_of_line = m_cur_pos;
				if (txt[begin_of_line] == '\n')
					--begin_of_line;
				while (begin_of_line > 0 && txt[begin_of_line] != '\n')
					--begin_of_line;
				if (begin_of_line)
					++begin_of_line;
				uint32_t begin_of_next_line = m_cur_pos;
				while
					(txt[begin_of_next_line] != '\n'
					 &&
					 begin_of_next_line < txt.size())
					++begin_of_next_line;
				begin_of_next_line += begin_of_next_line == txt.size() ? -1 : 1;
				uint32_t end_of_next_line = begin_of_next_line;
				while
					(txt[end_of_next_line] != '\n' && end_of_next_line < txt.size())
					++end_of_next_line;
				m_cur_pos =
					begin_of_next_line + m_cur_pos - begin_of_line
					>
					end_of_next_line ? end_of_next_line :
					begin_of_next_line + m_cur_pos - begin_of_line;
			}
			break;

		case SDLK_UP:
			if (m_cur_pos > 0) {
				uint32_t begin_of_line = m_cur_pos;
				if (txt[begin_of_line] == '\n')
					--begin_of_line;
				while (begin_of_line > 0 && txt[begin_of_line] != '\n')
					--begin_of_line;
				if (begin_of_line)
					++begin_of_line;
				uint32_t end_of_last_line = begin_of_line;
				if (begin_of_line)
					--end_of_last_line;
				uint32_t begin_of_lastline = end_of_last_line;
				if (txt[begin_of_lastline] == '\n')
					--begin_of_lastline;
				while (begin_of_lastline > 0 && txt[begin_of_lastline] != '\n')
					--begin_of_lastline;
				if (begin_of_lastline)
					++begin_of_lastline;
				m_cur_pos =
					begin_of_lastline + (m_cur_pos - begin_of_line)
					>
					end_of_last_line ? end_of_last_line :
					begin_of_lastline + (m_cur_pos - begin_of_line);
			}
			break;

		case SDLK_HOME:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				m_cur_pos = 0;
			else
				while (0 < m_cur_pos) {
					uint32_t const preceding_cur_pos = m_cur_pos - 1;
					if (txt[preceding_cur_pos] == '\n')
						break;
					else
						m_cur_pos = preceding_cur_pos;
				}
			break;

		case SDLK_END:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				m_cur_pos = txt.size();
			else
				while (m_cur_pos < txt.size() and txt[m_cur_pos] != '\n')
					++m_cur_pos;
			break;

		case SDLK_RETURN:
			c = '\n';
			// fallthrough
		default:
			if (c and txt.size() < m_maxchars) {
				txt.insert(m_cur_pos, 1, c);
				++m_cur_pos;
			}
			set_text(txt.c_str());
			break;
		}
		set_text(txt.c_str());
		changed.call();
		return true;
	}

	return false;
}

/**
 * Handle mousebutton events
 */
bool Multiline_Editbox::handle_mousepress
		(const Uint8 btn, int32_t x, int32_t y)
{
	if (btn == SDL_BUTTON_LEFT and not has_focus()) {
		focus();
		Multiline_Textarea::set_text(get_text().c_str());
		changed.call();
		return true;
	}
	return Multiline_Textarea::handle_mousepress(btn, x, y);
}
bool Multiline_Editbox::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}

/**
 * Redraw the Editbox
*/
void Multiline_Editbox::draw(RenderTarget & dst)
{
	//  make the whole area a bit darker
	dst.brighten_rect(Rect(Point(0, 0), get_w(), get_h()), ms_darken_value);
	if (get_text().size()) {
		g_fh->draw_string
			(dst,
			 m_fontname,
			 m_fontsize,
			 m_fcolor,
			 RGBColor(107, 87, 55),
			 Point(get_halign(), 0 - m_textpos),
			 get_text().c_str(),
			 m_align,
			 get_eff_w(),
			 m_cache_mode,
			 m_cache_id,
			 //  explicit cast is necessary to avoid a compiler warning
			 (has_focus() ? static_cast<int32_t>(m_cur_pos) : -1));
		draw_scrollbar();

		uint32_t w; // just to run g_fh->get_size_from_cache
		g_fh->get_size_from_cache(m_cache_id, w, m_textheight);

		m_cache_mode = Widget_Cache_Use;
	}
}

/**
 * Set text function needs to take care of the current
 * position
 */
void Multiline_Editbox::set_text(char const * const str)
{
	CalcLinePos();

	Multiline_Textarea::set_text(str);
}

/**
 * Calculate the height position of the cursor and write it to m_textpos
 * so the scrollbar can follow the cursor.
 */
void Multiline_Editbox::CalcLinePos()
{
	if (m_textheight < static_cast<uint32_t>(get_h())) {
		m_textpos = 0;
		return;
	}

	std::string const & str = get_text().c_str();
	size_t leng = str.size();
	uint32_t lbtt = 0; // linebreaks to top
	uint32_t lbtb = 0; // linebreaks to bottom

	for (size_t i = 0; i < m_cur_pos; ++i)
		if (str[i] == '\n')
			++lbtt;
	for (size_t i = m_cur_pos; i < leng; ++i)
		if (str[i] == '\n')
			++lbtb;

	m_textpos = (lbtt == 0) & (lbtb == 0) ?
		0 : (m_textheight - get_h()) * lbtt / (lbtb + lbtt);
}

};
