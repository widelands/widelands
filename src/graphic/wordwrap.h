/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef WORDWRAP_H
#define WORDWRAP_H

#include <string>

#include "font.h"
#include "point.h"
#include "ui_basic/align.h"

struct RenderTarget;

namespace UI {

/**
 * Helper struct that provides word wrapping and related functionality.
 */
struct WordWrap {
	WordWrap(const TextStyle & style, uint32_t wrapwidth = std::numeric_limits<uint32_t>::max());

	void wrap(const std::string & text, uint32_t caret = std::numeric_limits<uint32_t>::max());

	uint32_t width() const;
	uint32_t height() const;

	void draw(RenderTarget & dst, Point where, Align align = Align_Left);

	const std::vector<std::string> & lines() const {return m_lines;}
	uint32_t caret_line() const {return m_caret_line;}
	uint32_t caret_pos() const {return m_caret_pos;}

private:
	void compute_end_of_line
		(const std::string & text,
		 std::string::size_type line_start,
		 std::string::size_type & line_end,
		 std::string::size_type & next_line_start);

	TextStyle m_style;
	uint32_t m_wrapwidth;

	std::vector<std::string> m_lines;
	uint32_t m_caret_line;
	uint32_t m_caret_pos;
};

} // namespace UI

#endif // WORDWRAP_H
