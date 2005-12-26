/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include <vector>
#include "bob.h"
#include "field.h"
#include "geometry.h"

#define V3	(float)0.57735 // sqrt(1/3)
#define LIGHT_FACTOR		75

/**
 * Calculate brightness based upon the slopes.
 */
static float calc_brightness(int l, int r, int tl, int tr, int bl, int br)
{
	static Vector sun_vect = Vector(V3, -V3, -V3);	// |sun_vect| = 1

	Vector normal;

// find normal
// more guessed than thought about
// but hey, results say i'm good at guessing :)
// perhaps i'll paint an explanation for this someday
// florian
#define COS60	0.5
#define SIN60	0.86603
#ifdef _MSC_VER
// don't warn me about fuckin float conversion i know what i'm doing
#pragma warning(disable:4244)
#endif
		  normal = Vector(0, 0, FIELD_WIDTH);
		  normal.x -= MULTIPLY_WITH_HEIGHT_FACTOR(l);
		  normal.x += MULTIPLY_WITH_HEIGHT_FACTOR(r);
		  normal.x -= (float)(MULTIPLY_WITH_HEIGHT_FACTOR(tl)) * COS60;
		  normal.y -= (float)(MULTIPLY_WITH_HEIGHT_FACTOR(tl)) * SIN60;
		  normal.x += (float)(MULTIPLY_WITH_HEIGHT_FACTOR(tr)) * COS60;
		  normal.y -= (float)(MULTIPLY_WITH_HEIGHT_FACTOR(tr)) * SIN60;
		  normal.x -= (float)(MULTIPLY_WITH_HEIGHT_FACTOR(bl)) * COS60;
		  normal.y += (float)(MULTIPLY_WITH_HEIGHT_FACTOR(bl)) * SIN60;
		  normal.x += (float)(MULTIPLY_WITH_HEIGHT_FACTOR(br)) * COS60;
		  normal.y += (float)(MULTIPLY_WITH_HEIGHT_FACTOR(br)) * SIN60;
		  normal.normalize();
#ifdef _MSC_VER
#pragma warning(default:4244)
#endif

	float b = normal * sun_vect;
	b *= -LIGHT_FACTOR;

	return b;
}

/**
 * Set the field's brightness based upon the slopes.
 * Slopes are calulated as this field's height - neighbour's height.
 */
void Field::set_brightness(int l, int r, int tl, int tr, int bl, int br)
{
	static float flatbrightness = 0; // HACK to normalize flat terrain to zero brightness
	if (!flatbrightness)
		flatbrightness = calc_brightness(0, 0, 0, 0, 0, 0);

	float b = calc_brightness(l, r, tl, tr, bl, br) - flatbrightness;

	if (b > 0)
		b *= 1.5;

	if (b < -128) b = -128;
	else if (b > 127) b = 127;
	brightness = (char)b;
}


