/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/utf8.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "graphic/wordwrap.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"
namespace {
bool inline copy_paste_modifier() {
#ifdef __APPLE__
	return SDL_GetModState() & KMOD_GUI;
#endif
	return SDL_GetModState() & KMOD_CTRL;
}
}  // namespace
// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace UI {

struct MultilineEditbox::Data {
	Scrollbar scrollbar;

	/// The text in the edit box
	std::string text;

	/// Background color and texture + font style
	const UI::TextPanelStyleInfo& style;

	/// Position of the cursor inside the text.
	/// 0 indicates that the cursor is before the first character,
	/// text.size() inidicates that the cursor is after the last character.
	uint32_t cursor_pos;
	std::string caret_image_path;

	const int lineheight;

	/// Maximum length of the text string, in bytes
	const uint32_t maxbytes;

	enum class Mode { kNormal, kSelection };

	Mode mode;

	uint32_t selection_start;

	uint32_t selection_end;

	/// Cached wrapping info; see @ref refresh_ww and @ref update
	/*@{*/
	bool ww_valid;
	WordWrap ww;
	/*@}*/

	Data(MultilineEditbox&, const TextPanelStyleInfo& style);
	void refresh_ww();

	void update();

	void scroll_cursor_into_view();
	void set_cursor_pos(uint32_t cursor_pos);

	uint32_t prev_char(uint32_t cursor);
	uint32_t next_char(uint32_t cursor);
	uint32_t snap_to_char(uint32_t cursor);

	void erase_bytes(uint32_t start, uint32_t end);
	void insert(uint32_t where, const std::string& s);
	void reset_selection();
	void draw(RenderTarget& dst, bool with_caret);

	void calculate_selection_boundaries(uint32_t& start, uint32_t& end);

private:
	MultilineEditbox& owner;
};

/**
 * Initialize an editbox that supports multiline strings.
 */
MultilineEditbox::MultilineEditbox(
   Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style)
   : Panel(parent, style, x, y, w, h), d_(new Data(*this, g_style_manager->editbox_style(style))) {
	set_handle_mouse(true);
	set_can_focus(true);
	set_thinks(false);
	set_handle_textinput();
}

MultilineEditbox::Data::Data(MultilineEditbox& o, const UI::TextPanelStyleInfo& init_style)
   : scrollbar(&o, o.get_w() - Scrollbar::kSize, 0, Scrollbar::kSize, o.get_h(), o.panel_style_),
     style(init_style),
     cursor_pos(0),
     caret_image_path(o.panel_style_ == PanelStyle::kWui ? "images/ui_basic/caret_wui.png" :
                                                           "images/ui_basic/caret_fs.png"),
     lineheight(text_height(style.font())),
     maxbytes(std::min(g_gr->max_texture_size_for_font_rendering() *
                          g_gr->max_texture_size_for_font_rendering() /
                          (text_height(style.font()) * text_height(style.font())),
                       std::numeric_limits<int32_t>::max())),
     mode(Mode::kNormal),
     selection_start(0),
     selection_end(0),
     ww_valid(false),
     ww(style.font().size(), style.font().color(), o.get_w()),
     owner(o) {
	scrollbar.moved.connect([&o](int32_t a) { o.scrollpos_changed(a); });

	scrollbar.set_pagesize(owner.get_h() - 2 * lineheight);
	scrollbar.set_singlestepsize(lineheight);
}

/**
 * Call this function whenever some part of the data changes that potentially
 * requires some redrawing.
 */
void MultilineEditbox::Data::update() {
	ww_valid = false;
}

void MultilineEditbox::Data::reset_selection() {
	mode = Mode::kNormal;
	selection_start = cursor_pos;
	selection_end = cursor_pos;
}
void MultilineEditbox::Data::calculate_selection_boundaries(uint32_t& start, uint32_t& end) {
	start = snap_to_char(std::min(selection_start, selection_end));
	end = std::max(selection_start, selection_end);
	end = Utf8::is_utf8_extended(text[end]) ? next_char(end) : snap_to_char(end);
}

void MultilineEditbox::Data::draw(RenderTarget& dst, bool with_caret) {
	uint32_t start, end;
	calculate_selection_boundaries(start, end);
	ww.draw(dst, Vector2i(0, -int32_t(scrollbar.get_scrollpos())), UI::Align::kLeft,
	        with_caret ? cursor_pos : std::numeric_limits<uint32_t>::max(),
	        mode == Data::Mode::kSelection, start, end, scrollbar.get_scrollpos(), caret_image_path);
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
	if (text == d_->text) {
		return;
	}

	d_->text = text;
	while (d_->text.size() > d_->maxbytes) {
		d_->erase_bytes(d_->prev_char(d_->text.size()), d_->text.size());
	}

	d_->set_cursor_pos(0);
	d_->update();
	d_->scroll_cursor_into_view();

	changed();
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

	if (cursor_pos >= end) {
		set_cursor_pos(cursor_pos - nbytes);
	} else if (cursor_pos > start) {
		set_cursor_pos(start);
	}
}

