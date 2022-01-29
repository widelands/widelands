/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_WUI_MAPVIEWPIXELCONSTANTS_H
#define WL_WUI_MAPVIEWPIXELCONSTANTS_H

// These are constants with the unit map pixel, i.e. at zoom = 1 a triangle on
// screen is kTriangleWidth pixels wide. Map pixelspace is used in many places
// when the hexagonal fields coordinate space is insufficient. For example draw
// calculations first figure out where something needs to be drawn in map pixel
// space, then this is converted into screen space taking zoom into account.
constexpr int kTriangleWidth = 64;
constexpr int kTriangleHeight = 32;
constexpr int kHeightFactor = 5;
constexpr float kHeightFactorFloat =
   5.0f;  // Same a kHeightFactor to avoid implict upcasts to double

#endif  // end of include guard: WL_WUI_MAPVIEWPIXELCONSTANTS_H
