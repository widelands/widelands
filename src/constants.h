/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * \file constants.h
 * \brief Global compile time configuration and important constants
 *
 * Changes have wide impact on recompile time.
 * Lots more are scattered through header files
 */

///Version mark for the build. Appears in title screen
///\todo Tie this into build_id (which, in turn, should be tied into CVS numbering)
#define VERSION "cvsbuild"

/**
 * locale path: This would be needed to be changed by a packager
 *
 * The default search path for locales is in the locale directory
 * but if widelands is installed, this changes to something like
 * /usr/locale/share/... If so, change here
 */
#define LOCALE_PATH "locale"

/* \name Textures
 * textures have a fixed size and are squares
 * TEXTURE_H is just defined for easier understandement of the code*/
//@{
#define TEXTURE_W 64
#define TEXTURE_H TEXTURE_W
//@}

/// \name Fonts
/// Font constants, defined including size
//@{
#define UI_FONT_BIG "FreeSerif.ttf", 22
#define UI_FONT_SMALL "FreeSerif.ttf", 14
#define UI_FONT_ULTRASMALL "FreeSans.ttf", 10

#define UI_FONT_TOOLTIP UI_FONT_SMALL
#define PROSA_FONT "FreeSerif.ttf", 18
//@}

/// \name Font colors
/// No background color
//@{

/// Global UI font color
#define UI_FONT_CLR_FG RGBColor(255,255,0)
/// Prosa font color
#define PROSA_FONT_CLR_FG RGBColor(255,255,0)

//@}

/** \name Text colors
 * User interface text color constants
 *
 * Defined as "<fontcolor>, <background color>".
 * The background colors are chosen to match the user interface
 * backgrounds.
 */
//@{
#define UI_FONT_BIG_CLR UI_FONT_CLR_FG, RGBColor(107,87,55)
/// small is used for ultrasmall, too
#define UI_FONT_SMALL_CLR UI_FONT_CLR_FG, RGBColor(107,87,55)
#define UI_FONT_TOOLTIP_CLR RGBColor(100,100,255), RGBColor(230,200,50)
//@}

/// the actual game logic doesn't know about frames
/// (it works with millisecond-precise timing)
/// FRAME_LENGTH is just the default animation speed
#define FRAME_LENGTH 250

/// Maximum numbers of players in a game
#define MAX_PLAYERS 8

/// How often are statistics to be sampled
#define STATISTICS_SAMPLE_TIME 30000

/** Shorthand because the original is a lot to type \e and harder to read.
 * Use #define and not typedef so that we don't need to #include<string> here.
 * This would cause <string> to included in every constants.h "user" - instead
 * of every SSS_T user.
 */
#define SSS_T std::string::size_type

/// Networking
//@
#define WIDELANDS_LAN_DISCOVERY_PORT	7394
#define WIDELANDS_LAN_PROMOTION_PORT	7395
#define WIDELANDS_PORT			7396
//@}

#endif
