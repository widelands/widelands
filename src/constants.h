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

// define current version
#define VERSION "build-9half"

// locale path: This would be needed to be changed by a packager
// The default search path for locales is in the locale directory
// but if widelands is installed, this changes to something like
// /usr/locale/share/*. If so, change here
#define LOCALE_PATH "locale"

#define TEXTURE_W 64        // texture have a fixed size and are squares
#define TEXTURE_H TEXTURE_W // TEXTURE_H is just defined for easier understandement of the code

// The background colors are chosen to match the user interface 
// backgrounds
#define UI_FONT_BIG "Vera.ttf", 18
#define UI_FONT_SMALL "Vera.ttf", 10
#define UI_FONT_CLR_FG  RGBColor(255,255,0)
#define UI_FONT_BIG_CLR UI_FONT_CLR_FG, RGBColor(107,87,55)
#define UI_FONT_SMALL_CLR UI_FONT_CLR_FG, RGBColor(107,87,55)
#define UI_FONT_TOOLTIP UI_FONT_SMALL
#define UI_FONT_TOOLTIP_CLR RGBColor(100,100,255), RGBColor(230,200,50)
#define PROSA_FONT       "Vera.ttf", 14
#define PROSA_FONT_CLR_FG   RGBColor(255,255,0)


/*
================================================================================

      COMPILE TIME CONFIGURATION

================================================================================
*/

// the actual game logic doesn't know about frames (it works with millisecond-precise
// timing)
// FRAME_LENGTH is just the default animation speed
#define FRAME_LENGTH 250

// Maxi numbers of players in a game
#define MAX_PLAYERS		8

// How often are statistics to be sampled
#define STATISTICS_SAMPLE_TIME 30000

#endif
