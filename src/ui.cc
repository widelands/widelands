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
 * 			f	what window to create
 */
Window::Window(const unsigned int px, const unsigned int py, const unsigned int wi, const unsigned int he, const Flags f) {
		  assert(bg.get_w() && r_border.get_w() && l_border.get_w() && top.get_w() && bot.get_w() && "Window class is not fully initalized!");

		  x=px;
		  y=py;
		  w=wi;
		  h=he;
		  myf=f;
					 
		  nta=0;
		  ta=(Textarea**) malloc(sizeof(Textarea*));
					 
		  subids=0;

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
		  //unsigned int i;
		  
/*		  for(i=0 ; i< nta; i++) 
					 delete ta[i];
		  free(ta);
*/		  
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
		
		  if(myf != FLAT) {
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
					 for(j=y+CORNER; (int)j<=(int)((h+y)-CORNER-CORNER-usebg->get_h()); j+=usebg->get_h()) {
								for(i=x+CORNER; (int)i<=(int)((x+w)-CORNER-CORNER-usebg->get_w()); i+=usebg->get_w()) {
										  draw_pic(usebg, i, j, 0, 0, usebg->get_w(), usebg->get_h());
								} 
								draw_pic(usebg, i, j, 0, 0, w+x-i-CORNER, usebg->get_h());
					 }
					 for(i=x+CORNER; (int)i<=(int)((w+x)-CORNER-usebg->get_w()); i+=usebg->get_w()) {
								draw_pic(usebg, i, j, 0, 0, usebg->get_w(), y+h-j-CORNER);
					 } 
					 draw_pic(usebg, i, j, 0, 0, w+x-i-CORNER, y+h-j-bot.get_h());
		  } else {
					 // has no borders. Simply paste once the pic
					 unsigned int mw = usebg->get_w() > w ? w : usebg->get_w();
					 unsigned int mh = usebg->get_h() > h ? h : usebg->get_h();
		 
					 draw_pic(usebg, x, y, 0, 0, mw, mh);
		  }
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

/** 
 * unsigned int Window::create_textarea(const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h, const Textarea::Align t 
 *  	= Textarea::RIGHTA) 
 *
 *  This function creates a new textarea
 *  Args: x,y	position of the textarea relative to left/top edge of window
 *  		 h,w	height and width
 *  		 f		Alignment of the text in the area
 *  Returns: ID of this thingy
 */
unsigned int Window::create_textarea(const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h, const Textarea::Align t 
					   = Textarea::RIGHTA) {

		  return 0;

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
		  first = new win_p;
		  first->next=0;
		  first->prev=0;
		  first->w=0;

		  last=first;
}

/** User_Interface::~User_Interface(void) 
 *
 * Def Destructor
 *
 * Args: none
 * Returns: Nothing
 */
User_Interface::~User_Interface(void) {
		  win_p* t;
		  for(win_p* p=first; p &&  p->w; ) {
					 t=p->next;
					 delete p->w;
					 delete p;
					 p=t;
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
		  for(win_p* p=first; p && p->w; p=p->next) {
					 p->w->draw();
		  }
}

/** Window* User_Interface::create_window(const unsigned int x, const unsigned int y, const unsigned int w, 
 *   	const unsigned int h, const Window::Flags f=Window::DEFAULT) 
 *
 * This function creates a window
 *
 * Args:	x	initial x position
 * 		y	initial y position
 * 		w	width
 * 		h	height
 * 		f	Flags
 * returns: pt to window 
 */
Window*  User_Interface::create_window(const unsigned int x, const unsigned int y, const unsigned int w, 
					 const unsigned int h, const Window::Flags f=Window::DEFAULT) {
		  Window* win;
		  
		  if(f==Window::FLAT) {
					 win=new Window(x, y, w, h, f);
		  } else {
					 unsigned int get_b=Window::get_border();
					 unsigned int mw=w+get_b;
					 unsigned int mh=h+get_b;
					 if(x+mw > g_gr.get_xres()) mw=g_gr.get_xres();
					 if(y+mh > g_gr.get_yres()) mh=g_gr.get_yres(); 
					 win= new Window(x, y, mw, mh, f);
		  }

		  last->w=win;
		  last->next=new win_p;
		  
		  last->next->prev=last;
		  last=last->next;
		  last->next=0;
		  last->w=0;

		  return win;
}
		  
/** void User_Interface::delete_window(Window* win) 
 *
 * This function finally removes a window
 *
 * Args: win 	pointer to window to delete
 * Returns: Nothing
 */
void User_Interface::delete_window(Window* win) { 
		  assert(win);
		  
		  win_p* w=first;
		  while(w && w->w!=win) w=w->next;

		  assert(w!=last);


		  delete w->w;
		  w->w=0;
		  if(w==first) {
					 w->next->prev=0;
					 first=w->next;
		  } else {
					 w->prev->next=w->next;
					 w->next->prev=w->prev;
		  }
		  delete w;
}


////////////////////////////////////////////////////////////////////////////////////////7

/** class Textarea 
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */
					 
/*
Textarea(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const Align = LEFT);
					 ~Textarea(void);
					 
					 void set_text(const char*);
					 void draw(const unsigned int, const unsigned int) const ;
					 
		  private: 
					 static unsigned int nfont;
					 unsigned int x, y, w, h;
					 Align myal;
					 
					 Pic* txt;
};
*/
