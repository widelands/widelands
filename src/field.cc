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
					 

void Field::set_neighb(Field* mln, Field* mrn, Field* mtln, Field* mtrn, Field* mbln, Field* mbrn) {
		  ln=mln; 
		  rn=mrn;
		  tln=mtln;
		  trn=mtrn;
		  bln=mbln;
		  brn=mbrn;
}

