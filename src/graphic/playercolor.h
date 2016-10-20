/*
 * Copyright (C) 2016 by the Widelands Development Team
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

#ifndef WL_LOGIC_PLAYER_COLOR_H
#define WL_LOGIC_PLAYER_COLOR_H

#include "graphic/color.h"
#include "graphic/image.h"
#include "logic/constants.h"

// Hard coded player colors
const RGBColor kPlayerColors[MAX_PLAYERS] = {
   RGBColor(2, 2, 198),      // blue
   RGBColor(255, 41, 0),     // red
   RGBColor(255, 232, 0),    // yellow
   RGBColor(59, 223, 3),     // green
   RGBColor(57, 57, 57),     // black/dark gray
   RGBColor(255, 172, 0),    // orange
   RGBColor(215, 0, 218),    // purple
   RGBColor(255, 255, 255),  // white
   RGBColor(0, 120, 255),    // sky blue
   RGBColor(104, 0, 40),     // dark red
   RGBColor(120, 108, 0),    // dark yellow
   RGBColor(0, 112, 0),      // dark green
   RGBColor(255, 120, 160),  // rose
   RGBColor(148, 56, 0),     // brown
   RGBColor(96, 0, 84),      // dark purple
   RGBColor(144, 144, 144),  // light gray
};

Image* playercolor_image(const RGBColor* clr, const Image* image, const Image* color_mask);
Image* playercolor_image(int player_number, const Image* image, const Image* color_mask);

#endif  // end of include guard: WL_LOGIC_PLAYER_COLOR_H
