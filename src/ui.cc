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

#include "ui.h"


/////////////////////////////////////////////////////////////////////////////////////////////////


/** class Window
 *
 * This class offers a window. Should't be user directly
 *
 * The graphics (see static Pics) are used in the following manner: (Example)
 *
 *  <--20leftmostpixel_of_top--><60Pixels as often as possible to reach window with from top><20rightmost pixel of top>
 *  ^
 *  20 topmost pixels of l_border                                      <--- > same for r_border
 *  as often as needed: 60 pixels of l_border								  <--- > same for r_border
 *  20 bottom pixels of l_border													  <--- > same for r_borde
 *  <--20leftmostpixel_of_bot--><60Pixels as often as possible to reach window with from bot><20rightmost pixel of bot>
 * 
 * So: the l_border and the r_border pics MUST have a height of 100, while the width must be  20
 * 	 and the top and bot pics MUST have a width of 100, while the height must be 20
 * 	 
 * DEPENDS: Graph::Pic
 * 			Graph::draw_pic
 * 			Initalized g_gr object
 */

Pic Window::l_border;
Pic Window::r_border;
Pic Window::top;
Pic Window::bot;
Pic Window::bg;
	
/** Window::Window(const unsigned int px, const unsigned int py, const unsigned int wi, const unsigned int he, const unsigned int gid, 
 * 	const Flags f) 
 *
 * 	This is the constructor with which a window gets created
 *
 * 	Args:	x	x pos of window
 * 			y 	y pos of window
 * 			w	width of window
 * 			h	height of window
 * 			id the identifier of this window
 * 			f	what window to create
 */
Window::Window(const unsigned int px, const unsigned int py, const unsigned int wi, const unsigned int he, const unsigned int gid, const Flags f) {
		  assert(bg.get_w() && r_border.get_w() && l_border.get_w() && top.get_w() && bot.get_w() && "Window class is not fully initalized!");

		  x=px;
		  y=py;
		  w=wi;
		  h=he;
		  id=gid;
		  myf=f;

		  own_bg=0;
					
 
}

/** Window::~Window()
 *
 * Default destructor
 *
 * Args: None
 * Returns: Nothing
 */
Window::~Window(void) {
		  if(own_bg) delete own_bg;
}

/** void Window::draw(void)
 *
 * This function draws the current window on the g_gr object
 *
 * Args: None
 * Returns: Nothing
 */
void Window::draw(void) {
		  unsigned int i, j;
		 
		  Pic* usebg= own_bg ? own_bg : &bg ;
		
		  // Top n Bottom
		  // top
		  draw_pic(&top, x, y, 0, 0, CORNER, CORNER);
		  // bot
		  draw_pic(&bot, x, y+h-CORNER, 0, 0, CORNER, CORNER);
		  for(i=x+CORNER; i<(w+x)-CORNER-MIDDLE; i+=MIDDLE) {
					 // top
					 draw_pic(&top, i, y, CORNER, 0, MIDDLE, CORNER);
					 // bot
					 draw_pic(&bot, i, y+h-CORNER, CORNER, 0, MIDDLE, CORNER);
		  } 
		  // top
		  draw_pic(&top, i, y, CORNER, 0, w+x-CORNER-i, CORNER);
		  draw_pic(&top, (x+w)-CORNER, y, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);
		  // bot
		  draw_pic(&bot, i, y+h-CORNER, CORNER, 0, w+x-CORNER-i, CORNER);
		  draw_pic(&bot, (x+w)-CORNER, y+h-CORNER, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);

		  
		  // borders
		  // left
		  draw_pic(&l_border, x, y+CORNER, 0, 0, CORNER, CORNER);
		  // right
		  draw_pic(&r_border, x+w-CORNER, y+CORNER, 0, 0, CORNER, CORNER);
		  for(i=y+CORNER+CORNER; i<(h+y)-CORNER-CORNER-MIDDLE; i+=MIDDLE) {
					 // left
					 draw_pic(&l_border, x, i, 0, CORNER, CORNER, MIDDLE);
					 // right
					 draw_pic(&r_border, x+w-CORNER, i, 0, CORNER, CORNER, MIDDLE);
		  } 
		  // left
		  draw_pic(&l_border, x, i, 0, CORNER, CORNER, h+y-CORNER-i);
		  draw_pic(&l_border, x, y+h-CORNER-CORNER, 0, l_border.get_h()-CORNER, CORNER, CORNER);
		  // right
		  draw_pic(&r_border, x+w-CORNER, i, 0, CORNER, CORNER, h+y-CORNER-i);
		  draw_pic(&r_border, x+w-CORNER, y+h-CORNER-CORNER, 0, r_border.get_h()-CORNER, CORNER, CORNER);

		  // bg
		  for(j=y+CORNER; (int)j<(int)(h+y)-CORNER-CORNER-usebg->get_h(); j+=usebg->get_h()) {
					 for(i=x+CORNER; (int)i<(int)(x+w)-CORNER-CORNER-usebg->get_w(); i+=usebg->get_w()) {
								draw_pic(usebg, i, j, 0, 0, usebg->get_w(), usebg->get_h());
					 } 
					 draw_pic(usebg, i, j, 0, 0, w+x-i-CORNER, usebg->get_h());
		  }
		  for(i=x+CORNER; (int)i<(int)(w+x)-CORNER-usebg->get_w(); i+=usebg->get_w()) {
					 draw_pic(usebg, i, j, 0, 0, usebg->get_w(), y+h-j-CORNER);
		  } 
		  draw_pic(usebg, i, j, 0, 0, w+x-i-CORNER, y+h-j-bot.get_h());

}
					 
