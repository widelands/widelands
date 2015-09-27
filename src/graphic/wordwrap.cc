/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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

#include "graphic/wordwrap.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"

namespace UI {

/**
 * Initialize the wordwrap object with an unlimited line length
 * and a default-constructed text style.
 */
WordWrap::WordWrap() :
	m_wrapwidth(std::numeric_limits<uint32_t>::max()), m_draw_caret(false)
{
}

WordWrap::WordWrap(const TextStyle & style, uint32_t gwrapwidth) :
	m_style(style), m_draw_caret(false)
{
	m_wrapwidth = gwrapwidth;

	if (m_wrapwidth < std::numeric_limits<uint32_t>::max()) {
		if (m_wrapwidth < 2 * LINE_MARGIN)
			m_wrapwidth = 0;
		else
			m_wrapwidth -= 2 * LINE_MARGIN;
	}
}

/**
 * Set the text style for future wrapping operations.
 */
void WordWrap::set_style(const TextStyle & style)
{
	m_style = style;
}

/**
 * Set the wrap width (i.e. line width limit in pixels) for future wrapping operations.
 */
void WordWrap::set_wrapwidth(uint32_t gwrapwidth)
{
	m_wrapwidth = gwrapwidth;
}

/**
 * Return the wrap width. This can be larger than the actual @ref width of the final
 * text.
 */
uint32_t WordWrap::wrapwidth() const
{
	return m_wrapwidth;
}

/**
 * Perform the wrapping computations for the given text and fill in
 * the private data containing the wrapped results.
 */
void WordWrap::wrap(const std::string & text)
{
	m_lines.clear();
	std::vector<std::string> words_to_fit;

	if (text.empty()) return;

	std::vector<std::string> words;
	boost::split(words, text, boost::is_any_of(" "));
	for (const std::string& word : words) {
		// NOCOM(GunChleoc): This means that ligatures can't be broken apart while editing.
		words_to_fit.push_back(i18n::make_ligatures(word.c_str()));
	}
	size_t line_start = 0;
	size_t line_end = line_start;

	while (!words_to_fit.empty()) {
		std::vector<std::string> line_words = compute_end_of_line(&words_to_fit);
		assert(!line_words.empty());

		std::string line;
		for (std::string line_word : line_words) {
			line += line_word;
		}
		line_end += line.size();
		LineData ld;
		ld.start = line_start;
		ld.text = line;
		m_lines.push_back(ld);
		line_start = line_end;
	}
}


/**
 * Compute the position where the line that starts at \p line_start
 * ends.
 */
// NOCOM add length to linedata?
std::vector<std::string> WordWrap::compute_end_of_line(std::vector<std::string>* words_to_fit) {
	uint32_t margin = 4;
	assert(m_wrapwidth > margin);
	assert(!words_to_fit->empty());
	std::vector<std::string> result;
	uint32_t linewidth = 0;
	bool found_fitting = false;
	// We need the dots to prevent trimming of the whitespace.
	uint32_t space_width = UI::g_fh1->render(as_uifont(". ."))->width() -
								  UI::g_fh1->render(as_uifont(".."))->width();
	std::string word = words_to_fit->front();
	uint32_t text_width = 0;
	while (!words_to_fit->empty()) {
		text_width = UI::g_fh1->render(as_uifont(word))->width();
		if ((linewidth + text_width + space_width) > m_wrapwidth - margin) {
			break;
		}
		found_fitting = true;
		result.push_back(word + " ");
		words_to_fit->erase(words_to_fit->begin());
		word = words_to_fit->front();
		linewidth += text_width + space_width;
	}
	// If the first word didn't fit the line, split it.
	if (!found_fitting && !words_to_fit->empty()) {
		const icu::UnicodeString unicode_word(word.c_str());
		int32_t end = 0;
		text_width = 0;
		// We just do linear search ahead until we hit the max
		// Operating on single glyphs will keep the texture cache small.
		while (text_width < m_wrapwidth - margin && (end < unicode_word.length() - 1)) {
			UChar c = unicode_word.charAt(end);
			text_width += UI::g_fh1->render(as_uifont(i18n::icuchar2string(c)))->width();
			++end;
		}
		// Make sure that diacritics stay with their base letters, and that
		// start/end line rules are being followed. Leave an arbitrary minimum of 5 chars.
		while (end > 5 &&
				 !(i18n::can_start_line(unicode_word.charAt(end)) &&
					i18n::can_end_line(unicode_word.charAt(end - 1)))) {
			--end;
		}
		// Now split the string
		result.push_back(i18n::icustring2string(unicode_word.tempSubString(0, end)));
		words_to_fit->at(0) = i18n::icustring2string(unicode_word.tempSubString(end));
	}
	return result;
}


/**
 * Compute the width (including border) of the word-wrapped text.
 *
 * \note This is rather inefficient right now.
 */
uint32_t WordWrap::width() const
{
	uint32_t calculated_width = 0;

	for (uint32_t line = 0; line < m_lines.size(); ++line) {
		uint32_t linewidth = m_style.calc_bare_width(m_lines[line].text);
		if (linewidth > calculated_width)
			calculated_width = linewidth;
	}

	return calculated_width + 2 * LINE_MARGIN;
}

/**
 * Compute the total height of the word-wrapped text.
 */
uint32_t WordWrap::height() const
{
	uint16_t fontheight = 0;
	if (!m_lines.empty()) {
		fontheight = UI::g_fh1->render(as_uifont(m_lines[0].text))->height();
	}

	return fontheight * (m_lines.size());
}

/**
 * Given an offset @p caret into the original text, compute the @p line that it
 * appears in in the wrapped text, and also the @p pos within that line (as an offset).
 */
void WordWrap::calc_wrapped_pos(uint32_t caret, uint32_t & line, uint32_t & pos) const
{
	assert(m_lines.size());
	assert(m_lines[0].start == 0);

	uint32_t min = 0;
	uint32_t max = m_lines.size() - 1;

	while (max > min) {
		uint32_t mid = min + (max - min + 1) / 2;

		if (caret >= m_lines[mid].start)
			min = mid;
		else
			max = mid - 1;
	}

	assert(caret >= m_lines[min].start);

	line = min;
	pos = caret - m_lines[min].start;
}

/**
 * Return the starting offset of line number @p line in the original text.
 */
uint32_t WordWrap::line_offset(uint32_t line) const
{
	return m_lines[line].start;
}

/**
 * Draw the word-wrapped text onto \p dst, anchored at \p where with the given alignment.
 *
 * \note This also draws the caret, if any.
 */
void WordWrap::draw(RenderTarget & dst, Point where, Align align, uint32_t caret)
{
	if (m_lines.empty()) return;

	uint32_t caretline, caretpos;

	calc_wrapped_pos(caret, caretline, caretpos);

	if ((align & Align_Vertical) != Align_Top) {
		uint32_t h = height();

		if ((align & Align_Vertical) == Align_VCenter)
			where.y -= (h + 1) / 2;
		else
			where.y -= h;
	}
	++where.y;

	Align alignment = mirror_alignment(align);

	// Since this will eventually be used in edit boxes only, we always have standard font size here.
	uint16_t fontheight = UI::g_fh1->render(as_uifont(m_lines[0].text))->height();
	for (uint32_t line = 0; line < m_lines.size(); ++line, where.y += fontheight) {
		if (where.y >= dst.height() || int32_t(where.y + fontheight) <= 0)
			continue;

		Point point(where.x, where.y);

		const Image* entry_text_im = UI::g_fh1->render(as_uifont(m_lines[line].text));
		uint16_t text_width = entry_text_im->width();

		if (alignment & Align_Right) {
			point.x += m_wrapwidth;
		} else if (alignment & Align_HCenter) {
			point.x += m_wrapwidth / 2;
		}

		UI::correct_for_align(alignment, text_width, entry_text_im->height(), &point);
		dst.blit(point, entry_text_im);

		if (m_draw_caret && line == caretline) {
			int caret_x = UI::g_fh1->render(as_uifont(m_lines[line].text.substr(0, caretpos)))->width();
			const Image* caret_image = g_gr->images().get("pics/caret.png");
			Point caretpt;
			caretpt.x = point.x + caret_x - caret_image->width() + 1;
			caretpt.y = point.y + (entry_text_im->height() - caret_image->height()) / 2;
			dst.blit(caretpt, caret_image);
		}
	}
}


} // namespace UI
