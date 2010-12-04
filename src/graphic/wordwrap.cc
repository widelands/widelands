/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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
 */

/*
 * The original version of the word wrapping algorithm was taken
 * from Wesnoth -- http://www.wesnoth.org
 */

#include "wordwrap.h"

#include "log.h"

namespace UI {

WordWrap::WordWrap(const TextStyle & style, uint32_t wrapwidth) :
	m_style(style),
	m_caret_line(std::numeric_limits<uint32_t>::max()),
	m_caret_pos(std::numeric_limits<uint32_t>::max())
{
	m_wrapwidth = wrapwidth;

	if (m_wrapwidth < std::numeric_limits<uint32_t>::max()) {
		if (m_wrapwidth < 2 * LINE_MARGIN)
			m_wrapwidth = 0;
		else
			m_wrapwidth -= 2 * LINE_MARGIN;
	}
}

/**
 * Perform the wrapping computations for the given text and fill in
 * the private data containing the wrapped results.
 */
void WordWrap::wrap(const std::string & text, uint32_t caret)
{
	static int count = 0;
	log("word_wrap_text(%u): %i\n", m_wrapwidth, ++count);

	m_caret_line = std::numeric_limits<uint32_t>::max();
	m_caret_pos = std::numeric_limits<uint32_t>::max();

	std::string::size_type line_start = 0;

	while (line_start <= text.size()) {
		std::string::size_type next_line_start;
		std::string::size_type line_end;

		compute_end_of_line(text, line_start, line_end, next_line_start);

		m_lines.push_back(text.substr(line_start, line_end - line_start));

		if (caret >= line_start && caret <= line_end) {
			m_caret_line = m_lines.size() - 1;
			m_caret_pos = caret - line_start;
		}

		line_start = next_line_start;
	}
}


/**
 * Compute the position where the line that starts at \p line_start
 * ends.
 */
void WordWrap::compute_end_of_line
	(const std::string & text,
	 std::string::size_type line_start,
	 std::string::size_type & line_end,
	 std::string::size_type & next_line_start)
{
	std::string::size_type orig_end = text.find('\n', line_start);
	if (orig_end == std::string::npos)
		orig_end = text.size();

	if (m_wrapwidth == std::numeric_limits<uint32_t>::max() || orig_end - line_start <= 1) {
		// Special fast path when wrapping is disabled or
		// original text line contains at most one character
		line_end = orig_end;
		next_line_start = orig_end + 1;
		return;
	}

	// Optimism: perhaps the entire line fits?
	if (m_style.calc_bare_width(text.substr(line_start, orig_end - line_start)) <= m_wrapwidth) {
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

		if (m_style.calc_bare_width(text.substr(line_start, mid - line_start)) <= m_wrapwidth) {
			end_lower = mid;
		} else {
			end_upper = mid - 1;
		}
	}

	// Narrow it down to a word break
	// Invariant: space points to a space character such that [line_start, space) fits
	std::string::size_type space = end_lower;

	while (space > line_start && text[space] != ' ')
		--space;

	for (;;) {
		std::string::size_type nextspace = text.find(' ', space + 1);
		if (nextspace > end_upper)
			break; // we already know that this cannot possibly fit

		// check whether the next word still fits
		if (m_style.calc_bare_width(text.substr(line_start, nextspace - line_start)) > m_wrapwidth)
			break;

		space = nextspace;
	}

	if (space > line_start) {
		line_end = space;
		next_line_start = space + 1;
		return;
	}

	// Nasty special case: the line starts with a single word that is too big to fit
	// Continue the binary search until we narrowed down exactly how many characters fit
	while (end_upper > end_lower) {
		std::string::size_type mid = end_lower + (end_upper - end_lower + 1) / 2;

		if (m_style.calc_bare_width(text.substr(line_start, mid - line_start)) <= m_wrapwidth)
			end_lower = mid;
		else
			end_upper = mid - 1;
	}

	next_line_start = line_end = end_lower;
}


} // namespace UI
