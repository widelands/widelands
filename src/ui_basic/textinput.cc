/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_basic/textinput.h"

#include <algorithm>

#include "base/utf8.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "graphic/wordwrap.h"
#include "ui_basic/mouse_constants.h"
#include "ui_basic/scrollbar.h"
#include "wlapplication_options.h"

// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace UI {

struct AbstractTextInputPanel::Data {
	Scrollbar scrollbar;

	/// The text in the edit box
	std::string text;

	/// Background color and texture + font style
	PanelStyle style;
	inline const UI::TextPanelStyleInfo& get_style() const {
		return g_style_manager->editbox_style(style);
	}

	/// Position of the cursor inside the text.
	/// 0 indicates that the cursor is before the first character,
	/// text.size() inidicates that the cursor is after the last character.
	uint32_t cursor_pos{0U};
	std::string caret_image_path;

	uint32_t scrolloffset{0U};

	/// Maximum length of the text string, in bytes
	const uint32_t maxbytes;

	enum class Mode { kNormal, kSelection };

	Mode mode{Mode::kNormal};
	bool password{false};

	uint32_t selection_start{0U};
	uint32_t selection_end{0U};

	/// Cached wrapping info; see @ref refresh_ww and @ref update
	/*@{*/
	bool ww_valid{false};
	WordWrap ww;
	/*@}*/

	explicit Data(AbstractTextInputPanel& init_owner);
	void refresh_ww();

	void update();

	void set_cursor_pos(uint32_t newpos);

	uint32_t prev_char(uint32_t cursor);
	uint32_t next_char(uint32_t cursor);
	uint32_t snap_to_char(uint32_t cursor);
	std::pair<uint32_t, uint32_t> word_boundary(uint32_t cursor, bool require_non_blank);
	std::pair<uint32_t, uint32_t> paragraph_boundary(uint32_t cursor);

	void erase_bytes(uint32_t start, uint32_t end);
	void insert(uint32_t where, const std::string& s);
	void reset_selection();
	void draw(RenderTarget& dst, bool with_caret);

	void calculate_selection_boundaries(uint32_t& start, uint32_t& end);

	uint32_t snap_to_char(const std::string& txt, uint32_t cursor);

private:
	AbstractTextInputPanel& owner;
};

/**
 * Initialize an editbox that supports multiline strings.
 */
AbstractTextInputPanel::AbstractTextInputPanel(
   Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, const UI::PanelStyle style)
   : Panel(parent, style, x, y, w, h), d_(new Data(*this)) {
	set_handle_mouse(true);
	set_can_focus(true);
	set_thinks(false);
	set_handle_textinput();
}

AbstractTextInputPanel::~AbstractTextInputPanel() {  // NOLINT
	// Must be here because `Data` is an incomplete type in the header.
}

AbstractTextInputPanel::Data::Data(AbstractTextInputPanel& init_owner)
   : scrollbar(&init_owner,
               init_owner.get_w() - Scrollbar::kSize,
               0,
               Scrollbar::kSize,
               init_owner.get_h(),
               init_owner.panel_style_),
     style(init_owner.panel_style_),

     caret_image_path(init_owner.panel_style_ == PanelStyle::kWui ?
                         "images/ui_basic/caret_wui.png" :
                         "images/ui_basic/caret_fs.png"),
     maxbytes(std::min(g_gr->max_texture_size_for_font_rendering() *
                          g_gr->max_texture_size_for_font_rendering() /
                          (text_height(get_style().font()) * text_height(get_style().font())),
                       std::numeric_limits<int32_t>::max())),

     ww(get_style().font().size(), get_style().font().color(), init_owner.get_w()),
     owner(init_owner) {
	scrollbar.moved.connect([&init_owner](int32_t a) { init_owner.scrollpos_changed(a); });

	scrollbar.set_pagesize(owner.get_h() - 2 * ww.lineheight());
	scrollbar.set_singlestepsize(ww.lineheight());
}

EditBox::EditBox(Panel* parent, int32_t x, int32_t y, uint32_t w, UI::PanelStyle style)
   : AbstractTextInputPanel(parent,
                            x,
                            y,
                            w,
                            text_height(g_style_manager->editbox_style(style).font()) +
                               2 * g_style_manager->editbox_style(style).background().margin(),
                            style) {
	d_->scrollbar.set_visible(false);
}

