/*
 * Copyright (C) 2001 by Holger Rapp 
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


#include "map.h"

/** class Field
 *
 * a field like it is represented in the game
 */


#define V3	(float)0.57735 // sqrt(1/3)
#define LIGHT_FACTOR		75

/** Field::set_brightness(int l, int r, int tl, int tr, int bl, int br)
 *
 * Setup the field's brightness, based upon the slopes.
 * Slopes are calulated as this field's height - neighbour's height.
 */
void Field::set_brightness(int l, int r, int tl, int tr, int bl, int br)
{
	static Vector sun = Vector(V3, -V3, -V3);	// |sun| = 1

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
		  normal.x -= l * HEIGHT_FACTOR;
		  normal.x += r * HEIGHT_FACTOR;
		  normal.x -= (float)(tl * HEIGHT_FACTOR) * COS60;
		  normal.y -= (float)(tl * HEIGHT_FACTOR) * SIN60;
		  normal.x += (float)(tr * HEIGHT_FACTOR) * COS60;
		  normal.y -= (float)(tr * HEIGHT_FACTOR) * SIN60;
		  normal.x -= (float)(bl * HEIGHT_FACTOR) * COS60;
		  normal.y += (float)(bl * HEIGHT_FACTOR) * SIN60;
		  normal.x += (float)(br * HEIGHT_FACTOR) * COS60;
		  normal.y += (float)(br * HEIGHT_FACTOR) * SIN60;
		  normal.normalize();
#ifdef _MSC_VER
#pragma warning(default:4244)
#endif

	float b = normal * sun;
	b *= -LIGHT_FACTOR;

	// is this necessary?
	if (b < -128) b = -128;
	else if (b > 127) b = 127;
	brightness = (char)b;
}