/** void Window::set_new_bg(Pic* p);
 *
 * Setting a non default bg for a window. Window cares for clearup on close
 *
 * Args: p	new pic to set
 * returns: Nothing
 */
void Window::set_new_bg(Pic* p) {
		  assert(p);


		  if(own_bg) delete own_bg;
		  own_bg=p;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


/** class User_Interface
 *
 * This class offers a user interface. This means windows, buttons and
 * so on.
 *
 * It's a Singleton.
 * 
 * DEPENDS: on all the sub interface classes (Buttons, Textareas, windows a.s.on)
 * 			class Graph::Pic
 * 			func	Graph::draw_pic 
 * 			Initalized g_gr object
 */

/** User_Interface::User_Interface(void) 
 *
 * Def Constructor
 *
 * Args: none
 * Returns: Nothing
 */
User_Interface::User_Interface(void) {
		  wins=(Window**) malloc(sizeof(Window*));
		  nwins=0;
}

/** User_Interface::~User_Interface(void) 
 *
 * Def Destructor
 *
 * Args: none
 * Returns: Nothing
 */
User_Interface::~User_Interface(void) {
		  if(wins) {    
					 for(unsigned int i=0; i<nwins; i++) {
								delete wins[i];
					 }
					 free(wins);
					 nwins=0;
		  }

}

/** void User_Interface::draw(void) 
 *
 * Draws the whole user interface on the g_gr object
 *
 * Args: none
 * Returns: Nothing
 */
void User_Interface::draw(void) {
		  if(nwins) {
					 for(unsigned int i=0; i<nwins; i++) {
								wins[i]->draw();
					 }
		  }
}
		  
/** void User_Interface::set_win_bg(const unsigned int id, Pic* p);
 *
 * This sets a new background for the window
 * 
 *	Args: id	Id of window to set
 *			p	picture to use
 *	Returns: Nothing
 */
void User_Interface::set_win_bg(const unsigned int id, Pic* p) {
		  Window* win=get_window(id);

		  win->set_new_bg(p);

}

/** unsigned int User_Interface::create_window(const unsigned int x, const unsigned int y, const unsigned int w, 
 *   	const unsigned int h, const Window::Flags f=Window::DEFAULT) 
 *
 * This function creates a window
 *
 * Args:	x	initial x position
 * 		y	initial y position
 * 		w	width
 * 		h	height
 * 		f	Flags
 * returns: window id
 */
unsigned int User_Interface::create_window(const unsigned int x, const unsigned int y, const unsigned int w, 
					 const unsigned int h, const Window::Flags f=Window::DEFAULT) {
		  static unsigned int id=0;
		 
		  nwins++;
		  wins=(Window**) realloc(wins, sizeof(Window*)*nwins);
		  
		  wins[nwins-1]=new Window(x, y, w, h, id, f);
		  
		  ++id;
		  
		  return (id-1); 
}
