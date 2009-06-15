/*
 * Copyright (C) 2003, 2006-2008 by the Widelands Development Team
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

#include "editbox.h"

#include "mouse_constants.h"

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "helper.h"

#include <limits>
#include <SDL_keysym.h>

namespace UI {

struct EditBoxImpl {
	/// Background tile style.
	PictureID background;

	/// ID. Only used for the id-flavoured signals.
	int32_t id;

	/// Maximum number of characters in the input
	uint32_t maxLength;

	/// Current text in the box.
	std::string text;

	/// Position of the caret.
	uint32_t caret;

	/// Alignment of the text. Vertical alignment is always centered.
	Align align;
};

EditBox::EditBox
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const PictureID & background,
	 const int32_t id,
	 Align _align)
	:
	Panel(parent, x, y, w, h),
	m_fontname(UI_FONT_NAME),
	m_fontsize(UI_FONT_SIZE_SMALL),
	m_fontcolor(UI_FONT_CLR_FG),
	m(new EditBoxImpl)
{
	set_think(false);

	char buf[256];
	m->background = background;

	m->id = id;
	m->align = static_cast<Align>((_align & Align_Horizontal) | Align_VCenter);
	// yes, use *signed* max as maximum length; just a small safe-guard.
	m->caret = 0;
	m->maxLength = std::numeric_limits<int32_t>::max();

	set_handle_mouse(true);
	set_can_focus(true);
}

EditBox::~EditBox()
{
	// place a destructor where the compiler can find the EditBoxImpl destructor
}

/**
 * \return the current text entered in the edit box
 */
std::string const & EditBox::text() const
{
	return m->text;
}


/**
 * Set the current text in the edit box.
 *
 * The text is truncated if it is longer than the maximum length set by
 * \ref setMaxLength().
 */
void EditBox::setText(std::string const & t)
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
uint32_t EditBox::maxLength() const
{
	return m->maxLength;
}


/**
 * Set the maximum length of the input string.
 *
 * If the current string is longer than the new maximum length,
 * its end is cut off to fit into the maximum length.
 */
void EditBox::setMaxLength(uint32_t const n)
{
	m->maxLength = n;

	if (m->text.size() > m->maxLength) {
		m->text.erase(m->text.begin() + m->maxLength, m->text.end());
		if (m->caret > m->text.size())
			m->caret = m->text.size();

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
void EditBox::setAlign(Align _align)
{
	_align = static_cast<Align>((_align & Align_Horizontal) | Align_VCenter);
	if (_align != m->align) {
		m->align = _align;
		update();
	}
}


/**
 * The mouse was clicked on this editbox
*/
bool EditBox::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		update();
		return true;
	}

	return false;
}
bool EditBox::handle_mouserelease(const Uint8 btn, int32_t, int32_t)
{
	return btn == SDL_BUTTON_LEFT && get_can_focus();
}

/**
 * Handle keypress/release events
 *
 * \todo Text input works only because code.unicode happens to map to ASCII for
 * ASCII characters (--> //HERE). Instead, all user editable strings should be
 * real unicode.
*/
bool EditBox::handle_key(bool down, SDL_keysym code)
{
	if (down) {
		switch (code.sym) {
		case SDLK_ESCAPE:
			cancel.call();
			cancelid.call(m->id);
			return true;

		case SDLK_RETURN:
			ok.call();
			okid.call(m->id);
			return true;

		case SDLK_DELETE:
			if (m->caret < m->text.size()) {
				while ((m->text[++m->caret] & 0xc0) == 0x80) {};
				// now handle it like Backspace
			} else
				return true;

		case SDLK_BACKSPACE:
			if (m->caret > 0) {
				while ((m->text[--m->caret] & 0xc0) == 0x80)
					m->text.erase(m->text.begin() + m->caret);
				m->text.erase(m->text.begin() + m->caret);
				changed.call();
				changedid.call(m->id);
				update();
			}
			return true;

		case SDLK_LEFT:
			if (m->caret > 0) {
				while ((m->text[--m->caret] & 0xc0) == 0x80) {};
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_caret = m->caret;; m->caret = new_caret)
						if (0 == new_caret or isspace(m->text[--new_caret]))
							break;
				update();
			}
			return true;

		case SDLK_RIGHT:
			if (m->caret < m->text.size()) {
				while ((m->text[++m->caret] & 0xc0) == 0x80) {};
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
					for (uint32_t new_caret = m->caret;; ++new_caret)
						if
							(new_caret == m->text.size()
							 or
							 isspace(m->text[new_caret - 1]))
						{
							m->caret = new_caret;
							break;
						}
				update();
			}
			return true;

		case SDLK_HOME:
			if (m->caret != 0) {
				m->caret = 0;
				update();
			}
			return true;

		case SDLK_END:
			if (m->caret != m->text.size()) {
				m->caret = m->text.size();
				update();
			}
			return true;

		default:
			if (is_printable(code)) {
				if (m->text.size() < m->maxLength) {
					if (code.unicode < 0x80)         // 1 byte char
						m->text.insert
							(m->text.begin() + m->caret++, 1, code.unicode);
					else if (code.unicode < 0x800) { // 2 byte char
						m->text.insert
							(m->text.begin() + m->caret++,
							 (((code.unicode & 0x7c0) >> 6) | 0xc0));
						m->text.insert
							(m->text.begin() + m->caret++,
							 ((code.unicode & 0x3f) | 0x80));
					} else {                         // 3 byte char
						m->text.insert
							(m->text.begin() + m->caret++,
							 (((code.unicode & 0xf000) >> 12) | 0xe0));
						m->text.insert
							(m->text.begin() + m->caret++,
							 (((code.unicode & 0xfc0) >> 6) | 0x80));
						m->text.insert
							(m->text.begin() + m->caret++,
							 ((code.unicode & 0x3f) | 0x80));
					}
					changed.call();
					changedid.call(m->id);
					update();
				}
				return true;
			}
		}
	}

	return Panel::handle_key(down, code);
}


void EditBox::draw(RenderTarget & dst)
{
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

	g_fh->draw_string
		(dst,
		 m_fontname, m_fontsize, m_fontcolor, UI_FONT_CLR_BG,
		 pos,
		 m->text,
		 align(),
		 -1,
		 Widget_Cache_None,
		 g_gr->get_no_picture(),
		 has_focus() ? static_cast<int32_t>(m->caret) : -1);
}


};
