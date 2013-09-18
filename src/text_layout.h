/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef TEXT_LAYOUT_H
#define TEXT_LAYOUT_H

#include <string>

#include "constants.h"
#include "rgbcolor.h"


/**
 * Checks it the given string is RichText or not. Does not do validity checking.
 */
inline bool is_richtext(const std::string& text) {
	return text.compare(0, 3, "<rt") == 0;
}

/**
 * Convenience functions to convert simple text into a valid block
 * of rich text which can be rendered.
 */
std::string as_uifont
	(const std::string&, int ptsize = UI_FONT_SIZE_SMALL, const RGBColor& clr = UI_FONT_CLR_FG);
std::string as_tooltip(const std::string&);
std::string as_waresinfo(const std::string&);
std::string as_window_title(const std::string&);
std::string as_game_tip(const std::string&);

#endif /* end of include guard: TEXT_LAYOUT_H */

