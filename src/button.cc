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

/** class Button
 *
 * This defines a button.
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */

Pic Button::bg0;
Pic Button::bg1;
Pic Button::bg2;
Pic Button::bg0e;
Pic Button::bg1e;
Pic Button::bg2e;

/** Button::Button(const uint mx, const uint my, const uint mw, const uint mh, const uint bg, Pic* mdp, 
 * 		const uint addx, const uint addy);
 *
 * This function finally creates a button
 *
 * Args:	mx	posx in window
 * 		my	posy in window
 * 		mw	width	
 * 		mh	height
 * 		bg	background number to use
 * 		mdp	picture to draw in
 * 		addx	offset from the edge (for window borders)
 * 		addy	offset from the edge (for window borders)
 * Returns: Nothing
 */
Button::Button(const uint mx, const uint my, const uint mw, const uint mh, const uint bg, Pic* mdp, 
					 const uint addx, const uint addy) {
		  assert(bg0.get_w() && bg1.get_w() && bg2.get_w());
		  
		  x=mx;
		  y=my;
		  h=mh;
		  w=mw;

		  if(bg==0) { mybg=&bg0; myebg=&bg0e; }
		  if(bg==1) { mybg=&bg1; myebg=&bg1e; } 
		  if(bg==2) { mybg=&bg2; myebg=&bg2e; }

		  dp=mdp;
		  xp=addx;
		  yp=addy;

		  myp=0;

		  bpressed=false;
		  benlighted=false;
		  needs_draw=true;

		  func=0;
		  funca=0;
}
			
/** Button::~Button(void) 
 *
 * Cleanups
 *
 * Args: none
 * returns: Nothing
 */
Button::~Button(void) {
		  if(myp) delete myp;
		  // we do not care for function args. someone else must free them
//		  if(funca) free(funca);
}
					 

/** void Button::set_pic(Pic* p) 
 *
 * This sets the button picture
 *
 * Args:	p	picture to set
 * Returns: Nothing
 */
void Button::set_pic(Pic* p) {
		  assert(p);

		  if(myp) delete myp;
		  myp=p;
		  
		  needs_draw=true;
		  draw();
}

/** int Button::draw(void) 
 *
 * This draws the button in the current pic
 *
 * Args: none
 * Returns: 1 if it drawn something, 0 otherwise
 */
int Button::draw(void)
{
	if(!needs_draw) return 0;

	needs_draw=false;
		  	 
	if(!benlighted || bpressed) {
		Graph::copy_pic(dp, mybg, x+xp, y+yp, 0, 0, w, h);
	} else {
		Graph::copy_pic(dp, myebg, x+xp, y+yp, 0, 0, w, h);
	}
		 

	// if we got a picture, draw it
	if(myp) {
		// we simply center it, without checkin
		uint mw = myp->get_w() > w ? w : myp->get_w();
		uint mh = myp->get_h() > h ? h : myp->get_h();

		Graph::copy_pic(dp, myp, (x+xp+(w>>1))-(mw>>1), (y+yp+(h>>1))-(mh>>1), 0, 0, mw, mh);
	}

	// draw border
	uint myx = x + xp;
	uint myy = y + yp;
#define BLACK 0x000000	//TEMP
	if (!bpressed)
	{
		// top edge
		dp->brighten_rect(myx + 0, myy + 0, w, 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// left edge
		dp->brighten_rect(myx + 0, myy + 2, 2, h-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// bottom edge
		dp->fill_rect(myx + 2, myy + h-2, w-2, 1, BLACK);
		dp->fill_rect(myx + 1, myy + h-1, w-1, 1, BLACK);
		// right edge
		dp->fill_rect(myx + w-2, myy + 2, 1, h-2, BLACK);
		dp->fill_rect(myx + w-1, myy + 1, 1, h-1, BLACK);
	}
	else
	{
		// bottom edge
		dp->brighten_rect(myx + 0, myy + h-2, w, 2, BUTTON_EDGE_BRIGHT_FACTOR);
		// right edge
		dp->brighten_rect(myx + w-2, myy + 0, 2, h-2, BUTTON_EDGE_BRIGHT_FACTOR);
		// top edge
		dp->fill_rect(myx + 0, myy + 0, w-1, 1, BLACK);
		dp->fill_rect(myx + 0, myy + 1, w-2, 1, BLACK);
		// left edge
		dp->fill_rect(myx + 0, myy + 0, 1, h-1, BLACK);
		dp->fill_rect(myx + 1, myy + 0, 1, h-2, BLACK);
	}
#undef BLACK
		 
	return 1;
}
		  
