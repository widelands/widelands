/*
 * Copyright (C) 2002-2003, 2006-2011 by the Widelands Development Team
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

/**
 * \file constants.h
 * \brief Global compile time configuration and important constants
 *
 * Changes have wide impact on recompile time.
 * Lots more are scattered through header files
 */

/// \name Textures
/// Textures have a fixed size and are squares.
/// TEXTURE_HEIGHT is just defined for easier understanding of the code.
//@{
#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT TEXTURE_WIDTH
//@}

//sizes for the images in the build menu (containing building icons)
#define BUILDMENU_IMAGE_SIZE 30. // used for width and height

#define XRES 800 ///< Fullscreen Menu Width
#define YRES 600 ///< Fullscreen Menu Height

#define FALLBACK_GRAPHICS_WIDTH 800
#define FALLBACK_GRAPHICS_HEIGHT 600
#define FALLBACK_GRAPHICS_DEPTH 32

/// \name Fonts
/// Font constants, defined including size
//@{
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
//@}

/// \name Font colors
/// A background color is not explicitly defined
//@{

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
//@}

/** \name Text colors
 * User interface text color constants
 *
 * Defined as "\<fontcolor\>, \<background color\>".
 * The background colors are chosen to match the user interface
 * backgrounds.
 */
//@{
#define UI_FONT_BIG_CLR     UI_FONT_CLR_FG, UI_FONT_CLR_BG
/// small is used for ultrasmall, too
#define UI_FONT_SMALL_CLR   UI_FONT_CLR_FG, UI_FONT_CLR_BG
#define UI_FONT_TOOLTIP_CLR RGBColor(255, 255, 0)
//@}

/// the actual game logic doesn't know about frames
/// (it works with millisecond-precise timing)
/// FRAME_LENGTH is just the default animation speed
#define FRAME_LENGTH 250

/// Networking
//@{
#define WIDELANDS_LAN_DISCOVERY_PORT 7394
#define WIDELANDS_LAN_PROMOTION_PORT 7395
#define WIDELANDS_PORT               7396
//@}

/// Maximum numbers of players in a game. The game logic code reserves 5 bits
/// for player numbers, so it can keep track of 32 different player numbers, of
/// which the value 0 means neutral and the values 1 .. 31 can be used as the
/// numbers for actual players. So the upper limit of this value is 31.
#define MAX_PLAYERS 8

/// How often are statistics to be sampled.
#define STATISTICS_SAMPLE_TIME 30000

/// Constants for user-defined SDL events that get handled by SDL's mainloop
//@{
enum {
	CHANGE_MUSIC
};
//@}

/**
 * C++ is really bad at integer types. For example this constant is not
 * recognized as a valid value of type Workarea_Info::size_type without a cast.
 */
#define NUMBER_OF_WORKAREA_PICS static_cast<Workarea_Info::size_type>(6)

/// The size of the transient (i.e. temporary) surfaces in the cache in bytes.
/// These are all surfaces that are not loaded from disk.
const uint32_t TRANSIENT_SURFACE_CACHE_SIZE = 160 << 20;   // shifting converts to MB

#endif