MultilineEditbox::MultilineEditbox(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style)
   : AbstractTextInputPanel(parent, x, y, w, h, style) {
}

/**
 * Call this function whenever some part of the data changes that potentially
 * requires some redrawing.
 */
void AbstractTextInputPanel::Data::update() {
	ww_valid = false;
}

void AbstractTextInputPanel::Data::reset_selection() {
	mode = Mode::kNormal;
	selection_start = cursor_pos;
	selection_end = cursor_pos;
}
void AbstractTextInputPanel::Data::calculate_selection_boundaries(uint32_t& start, uint32_t& end) {
	start = snap_to_char(std::min(selection_start, selection_end));
	end = std::max(selection_start, selection_end);
	if (end < text.size()) {
		end = Utf8::is_utf8_extended(text[end]) ? next_char(end) : snap_to_char(end);
	}
}

void AbstractTextInputPanel::Data::draw(RenderTarget& dst, bool with_caret) {
	uint32_t start;
	uint32_t end;
	calculate_selection_boundaries(start, end);
	int margin = get_style().background().margin();
	ww.draw(dst, Vector2i(margin - scrolloffset, margin - scrollbar.get_scrollpos()),
	        UI::Align::kLeft, with_caret ? cursor_pos : std::numeric_limits<uint32_t>::max(),
	        with_caret && mode == Data::Mode::kSelection,
	        owner.should_expand_selection() ?
              std::optional<std::pair<int32_t, int32_t>>(std::make_pair(0, owner.get_h())) :
              std::nullopt,
	        start, end, scrollbar.get_scrollpos(), caret_image_path);
}

void AbstractTextInputPanel::layout() {
	Panel::layout();
	d_->scrollbar.set_pos(Vector2i(get_w() - Scrollbar::kSize, 0));
}

/**
 * Return the text currently stored by the editbox.
 */
const std::string& AbstractTextInputPanel::get_text() const {
	return d_->text;
}

/**
 * Replace the currently stored text with something else.
 */
void AbstractTextInputPanel::set_text(const std::string& text) {
	if (text == d_->text) {
		return;
	}

	d_->reset_selection();
	d_->text = text;

	escape_illegal_characters();

	d_->set_cursor_pos(d_->text.size());
	d_->update();
	scroll_cursor_into_view();

	changed();
}

/**
 * Erase the given range of bytes, adjust the cursor position, and update.
 */
