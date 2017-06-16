/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "ui_basic/multilineeditbox.h"

#include <boost/bind.hpp>

#include "base/utf8.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "graphic/wordwrap.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"

// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace UI {

struct MultilineEditbox::Data {
	Scrollbar scrollbar;

	/// The text in the edit box
	std::string text;

	/// Background tile style.
	const Image* background;

	/// Position of the cursor inside the text.
	/// 0 indicates that the cursor is before the first character,
	/// text.size() inidicates that the cursor is after the last character.
	uint32_t cursor_pos;

	/// Font and style
	UI::TextStyle textstyle;

	/// Maximum length of the text string, in bytes
	uint32_t maxbytes;

	/// Cached wrapping info; see @ref refresh_ww and @ref update
	/*@{*/
	bool ww_valid;
	WordWrap ww;
	/*@}*/

	Data(MultilineEditbox&);
	void refresh_ww();

	void update();

	void scroll_cursor_into_view();
	void set_cursor_pos(uint32_t cursor_pos);

	uint32_t prev_char(uint32_t cursor);
	uint32_t next_char(uint32_t cursor);
	uint32_t snap_to_char(uint32_t cursor);

	void erase_bytes(uint32_t start, uint32_t end);
	void insert(uint32_t where, const std::string& s);

private:
	MultilineEditbox& owner;
};

/**
 * Initialize an editbox that supports multiline strings.
*/
MultilineEditbox::MultilineEditbox(Panel* parent,
                                   int32_t x,
                                   int32_t y,
                                   uint32_t w,
                                   uint32_t h,
                                   const std::string& text,
                                   const Image* background)
   : Panel(parent, x, y, w, h), d_(new Data(*this)) {
	d_->background = background;
	set_handle_mouse(true);
	set_can_focus(true);
	set_thinks(false);
	set_handle_textinput();

	set_text(text);
}

MultilineEditbox::Data::Data(MultilineEditbox& o)
   : scrollbar(&o, o.get_w() - Scrollbar::kSize, 0, Scrollbar::kSize, o.get_h(), false),
     cursor_pos(0),
     maxbytes(std::min(g_gr->max_texture_size() / UI_FONT_SIZE_SMALL, 0xffff)),
     ww_valid(false),
     owner(o) {
	scrollbar.moved.connect(boost::bind(&MultilineEditbox::scrollpos_changed, &o, _1));

	scrollbar.set_pagesize(owner.get_h() - 2 * textstyle.font->height());
	scrollbar.set_singlestepsize(textstyle.font->height());
}

/**
 * Call this function whenever some part of the data changes that potentially
 * requires some redrawing.
 */
void MultilineEditbox::Data::update() {
	ww_valid = false;
}

/**
 * Return the text currently stored by the editbox.
 */
const std::string& MultilineEditbox::get_text() const {
	return d_->text;
}

/**
 * Replace the currently stored text with something else.
 */
void MultilineEditbox::set_text(const std::string& text) {
	if (text == d_->text)
		return;

	d_->text = text;
	while (d_->text.size() > d_->maxbytes) {
		d_->erase_bytes(d_->prev_char(d_->text.size()), d_->text.size());
	}

	d_->set_cursor_pos(d_->text.size());

	d_->update();
	d_->scroll_cursor_into_view();

	changed();
}

/**
 * Set the maximum number of bytes in the scrollbar text.
 *
 * This will shorten the currently stored text when necessary.
 */
void MultilineEditbox::set_maximum_bytes(const uint32_t n) {
	while (n < d_->text.size())
		d_->erase_bytes(d_->prev_char(d_->text.size()), d_->text.size());
	d_->maxbytes = n;

	// do not need to update here, because erase() will
	// update when necessary
}

/**
 * Return the currently set maximum number of bytes.
 */
uint32_t MultilineEditbox::get_maximum_bytes() const {
	return d_->maxbytes;
}

/**
 * Erase the given range of bytes, adjust the cursor position, and update.
 */
void MultilineEditbox::Data::erase_bytes(uint32_t start, uint32_t end) {
	assert(start <= end);
	assert(end <= text.size());

	uint32_t nbytes = end - start;
	text.erase(start, nbytes);
	update();

	if (cursor_pos >= end)
		set_cursor_pos(cursor_pos - nbytes);
	else if (cursor_pos > start)
		set_cursor_pos(start);
}