/**
 * Find the starting byte of the previous character
 */
uint32_t MultilineEditbox::Data::prev_char(uint32_t cursor) {
	assert(cursor <= text.size());

	if (cursor == 0) {
		return cursor;
	}

	do {
		--cursor;
		// TODO(GunChleoc): See if we can go full ICU here.
	} while (cursor > 0 && Utf8::is_utf8_extended(text[cursor]));

	return cursor;
}

/**
 * Find the starting byte of the next character
 */
uint32_t MultilineEditbox::Data::next_char(uint32_t cursor) {
	assert(cursor <= text.size());

	if (cursor >= text.size()) {
		return cursor;
	}

	do {
		++cursor;
	} while (cursor < text.size() && Utf8::is_utf8_extended(text[cursor]));

	return cursor;
}

/**
 * Return the starting offset of the (multi-byte) character that @p cursor points to.
 */
uint32_t MultilineEditbox::Data::snap_to_char(uint32_t cursor) {
	while (cursor > 0 && Utf8::is_utf8_extended(text[cursor])) {
		--cursor;
	}
	return cursor;
}

/**
 * The mouse was clicked on this editbox
 */
bool MultilineEditbox::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		return true;
	}
	return false;
}

/**
 * This is called by the UI code whenever a key press or release arrives
 */
bool MultilineEditbox::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		case SDLK_v:
			if (copy_paste_modifier() && SDL_HasClipboardText()) {
				if (d_->mode == Data::Mode::kSelection) {
					delete_selected_text();
				}
				handle_textinput(SDL_GetClipboardText());
				return true;
			}
			return false;
		case SDLK_c:
			if (copy_paste_modifier() && d_->mode == Data::Mode::kSelection) {
				copy_selected_text();
				return true;
			}
			return false;
		case SDLK_a:
			if (copy_paste_modifier()) {
				d_->selection_start = 0;
				d_->selection_end = d_->text.size();
				d_->mode = Data::Mode::kSelection;
				return true;
			}
			return false;
		case SDLK_x:
			if (copy_paste_modifier() && d_->mode == Data::Mode::kSelection) {
				copy_selected_text();
				delete_selected_text();
				return true;
			}
			return false;
		case SDLK_TAB:
			// Let the panel handle the tab key
			return get_parent()->handle_key(true, code);
		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DELETE:
			if (d_->mode == Data::Mode::kSelection) {
				delete_selected_text();
			} else if (d_->cursor_pos < d_->text.size()) {
				d_->erase_bytes(d_->cursor_pos, d_->next_char(d_->cursor_pos));
				changed();
			}
			break;

		case SDLK_BACKSPACE:
			if (d_->mode == Data::Mode::kSelection) {
				delete_selected_text();
			} else if (d_->cursor_pos > 0) {
				d_->erase_bytes(d_->prev_char(d_->cursor_pos), d_->cursor_pos);
				changed();
			}
			break;

		case SDLK_LEFT: {
			if (d_->cursor_pos > 0) {
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
					uint32_t newpos = d_->prev_char(d_->cursor_pos);
					while (newpos > 0 && isspace(d_->text[newpos])) {
						newpos = d_->prev_char(newpos);
					}
					while (newpos > 0) {
						uint32_t prev = d_->prev_char(newpos);
						if (isspace(d_->text[prev])) {
							break;
						}
						newpos = prev;
					}
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(d_->prev_char(d_->cursor_pos));
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->prev_char(d_->cursor_pos));
				}
			}
			break;
		}

		case SDLK_RIGHT:
			if (d_->cursor_pos < d_->text.size()) {
				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
					uint32_t newpos = d_->next_char(d_->cursor_pos);
					while (newpos < d_->text.size() && isspace(d_->text[newpos])) {
						newpos = d_->next_char(newpos);
					}
					while (newpos < d_->text.size() && !isspace(d_->text[newpos])) {
						newpos = d_->next_char(newpos);
					}
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(d_->next_char(d_->cursor_pos));
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->next_char(d_->cursor_pos));
				}
			}
			break;

		case SDLK_DOWN:
			if (d_->cursor_pos < d_->text.size()) {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline + 1 < d_->ww.nrlines()) {
					uint32_t lineend = d_->text.size();
					if (cursorline + 2 < d_->ww.nrlines()) {
						lineend = d_->prev_char(d_->ww.line_offset(cursorline + 2));
					}

					uint32_t newpos = d_->ww.line_offset(cursorline + 1) + cursorpos;
					if (newpos > lineend) {
						newpos = lineend;
					} else {
						newpos = d_->snap_to_char(newpos);
					}
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(d_->text.size());
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->text.size());
				}
			}
			break;

		case SDLK_UP:
			if (d_->cursor_pos > 0) {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline > 0) {
					uint32_t newpos = d_->ww.line_offset(cursorline - 1) + cursorpos;
					uint32_t lineend = d_->prev_char(d_->ww.line_offset(cursorline));

					if (newpos > lineend) {
						newpos = lineend;
					} else {
						newpos = d_->snap_to_char(newpos);
					}
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(0);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(0);
				}
			}
			break;

		case SDLK_HOME:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				if (SDL_GetModState() & KMOD_SHIFT) {
					select_until(0);
				} else {
					d_->reset_selection();
				}
				d_->set_cursor_pos(0);
			} else {
				d_->refresh_ww();
				uint32_t cursorline, cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (SDL_GetModState() & KMOD_SHIFT) {
					select_until(d_->ww.line_offset(cursorline));
				} else {
					d_->reset_selection();
				}
				d_->set_cursor_pos(d_->ww.line_offset(cursorline));
			}
			break;

		case SDLK_END:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
				if (SDL_GetModState() & KMOD_SHIFT) {
					select_until(d_->text.size());
				} else {
					d_->reset_selection();
				}
				d_->set_cursor_pos(d_->text.size());
			} else {
				d_->refresh_ww();

				uint32_t cursorline, cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline + 1 < d_->ww.nrlines()) {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(d_->prev_char(d_->ww.line_offset(cursorline + 1)));
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->prev_char(d_->ww.line_offset(cursorline + 1)));
				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(d_->text.size());
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->text.size());
				}
			}
			break;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			d_->insert(d_->cursor_pos, "\n");
			d_->update();
			d_->reset_selection();
			changed();
			break;

		default:
			break;
		}
		return true;
	}

	return Panel::handle_key(down, code);
}

