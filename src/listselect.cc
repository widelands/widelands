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
#include "font.h"
#include <string.h>

/** class Listselect
 *
 * This class defines a list-select box. 
 *
 * Depends: class Graph::Pic
 * 			g_fh
 * 			class Button
 */
uint Listselect::nfont;
ushort Listselect::frameclr, Listselect::bgclr, Listselect::selclr;

/** Listselect::Listselect(uint mx, uint my, uint mw, uint mh, Pic* mdp, uint addx, uint addy) 
 *
 * This function makes a listselect 
 *
 * Args:	mx	posx in window
 * 		my	posy in window
 * 		mw	width	(pixels)
 * 		mh	height (pixels, but this will be cut to a complete row count) 
 * 		mdp	picture to draw in
 * 		addx	offset from the edge (for window borders)
 * 		addy	offset from the edge (for window borders)
 * Returns: Nothing
 */
Listselect::Listselect(uint mx, uint my, uint mw, uint mh, Pic* mdp, uint addx, uint addy)  {
		  x=mx; 
		  y=my;
		  w=mw;
		  h=(mh/(g_fh.get_fh(nfont)+2)); 
		  dp=mdp;
		  xp=addx;
		  yp=addy;

		  nent=0;
		  firstvis=0;
		  cursel=-1;
}

/** Listselect::~Listselect(void) 
 *
 * Cleanups
 *
 * Args: None
 * Returns: Nothing
 */
Listselect::~Listselect(void) {
		  for(uint i=0; i<nent; i++) {
					 delete ent[i].p;
		  }
		 
		  return;
}

/** int Listselect::draw(void) 
 *
 * Draws the listselect box
 *
 * Args: none
 * returns: 0 when nothing was drawed, 1 elseways
 */
int Listselect::draw(void) {
		  // Draw the frame
		  for(uint mx=x+xp; mx<x+xp+w; mx++) {
					 dp->set_pixel(mx, y+yp, frameclr);
					 dp->set_pixel(mx, y+yp+this->get_h()-1, frameclr);
		  }
		  uint my;
		  for(my=y+yp; my<y+yp+this->get_h(); my++) {
					 dp->set_pixel(x+xp, my, frameclr);
					 dp->set_pixel(x+xp+w-1, my, frameclr);
		  }

		  // Draw a box into the picture
		  for(my=y+yp+1; my<(y+yp+this->get_h()-1); my++) {
					 dp->set_pixel(x+xp+1, my, bgclr);
					 for(uint mx=x+xp+2; mx<(x+xp+w-1); mx++) {
								dp->set_npixel(bgclr);	
					 }
		  }

		  for(uint i=0; (i<h) && (i+firstvis < nent); i++) {
					 if(i+firstvis == (uint)cursel) {
								// Draw a box
								for(uint my=y+yp+(i*(g_fh.get_fh(nfont)+2)); my<y+yp+(i*(g_fh.get_fh(nfont)+2))+ent[i+firstvis].p->get_h(); my++) {
										  dp->set_pixel(x+xp, my, selclr);
										  for(uint mx=x+xp+1; mx<x+xp+w; mx++) {
													 dp->set_npixel(selclr);
										  }
								}
					 }
					 Graph::copy_pic(dp, ent[i+firstvis].p, x+xp, y+yp+(i*(g_fh.get_fh(nfont)+2)), 0, 0, 
										  ent[i+firstvis].p->get_w() > w ? w : ent[i+firstvis].p->get_w(), ent[i+firstvis].p->get_h());
		  }
		  return 1;
}


/** void Listselect::add_entry(const char* name, const char* value =0) 
 *
 * This adds a new entry to the list
 *
 * Args: 	name 	this is the line that will appear in the box
 * 			value	this is the value that will be returned by get_selection
 * 			 (if this is zero, name will used as value)
 * Returns: Nothing
 */
void Listselect::add_entry(const char* name, const char* value) {
		  if(!name) return;

		  const char* realval=value;
		  
		  if(!value) realval=name;
		 
		  
		  strncpy(ent[nent].value, realval, 255);
		  ent[nent].p= g_fh.get_string(name, nfont);

		  ++nent;

		  draw();

		  assert(nent < MAX_LISTENTRYS);
}

/** void Listselect::select(uint y)
 *
 * a click in the box has happened.
 *
 * Args: y 	ypos of click in box
 * Returns: Nothing
 */
void Listselect::select(uint y) {
		  uint sel;

		  sel=(y/(g_fh.get_fh(nfont)+2)+firstvis); 
		  
		  if(sel>=nent) sel=nent-1;

		  cursel=sel;
		  bnew_selection=true;
}

/** void listselect_but_up(void *a) 
 *
 * This is the click function for the up button
 *
 * Args: a == pt to Listselect
 * Returns: Nothing
 */
void listselect_but_up(Window* par, void* a) {
		  Listselect* sel=(Listselect*) a;

		  sel->move_up(1);
		  g_gr.register_update_rect(par->get_xpos()+sel->get_xpos(),
								par->get_ypos()+sel->get_ypos(),
								sel->get_w(),
								sel->get_h());

}

/** void down_up(void *a) 
 *
 * This is the click function for the down button
 *
 * Args: par parent window
 * 		a == pt to Listselect
 * Returns: Nothing
 */
void listselect_but_down(Window* par, void* a) {
		  Listselect* sel=(Listselect*) a;

		  sel->move_down(1);
		  g_gr.register_update_rect(par->get_xpos()+sel->get_xpos(),
								par->get_ypos()+sel->get_ypos(),
								sel->get_w(),
								sel->get_h());

}
					 
		  