/**
 * Find the starting byte of the previous character
 */
uint32_t MultilineEditbox::Data::prev_char(uint32_t cursor) {
	assert(cursor <= text.size());

	if (cursor == 0)
		return cursor;

	do {
		--cursor;
		// TODO(GunChleoc): When switchover to g_fh1 is complete, see if we can go full ICU here.
	} while (cursor > 0 && Utf8::is_utf8_extended(text[cursor]));

	return cursor;
}

/**
 * Find the starting byte of the next character
 */
uint32_t MultilineEditbox::Data::next_char(uint32_t cursor) {
	assert(cursor <= text.size());

	if (cursor >= text.size())
		return cursor;

	do {
		++cursor;
	} while (cursor < text.size() && Utf8::is_utf8_extended(text[cursor]));

	return cursor;
}

/**
 * Return the starting offset of the (multi-byte) character that @p cursor points to.
 */
uint32_t MultilineEditbox::Data::snap_to_char(uint32_t cursor) {
	while (cursor > 0 && Utf8::is_utf8_extended(text[cursor]))
		--cursor;
	return cursor;
}

/**
 * This is called by the UI code whenever a key press or release arrives
 */
bool MultilineEditbox::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		case SDLK_TAB:
			// Let the panel handle the tab key
			return get_parent()->handle_key(true, code);
		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DELETE:
			if (d_->cursor_pos < d_->text.size()) {
				d_->erase_bytes(d_->cursor_pos, d_->next_char(d_->cursor_pos));
				changed();
			}
			break;

		case SDLK_BACKSPACE:
			if (d_->cursor_pos > 0) {
				d_->erase_bytes(d_->prev_char(d_->cursor_pos), d_->cursor_pos);
				changed();
			}
			break;

		case SDLK_KP_4:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_LEFT: {
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				uint32_t newpos = d_->prev_char(d_->cursor_pos);
				while (newpos > 0 && isspace(d_->text[newpos]))
					newpos = d_->prev_char(newpos);
				while (newpos > 0) {
					uint32_t prev = d_->prev_char(newpos);
					if (isspace(d_->text[prev]))
						break;
					newpos = prev;
				}
				d_->set_cursor_pos(newpos);
			} else {
				d_->set_cursor_pos(d_->prev_char(d_->cursor_pos));
			}
			break;
		}

		case SDLK_KP_6:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_RIGHT:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				uint32_t newpos = d_->next_char(d_->cursor_pos);
				while (newpos < d_->text.size() && isspace(d_->text[newpos]))
					newpos = d_->next_char(newpos);
				while (newpos < d_->text.size() && !isspace(d_->text[newpos]))
					newpos = d_->next_char(newpos);
				d_->set_cursor_pos(newpos);
			} else {
				d_->set_cursor_pos(d_->next_char(d_->cursor_pos));
			}
			break;

		case SDLK_KP_2:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DOWN:
			if (d_->cursor_pos < d_->text.size()) {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline + 1 < d_->ww.nrlines()) {
					uint32_t lineend = d_->text.size();
					if (cursorline + 2 < d_->ww.nrlines())
						lineend = d_->prev_char(d_->ww.line_offset(cursorline + 2));

					uint32_t newpos = d_->ww.line_offset(cursorline + 1) + cursorpos;
					if (newpos > lineend)
						newpos = lineend;
					else
						newpos = d_->snap_to_char(newpos);
					d_->set_cursor_pos(newpos);
				} else {
					d_->set_cursor_pos(d_->text.size());
				}
			}
			break;

		case SDLK_KP_8:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_UP:
			if (d_->cursor_pos > 0) {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline > 0) {
					uint32_t newpos = d_->ww.line_offset(cursorline - 1) + cursorpos;
					uint32_t lineend = d_->prev_char(d_->ww.line_offset(cursorline));

					if (newpos > lineend)
						newpos = lineend;
					else
						newpos = d_->snap_to_char(newpos);
					d_->set_cursor_pos(newpos);
				} else {
					d_->set_cursor_pos(0);
				}
			}
			break;

		case SDLK_KP_7:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_HOME:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				d_->set_cursor_pos(0);
			} else {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				d_->set_cursor_pos(d_->ww.line_offset(cursorline));
			}
			break;

		case SDLK_KP_1:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_END:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				d_->set_cursor_pos(d_->text.size());
			} else {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline + 1 < d_->ww.nrlines())
					d_->set_cursor_pos(d_->prev_char(d_->ww.line_offset(cursorline + 1)));
				else
					d_->set_cursor_pos(d_->text.size());
			}
			break;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			d_->insert(d_->cursor_pos, "\n");
			changed();
			break;

		default:
			break;
		}
		return true;
	}

	return Panel::handle_key(down, code);
}

