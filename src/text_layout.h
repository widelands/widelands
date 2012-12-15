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
 * Convenience functions to convert simple text into a valid block
 * of rich text which can be rendered.
 */
std::string as_uifont(const std::string &, int = UI_FONT_SIZE_SMALL, const std::string & = UI_FONT_NAME, RGBColor = UI_FONT_CLR_FG);


#endif /* end of include guard: TEXT_LAYOUT_H */

