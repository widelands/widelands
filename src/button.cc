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
int Button::draw(void) {
		  if(!needs_draw) return 0;

		  needs_draw=false;
		  
		  uint j;
		  ushort clr;
		 
		  if(!benlighted || bpressed) {
					 Graph::copy_pic(dp, mybg, x+xp+2, y+yp+2, 2, 2, w-4, h-4);
		  } else {
					 Graph::copy_pic(dp, myebg, x+xp+2, y+yp+2, 2, 2, w-4, h-4);
		  }
		 

		  // if we got a picture, draw it
		  if(myp) {
					 // we simply center it, without checkin
					 uint mw = myp->get_w() > w ? w : myp->get_w();
					 uint mh = myp->get_h() > h ? h : myp->get_h();

					 Graph::copy_pic(dp, myp, (x+xp+(w>>1))-(mw>>1), (y+yp+(h>>1))-(mh>>1), 0, 0, mw, mh);
		  }
		  
		  // draw the borders
		  if(!bpressed) {
					 // left edge. we need to bright up the colors of the texture
					 // top row
					 j=0;
					 clr=mybg->get_pixel(j, 0);
					 dp->set_pixel(xp+x+j, y+yp, Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
					 while(j<(w-1)) { 
								clr=mybg->get_npixel();
								dp->set_npixel(Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								j++;
					 }
					 // Second row
					 j=0;
					 clr=mybg->get_pixel(j, 1);
					 dp->set_pixel(xp+x+j, y+yp+1, Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
					 while(j<(w-1)) { 
								clr=mybg->get_npixel();
								dp->set_npixel(Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								j++;
					 } 

					 // left edge
					 j=2;
					 while(j<(h-1)) {
								clr=mybg->get_pixel(0, j);
								dp->set_pixel(xp+x, yp+y+j, Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								clr=mybg->get_npixel();
								dp->set_npixel(Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								j++;
					 }


					 // right edge
					 dp->set_pixel(x+xp+w-1, y+yp, 0, 0, 0);
					 j=y+yp+1;
					 while(j < y+yp+h-1) { dp->set_pixel(x+xp+w-2, j, 0, 0, 0); dp->set_npixel(0, 0, 0); ++j; }

					 // bottom edge
					 j=x+xp;
					 dp->set_pixel(j, y+yp+h-1, 0, 0, 0);
					 while(j < x+xp+w-1) { dp->set_npixel(0,0,0); ++j; }
					 j=x+xp+1;
					 dp->set_pixel(j, y+yp+h-2, 0, 0, 0);
					 while(j < x+xp+w-1) { dp->set_npixel(0,0,0); ++j; }

		  } else {
					 // top edge
					 j=xp+x+w-1;
					 dp->set_pixel(j, yp+y, 0, 0, 0);
					 while(j>xp+x) { --j;  dp->set_ppixel(0,0,0); }
					 j=xp+x+w-2;
					 dp->set_pixel(j, yp+y+1, 0, 0, 0);
					 while(j>xp+x) { --j;  dp->set_ppixel(0,0,0); }
					
					 // left edge
					 for(j=yp+y+2; j<yp+y+h-1; j++) {
								dp->set_pixel(xp+x, j, 0, 0, 0);
								dp->set_npixel(0, 0, 0);
					 }
					 dp->set_pixel(xp+x, yp+y+h-1, 0, 0, 0); 
		  
					 // Bottom edge
					 // Top row
					 j=2;
					 clr=mybg->get_pixel(j, 0);
					 dp->set_pixel(xp+x+j, y+yp+h-2, Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
					 while(j<(w-1)) { 
								clr=mybg->get_npixel();
								dp->set_npixel(Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								j++;
					 } 
					 // bottom row
					 j=1;
					 clr=mybg->get_pixel(j, 1);
					 dp->set_pixel(xp+x+j, y+yp+h-1, Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
					 while(j<(w-1)) { 
								clr=mybg->get_npixel();
								dp->set_npixel(Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								j++;
					 } 

					 // Right edge
					 // left edge
					 clr=mybg->get_pixel(0, 0);
					 dp->set_pixel(xp+x+w-1, yp+y+1,  Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
					 j=2;
					 while(j<h) {
								clr=mybg->get_pixel(0, j);
								dp->set_pixel(xp+x+w-1, yp+y+j, Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								clr=mybg->get_npixel();
								dp->set_ppixel(Graph::bright_up_clr(clr, BUTTON_EDGE_BRIGHT_FACTOR));
								j++;
					 }

 
		  }
		 
		  return 1;
}
		  
