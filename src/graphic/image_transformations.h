/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#ifndef IMAGE_TRANSFORMATIONS_H
#define IMAGE_TRANSFORMATIONS_H

struct RGBColor;
class Image;

namespace ImageTransformations {

// NOCOM(#sirver): docu
void initialize();
const Image* resize(const Image* original, uint16_t w, uint16_t h);
const Image* gray_out(const Image* original);
const Image* change_luminosity(const Image* original, float factor, bool halve_alpha);
const Image* player_colored(const RGBColor& clr, const Image* original, const Image* mask);

}


#endif /* end of include guard: IMAGE_TRANSFORMATIONS_H */

