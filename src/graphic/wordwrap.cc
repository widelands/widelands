/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

/*
 * The original version of the word wrapping algorithm was taken
 * from Wesnoth -- https://www.wesnoth.org
 */

#include "graphic/wordwrap.h"

#include <memory>

#include <ui_basic/mouse_constants.h>
#include <unicode/unistr.h>

#include "base/string.h"
#include "graphic/color.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_io.h"
#include "graphic/text_layout.h"

namespace {
inline std::string as_editorfont(const std::string& text, int ptsize, const RGBColor& clr) {
	// UI Text is always bold due to historic reasons
	return bformat(
	   "<rt keep_spaces=1><p><font face=sans size=%i bold=1 shadow=1 color=%s>%s</font></p></rt>",
	   ptsize, clr.hex_value(), richtext_escape(text));
}

int text_width(const std::string& text, int ptsize) {
	RGBColor color(0, 0, 0);
	return UI::g_fh->render(as_editorfont(text, ptsize - UI::g_fh->fontset()->size_offset(), color))
	   ->width();
}

int text_height(int ptsize) {
	RGBColor font_color(0, 0, 0);
	const UI::FontStyleInfo font_info("sans", font_color, ptsize, false, false, false, false);
	return UI::g_fh
	   ->render(as_richtext_paragraph(UI::g_fh->fontset()->representative_character(), font_info))
	   ->height();
}
}  // namespace