void AbstractTextInputPanel::Data::erase_bytes(uint32_t start, uint32_t end) {
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
uint32_t AbstractTextInputPanel::Data::prev_char(uint32_t cursor) {
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
uint32_t AbstractTextInputPanel::Data::next_char(uint32_t cursor) {

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
uint32_t AbstractTextInputPanel::Data::snap_to_char(uint32_t cursor) {
	if (cursor >= text.size()) {
		return cursor;
	}

	while (cursor > 0 && Utf8::is_utf8_extended(text[cursor])) {
		--cursor;
	}
	return cursor;
}

uint32_t AbstractTextInputPanel::Data::snap_to_char(const std::string& txt, uint32_t cursor) {
	if (cursor >= txt.size()) {
		return cursor;
	}

	while (cursor > 0 && Utf8::is_utf8_extended(txt[cursor])) {
		--cursor;
	}
	return cursor;
}

std::pair<uint32_t, uint32_t> AbstractTextInputPanel::Data::word_boundary(uint32_t cursor,
                                                                          bool require_non_blank) {
	uint32_t start = snap_to_char(cursor);
	uint32_t end = start;

	bool found_non_blank = false;
	while (start > 0) {
		uint32_t newpos = prev_char(start);
		if (isspace(text[newpos]) != 0) {
			if (!require_non_blank || found_non_blank) {
				break;
			}
		} else {
			found_non_blank = true;
		}

		start = newpos;
	}

	found_non_blank = false;
	while (end < text.size()) {
		if (isspace(text[end]) != 0) {
			if (!require_non_blank || found_non_blank) {
				break;
			}
		} else {
			found_non_blank = true;
		}

		end = next_char(end);
	}

	return {start, end};
}

std::pair<uint32_t, uint32_t> AbstractTextInputPanel::Data::paragraph_boundary(uint32_t cursor) {
	uint32_t start = snap_to_char(cursor);
	uint32_t end = start;

	while (start > 0 && '\n' != text[prev_char(start)]) {
		start = prev_char(start);
	}

	while (end < text.size() && '\n' != text[end]) {
		end = next_char(end);
	}

	return {start, end};
}

/**
 * The mouse was clicked on this editbox
 */
bool AbstractTextInputPanel::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		const uint32_t time = SDL_GetTicks();
		const bool is_multiclick = (time - multiclick_timer_) < DOUBLE_CLICK_INTERVAL;
		multiclick_timer_ = time;

		set_caret_to_cursor_pos(x, y);

		if (is_multiclick) {
			++multiclick_counter_;
			d_->mode = Data::Mode::kSelection;

			std::pair<uint32_t, uint32_t> boundaries;
			if ((multiclick_counter_ % 2) != 0) {  // Select current word
				boundaries = d_->word_boundary(d_->cursor_pos, false);
			} else {  // Select entire paragraph
				boundaries = d_->paragraph_boundary(d_->cursor_pos);
			}
			d_->selection_start = boundaries.first;
			d_->selection_end = boundaries.second;

			update_primary_selection_buffer();
		} else {
			multiclick_counter_ = 0;
			d_->reset_selection();
		}

		focus();
		clicked();
		return true;
	}
#if HAS_PRIMARY_SELECTION_BUFFER
	else if (btn == SDL_BUTTON_MIDDLE) {
		/* Primary buffer is inserted without affecting cursor position, selection, and focus. */
		const uint32_t old_caret_pos = d_->cursor_pos;
		const uint32_t old_selection_start = d_->selection_start;
		const uint32_t old_selection_end = d_->selection_end;
		const AbstractTextInputPanel::Data::Mode old_mode = d_->mode;

		d_->reset_selection();
		set_caret_to_cursor_pos(x, y);
		const uint32_t new_caret_pos = d_->cursor_pos;

		std::string text_to_insert = SDL_GetPrimarySelectionText();

		if (old_mode == AbstractTextInputPanel::Data::Mode::kSelection &&
		    ((old_selection_start <= new_caret_pos && new_caret_pos <= old_selection_end) ||
		     (old_selection_end <= new_caret_pos && new_caret_pos <= old_selection_start))) {
			text_to_insert.clear();  // Can't paste into the active selection.
		} else {
			std::string old_text = d_->text;
			handle_textinput(text_to_insert);
			if (old_text == d_->text) {
				text_to_insert.clear();  // Text wasn't pasted, perhaps too long.
			}
		}

		d_->mode = old_mode;
		d_->cursor_pos = old_caret_pos;
		d_->selection_start = old_selection_start;
		d_->selection_end = old_selection_end;
		if (new_caret_pos <= old_caret_pos) {
			const uint32_t delta = text_to_insert.size();
			d_->cursor_pos += delta;
			d_->selection_start += delta;
			d_->selection_end += delta;
		}

		changed();
		return true;
	}
#endif

	return false;
}
bool AbstractTextInputPanel::handle_mousemove(
   uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) {
	// state != 0 -> mouse button is pressed
	if ((state != 0u) && get_can_focus()) {
		select_until(d_->cursor_pos);
		set_caret_to_cursor_pos(x, y);
		select_until(d_->cursor_pos);
		return true;
	}

	return Panel::handle_mousemove(state, x, y, xdiff, ydiff);
}

bool AbstractTextInputPanel::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	return d_->scrollbar.handle_mousewheel(x, y, modstate);
}

void AbstractTextInputPanel::set_caret_to_cursor_pos(int32_t x, int32_t y) {
	y += d_->scrollbar.get_scrollpos();

	unsigned previous_line_index = d_->ww.offset_of_line_at(y);

	std::string line = d_->ww.text_of_line_at(y);
	int total_line_width = d_->ww.text_width_of(line);

	if (x > total_line_width) {
		// clicked end of line
		d_->set_cursor_pos(previous_line_index + line.size());
		return;
	}
	int current_line_index = approximate_cursor(line, x, 0);

	d_->set_cursor_pos(previous_line_index + current_line_index);
}

