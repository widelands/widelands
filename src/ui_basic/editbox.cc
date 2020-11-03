/*
 * Copyright (C) 2003-2020 by the Widelands Development Team
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

#include <memory>

#include <SDL_clipboard.h>
#include <SDL_mouse.h>

#include "base/utf8.h"
#include "graphic/color.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "ui_basic/mouse_constants.h"

// TODO(GunChleoc): Arabic: Fix positioning for Arabic

namespace {

constexpr int kMarginX = 4;
constexpr int kLineMargin = 1;
bool inline copy_paste_modifier() {
#ifdef __APPLE__
	return SDL_GetModState() & KMOD_GUI;
#endif
	return SDL_GetModState() & KMOD_CTRL;
}

}  // namespace

namespace UI {

struct EditBoxImpl {
	explicit EditBoxImpl(const UI::TextPanelStyleInfo& init_style)
	   : background_style(&init_style.background()),
	     font_style(&init_style.font()),
	     margin(init_style.background().margin()),
	     font_scale(1.0f) {
	}

	/// Background color and texture
	const UI::PanelStyleInfo* background_style;

	/// Font style
	const UI::FontStyleInfo* font_style;

	/// Margin around the test
	int margin;

	/// Scale for font size
	float font_scale;

	/// Maximum number of characters in the input
	uint32_t maxLength;

	/// Current text in the box.
	std::string text;

	/// Position of the caret.
	uint32_t caret;

	/// Position of the caret at text selection end.
	uint32_t selection_end;

	/// Initial position of text when selection was started
	uint32_t selection_start;

	enum class Mode { kNormal, kSelection };

	Mode mode;

	/// Current scrolling offset to the text anchor position, in pixels
	int32_t scrolloffset;

	/// Alignment of the text. Vertical alignment is always centered.
	Align align;
};

EditBox::EditBox(Panel* const parent, int32_t x, int32_t y, uint32_t w, UI::PanelStyle style)
   : Panel(parent,
           style,
           x,
           y,
           w,
           text_height(g_style_manager->editbox_style(style).font()) +
              2 * g_style_manager->editbox_style(style).background().margin()),
     m_(new EditBoxImpl(g_style_manager->editbox_style(style))),
     history_active_(false),
     history_position_(-1),
     password_(false),
     warning_(false) {
	set_thinks(false);

	// Set alignment to the UI language's principal writing direction
	m_->align = UI::g_fh->fontset()->is_rtl() ? UI::Align::kRight : UI::Align::kLeft;
	m_->caret = 0;
	m_->mode = EditBoxImpl::Mode::kNormal;
	m_->selection_end = 0;
	m_->selection_start = 0;
	m_->scrolloffset = 0;
	// yes, use *signed* max as maximum length; just a small safe-guard.
	set_max_length(std::numeric_limits<int32_t>::max());

	set_thinks(false);
	set_handle_mouse(true);
	set_can_focus(true);
	set_handle_textinput();
}

EditBox::~EditBox() {  // NOLINT
	                    // place a destructor where the compiler can find the EditBoxImpl destructor
}

/**
 * \return the current text entered in the edit box
 */
const std::string& EditBox::text() const {
	return m_->text;
}

/**
 * Set the current text in the edit box.
 *
 * The text is truncated if it is longer than the maximum length set by
 * \ref set_max_length().
 */
void EditBox::set_text(const std::string& t) {
	if (t == m_->text) {
		return;
	}

	bool caretatend = m_->caret == m_->text.size();

	m_->text = t;
	if (m_->text.size() > m_->maxLength) {
		m_->text.erase(m_->text.begin() + m_->maxLength, m_->text.end());
	}
	if (caretatend || m_->caret > m_->text.size()) {
		m_->caret = m_->text.size();
	}
}

/**
 * Set the maximum length of the input string.
 *
 * If the current string is longer than the new maximum length,
 * its end is cut off to fit into the maximum length.
 */
void EditBox::set_max_length(int const n) {
	m_->maxLength =
	   std::min(g_gr->max_texture_size_for_font_rendering() / text_height(*m_->font_style), n);

	if (m_->text.size() > m_->maxLength) {
		m_->text.erase(m_->text.begin() + m_->maxLength, m_->text.end());
		if (m_->caret > m_->text.size()) {
			m_->caret = m_->text.size();
		}
		check_caret();
	}
}

void EditBox::set_font_scale(float scale) {
	m_->font_scale = scale;
}

void EditBox::set_font_style(const UI::FontStyleInfo& style) {
	m_->font_style = &style;
	const int new_height = text_height(style) + 2 * m_->margin;
	set_size(get_w(), new_height);
	set_desired_size(get_w(), new_height);
}

void EditBox::set_font_style_and_margin(const UI::FontStyleInfo& style, int margin) {
	m_->margin = margin;
	set_font_style(style);
}

