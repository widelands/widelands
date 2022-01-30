/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_PLAYERCOLOR_H
#define WL_GRAPHIC_PLAYERCOLOR_H

#include "graphic/color.h"
#include "graphic/image.h"

/// Maximum numbers of players in a game. The game logic code reserves 5 bits
/// for player numbers, so it can keep track of 32 different player numbers, of
/// which the value 0 means neutral and the values 1 .. 31 can be used as the
/// numbers for actual players. So the upper limit of this value is 31.
constexpr uint8_t kMaxPlayers = 16;

// Hard coded player colors
const RGBColor kPlayerColors[kMaxPlayers] = {
   RGBColor(65, 105, 225),   // royal blue
   RGBColor(255, 41, 0),     // red
   RGBColor(255, 232, 0),    // yellow
   RGBColor(59, 223, 3),     // green
   RGBColor(128, 128, 128),  // gray
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
   RGBColor(192, 192, 192),  // silver
};

// Hard coded team colors
const RGBColor kTeamColors[kMaxPlayers / 2 + 1] = {
   RGBColor(100, 100, 100),  // No team
   RGBColor(2, 2, 198),      // blue
   RGBColor(255, 41, 0),     // red
   RGBColor(255, 232, 0),    // yellow
   RGBColor(59, 223, 3),     // green
   RGBColor(57, 57, 57),     // black/dark gray
   RGBColor(255, 172, 0),    // orange
   RGBColor(215, 0, 218),    // purple
   RGBColor(255, 255, 255),  // white
};

/// Looks for a player color mask image, and if it finds one,
/// returns the image with added playercolor. If no player color
/// image file is found, gets the image from 'image_filename'
/// without player color added. File name format: if image_filename
/// == 'foo.png', the playercolor mask image's filename must be
/// 'foo_pc.png'.
const Image* playercolor_image(const RGBColor& clr, const std::string& image_filename);
const Image* playercolor_image(int player_number, const std::string& image_filename);

#endif  // end of include guard: WL_GRAPHIC_PLAYERCOLOR_H