int AbstractTextInputPanel::approximate_cursor(const std::string& line,
                                               int32_t cursor_pos_x,
                                               int approx_caret_pos) const {
	static constexpr int error = 4;

	// approximate using the first guess as start and increasing/decreasing text until error is small
	int text_w = calculate_text_width(line, approx_caret_pos);
	if (cursor_pos_x > text_w) {
		while (cursor_pos_x - text_w > error) {
			text_w = calculate_text_width(line, ++approx_caret_pos);
		}
	} else if (cursor_pos_x < text_w) {
		while (text_w - cursor_pos_x > error) {
			text_w = calculate_text_width(line, --approx_caret_pos);
		}
	}
	return d_->snap_to_char(line, approx_caret_pos);
}

int AbstractTextInputPanel::calculate_text_width(const std::string& text, int pos) const {
	std::string prefix = text.substr(0, d_->snap_to_char(text, pos));
	return d_->ww.text_width_of(prefix);
}

bool AbstractTextInputPanel::has_selection() const {
	return (d_->mode == Data::Mode::kSelection);
}

std::string AbstractTextInputPanel::get_selected_text() {
	assert(d_->mode == Data::Mode::kSelection);
	uint32_t start;
	uint32_t end;
	d_->calculate_selection_boundaries(start, end);
	return d_->text.substr(start, end - start);
}

void AbstractTextInputPanel::replace_selected_text(const std::string& text) {
	assert(d_->mode == Data::Mode::kSelection);
	uint32_t start;
	uint32_t end;
	d_->calculate_selection_boundaries(start, end);
	std::string str = d_->text.substr(0, start);
	str += text;
	str += d_->text.substr(end);
	set_text(str);
	set_caret_pos(start);
	select_until(get_caret_pos() + text.size());
}

size_t AbstractTextInputPanel::get_caret_pos() const {
	return d_->cursor_pos;
}

void AbstractTextInputPanel::set_caret_pos(const size_t caret) const {
	d_->set_cursor_pos(d_->snap_to_char(caret));
}

/**
 * This is called by the UI code whenever a key press or release arrives
 */
