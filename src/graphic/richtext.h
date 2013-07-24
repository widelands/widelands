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
 *
 */

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <limits>
#include <memory>
#include <string>

#include "point.h"
#include "rgbcolor.h"


class RenderTarget;

namespace UI {

struct RichTextImpl;

/**
 * Provides rich-text layouting and rendering functionality.
 *
 * Layouts the rich text as a tight fit, i.e. without any border around the
 * text. The final, layouted extents of the text can be obtained from @ref width and @ref height.
 */
struct RichText {
	RichText();
	~RichText();

	void set_width(uint32_t width = std::numeric_limits<uint32_t>::max());
	void set_background_color(RGBColor color);

	uint32_t width();
	uint32_t height();

	void parse(const std::string & text);
	void draw(RenderTarget & dst, const Point& offset, bool background = false);

private:
	std::unique_ptr<RichTextImpl> m;
};

} // namespace UI

#endif // RICHTEXT_H
