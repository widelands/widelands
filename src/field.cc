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

/** Field::Field(ushort x, ushort y, uchar h, Field* mln, Field* mrn, Field* mtln, Field* mtrn, 
 * 		Field* mbln, Field* mbrn)
 *
 * Init
 *
 * Args:	x	xpos of field
 * 		y	ypos of field
 * 		h	height of field
 * 		mln, mrn	left	and right neigbor fields
 * 		mtln, mtrn	top left and top right neighbour fields
 * 		mbln, mbrn  bottom left and bottom right neighbour fields
 */
Field::Field(ushort x, ushort y, uchar h, Field* mln, Field* mrn, Field* mtln, Field* mtrn,
					 Field* mbln, Field* mbrn) {
		  
		  height=h;
		  xpos=x;
		  ypos=y;

		  ln=mln; 
		  rn=mrn;
		  tln=mtln;
		  trn=mtrn;
		  bln=mbln;
		  brn=mbrn;

        ypix=(((y+1)<<FIELD_SW_H)>>1)-(h<<HEIGHT_FACTOR);

		  if((y&1)) { // %2 
					 xpix=((((x<<1)+1)<<FIELD_SW_W)>>1);
		  } else {
					 xpix=(x<<FIELD_SW_W);
		  }

}


/** Field::~Field(void) 
 *
 * Cleanups
 */
Field::~Field(void) {
}

