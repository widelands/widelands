/*
 * Copyright (C) 2010-2011 by the Widelands Development Team
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

#ifndef WORDWRAP_H
#define WORDWRAP_H

#include <string>

#include "align.h"
#include "graphic/font.h"
#include "point.h"

class RenderTarget;

namespace UI {

/**
 * Helper struct that provides word wrapping and related functionality.
 */
struct WordWrap {
	WordWrap();
	WordWrap(const TextStyle & style, uint32_t wrapwidth = std::numeric_limits<uint32_t>::max());

	void set_style(const TextStyle & style);
	void set_wrapwidth(uint32_t wrapwidth);

	uint32_t wrapwidth() const;

	void wrap(const std::string & text);

	uint32_t width() const;
	uint32_t height() const;

	void draw
		(RenderTarget & dst, Point where, Align align = Align_Left,
		 uint32_t caret = std::numeric_limits<uint32_t>::max());

	void calc_wrapped_pos(uint32_t caret, uint32_t & line, uint32_t & pos) const;
	uint32_t nrlines() const {return m_lines.size();}
	uint32_t line_offset(uint32_t line) const;

private:
	struct LineData {
		/// Textual content of the line
		std::string text;

		/// Starting offset of this line within the original un-wrapped text
		uint32_t start;
	};

	void compute_end_of_line
		(const std::string & text,
		 std::string::size_type line_start,
		 std::string::size_type & line_end,
		 std::string::size_type & next_line_start);

	TextStyle m_style;
	uint32_t m_wrapwidth;

	std::vector<LineData> m_lines;
};

} // namespace UI

#endif // WORDWRAP_H