namespace UI {

constexpr int CARET_BLINKING_DELAY = 1000;

WordWrap::WordWrap(int fontsize, const RGBColor& color, uint32_t gwrapwidth)
   : draw_caret_(false),
     fontsize_(fontsize),
     color_(color),
     font_(RT::load_font(UI::g_fh->fontset()->sans_bold(), fontsize_)),
     caret_timer_("", true) {
	wrapwidth_ = gwrapwidth;
	caret_timer_.ms_since_last_query();
	caret_ms = 0;

	if (wrapwidth_ < std::numeric_limits<uint32_t>::max()) {
		if (wrapwidth_ < 2 * kLineMargin) {
			wrapwidth_ = 0;
		} else {
			wrapwidth_ -= 2 * kLineMargin;
		}
	}
}

/**
 * Set the wrap width (i.e. line width limit in pixels) for future wrapping operations.
 */
void WordWrap::set_wrapwidth(uint32_t gwrapwidth) {
	wrapwidth_ = gwrapwidth;
}

/**
 * Return the wrap width. This can be larger than the actual @ref width of the final
 * text.
 */
uint32_t WordWrap::wrapwidth() const {
	return wrapwidth_;
}

/**
 * Perform the wrapping computations for the given text and fill in
 * the private data containing the wrapped results.
 */
void WordWrap::wrap(const std::string& text) {
	lines_.clear();

	std::string::size_type line_start = 0;
	uint32_t margin = quick_width(0x2003);  // Em space

	while (line_start <= text.size()) {
		std::string::size_type next_line_start;
		std::string::size_type line_end;

		compute_end_of_line(text, line_start, line_end, next_line_start, margin);

		LineData ld;
		ld.start = line_start;
		ld.text = text.substr(line_start, line_end - line_start);
		lines_.push_back(ld);

		line_start = next_line_start;
	}
}

/**
 * Compute the position where the line that starts at \p line_start
 * ends.
 */
void WordWrap::compute_end_of_line(const std::string& text,
                                   std::string::size_type line_start,
                                   std::string::size_type& line_end,
                                   std::string::size_type& next_line_start,
                                   uint32_t safety_margin) {
	std::string::size_type minimum_chars = 1;  // So we won't get empty lines
	assert(text.empty() || wrapwidth_ > safety_margin);

	std::string::size_type orig_end = text.find('\n', line_start);
	if (orig_end == std::string::npos) {
		orig_end = text.size();
	}

	if (wrapwidth_ == std::numeric_limits<uint32_t>::max() ||
	    orig_end - line_start <= minimum_chars) {
		// Special fast path when wrapping is disabled or
		// original text line contains at most minimum_chars characters
		line_end = orig_end;
		next_line_start = orig_end + 1;
		return;
	}

	// Optimism: perhaps the entire line fits?
	if (uint32_t(text_width(text.substr(line_start, orig_end - line_start), fontsize_)) <=
	    wrapwidth_ - safety_margin) {
		line_end = orig_end;
		next_line_start = orig_end + 1;
		return;
	}

	// Okay, we really do need to wrap; get a first rough estimate using binary search
	// Invariant: [line_start, end_lower) fits,
	// but everything strictly longer than [line_start, end_upper) does not fit
	// We force the lower bound to allow at least one character,
	// otherwise the word wrap might get into an infinite loop.
	std::string::size_type end_upper = orig_end - 1;
	std::string::size_type end_lower = line_start + 1;

	while (end_upper - end_lower > 4) {
		std::string::size_type mid = end_lower + (end_upper - end_lower + 1) / 2;

		if (line_fits(i18n::make_ligatures(text.substr(line_start, mid - line_start).c_str()),
		              safety_margin)) {
			end_lower = mid;
		} else {
			end_upper = mid - 1;
		}
	}

	// Narrow it down to a word break
	// Invariant: space points to a space character such that [line_start, space) fits
	std::string::size_type space = end_lower;

	while (space > line_start && text[space] != ' ') {
		--space;
	}

	for (;;) {
		std::string::size_type nextspace = text.find(' ', space + 1);
		if (nextspace > end_upper) {
			break;  // we already know that this cannot possibly fit
		}

		// check whether the next word still fits
		if (!line_fits(i18n::make_ligatures(text.substr(line_start, nextspace - line_start).c_str()),
		               safety_margin)) {
			break;
		}
		space = nextspace;
	}

	if (space > line_start) {
		line_end = space;
		next_line_start = space + 1;
		return;
	}

	// The line didn't fit.
	// We just do a linear search ahead until we hit the max.
	const icu::UnicodeString unicode_word(text.substr(line_start, orig_end).c_str(), "UTF-8");
	uint32_t line_width = 0;
	int32_t end = -1;
	icu::UnicodeString unicode_line;

	while ((line_width < (wrapwidth_ - safety_margin)) && (end < unicode_word.length())) {
		++end;
		UChar c = unicode_word.charAt(end);
		// Diacritics do not add to the line width
		if (!i18n::is_diacritic(c)) {
			// This only estimates the width
			line_width += quick_width(c);
		}
		unicode_line += c;
	}

	// Now make sure that it really fits.
	std::string::size_type test_cutoff = line_start + end * 2 / 3;
	while ((end > 0) && (static_cast<uint32_t>(line_start + end) > test_cutoff)) {
		if (uint32_t(text_width(text.substr(line_start, end), fontsize_)) >
		    wrapwidth_ - safety_margin) {
			--end;
		} else {
			break;
		}
	}

	// Find last space
	int32_t last_space = unicode_line.lastIndexOf(0x0020);  // space character
	if ((last_space > 0) && (static_cast<uint32_t>(last_space) > minimum_chars)) {
		end = last_space;
	}

	// Make sure that diacritics stay with their base letters, and that
	// start/end line rules are being followed.
	while ((end > 0) && (static_cast<uint32_t>(end) > minimum_chars) &&
	       (i18n::cannot_start_line(unicode_line.charAt(end)) ||
	        i18n::cannot_end_line(unicode_line.charAt(end - 1)))) {
		--end;
	}
	assert(end > 0);

	next_line_start = line_end =
	   (i18n::icustring2string(unicode_word.tempSubString(0, end)).size() + line_start);
}

// Returns true if the text won't fit into the alotted width.
bool WordWrap::line_fits(const std::string& text, uint32_t safety_margin) const {
	// calc_width_for_wrapping is fast, but it will underestimate the width.
	// So, we test again with text_width to make sure that the line really fits.
	return quick_width(i18n::make_ligatures(text.c_str())) <= wrapwidth_ - safety_margin &&
	       uint32_t(text_width(text, fontsize_)) <= wrapwidth_ - safety_margin;
}

/**
 * Compute the width (including border) of the word-wrapped text.
 *
 * \note This is rather inefficient right now.
 */
uint32_t WordWrap::width() const {
	uint32_t calculated_width = 0;

	for (const LineData& line : lines_) {
		uint32_t linewidth = text_width(line.text, fontsize_);
		if (linewidth > calculated_width) {
			calculated_width = linewidth;
		}
	}

	return calculated_width + 2 * kLineMargin;
}

/**
 * Compute the total height of the word-wrapped text.
 */
uint32_t WordWrap::height() const {
	return text_height(fontsize_) * (lines_.size()) + 2 * kLineMargin;
}

uint32_t WordWrap::line_index(int32_t y) const {
	return std::min(size_t((y - 2 * kLineMargin) / text_height(fontsize_)), lines_.size() - 1);
}
uint32_t WordWrap::offset_of_line_at(int32_t y) const {
	return line_offset(line_index(y));
}
std::string WordWrap::text_of_line_at(int32_t y) const {
	size_t line_idx = line_index(y);
	line_idx = std::min(line_idx, lines_.size() - 1);
	return lines_[line_idx].text;
}

int WordWrap::text_width_of(std::string& text) const {
	return text_width(text, fontsize_);
}

/**
 * Given an offset @p caret into the original text, compute the @p line that it
 * appears in in the wrapped text, and also the @p pos within that line (as an offset).
 */
void WordWrap::calc_wrapped_pos(uint32_t caret, uint32_t& line, uint32_t& pos) const {
	assert(!lines_.empty());
	assert(lines_[0].start == 0);

	uint32_t min = 0;
	uint32_t max = lines_.size() - 1;

	while (max > min) {
		uint32_t mid = min + (max - min + 1) / 2;

		if (caret >= lines_[mid].start) {
			min = mid;
		} else {
			max = mid - 1;
		}
	}

	assert(caret >= lines_[min].start);

	line = min;
	pos = caret - lines_[min].start;
}

/**
 * Return the starting offset of line number @p line in the original text.
 */
uint32_t WordWrap::line_offset(uint32_t line) const {
	return lines_[line].start;
}

/**
 * Draw the word-wrapped text onto \p dst, anchored at \p where with the given alignment.
 *
 * \note This also draws the caret, if any.
 */
void WordWrap::draw(RenderTarget& dst,
                    Vector2i where,
                    Align align,
                    uint32_t caret,
                    bool with_selection,
                    uint32_t selection_start,
                    uint32_t selection_end,
                    uint32_t scrollbar_position,
                    const std::string& caret_image_path) {
	if (lines_.empty()) {
		return;
	}

	uint32_t caretline, caretpos;
	uint32_t selection_start_line, selection_start_x;
	uint32_t selection_end_line, selection_end_x;

	calc_wrapped_pos(caret, caretline, caretpos);
	calc_wrapped_pos(selection_start, selection_start_line, selection_start_x);
	calc_wrapped_pos(selection_end, selection_end_line, selection_end_x);

	++where.y;

	Align alignment = mirror_alignment(align, g_fh->fontset()->is_rtl());

	const int fontheight = text_height(fontsize_);
	for (uint32_t line = 0; line < lines_.size(); ++line, where.y += fontheight) {
		if (where.y >= dst.height() || (where.y + fontheight) <= 0) {
			continue;
		}

		Vector2i point(where.x, where.y);

		if (alignment == UI::Align::kRight) {
			point.x += wrapwidth_ - kLineMargin;
		}

		std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh->render(
		   as_editorfont(lines_[line].text, fontsize_ - UI::g_fh->fontset()->size_offset(), color_));
		UI::correct_for_align(alignment, rendered_text->width(), &point);
		rendered_text->draw(dst, point);

		if (with_selection) {
			highlight_selection(dst, scrollbar_position, selection_start_line, selection_start_x,
			                    selection_end_line, selection_end_x, fontheight, line, point);
		}

		if (draw_caret_ && line == caretline) {
			std::string line_to_caret = lines_[line].text.substr(0, caretpos);
			// TODO(GunChleoc): Arabic: Fix cursor position for BIDI text.
			int caret_x = text_width(line_to_caret, fontsize_);

			const Image* caret_image = g_image_cache->get(caret_image_path);
			Vector2i caretpt = Vector2i::zero();
			caretpt.x = point.x + caret_x - caret_image->width() + kLineMargin;
			caretpt.y = point.y + (fontheight - caret_image->height()) / 2;

			if (caret_ms > CARET_BLINKING_DELAY) {
				dst.blit(caretpt, caret_image);
			}
			if (caret_ms > 2 * CARET_BLINKING_DELAY) {
				caret_ms = 0;
			}
			caret_ms += caret_timer_.ms_since_last_query();
		}
	}
}
void WordWrap::highlight_selection(RenderTarget& dst,
                                   uint32_t scrollbar_position,
                                   uint32_t selection_start_line,
                                   uint32_t selection_start_x,
                                   uint32_t selection_end_line,
                                   uint32_t selection_end_x,
                                   const int fontheight,
                                   uint32_t line,
                                   const Vector2i& point) const {

	Vector2i highlight_start = Vector2i::zero();
	Vector2i highlight_end = Vector2i::zero();
	if (line == selection_start_line) {
		std::string text_before_selection = lines_[line].text.substr(0, selection_start_x);
		highlight_start = Vector2i(text_width(text_before_selection, fontsize_) + point.x,
		                           (line * fontheight) - scrollbar_position);

		if (line == selection_end_line) {
			size_t nr_characters = selection_end_x - selection_start_x;
			std::string selected_text = lines_[line].text.substr(selection_start_x, nr_characters);
			highlight_end = Vector2i(text_width(selected_text, fontsize_), fontheight);
		} else {
			std::string selected_text = lines_[line].text.substr(selection_start_x);
			highlight_end = Vector2i(text_width(selected_text, fontsize_), fontheight);
		}

	} else if (line > selection_start_line && line < selection_end_line) {
		highlight_start = Vector2i(point.x, (line * fontheight) - scrollbar_position);
		highlight_end = Vector2i(text_width(lines_[line].text, fontsize_), fontheight);

	} else if (line == selection_end_line) {
		highlight_start = Vector2i(point.x, (line * fontheight) - scrollbar_position);
		highlight_end =
		   Vector2i(text_width(lines_[line].text.substr(0, selection_end_x), fontsize_), fontheight);
	}
	dst.brighten_rect(
	   Recti(highlight_start, highlight_end.x, highlight_end.y), BUTTON_EDGE_BRIGHT_FACTOR);
}

/**
 * Get a width estimate for text wrapping.
 */
uint32_t WordWrap::quick_width(const UChar& c) const {
	int result = 0;
	TTF_GlyphMetrics(font_->get_ttf_font(), c, nullptr, nullptr, nullptr, nullptr, &result);
	return result;
}

uint32_t WordWrap::quick_width(const std::string& text) const {
	int result = 0;
	const icu::UnicodeString parseme(text.c_str(), "UTF-8");
	for (int i = 0; i < parseme.length(); ++i) {
		UChar c = parseme.charAt(i);
		if (!i18n::is_diacritic(c)) {
			result += quick_width(c);
		}
	}
	return result;
}
void WordWrap::focus() {
	caret_ms = CARET_BLINKING_DELAY;
	caret_timer_.ms_since_last_query();
}

}  // namespace UI
