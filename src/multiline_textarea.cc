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

////////////////////////////////////////////////////////////////////////////////////////7

/** class Multiline_textarea 
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */

uint Multiline_Textarea::nfont=0;

/* Multiline_Textarea::Multiline_Textarea(const uint px, const uint py, const uint myw, const Align a, Pic* mdp)
 *
 * This creates a Textarea.
 *
 * Args: px	xpos in win
 * 		py 	ypos in win
 * 		myw,myh	sizes of area
 * 		a	alignment to use
 * 		mdp picture to draw into
 * 		addx	offset from the edge (for frame)
 * 		addy	offset from the edge (for frame)
 * Returns: Nothing
 */
Multiline_Textarea::Multiline_Textarea(const uint px, const uint py, const uint myw, const uint myh, const Align a, Pic* mdp, const uint addx, 
					 const uint addy) {
		  w=myw; 
		  h=(myh/(g_fh.get_fh(nfont)+2));
		  dp=mdp;
		  al=a;

		  xp=addx;
		  yp=addy;
		  x=px;
		  y=py;
		  bak=new Pic();
		  bak->set_size(w, get_h());
		  Graph::copy_pic(bak, dp, 0,0, x+xp, y+yp, w, get_h());
		 
		  lines=0;
		  ar= new Growable_Array(1, 5);
		  firstvis=0;
}

/** void Multiline_Textarea::set_text(const char* str)
 *
 * This sets the string of the Textarea
 *
 * Args: mystr	string to set
 * Returns: Nothing
 */
void Multiline_Textarea::set_text(const char* mystr) {
		  if(lines) {
					 for(uint i=0; i<lines; i++) 
								delete (Pic*) ar->element_at(i);
					 ar->flush(1);
					 lines=0;
		  }
		 
		  int i=0;
		  int n=0;
		  int len=strlen(mystr);
		  char *str = new char[len+1];
		  strcpy(str, mystr);
		  str[len]='\n';
		  char *buf = new char[len+1];
		  do {
					 buf[n]=str[i];
					 if(buf[n]=='\n' || str[i+1]=='\0') {
								buf[n]='\0';
								Pic* add=g_fh.get_string(buf, nfont);
								if(add->get_w() > w) {
up:
										  // Big trouble, line doesn't fitt in one, we must do a break
										  while(buf[n]!=' ' && buf[n]!='.' && buf[n]!=',' && n) { --n; }
										  if(!n) { 
													 // well, this huge line is just one word, so we just have
													 // to take it over, the last few chars are then cut in draw()
													 ar->add(add);
													 ++lines;
										  } else {
													 i-=strlen(buf);
													 if(buf[n]==' ') buf[n]='\0';
													 else buf[n+1]='\0';
													 i+=strlen(buf);
													 delete add;
													 add=(g_fh.get_string(buf, nfont));
													 if(add->get_w() > w) {
																--n;
																goto up;
													 }
													
													 ar->add(g_fh.get_string(buf, nfont));
													 ++lines;
										  }
								} else {
										  // Everything is fine
										  ar->add(add);
										  ++lines;
								}
								n=-1;
					 }
					 ++n;
					 ++i;
		  } while(i<=len);

		  delete[] str;
		  delete[] buf;
		  draw();
}

/** Multiline_Textarea::~Multiline_Textarea(void)
 *
 * Destructor
 *
 * Args: None
 * Returns: Nothing
 */
Multiline_Textarea::~Multiline_Textarea(void) {
					 Pic* txt;
		  for(uint i=0; i<lines; i++) {
					 txt=(Pic*) ar->element_at(i);
					 delete txt;
		  }
		  delete ar;
		  
		  delete bak;
}

/** void Multiline_Textarea::draw(const uint xp, const uint yp) const 
 *
 * Draws a Textarea into the windows picture
 *
 *	Args: None
 * Returns: Nothing
 */
void Multiline_Textarea::draw(void) const {
		  if(!lines) return;
		  uint posx, myw;
		  Pic* txt;
		 
		  Graph::copy_pic(dp, bak, xp+x, yp+y, 0, 0, w, get_h());

		  for(uint i=0; (i<h) && (i+firstvis<lines); i++) {
					 txt= (Pic*) ar->element_at(i+firstvis);

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
								return;
					 }

					 Graph::copy_pic(dp, txt, posx, yp+y+i*(g_fh.get_fh(nfont)+2), 0, 0, myw, txt->get_h());
		  }

}

/** void multiline_textarea_but_up(void *a) 
 *
 * This is the click function for the up button
 *
 * Args: a == pt to Listselect
 * Returns: Nothing
 */
void multiline_textarea_but_up(Window* par, void* a) {
		  Multiline_Textarea* multexta=(Multiline_Textarea*) a;

		  multexta->move_up(1);
		  g_gr.register_update_rect(par->get_xpos()+multexta->get_xpos(),
								par->get_ypos()+multexta->get_ypos(),
								multexta->get_w(),
								multexta->get_h());

}

/** void multiline_textarea_but_down(void *a) 
 *
 * This is the click function for the down button
 *
 * Args: par parent window
 * 		a == pt to Multiline_Textarea
 * Returns: Nothing
 */
void multiline_textarea_but_down(Window* par, void* a) {
		  Multiline_Textarea* multexta=(Multiline_Textarea*) a;

		  multexta->move_down(1);
		  g_gr.register_update_rect(par->get_xpos()+multexta->get_xpos(),
								par->get_ypos()+multexta->get_ypos(),
								multexta->get_w(),
								multexta->get_h());

}
					 
	