bool AbstractTextInputPanel::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		if (matches_shortcut(KeyboardShortcut::kCommonTextPaste, code) &&
		    (SDL_HasClipboardText() != 0u)) {
			if (d_->mode == Data::Mode::kSelection) {
				delete_selected_text();
			}
			handle_textinput(SDL_GetClipboardText());
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kCommonTextCopy, code) &&
		    d_->mode == Data::Mode::kSelection) {
			copy_selected_text();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kCommonTextCut, code) &&
		    d_->mode == Data::Mode::kSelection) {
			copy_selected_text();
			delete_selected_text();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kCommonSelectAll, code)) {
			d_->selection_start = 0;
			d_->selection_end = d_->text.size();
			d_->mode = Data::Mode::kSelection;
			update_primary_selection_buffer();
			return true;
		}

		switch (code.sym) {
		case SDLK_TAB:
			// Let the panel handle the tab key
			return get_parent()->handle_key(true, code);
		case SDLK_DELETE:
			if (d_->mode == Data::Mode::kSelection) {
				delete_selected_text();
			} else if (d_->cursor_pos < d_->text.size()) {
				if ((code.mod & KMOD_CTRL) != 0) {
					d_->erase_bytes(d_->cursor_pos, d_->word_boundary(d_->cursor_pos, true).second);
				} else {
					d_->erase_bytes(d_->cursor_pos, d_->next_char(d_->cursor_pos));
				}
				changed();
			}
			break;

		case SDLK_BACKSPACE:
			if (d_->mode == Data::Mode::kSelection) {
				delete_selected_text();
			} else if (d_->cursor_pos > 0) {
				if ((code.mod & KMOD_CTRL) != 0) {
					d_->erase_bytes(d_->word_boundary(d_->cursor_pos, true).first, d_->cursor_pos);
				} else {
					d_->erase_bytes(d_->prev_char(d_->cursor_pos), d_->cursor_pos);
				}
				changed();
			}
			break;

		case SDLK_LEFT: {
			if (d_->cursor_pos > 0) {
				d_->ww.enter_cursor_movement_mode();
				if ((code.mod & KMOD_CTRL) != 0) {
					uint32_t newpos = d_->prev_char(d_->cursor_pos);
					while (newpos > 0 && (isspace(d_->text[newpos]) != 0)) {
						newpos = d_->prev_char(newpos);
					}
					while (newpos > 0) {
						uint32_t prev = d_->prev_char(newpos);
						if (isspace(d_->text[prev]) != 0) {
							break;
						}
						newpos = prev;
					}
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if ((code.mod & KMOD_SHIFT) != 0) {
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
				d_->ww.enter_cursor_movement_mode();
				if ((code.mod & KMOD_CTRL) != 0) {
					uint32_t newpos = d_->next_char(d_->cursor_pos);
					while (newpos < d_->text.size() && (isspace(d_->text[newpos]) != 0)) {
						newpos = d_->next_char(newpos);
					}
					while (newpos < d_->text.size() && (isspace(d_->text[newpos]) == 0)) {
						newpos = d_->next_char(newpos);
					}
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if ((code.mod & KMOD_SHIFT) != 0) {
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
				d_->ww.enter_cursor_movement_mode();
				d_->refresh_ww();

				uint32_t cursorline;
				uint32_t cursorpos = 0;
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
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if ((code.mod & KMOD_SHIFT) != 0) {
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
				d_->ww.enter_cursor_movement_mode();
				d_->refresh_ww();

				uint32_t cursorline;
				uint32_t cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline > 0) {
					uint32_t newpos = d_->ww.line_offset(cursorline - 1) + cursorpos;
					uint32_t lineend = d_->prev_char(d_->ww.line_offset(cursorline));

					if (newpos > lineend) {
						newpos = lineend;
					} else {
						newpos = d_->snap_to_char(newpos);
					}
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(newpos);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(newpos);
				} else {
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(0);
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(0);
				}
			}
			break;

		case SDLK_HOME:
			if ((code.mod & KMOD_CTRL) != 0) {
				if ((code.mod & KMOD_SHIFT) != 0) {
					select_until(0);
				} else {
					d_->reset_selection();
				}
				d_->set_cursor_pos(0);
			} else {
				d_->refresh_ww();
				uint32_t cursorline;
				uint32_t cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if ((code.mod & KMOD_SHIFT) != 0) {
					select_until(d_->ww.line_offset(cursorline));
				} else {
					d_->reset_selection();
				}
				d_->set_cursor_pos(d_->ww.line_offset(cursorline));
			}
			break;

		case SDLK_END:
			if ((code.mod & KMOD_CTRL) != 0) {
				if ((code.mod & KMOD_SHIFT) != 0) {
					select_until(d_->text.size());
				} else {
					d_->reset_selection();
				}
				d_->set_cursor_pos(d_->text.size());
			} else {
				d_->refresh_ww();

				uint32_t cursorline;
				uint32_t cursorpos = 0;
				d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

				if (cursorline + 1 < d_->ww.nrlines()) {
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(d_->prev_char(d_->ww.line_offset(cursorline + 1)));
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->prev_char(d_->ww.line_offset(cursorline + 1)));
				} else {
					if ((code.mod & KMOD_SHIFT) != 0) {
						select_until(d_->text.size());
					} else {
						d_->reset_selection();
					}
					d_->set_cursor_pos(d_->text.size());
				}
			}
			break;

		case SDLK_ESCAPE:
			cancel();
			return true;

		case SDLK_RETURN:
			if ((code.mod & KMOD_CTRL) != 0) {
				return false;
			}
			d_->insert(d_->cursor_pos, "\n");
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

bool EditBox::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			if ((code.mod & KMOD_CTRL) != 0) {
				return false;
			}

			// Save history if active and text is not empty
			if (history_active_ && !d_->text.empty()) {
				for (unsigned i = kHistorySize - 1; i > 0; --i) {
					history_[i] = history_[i - 1];
				}
				history_[0] = d_->text;
				history_position_ = -1;
			}

			ok();
			return true;

		case SDLK_UP:
			// Load entry from history if active and text is not empty
			if (history_active_) {
				if (history_position_ > static_cast<int>(kHistorySize) - 2) {
					history_position_ = kHistorySize - 2;
				}
				if (!history_[++history_position_].empty()) {
					d_->text = history_[history_position_];
					set_caret_pos(d_->text.size());
					d_->reset_selection();
					changed();
					d_->update();
				}
			}
			return true;

		case SDLK_DOWN:
			// Load entry from history if active and text is not equivalent to the current one
			if (history_active_) {
				if (history_position_ < 1) {
					history_position_ = 1;
				}
				if (history_[--history_position_] != d_->text) {
					d_->text = history_[history_position_];
					set_caret_pos(d_->text.size());
					d_->reset_selection();
					changed();
					d_->update();
				}
			}
			return true;

		default:
			break;
		}
	}

	return AbstractTextInputPanel::handle_key(down, code);
}

void AbstractTextInputPanel::copy_selected_text() const {
	uint32_t start;
	uint32_t end;
	d_->calculate_selection_boundaries(start, end);

	auto nr_characters = end - start;
	std::string selected_text = d_->text.substr(start, nr_characters);

	SDL_SetClipboardText(selected_text.c_str());
}
void AbstractTextInputPanel::update_primary_selection_buffer() const {
#if HAS_PRIMARY_SELECTION_BUFFER
	uint32_t start;
	uint32_t end;
	d_->calculate_selection_boundaries(start, end);

	auto nr_characters = end - start;
	std::string selected_text = d_->text.substr(start, nr_characters);
	SDL_SetPrimarySelectionText(selected_text.c_str());
#endif
}

/**
 * Selects text from @p cursor until @p end
 */
void AbstractTextInputPanel::select_until(uint32_t end) const {
	if (d_->mode == Data::Mode::kNormal) {
		d_->selection_start = d_->cursor_pos;
		d_->mode = Data::Mode::kSelection;
	}
	d_->selection_end = end;
	update_primary_selection_buffer();
}
void AbstractTextInputPanel::delete_selected_text() const {
	uint32_t start;
	uint32_t end;
	d_->calculate_selection_boundaries(start, end);
	d_->erase_bytes(start, end);
	changed();
	d_->reset_selection();
}

bool AbstractTextInputPanel::handle_textinput(const std::string& input_text) {
	if (d_->mode == Data::Mode::kSelection) {
		delete_selected_text();
	}
	if (d_->text.size() + input_text.size() <= d_->maxbytes) {
		d_->insert(d_->cursor_pos, input_text);
		d_->reset_selection();
		changed();
	}
	return true;
}

/**
 * Grab the focus and redraw.
 */
void AbstractTextInputPanel::focus(bool topcaller) {
	Panel::focus(topcaller);
	d_->ww.focus();
}

/**
 * Redraw the Editbox
 */
void AbstractTextInputPanel::draw(RenderTarget& dst) {
	draw_background(dst, d_->get_style().background());

	// Draw border.
	if (get_w() >= 4 && get_h() >= 4 && !warning_) {
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
	} else {
		// Draw a red border for warnings.
		static const RGBColor red(255, 22, 22);

		// bottom edge
		dst.fill_rect(Recti(0, get_h() - 2, get_w(), 2), red);
		// right edge
		dst.fill_rect(Recti(get_w() - 2, 0, 2, get_h() - 2), red);
		// top edge
		dst.fill_rect(Recti(0, 0, get_w() - 1, 1), red);
		dst.fill_rect(Recti(0, 1, get_w() - 2, 1), red);
		dst.brighten_rect(Recti(0, 0, get_w() - 1, 1), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect(Recti(0, 1, get_w() - 2, 1), BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dst.fill_rect(Recti(0, 0, 1, get_h() - 1), red);
		dst.fill_rect(Recti(1, 0, 1, get_h() - 2), red);
		dst.brighten_rect(Recti(0, 0, 1, get_h() - 1), BUTTON_EDGE_BRIGHT_FACTOR);
		dst.brighten_rect(Recti(1, 0, 1, get_h() - 2), BUTTON_EDGE_BRIGHT_FACTOR);
	}

	if (has_focus()) {
		dst.brighten_rect(Recti(0, 0, get_w(), get_h()), MOUSE_OVER_BRIGHT_FACTOR);
	}

	d_->refresh_ww();

	const bool draw_caret = has_focus() && has_top_level_focus();
	d_->ww.set_draw_caret(draw_caret);
	d_->draw(dst, draw_caret);
}

/**
 * Insert the given string starting at cursor position @p where.
 * Update the cursor so that it stays in the same place, but note that the cursor is
 * "right-magnetic":
 * If @p where is equal to the current cursor position, then the cursor is moved.
 * This is usually what one wants.
 */
void AbstractTextInputPanel::Data::insert(uint32_t where, const std::string& s) {
	text.insert(where, s);
	owner.escape_illegal_characters();

	if (cursor_pos >= where) {
		set_cursor_pos(cursor_pos + s.size());
	}

	update();
	owner.scroll_cursor_into_view();
}

/**
 * Change the position of the cursor, cause a display refresh and scroll the cursor
 * into view when necessary.
 */
void AbstractTextInputPanel::Data::set_cursor_pos(uint32_t newpos) {
	assert(newpos <= text.size());

	if (cursor_pos == newpos) {
		return;
	}

	cursor_pos = newpos;

	owner.scroll_cursor_into_view();
}

/**
 * Ensure that the cursor is visible.
 */
void AbstractTextInputPanel::scroll_cursor_into_view() {
	const int margin = d_->get_style().background().margin();
	if (get_inner_h() <= 2 * margin) {
		return;  // Height not yet initialized.
	}

	d_->refresh_ww();

	uint32_t cursorline;
	uint32_t cursorpos = 0;
	d_->ww.calc_wrapped_pos(d_->cursor_pos, cursorline, cursorpos);

	int32_t top = cursorline * d_->ww.lineheight();
	int32_t bottom = top + d_->ww.lineheight();

	if (top - static_cast<int32_t>(d_->scrollbar.get_scrollpos()) < 0) {
		d_->scrollbar.set_scrollpos(top);
	} else if (bottom - static_cast<int32_t>(d_->scrollbar.get_scrollpos()) > get_h() - 2 * margin) {
		d_->scrollbar.set_scrollpos(bottom - get_h() + 2 * margin);
	}
}

void EditBox::scroll_cursor_into_view() {
	AbstractTextInputPanel::scroll_cursor_into_view();
	const int margin = d_->get_style().background().margin();

	if (get_inner_w() <= 2 * margin) {
		return;  // Width not yet initialized.
	}

	int32_t real_caret_x = calculate_text_width(get_text(), get_caret_pos());

	if (real_caret_x - static_cast<int32_t>(d_->scrolloffset) < 0) {
		d_->scrolloffset = real_caret_x;
	} else if (real_caret_x - static_cast<int32_t>(d_->scrolloffset) > get_w() - 2 * margin) {
		d_->scrolloffset = real_caret_x - get_w() + 2 * margin;
	}
}

/**
 * Callback function called by the scrollbar.
 */
void AbstractTextInputPanel::scrollpos_changed(int32_t /* pos */) {
}

void AbstractTextInputPanel::set_password(bool password) {
	d_->password = password;
}
bool AbstractTextInputPanel::is_password() const {
	return d_->password;
}

void AbstractTextInputPanel::escape_illegal_characters() const {
	while (d_->text.size() > d_->maxbytes) {
		d_->erase_bytes(d_->prev_char(d_->text.size()), d_->text.size());
	}
}
void EditBox::escape_illegal_characters() const {
	std::replace(d_->text.begin(), d_->text.end(), '\n', ' ');

	AbstractTextInputPanel::escape_illegal_characters();
}

uint32_t AbstractTextInputPanel::max_text_width_for_wrap() const {
	return get_w() > Scrollbar::kSize ? get_w() - Scrollbar::kSize : 0U;
}
uint32_t EditBox::max_text_width_for_wrap() const {
	return std::numeric_limits<uint32_t>::max();
}

/**
 * Re-wrap the string and update the scrollbar range accordingly.
 */
void AbstractTextInputPanel::Data::refresh_ww() {
	const uint32_t new_wrap_width = owner.max_text_width_for_wrap();
	ww_valid &= ww.wrapwidth() == new_wrap_width;
	if (ww_valid) {
		return;
	}

	ww.set_wrapwidth(new_wrap_width);

	if (password) {
		ww.wrap(std::string(text.size(), '*'));
	} else {
		ww.wrap(text);
	}
	ww_valid = true;

	int32_t textheight = ww.height();
	scrollbar.set_steps(textheight - owner.get_h() + 2 * get_style().background().margin());
}

}  // namespace UI
