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

/** Field::Field(void) 
 *
 * Init. Trivial init (does nothing), because this
 * gets never called (no new, rather malloc)
 *
 * Args: None
 * Returns: Nothing
 */
Field::Field(void) {

}

/** Field::~Field(void) 
 *
 * Cleanups
 */
Field::~Field(void) {
}
					 

void Field::set_neighb(Field* mln, Field* mrn, Field* mtln, Field* mtrn, Field* mbln, Field* mbrn)
{
	ln=mln; 
	rn=mrn;
	tln=mtln;
	trn=mtrn;
	bln=mbln;
	brn=mbrn;
}

void Field::set_normal()
{
	// find normal
	// this is experimental, may be completely stupid;
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
	normal.x -= (height - ln->height) << HEIGHT_FACTOR;
	normal.x += (height - rn->height) << HEIGHT_FACTOR;
	normal.x -= (float)((height - tln->height) << HEIGHT_FACTOR) * COS60;
	normal.y -= (float)((height - tln->height) << HEIGHT_FACTOR) * SIN60;
	normal.x += (float)((height - trn->height) << HEIGHT_FACTOR) * COS60;
	normal.y -= (float)((height - trn->height) << HEIGHT_FACTOR) * SIN60;
	normal.x -= (float)((height - bln->height) << HEIGHT_FACTOR) * COS60;
	normal.y += (float)((height - bln->height) << HEIGHT_FACTOR) * SIN60;
	normal.x += (float)((height - brn->height) << HEIGHT_FACTOR) * COS60;
	normal.y += (float)((height - brn->height) << HEIGHT_FACTOR) * SIN60;
	normal.normalize();
#ifdef _MSC_VER
#pragma warning(default:4244)
#endif
}