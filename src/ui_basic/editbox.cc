/*
 * Copyright (C) 2003, 2006-2008, 2010-2011 by the Widelands Development Team
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

#include "ui_basic/editbox.h"

#include <limits>

#include <SDL_keycode.h>

#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "ui_basic/mouse_constants.h"

// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace UI {

struct EditBoxImpl {
	/**
	 * Font used for rendering text.
	 */
	/*@{*/
	std::string fontname;
	uint32_t fontsize;
	RGBColor fontcolor;
	/*@}*/

	/// Background tile style.
	const Image* background;

	/// Maximum number of characters in the input
	uint32_t maxLength;

	/// Current text in the box.
	std::string text;

	/// Position of the caret.
	uint32_t caret;

	/// Current scrolling offset to the text anchor position, in pixels
	int32_t scrolloffset;

	/// Alignment of the text. Vertical alignment is always centered.
	Align align;
};

EditBox::EditBox
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const Image* background,
	 Align _align)
	:
	Panel(parent, x, y, w, h),
	m(new EditBoxImpl),
	m_history_active(false),
	m_history_position(-1)
{
	set_thinks(false);

	m->background = background;
	m->fontname = UI::g_fh1->fontset()->serif();
	m->fontsize = UI_FONT_SIZE_SMALL;
	m->fontcolor = UI_FONT_CLR_FG;

	m->align = static_cast<Align>((_align & Align_Horizontal) | Align_VCenter);
	m->caret = 0;
	m->scrolloffset = 0;
	// yes, use *signed* max as maximum length; just a small safe-guard.
	m->maxLength = std::numeric_limits<int32_t>::max();

	set_handle_mouse(true);
	set_can_focus(true);
	set_handle_textinput();

	// Initialize history as empty string
	for (uint8_t i = 0; i < CHAT_HISTORY_SIZE; ++i)
		m_history[i] = "";
}

EditBox::~EditBox()
{
	// place a destructor where the compiler can find the EditBoxImpl destructor
}

/**
 * \return the current text entered in the edit box
 */
const std::string & EditBox::text() const
{
	return m->text;
}

/**
 * Set the font used by the edit box.
 */
void EditBox::set_font(const std::string & name, int32_t size, RGBColor color)
{
	m->fontname = name;
	m->fontsize = size;
	m->fontcolor = color;
}

/**
 * Set the current text in the edit box.
 *
 * The text is truncated if it is longer than the maximum length set by
 * \ref setMaxLength().
 */
void EditBox::set_text(const std::string & t)
{
	if (t == m->text)
		return;

	bool caretatend = m->caret == m->text.size();

	m->text = t;
	if (m->text.size() > m->maxLength)
		m->text.erase(m->text.begin() + m->maxLength, m->text.end());
	if (caretatend || m->caret > m->text.size())
		m->caret = m->text.size();

	update();
}


/**
 * \return the maximum length of the input string
 */
uint32_t EditBox::max_length() const
{
	return m->maxLength;
}


/**
 * Set the maximum length of the input string.
 *
 * If the current string is longer than the new maximum length,
 * its end is cut off to fit into the maximum length.
 */
void EditBox::set_max_length(uint32_t const n)
{
	m->maxLength = n;

	if (m->text.size() > m->maxLength) {
		m->text.erase(m->text.begin() + m->maxLength, m->text.end());
		if (m->caret > m->text.size())
			m->caret = m->text.size();

		check_caret();
		update();
	}
}


/**
 * \return the text alignment
 */
Align EditBox::align() const
{
	return m->align;
}


/**
 * Set the new alignment.
 *
 * Note that vertical alignment is always centered, independent of what
 * you select here.
 */
void EditBox::set_align(Align _align)
{
	_align = static_cast<Align>((_align & Align_Horizontal) | Align_VCenter);
	if (_align != m->align) {
		m->align = _align;
		m->scrolloffset = 0;
		check_caret();
		update();
	}
}


/**
 * The mouse was clicked on this editbox
*/
bool EditBox::handle_mousepress(const uint8_t btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		update();
		return true;
	}

	return false;
}
bool EditBox::handle_mouserelease(const uint8_t btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT && get_can_focus();
}

/**
 * Handle keypress/release events
 */