bool MultilineEditbox::handle_textinput(const std::string& input_text) {
	if (d_->text.size() + input_text.size() <= d_->maxbytes) {
		d_->insert(d_->cursor_pos, input_text);
		changed();
		d_->update();
	}
	return true;
}

/**
 * Grab the focus and redraw.
 */
void MultilineEditbox::focus(bool topcaller) {
	Panel::focus(topcaller);
}

/**
 * Redraw the Editbox
 */
void MultilineEditbox::draw(RenderTarget& dst) {
	// Draw the background
	dst.tile(Rect(Point(0, 0), get_w(), get_h()), d_->background, Point(get_x(), get_y()));

	// Draw border.
	if (get_w() >= 4 && get_h() >= 4) {
		static const RGBColor black(0, 0, 0);

		// bottom edge
		dst.brighten_rect(Rect(Point(0, get_h() - 2), get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect(Rect(Point(get_w() - 2, 0), 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Rect(Point(0, 0), get_w() - 1, 1), black);
		dst.fill_rect(Rect(Point(0, 1), get_w() - 2, 1), black);
		// left edge
		dst.fill_rect(Rect(Point(0, 0), 1, get_h() - 1), black);
		dst.fill_rect(Rect(Point(1, 0), 1, get_h() - 2), black);
	}

	if (has_focus())
		dst.brighten_rect(Rect(Point(0, 0), get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);

	d_->refresh_ww();

	d_->ww.set_draw_caret(has_focus());

	d_->ww.draw(dst, Point(0, -int32_t(d_->scrollbar.get_scrollpos())), UI::Align::kLeft,
	            has_focus() ? d_->cursor_pos : std::numeric_limits<uint32_t>::max());
}

/**
 * Insert the given string starting at cursor position @p where.
 * Update the cursor so that it stays in the same place, but note that the cursor is
 * "right-magnetic":
 * If @p where is equal to the current cursor position, then the cursor is moved.
 * This is usually what one wants.
 */
void MultilineEditbox::Data::insert(uint32_t where, const std::string& s) {
	text.insert(where, s);
	if (cursor_pos >= where)
		set_cursor_pos(cursor_pos + s.size());
}

/**
 * Change the position of the cursor, cause a display refresh and scroll the cursor
 * into view when necessary.
 */
void MultilineEditbox::Data::set_cursor_pos(uint32_t newpos) {
	assert(newpos <= text.size());

	if (cursor_pos == newpos)
		return;

	cursor_pos = newpos;

	scroll_cursor_into_view();
}

/**
 * Ensure that the cursor is visible.
 */
void MultilineEditbox::Data::scroll_cursor_into_view() {
	refresh_ww();

	uint32_t cursorline, cursorpos;
	ww.calc_wrapped_pos(cursor_pos, cursorline, cursorpos);

	int32_t lineheight = textstyle.font->height();
	int32_t lineskip = textstyle.font->lineskip();
	int32_t top = cursorline * lineskip;

	if (top < int32_t(scrollbar.get_scrollpos())) {
		scrollbar.set_scrollpos(top - lineheight);
	} else if (top + lineheight > int32_t(scrollbar.get_scrollpos()) + owner.get_h()) {
		scrollbar.set_scrollpos(top - owner.get_h() + 2 * lineheight);
	}
}

/**
 * Callback function called by the scrollbar.
 */
void MultilineEditbox::scrollpos_changed(int32_t) {
}

/**
 * Re-wrap the string and update the scrollbar range accordingly.
 */
void MultilineEditbox::Data::refresh_ww() {
	if (int32_t(ww.wrapwidth()) != owner.get_w() - Scrollbar::kSize)
		ww_valid = false;
	if (ww_valid)
		return;

	ww.set_style(textstyle);
	ww.set_wrapwidth(owner.get_w() - Scrollbar::kSize);

	ww.wrap(text);
	ww_valid = true;

	int32_t textheight = ww.height();
	scrollbar.set_steps(textheight - owner.get_h());
}

}  // namespace UI