/**
 * The mouse was clicked on this editbox
 */
bool EditBox::handle_mousepress(const uint8_t btn, int32_t, int32_t) {
	if (btn == SDL_BUTTON_LEFT && get_can_focus()) {
		focus();
		clicked();
		return true;
	}

	return false;
}

/**
 * Handle keypress/release events
 */
// TODO(unknown): Text input works only because code.unicode happens to map to ASCII for
// ASCII characters (--> // HERE). Instead, all user editable strings should be
// real unicode.
bool EditBox::handle_key(bool const down, SDL_Keysym const code) {
	if (down) {
		switch (code.sym) {
		case SDLK_v:
			if (copy_paste_modifier() && SDL_HasClipboardText()) {
				if (m_->mode == EditBoxImpl::Mode::kSelection) {
					delete_selected_text();
				}
				handle_textinput(SDL_GetClipboardText());
				return true;
			}
			return false;
		case SDLK_c:
			if (copy_paste_modifier() && m_->mode == EditBoxImpl::Mode::kSelection) {
				copy_selected_text();
				return true;
			}
			return false;

		case SDLK_a:
			if (copy_paste_modifier()) {
				m_->selection_start = 0;
				m_->selection_end = m_->text.size();
				m_->mode = EditBoxImpl::Mode::kSelection;
				return true;
			}
			return false;
		case SDLK_x:
			if (copy_paste_modifier() && m_->mode == EditBoxImpl::Mode::kSelection) {
				copy_selected_text();
				delete_selected_text();
			}
			return false;
		case SDLK_ESCAPE:
			cancel();
			return true;

		case SDLK_TAB:
			// Let the panel handle the tab key
			return get_parent()->handle_key(true, code);

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			// Save history if active and text is not empty
			if (history_active_) {
				if (!m_->text.empty()) {
					for (uint8_t i = CHAT_HISTORY_SIZE - 1; i > 0; --i) {
						history_[i] = history_[i - 1];
					}
					history_[0] = m_->text;
					history_position_ = -1;
				}
			}
			ok();
			return true;

		case SDLK_KP_PERIOD:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
		case SDLK_DELETE:
			if (m_->mode == EditBoxImpl::Mode::kSelection) {
				delete_selected_text();
				check_caret();
				changed();
				return true;
			}

			if (m_->caret < m_->text.size()) {
				while ((m_->text[++m_->caret] & 0xc0) == 0x80) {
				}
				// Now fallthrough to handle it like Backspace
			} else {
				return true;
			}
			FALLS_THROUGH;
		case SDLK_BACKSPACE:
			if (m_->mode == EditBoxImpl::Mode::kSelection) {
				delete_selected_text();
				check_caret();
				changed();
				return true;
			}
			if (m_->caret > 0) {
				while ((m_->text[--m_->caret] & 0xc0) == 0x80) {
					m_->text.erase(m_->text.begin() + m_->caret);
				}
				m_->text.erase(m_->text.begin() + m_->caret);
				check_caret();
				reset_selection();
				changed();
			}
			return true;

		case SDLK_LEFT:
			if (m_->caret > 0) {

				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
					uint32_t newpos = prev_char(m_->caret);
					while (newpos > 0 && isspace(m_->text[newpos])) {
						newpos = prev_char(newpos);
					}
					while (newpos > 0) {
						uint32_t prev = prev_char(newpos);
						if (isspace(m_->text[prev])) {
							break;
						}
						newpos = prev;
					}
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(newpos);
					} else {
						reset_selection();
					}
					m_->caret = newpos;

				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(prev_char(m_->caret));
					} else {
						reset_selection();
					}
					m_->caret = prev_char(m_->caret);
				}
				check_caret();
			}
			return true;

		case SDLK_RIGHT:
			if (m_->caret < m_->text.size()) {

				if (code.mod & (KMOD_LCTRL | KMOD_RCTRL)) {
					uint32_t newpos = next_char(m_->caret);
					while (newpos < m_->text.size() && isspace(m_->text[newpos])) {
						newpos = next_char(newpos);
					}
					while (newpos < m_->text.size() && !isspace(m_->text[newpos])) {
						newpos = next_char(newpos);
					}
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(newpos);
					} else {
						reset_selection();
					}
					m_->caret = newpos;

				} else {
					if (SDL_GetModState() & KMOD_SHIFT) {
						select_until(next_char(m_->caret));
					} else {
						reset_selection();
					}
					m_->caret = next_char(m_->caret);
				}
				check_caret();
			}
			return true;

		case SDLK_HOME:
			if (m_->caret > 0) {
				if (SDL_GetModState() & KMOD_SHIFT) {
					select_until(0);
				} else {
					reset_selection();
				}
				m_->caret = 0;
				check_caret();
			}
			return true;

		case SDLK_END:
			if (m_->caret != m_->text.size()) {
				if (SDL_GetModState() & KMOD_SHIFT) {
					select_until(m_->text.size());
				} else {
					reset_selection();
				}
				m_->caret = m_->text.size();
				check_caret();
			}
			return true;

		case SDLK_UP:
			// Load entry from history if active and text is not empty
			if (history_active_) {
				if (history_position_ > CHAT_HISTORY_SIZE - 2) {
					history_position_ = CHAT_HISTORY_SIZE - 2;
				}
				if (!history_[++history_position_].empty()) {
					m_->text = history_[history_position_];
					m_->caret = m_->text.size();
					check_caret();
					reset_selection();
				}
			}
			return true;

		case SDLK_DOWN:
			// Load entry from history if active and text is not equivalent to the current one
			if (history_active_) {
				if (history_position_ < 1) {
					history_position_ = 1;
				}
				if (history_[--history_position_] != m_->text) {
					m_->text = history_[history_position_];
					m_->caret = m_->text.size();
					check_caret();
					reset_selection();
				}
			}
			return true;

		default:
			break;
		}
	}

	return false;
}
void EditBox::copy_selected_text() {
	uint32_t start, end;
	calculate_selection_boundaries(start, end);

	auto nr_characters = end - start;
	std::string selected_text = m_->text.substr(start, nr_characters);
	SDL_SetClipboardText(selected_text.c_str());
}

