/*
 * Copyright (C) 2002 by Holger Rapp 
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

#include "ui.h"

/** class Checkbox
 *
 * This defines a checkbox, which will be marked or unmarked, depending on their state
 * 
 * Depends:	g_gr
 * 			class Graph::Pic
 */

Pic Checkbox::gr;

/** Checkbox::~Checkbox(const uint mx, const uint my, const bool b, Pic* mdp, 
 * 		const uint addx, const uint addy);
 *
 * This function finally creates a button
 *
 * Args:	mx	posx in window
 * 		my	posy in window
 * 		b	initial state
 * 		mdp	picture to draw in
 * 		addx	offset from the edge (for window borders)
 * 		addy	offset from the edge (for window borders)
 * Returns: Nothing
 */
Checkbox::Checkbox(const uint mx, const uint my, const bool b, Pic* mdp, const uint addx, const uint addy) {
		  assert(gr.get_w() && gr.get_h());
		  
		  x=mx;
		  y=my;
		  bstate=b;
		  dp=mdp;
		  xp=addx;
		  yp=addy;
}
			

/** Checkbox::~Checkbox(void)
 *
 * Cleanups
 *
 * Args: none
 * returns: nothing
 */
Checkbox::~Checkbox(void) {

		  return;
}


/** int Checkbox::draw(void) 
 *
 * This draws the box in the current pic
 *
 * Args: none
 * Returns: 1 if it drawn something, 0 otherwise
 */
int Checkbox::draw(void) {

		  uint xoffs=0;

		  if(bstate) 
					 xoffs=CHECKBOX_WIDTH;

		  
		  Graph::copy_pic(dp, &gr, x+xp, y+yp, xoffs, 0, CHECKBOX_WIDTH, CHECKBOX_HEIGHT);

		  return 1;
}