// TODO(unknown): Text input works only because code.unicode happens to map to ASCII for
// ASCII characters (--> //HERE). Instead, all user editable strings should be
// real unicode.
bool EditBox::handle_key(bool const down, SDL_Keysym const code)
{
	if (down) {
		switch (code.sym) {
		case SDLK_ESCAPE:
			cancel();
			return true;

		case SDLK_TAB:
			//let the panel handle the tab key
			return false;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			// Save history if active and text is not empty
			if (m_history_active) {
				if (!m->text.empty()) {
					for (uint8_t i = CHAT_HISTORY_SIZE - 1; i > 0; --i)
						m_history[i] = m_history[i - 1];
					m_history[0] = m->text;
					m_history_position = -1;
				}
			}
			ok();
			return true;

		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_DELETE:
			if (m->caret < m->text.size()) {
				while ((m->text[++m->caret] & 0xc0) == 0x80) {};
				// now handle it like Backspace
			} else
				return true;
			/* no break */
		case SDLK_BACKSPACE:
			if (m->caret > 0) {
				while ((m->text[--m->caret] & 0xc0) == 0x80)
					m->text.erase(m->text.begin() + m->caret);
				m->text.erase(m->text.begin() + m->caret);
				check_caret();
				changed();
				update();
			}
			return true;

		case SDLK_KP_4:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_LEFT:
			if (m->caret > 0) {
				while ((m->text[--m->caret] & 0xc0) == 0x80) {};
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_caret = m->caret;; m->caret = new_caret)
						if (0 == new_caret || isspace(m->text[--new_caret]))
							break;

				check_caret();

				update();
			}
			return true;

		case SDLK_KP_6:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_RIGHT:
			if (m->caret < m->text.size()) {
				while ((m->text[++m->caret] & 0xc0) == 0x80) {};
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_caret = m->caret;; ++new_caret)
						if
							(new_caret == m->text.size()
							 ||
							 isspace(m->text[new_caret - 1]))
						{
							m->caret = new_caret;
							break;
						}

				check_caret();
				update();
			}
			return true;

		case SDLK_KP_7:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_HOME:
			if (m->caret != 0) {
				m->caret = 0;

				check_caret();
				update();
			}
			return true;

		case SDLK_KP_1:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_END:
			if (m->caret != m->text.size()) {
				m->caret = m->text.size();
				check_caret();
				update();
			}
			return true;

		case SDLK_KP_8:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_UP:
			// Load entry from history if active and text is not empty
			if (m_history_active) {
				if (m_history_position > CHAT_HISTORY_SIZE - 2)
					m_history_position = CHAT_HISTORY_SIZE - 2;
				if (m_history[++m_history_position].size() > 0) {
					m->text = m_history[m_history_position];
					m->caret = m->text.size();
					check_caret();
					update();
				}
			}
			return true;

		case SDLK_KP_2:
			if (code.mod & KMOD_NUM) {
				break;
			}
			/* no break */
		case SDLK_DOWN:
			// Load entry from history if active and text is not equivalent to the current one
			if (m_history_active) {
				if (m_history_position < 1)
					m_history_position = 1;
				if (m_history[--m_history_position] != m->text) {
					m->text = m_history[m_history_position];
					m->caret = m->text.size();
					check_caret();
					update();
				}
			}
			return true;

		default:
			break;
		}
	}

	return false;
}

bool EditBox::handle_textinput(const std::string& input_text) {
	if ((m->text.size() +  input_text.length()) < m->maxLength) {
		m->text.insert(m->caret, input_text);
		m->caret += input_text.length();
		check_caret();
		changed();
		update();
	}
	return true;
}

void EditBox::draw(RenderTarget & odst)
{
	RenderTarget & dst = odst;

	// Draw the background
	dst.tile
		(Rect(Point(0, 0), get_w(), get_h()),
		 m->background,
		 Point(get_x(), get_y()));

	// Draw border.
	if (get_w() >= 4 && get_h() >= 4) {
		static const RGBColor black(0, 0, 0);

		// bottom edge
		dst.brighten_rect
			(Rect(Point(0, get_h() - 2), get_w(), 2),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect
			(Rect(Point(get_w() - 2, 0), 2, get_h() - 2),
			 BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Rect(Point(0, 0), get_w() - 1, 1), black);
		dst.fill_rect(Rect(Point(0, 1), get_w() - 2, 1), black);
		// left edge
		dst.fill_rect(Rect(Point(0, 0), 1, get_h() - 1), black);
		dst.fill_rect(Rect(Point(1, 0), 1, get_h() - 2), black);
	}

	if (has_focus())
		dst.brighten_rect
			(Rect(Point(0, 0), get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);

	Point pos(4, get_h() >> 1);

	switch (m->align & Align_Horizontal) {
	case Align_HCenter:
		pos.x = get_w() >> 1;
		break;
	case Align_Right:
		pos.x = get_w() - 4;
		break;
	default:
		break;
	}

	pos.x += m->scrolloffset;

	UI::g_fh->draw_text
		(dst,
		 TextStyle::makebold(Font::get(m->fontname, m->fontsize), m->fontcolor),
		 pos,
		 m->text,
		 align(),
		 has_focus() ? static_cast<int32_t>(m->caret) : std::numeric_limits<uint32_t>::max());
}

/**
 * Check the caret's position and scroll it into view if necessary.
 */
void EditBox::check_caret()
{
	std::string leftstr(m->text, 0, m->caret);
	std::string rightstr(m->text, m->caret, std::string::npos);
	uint32_t leftw;
	uint32_t rightw;
	uint32_t tmp;

	UI::g_fh->get_size(m->fontname, m->fontsize, leftstr, leftw, tmp);
	UI::g_fh->get_size(m->fontname, m->fontsize, rightstr, rightw, tmp);

	int32_t caretpos;

	switch (m->align & Align_Horizontal) {
	case Align_HCenter:
		caretpos  = (get_w() - static_cast<int32_t>(leftw + rightw)) / 2;
		caretpos += m->scrolloffset + leftw;
		break;
	case Align_Right:
		caretpos = get_w() - 4 + m->scrolloffset - rightw;
		break;
	default:
		caretpos = 4 + m->scrolloffset + leftw;
		break;
	}

	if (caretpos < 4)
		m->scrolloffset += 4 - caretpos + get_w() / 5;
	else if (caretpos > get_w() - 4)
		m->scrolloffset -= caretpos - get_w() + 4 + get_w() / 5;

	if ((m->align & Align_Horizontal) == Align_Left) {
		if (m->scrolloffset > 0)
			m->scrolloffset = 0;
	} else if ((m->align & Align_Horizontal) == Align_Right) {
		if (m->scrolloffset < 0)
			m->scrolloffset = 0;
	}
}

}
