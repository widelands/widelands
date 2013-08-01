/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#include "logic/field.h"

#include "wui/mapviewpixelfunctions.h"

namespace Widelands {

/**
 * Set the field's brightness based upon the slopes.
 * Slopes are calulated as this field's height - neighbour's height.
 */
void Field::set_brightness
	(int32_t const l,
	 int32_t const r,
	 int32_t const tl,
	 int32_t const tr,
	 int32_t const bl,
	 int32_t const br)
{
	// HACK to normalize flat terrain to zero brightness
	static float flatbrightness =
		MapviewPixelFunctions::calc_brightness(0, 0, 0, 0, 0, 0);

	float b =
		MapviewPixelFunctions::calc_brightness
			(l, r, tl, tr, bl, br) - flatbrightness;

	if (b > 0)
		b *= 1.5;

	if      (b < -128)
		b = -128;
	else if (b >  127)
		b =  127;
	brightness = static_cast<int8_t>(b); //FIXME: ARGH !!
}

}
