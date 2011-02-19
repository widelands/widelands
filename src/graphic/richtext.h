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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <limits>
#include <string>

#include <boost/scoped_ptr.hpp>

#include "point.h"


struct RenderTarget;

namespace UI {

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

	uint32_t width();
	uint32_t height();

	void parse(const std::string & text);
	void draw(RenderTarget & dst, Point offset);

private:
	struct Impl;

	boost::scoped_ptr<Impl> m;
};

} // namespace UI

#endif // RICHTEXT_H
