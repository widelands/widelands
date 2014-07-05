/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

// NOCOM(#sirver): rename do font_constants?
#ifndef TEXT_CONSTANTS_H
#define TEXT_CONSTANTS_H

// Basic constants for often used text attributes.

#define UI_FONT_NAME_SERIF      "DejaVuSerif.ttf"
#define UI_FONT_NAME_SANS       "DejaVuSans.ttf"
#define UI_FONT_NAME_WIDELANDS  "Widelands/Widelands.ttf"

#define UI_FONT_NAME            UI_FONT_NAME_SERIF
#define UI_FONT_NAME_NO_EXT     "DejaVuSerif"
#define UI_FONT_SIZE_BIG        22
#define UI_FONT_SIZE_SMALL      14
#define UI_FONT_SIZE_ULTRASMALL 10

#define UI_FONT_BIG             UI_FONT_NAME, UI_FONT_SIZE_BIG
#define UI_FONT_SMALL           UI_FONT_NAME, UI_FONT_SIZE_SMALL
#define UI_FONT_ULTRASMALL      UI_FONT_NAME, UI_FONT_SIZE_ULTRASMALL

#define UI_FONT_TOOLTIP         UI_FONT_SMALL
#define PROSA_FONT              UI_FONT_NAME_SERIF, 18

/// Font colors

/// Global UI font color
#define UI_FONT_CLR_FG       RGBColor(255, 255,   0)
#define UI_FONT_CLR_BG       RGBColor(107,  87,  55)
#define UI_FONT_CLR_DISABLED RGBColor(127, 127, 127)
#define UI_FONT_CLR_WARNING  RGBColor(255,  22,  22)

/// Prosa font color
#define PROSA_FONT_CLR_FG    RGBColor(255, 255,   0)

/// Colors for good/ok/bad
#define UI_FONT_CLR_BRIGHT_HEX    "fffaaa"
#define UI_FONT_CLR_DARK_HEX      "a39013"
#define UI_FONT_CLR_BAD_HEX       "bb0000"
#define UI_FONT_CLR_OK_HEX        "ffe11e"
#define UI_FONT_CLR_GOOD_HEX      "00bb00"
#define UI_FONT_CLR_IDLE_HEX      "0090ff"

/** \name Text colors
 * User interface text color constants
 *
 * Defined as "\<fontcolor\>, \<background color\>".
 * The background colors are chosen to match the user interface
 * backgrounds.
 */
#define UI_FONT_BIG_CLR     UI_FONT_CLR_FG, UI_FONT_CLR_BG
/// small is used for ultrasmall, too
#define UI_FONT_SMALL_CLR   UI_FONT_CLR_FG, UI_FONT_CLR_BG
#define UI_FONT_TOOLTIP_CLR RGBColor(255, 255, 0)

#endif