void MultilineEditbox::copy_selected_text() const {
	uint32_t start, end;
	d_->calculate_selection_boundaries(start, end);

	auto nr_characters = end - start;
	std::string selected_text = d_->text.substr(start, nr_characters);

	SDL_SetClipboardText(selected_text.c_str());
}
/**
 * Selects text from @p cursor until @p end
 */
void MultilineEditbox::select_until(uint32_t end) const {
	if (d_->mode == Data::Mode::kNormal) {
		d_->selection_start = d_->cursor_pos;
		d_->mode = Data::Mode::kSelection;
	}
	d_->selection_end = end;
}
void MultilineEditbox::delete_selected_text() const {
	uint32_t start, end;
	d_->calculate_selection_boundaries(start, end);
	d_->erase_bytes(start, end);
	changed();
	d_->reset_selection();
}

bool MultilineEditbox::handle_textinput(const std::string& input_text) {
	if (d_->text.size() + input_text.size() <= d_->maxbytes) {
		d_->insert(d_->cursor_pos, input_text);
		d_->reset_selection();
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
	draw_background(dst, d_->style.background());

	// Draw border.
	if (get_w() >= 4 && get_h() >= 4) {
		static const RGBColor black(0, 0, 0);

		// bottom edge
		dst.brighten_rect(Recti(0, get_h() - 2, get_w(), 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dst.brighten_rect(Recti(get_w() - 2, 0, 2, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dst.fill_rect(Recti(0, 0, get_w() - 1, 1), black);
		dst.fill_rect(Recti(0, 1, get_w() - 2, 1), black);
		// left edge
		dst.fill_rect(Recti(0, 0, 1, get_h() - 1), black);
		dst.fill_rect(Recti(1, 0, 1, get_h() - 2), black);
	}

	if (has_focus()) {
		dst.brighten_rect(Recti(0, 0, get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);
	}

	d_->refresh_ww();

	d_->ww.set_draw_caret(has_focus());
	d_->draw(dst, has_focus());
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
	if (cursor_pos >= where) {
		set_cursor_pos(cursor_pos + s.size());
	}
}

/**
 * Change the position of the cursor, cause a display refresh and scroll the cursor
 * into view when necessary.
 */
void MultilineEditbox::Data::set_cursor_pos(uint32_t newpos) {
	assert(newpos <= text.size());

	if (cursor_pos == newpos) {
		return;
	}

	cursor_pos = newpos;

	scroll_cursor_into_view();
}

/**
 * Ensure that the cursor is visible.
 */
void MultilineEditbox::Data::scroll_cursor_into_view() {
	refresh_ww();

	uint32_t cursorline, cursorpos = 0;
	ww.calc_wrapped_pos(cursor_pos, cursorline, cursorpos);

	int32_t top = cursorline * lineheight;
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
	if (int32_t(ww.wrapwidth()) != owner.get_w() - Scrollbar::kSize) {
		ww_valid = false;
	}
	if (ww_valid) {
		return;
	}

	ww.set_wrapwidth(owner.get_w() - Scrollbar::kSize);

	ww.wrap(text);
	ww_valid = true;

	int32_t textheight = ww.height();
	scrollbar.set_steps(textheight - owner.get_h());
}

}  // namespace UI