bool EditBox::handle_textinput(const std::string& input_text) {
	if ((m_->text.size() + input_text.length()) < m_->maxLength) {
		m_->text.insert(m_->caret, input_text);
		m_->caret += input_text.length();
		check_caret();
		reset_selection();
		changed();
	}
	return true;
}

void EditBox::delete_selected_text() {
	uint32_t start, end;
	calculate_selection_boundaries(start, end);
	uint32_t nbytes = end - start;
	m_->text.erase(start, nbytes);
	m_->caret = start;
	reset_selection();
	changed();
}

void EditBox::draw(RenderTarget& dst) {
	draw_background(dst, *m_->background_style);

	// Draw border.
	if (get_w() >= 2 && get_h() >= 2 && !warning_) {
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

	const int max_width = get_w() - 2 * kMarginX;
	FontStyleInfo scaled_style(*m_->font_style);
	scaled_style.set_size(scaled_style.size() * m_->font_scale);
	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_editor_richtext_paragraph(
	      password_ ? text_to_asterisk() : richtext_escape(m_->text), scaled_style));

	const int linewidth = rendered_text->width();
	const int lineheight = m_->text.empty() ? text_height(scaled_style) : rendered_text->height();

	Vector2i point(kMarginX, get_h() / 2);
	if (m_->align == UI::Align::kRight) {
		point.x += max_width - linewidth;
	}
	UI::center_vertically(lineheight, &point);

	// Crop to max_width while blitting
	if (max_width < linewidth || m_->scrolloffset != 0) {
		// Fix positioning for BiDi languages.
		if (UI::g_fh->fontset()->is_rtl()) {
			point.x = 0.f;
		}
		// We want this always on, e.g. for mixed language savegame filenames
		if (i18n::has_rtl_character(m_->text.c_str(), 100)) {  // Restrict check for efficiency
			// TODO(GunChleoc): Arabic: Fix scrolloffset
			rendered_text->draw(dst, point, Recti(linewidth - max_width, 0, linewidth, lineheight));
		} else {
			if (m_->align == UI::Align::kRight) {
				// TODO(GunChleoc): Arabic: Fix scrolloffset
				rendered_text->draw(
				   dst, point, Recti(point.x + m_->scrolloffset + kMarginX, 0, max_width, lineheight));
			} else {
				rendered_text->draw(dst, point, Recti(-m_->scrolloffset, 0, max_width, lineheight));
			}
		}
	} else {
		rendered_text->draw(dst, point, Recti(0, 0, max_width, lineheight));
	}

	if (has_focus()) {
		// Draw the caret
		std::string line_to_caret;

		if (password_) {
			line_to_caret = text_to_asterisk().substr(0, m_->caret);
		} else {
			line_to_caret = m_->text.substr(0, m_->caret);
		}

		// TODO(GunChleoc): Arabic: Fix cursor position for BIDI text.
		int caret_x = text_width(line_to_caret, *m_->font_style, m_->font_scale);

		const uint16_t fontheight = text_height(*m_->font_style, m_->font_scale);

		const Image* caret_image =
		   g_image_cache->get(panel_style_ == PanelStyle::kWui ? "images/ui_basic/caret_wui.png" :
		                                                         "images/ui_basic/caret_fs.png");
		Vector2i caretpt = Vector2i::zero();
		caretpt.x = point.x + m_->scrolloffset + caret_x - caret_image->width() + kLineMargin;
		caretpt.y = point.y + (fontheight - caret_image->height()) / 2;
		dst.blit(caretpt, caret_image);

		if (m_->mode == EditBoxImpl::Mode::kSelection) {
			highlight_selection(dst, point, fontheight);
		}
	}
}

