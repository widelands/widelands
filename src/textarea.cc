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

////////////////////////////////////////////////////////////////////////////////////////7

/** class Textarea 
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */

uint Textarea::nfont=0;

/* Textarea::Textarea(const uint px, const uint py, const char* t, const Align a, const uint winw, const uint winh, Pic* mdp)
 *
 * This creates a textarea out of a fixed string.
 *
 * Args: px	xpos in win
 * 		py 	ypos in win
 * 		t	text to set
 * 		a	alignment to use
 * 		winw	width of window
 * 		winh	height of window
 * 		mdp	Picture to draw in
 * 		addx	offset from the edge (for frame)
 * 		addy	offset from the edge (for frame)
 * Returns: Nothing
 */
Textarea::Textarea(const uint px, const uint py, const char* t, const Align a, const uint winw, const uint winh, Pic* mdp, const uint addx, const uint addy) {
		 
		  txt=g_fh.get_string(t, nfont);

		  int myx=px;
		  int myy=py;
		  uint myw=txt->get_w();
		  uint myh=txt->get_h();
		  
		  if(myx+txt->get_w() > winw) myx=winw-txt->get_w();
		  if(myx<0) { myx=px; myw=winw-px; }
		  if(myy+txt->get_h() > winh) myy=winh-txt->get_h();
		  if(myy<0) { myy=py; myh=winh-py; }
		  
		  x=myx; 
		  y=myy; 
		  w=myw; 
		  h=myh;
		  dp=mdp;
		  
		  xp=addx; 
		  yp=addy;

		  al=a;
		 
		  bak=new Pic();
		  bak->set_size(w, h);
		  Graph::copy_pic(bak, dp, 0,0, x+xp, y+yp, w, h);
		  draw();
}

/* Textarea::Textarea(const uint px, const uint py, const uint myw, const Align a, Pic* mdp)
 *
 * This creates a textarea.
 *
 * Args: px	xpos in win
 * 		py 	ypos in win
 * 		w	width of area
 * 		a	alignment to use
 * 		mdp picture to draw into
 * 		addx	offset from the edge (for frame)
 * 		addy	offset from the edge (for frame)
 * Returns: Nothing
 */
Textarea::Textarea(const uint px, const uint py, const uint myw, const Align a, Pic* mdp, const uint addx, 
					 const uint addy) {
		 
		  txt=0; 
		  w=myw; 
		  h=get_fh();
		  dp=mdp;
		  al=a;

		  xp=addx;
		  yp=addy;
		  x=px;
		  y=py;
		  bak=new Pic();
		  bak->set_size(w, h);
		  Graph::copy_pic(bak, dp, 0,0, x+xp, y+yp, w, h);
}

/** void Textarea::set_text(const char* str)
 *
 * This sets the string of the textarea
 *
 * Args: str	string to set
 * Returns: Nothing
 */
void Textarea::set_text(const char* str) {
		  if(txt) delete txt;
        txt=g_fh.get_string(str, nfont);
		  
		  draw();
}

/** Textarea::~Textarea(void)
 *
 * Destructor
 *
 * Args: None
 * Returns: Nothing
 */
Textarea::~Textarea(void) {

		  if(txt) delete txt;
		  delete bak;

		  txt=0;
}

/** void Textarea::draw(const uint xp, const uint yp) const 
 *
 * Draws a textarea into the windows picture
 *
 *	Args: None
 * Returns: Nothing
 */
void Textarea::draw(void) const {
		  if(!txt) return;
		  uint posx, myw;
		  
		  myw= w < txt->get_w() ? w : txt->get_w();
		 

		  if(al==RIGHTA) {
					 posx=xp+x+w-myw;
		  } else if(al==LEFTA) {
					 posx=xp+x;
		  } else if(al==CENTER) {
					 posx=xp+x+((w>>1) - (myw>>1));
		  } else {
					 // Never here!!
					 assert(0);
		  }

		  Graph::copy_pic(dp, bak, posx, yp+y, 0, 0, w, h);
		  Graph::copy_pic(dp, txt, posx, yp+y, 0, 0, myw, h);
}
