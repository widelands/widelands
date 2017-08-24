/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_CONSTANTS_H
#define WL_GRAPHIC_TEXT_CONSTANTS_H

// Basic constants for often used text attributes.

/// Font Sizes
#define UI_FONT_SIZE_BIG 22
#define UI_FONT_SIZE_SMALL 14
#define UI_FONT_SIZE_MESSAGE 12
#define UI_FONT_SIZE_ULTRASMALL 10
constexpr int kMinimumFontSize = 6;
constexpr int kLineMargin = 1;

/// Font colors

/// Global UI font color
#define UI_FONT_CLR_FG RGBColor(255, 255, 0)
#define UI_FONT_CLR_DISABLED RGBColor(127, 127, 127)
#define UI_FONT_CLR_WARNING RGBColor(255, 22, 22)

/// Tooltip font color
#define UI_FONT_TOOLTIP_CLR RGBColor(255, 255, 0)

/// Colors for good/ok/bad
#define UI_FONT_CLR_BRIGHT RGBColor(255, 250, 170)
#define UI_FONT_CLR_DARK RGBColor(163, 144, 19)
#define UI_FONT_CLR_BAD RGBColor(187, 0, 0)
#define UI_FONT_CLR_OK RGBColor(255, 225, 30)
#define UI_FONT_CLR_GOOD RGBColor(0, 187, 0)

#endif  // end of include guard: WL_GRAPHIC_TEXT_CONSTANTS_H