void EditBox::highlight_selection(RenderTarget& dst,
                                  const Vector2i& point,
                                  const uint16_t fontheight) {

	uint32_t start, end;
	calculate_selection_boundaries(start, end);
	auto nr_characters = end - start;

	std::string selected_text = m_->text.substr(start, nr_characters);
	std::string text_before_selection = m_->text.substr(0, start);

	Vector2i selection_start = Vector2i(
	   text_width(text_before_selection, *m_->font_style, m_->font_scale) + point.x, point.y);
	Vector2i selection_end =
	   Vector2i(text_width(selected_text, *m_->font_style, m_->font_scale), fontheight);
	if (m_->scrolloffset != 0) {
		selection_start.x += m_->scrolloffset;
	}
	dst.brighten_rect(
	   Recti(selection_start, selection_end.x, selection_end.y), BUTTON_EDGE_BRIGHT_FACTOR);
}

void EditBox::reset_selection() {
	m_->mode = EditBoxImpl::Mode::kNormal;
	m_->selection_start = m_->caret;
	m_->selection_end = m_->caret;
}

/**
 * Return the starting offset of the (multi-byte) character that @p cursor points to.
 */
uint32_t EditBox::snap_to_char(uint32_t cursor) {
	while (cursor > 0 && Utf8::is_utf8_extended(m_->text[cursor])) {
		--cursor;
	}
	return cursor;
}

/**
 * Find the starting byte of the next character
 */
uint32_t EditBox::next_char(uint32_t cursor) const {
	assert(cursor <= m_->text.size());

	if (cursor >= m_->text.size()) {
		return cursor;
	}

	do {
		++cursor;
	} while (cursor < m_->text.size() && Utf8::is_utf8_extended(m_->text[cursor]));

	return cursor;
}

/**
 * Find the starting byte of the previous character
 */
uint32_t EditBox::prev_char(uint32_t cursor) const {
	assert(cursor <= m_->text.size());

	if (cursor == 0) {
		return cursor;
	}

	do {
		--cursor;
		// TODO(GunChleoc): See if we can go full ICU here.
	} while (cursor > 0 && Utf8::is_utf8_extended(m_->text[cursor]));

	return cursor;
}

/**
 * Selects text from @p cursor until @p end
 */
void EditBox::select_until(uint32_t end) const {
	if (m_->mode == EditBoxImpl::Mode::kNormal) {
		m_->selection_start = m_->caret;
		m_->mode = EditBoxImpl::Mode::kSelection;
	}
	m_->selection_end = end;
}

void EditBox::calculate_selection_boundaries(uint32_t& start, uint32_t& end) {
	start = snap_to_char(std::min(m_->selection_start, m_->selection_end));
	end = std::max(m_->selection_start, m_->selection_end);
	end = Utf8::is_utf8_extended(m_->text[end]) ? next_char(end) : snap_to_char(end);
}

/**
 * Check the caret's position and scroll it into view if necessary.
 */
void EditBox::check_caret() {
	if (m_->caret > m_->text.size()) {
		m_->caret = m_->text.size();
	}
	std::string leftstr(m_->text, 0, m_->caret);
	std::string rightstr(m_->text, m_->caret, std::string::npos);
	int32_t leftw = text_width(leftstr, *m_->font_style, m_->font_scale);
	int32_t rightw = text_width(rightstr, *m_->font_style, m_->font_scale);

	int32_t caretpos = 0;

	switch (m_->align) {
	case UI::Align::kRight:
		caretpos = get_w() - kMarginX + m_->scrolloffset - rightw;
		break;
	case UI::Align::kCenter:
	case UI::Align::kLeft:
		caretpos = kMarginX + m_->scrolloffset + leftw;
	}
	if (caretpos < kMarginX) {
		m_->scrolloffset += kMarginX - caretpos + get_w() / 5;
	} else if (caretpos > get_w() - kMarginX) {
		m_->scrolloffset -= caretpos - get_w() + kMarginX + get_w() / 5;
	}

	if (m_->align == UI::Align::kLeft) {
		if (m_->scrolloffset > 0) {
			m_->scrolloffset = 0;
		}
	} else if (m_->align == UI::Align::kRight) {
		if (m_->scrolloffset < 0) {
			m_->scrolloffset = 0;
		}
	}
}

/**
 * Return text as asterisks.
 */
std::string EditBox::text_to_asterisk() {
	std::string asterisk;
	for (int i = 0; i < int(m_->text.size()); i++) {
		asterisk.append("*");
	}
	return asterisk;
}
}  // namespace UI
