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
#include "input.h"

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
	
/** Window::Window(const uint px, const uint py, const uint wi, const uint he, const uint gid, 
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
Window::Window(const uint px, const uint py, const uint wi, const uint he, const Flags f) {
		  assert(bg.get_w() && r_border.get_w() && l_border.get_w() && top.get_w() && bot.get_w() && "Window class is not fully initalized!");

		  x=px;
		  y=py;
		  w=wi;
		  h=he;
		  myf=f;
					 
		  nta=0;
		  ta=(Textarea**) malloc(sizeof(Textarea*)*MAX_TA);
		
		  nbut=0;
		  but=(Button**) malloc(sizeof(Button*)*MAX_BUT);

		  winpic=new Pic();
		  winpic->set_size(w, h);
		  
		  own_bg=0;					

		  redraw_win();
		  g_gr.register_update_rect(x, y, w, h);
}

/** Window::~Window()
 *
 * Default destructor
 *
 * Args: None
 * Returns: Nothing
 */
Window::~Window(void) {
		  uint i;
		  
		  g_gr.register_update_rect(x, y, w, h);
		  
		  for(i=0 ; i< nta; i++) 
					 delete ta[i];
		  free(ta);
		 
		  for(i=0; i< nbut; i++) 
					 delete but[i];
		  free(but);
		  
		  delete winpic;
		  if(own_bg) delete own_bg;
		  
}

/** void Window::set_pos(uint posx, uint posy)
 *
 * This gives the window a new position on the screen
 *
 * PRIVATE function so that only friends can move us (because we have 
 * to care for the size of the screen) 
 *
 * Args:	posx 	new xpos
 * 		posy	new ypos
 * Returns: Nothing
 */
void Window::set_pos(uint posx, uint posy) {
		  x=posx; y=posy;
		  g_gr.register_update_rect(x, y, w, h);
}
					 
/** Textarea Window::create_textarea(const uint px, const uint py, const char* t ,  Textarea::Align a = Textarea::LEFTA)
 * 
 * This function creates a textarea with a given text. The size will be set through the 
 * text width
 *
 * Args:	px	xpos in window
 * 		py	ypos in window
 * 		t	text to use
 * 		a	alignment to use 
 * Returns: textarea just created
 */
Textarea* Window::create_textarea(const uint px, const uint py, const char* t ,  Textarea::Align a) {
	
		  uint myw=w;
		  uint myh=h;
		  uint add=0;
		  
		  if(myf!=FLAT) {
					 myw-=get_border();
					 myh-=get_border();
					 add=get_border()>>1;
		  }
		  
		  ta[nta]=new Textarea(px, py, t, a, myw, myh, winpic, add, add);
		  ta[nta]->draw();
		  g_gr.register_update_rect(x+ta[nta]->get_xpos(), y+ta[nta]->get_ypos(), ta[nta]->get_w(), ta[nta]->get_h());
		  nta++;

		  assert(nta<MAX_TA);

		  return ta[nta-1];
}
  
/** Textarea Window::create_textarea(const uint px, const uint py, const unsigend int myw,  Textarea::Align a = Textarea::LEFTA)
 * 
 * This function creates a textarea with a given width. 
 *
 * Args:	px	xpos in window
 * 		py	ypos in window
 * 		myw	width of area
 * 		a	alignment to use 
 * Returns: textarea just created
 */
Textarea* Window::create_textarea(const uint px, const uint py, const uint myw ,  Textarea::Align a) {

		  uint add=0;
		  if(myf!=FLAT) add=get_border();
		 
		  int rw=myw;
		  int mypy=py;
		  if(px+add+rw > w) rw=w-add-px;
		  if(py+add+Textarea::get_fh() > h) mypy=h-Textarea::get_fh();

		  ta[nta]=new Textarea(px, mypy, rw, a, winpic, add>>1, add>>1);
		  ta[nta]->draw();
		  g_gr.register_update_rect(x+ta[nta]->get_xpos(), y+ta[nta]->get_ypos(), ta[nta]->get_w(), ta[nta]->get_h());
		  nta++;

		  assert(nta<MAX_TA);

		  return ta[nta-1];
}

/** void Window::redraw_win(void) 
 *
 * PRIVATE FUNCTION to update the whole winpic
 *
 * Args: None
 * Returns: Nothing
 */
void Window::redraw_win(void) {
		  uint i, j;
		  uint px=x; 
		  uint py=y;
		
		  g_gr.register_update_rect(x, y, w, h);

		  Pic* usebg= own_bg ? own_bg : &bg ;
		
		  if(myf != FLAT) {
					 px+=get_border()>>1;
					 py+=get_border()>>1;

					 // Top n Bottom
					 // top
					 Graph::copy_pic(winpic, &top, 0, 0, 0, 0, CORNER, CORNER);
					 // bot
					 Graph::copy_pic(winpic, &bot, 0, h-CORNER, 0, 0, CORNER, CORNER);
					 for(i=CORNER; i<w-CORNER-MIDDLE; i+=MIDDLE) {
								// top
								Graph::copy_pic(winpic, &top, i, 0, CORNER, 0, MIDDLE, CORNER);
								// bot
								Graph::copy_pic(winpic, &bot, i, h-CORNER, CORNER, 0, MIDDLE, CORNER);
					 } 
					 // top
					 Graph::copy_pic(winpic, &top, i, 0, CORNER, 0, w-CORNER-i, CORNER);
					 Graph::copy_pic(winpic, &top, w-CORNER, 0, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);
					 // bot
					 Graph::copy_pic(winpic, &bot, i, h-CORNER, CORNER, 0, w-CORNER-i, CORNER);
					 Graph::copy_pic(winpic, &bot, w-CORNER, h-CORNER, MUST_HAVE_NPIX-CORNER, 0, CORNER, CORNER);

					 // borders
					 // left
					 Graph::copy_pic(winpic, &l_border, 0, CORNER, 0, 0, CORNER, CORNER);
					 // right
					 Graph::copy_pic(winpic, &r_border, w-CORNER, CORNER, 0, 0, CORNER, CORNER);
					 for(i=CORNER+CORNER; i<h-CORNER-CORNER-MIDDLE; i+=MIDDLE) {
								// left
								Graph::copy_pic(winpic, &l_border, 0, i, 0, CORNER, CORNER, MIDDLE);
								// right
								Graph::copy_pic(winpic, &r_border, w-CORNER, i, 0, CORNER, CORNER, MIDDLE);
					 } 
					 // left
					 Graph::copy_pic(winpic, &l_border, 0, i, 0, CORNER, CORNER, h-CORNER-i);
					 Graph::copy_pic(winpic, &l_border, 0, h-CORNER-CORNER, 0, l_border.get_h()-CORNER, CORNER, CORNER);
					 // right
					 Graph::copy_pic(winpic, &r_border, w-CORNER, i, 0, CORNER, CORNER, h-CORNER-i);
					 Graph::copy_pic(winpic, &r_border, w-CORNER, h-CORNER-CORNER, 0, r_border.get_h()-CORNER, CORNER, CORNER);

					 // bg
					 for(j=CORNER; (int)j<=(int)(h-CORNER-CORNER-usebg->get_h()); j+=usebg->get_h()) {
								for(i=CORNER; (int)i<=(int)(w-CORNER-CORNER-usebg->get_w()); i+=usebg->get_w()) {
										  Graph::copy_pic(winpic, usebg, i, j, 0, 0, usebg->get_w(), usebg->get_h());
								} 
								Graph::copy_pic(winpic, usebg, i, j, 0, 0, w-i-CORNER, usebg->get_h());
					 }
					 for(i=CORNER; (int)i<=(int)(w-CORNER-CORNER-usebg->get_w()); i+=usebg->get_w()) {
								Graph::copy_pic(winpic, usebg, i, j, 0, 0, usebg->get_w(), h-j-CORNER);
					 } 
					 Graph::copy_pic(winpic, usebg, i, j, 0, 0, w-i-CORNER, h-j-bot.get_h());
		  } else {
					 // has no borders. Simply paste once the pic
					 uint mw = usebg->get_w() > w ? w : usebg->get_w();
					 uint mh = usebg->get_h() > h ? h : usebg->get_h();
		 
					 Graph::copy_pic(winpic, usebg, 0, 0, 0, 0, mw, mh);
		  }
		  
		  // Draw textareas
		  for(i=0 ; i< nta; i++) 
					 ta[i]->draw();


		  // Draw Buttons
		  for(i=0; i< nbut; i++) 
					 but[i]->draw();
}
					 
/** Button* Window::create_button(const uint px, const uint py, const uint rw, const uint rh, const uint bg)
 *
 * This functions creates a button on the given point
 *
 * Args: px, py	position
 * 		rw, rh	width/height
 * 		bg  background to use
 * Returns: The created button
 */
Button* Window::create_button(const uint px, const uint py, const uint rw, const uint rh, const uint bg) {
		  uint add=0;
		  if(myf!=FLAT) add=get_border();
		 
		  int mypx=px;
		  int mypy=py;
		  uint myw=rw+Button::get_border();
		  uint myh=rh+Button::get_border();


		  if(px+add+rw > w) mypx=w-(add+rw);
		  if(mypx<0) return NULL;
		  if(py+add+rh > h) mypy=h-(add+rh);

		  
		  
		  but[nbut]=new Button(mypx, mypy, myw, myh, bg, winpic, add>>1, add>>1);
		  but[nbut]->draw();
		  g_gr.register_update_rect(x+but[nbut]->get_xpos(), y+but[nbut]->get_ypos(), but[nbut]->get_w(), but[nbut]->get_h());
		  nbut++;

		  assert(nbut<MAX_BUT);

		  return but[nbut-1];
}


/** void Window::draw(void)
 *
 * This function draws the current window on the g_gr object
 *
 * Args: None
 * Returns: Nothing
 */
void Window::draw(void) {
		  Graph::draw_pic(winpic, x, y, 0, 0, w, h);
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

		  redraw_win();
		  
		  g_gr.register_update_rect(x, y, w, h);
}
					 
/** int Window::handle_mm(const uint x, const uint y, const bool b1, const bool b2)
 *
 * This func cares for mouse movements
 *
 * Args:	x	xpos in window
 * 		y	ypos in window
 * 		b1, b2	state of the mouse buttons
 * Returns: INPUT_HANDLED or -1
 */
int Window::handle_mm(const uint x, const uint y, const bool b1, const bool b2) {
		  uint i;
		  
		  // check for buttons
		  for(i=0; i<nbut; i++) {
					 if(but[i]->get_xpos()<x && but[i]->get_xpos()+but[i]->get_w()>x &&
										  but[i]->get_ypos()<y && but[i]->get_ypos()+but[i]->get_h()>y) {
								// is inside!
								but[i]->set_bright(true);
								but[i]->set_pressed(b1);
					 } else {
								but[i]->set_bright(false);
								but[i]->set_pressed(false);
					 }
		  
					 if(but[i]->draw()) g_gr.register_update_rect(this->x+but[i]->get_xpos(), this->y+but[i]->get_ypos(), but[i]->get_w(), but[i]->get_h());
		  }
					 

		  // we do not care for ta, because they are non responsive to mouse movements or clicks
		  
		  return INPUT_HANDLED;
}

/** int Window::handle_click(const uint pbut, const bool b, const uint x, const uint y) 
 *
 * This cares for mouseclicks into the window
 *
 * Args:	pbut	1 / 2
 * 		b		true if pressed, false if released
 * 		x, y	position in window
 *
 * Returns: INPUT_HANDLED, or INPUT_UNHANDLED
 */
int Window::handle_click(const uint pbut, const bool b, const uint x, const uint y) {
		  if(pbut==2) return INPUT_UNHANDLED; // we do not react on this
		  uint i;

		  
		  // check buttons
		  for(i=0; i<nbut; i++) {
					 if(but[i]->get_xpos()<x && but[i]->get_xpos()+but[i]->get_w()>x &&
										  but[i]->get_ypos()<y && but[i]->get_ypos()+but[i]->get_h()>y) {
								// is inside!
								if(but[i]->is_pressed() && !b) {
										  // button was pressed, mouse is now released over the button
										  // Run the button func!
										  but[i]->run();
								}
								but[i]->set_pressed(b);
					 } else {
								but[i]->set_pressed(false);
					 }
					 
					 if(but[i]->draw()) g_gr.register_update_rect(this->x+but[i]->get_xpos(), this->y+but[i]->get_ypos(), but[i]->get_w(), but[i]->get_h());
		  }
			 
		  

		  // we do not care for textareas, they are unresponsive to clicks
		  
		  return INPUT_UNHANDLED;
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
		  dragwin=0;
		  
		  last=first;
}
		  
/** int User_Interface::handle_click(const uint but, const bool b, const uint x, const uint y, void* a);
 *
 * This functions cares for clicks on the User_Interface
 *
 * Args:	but	1 / 2
 * 		b		true if pressed, false if released
 * 		x, y	position on the screen
 * 		a 		user defined argument
 * Returns: INPUT_HANDLED / INPUT_UNHANDLED
*/
int User_Interface::handle_click(const uint but, const bool b, const uint x, const uint y, void* a) {

		  
		  // Check every window
		  for(win_p* p=first; p &&  p->w; p=p->next) {
					 if(p->w->get_xpos()<x && p->w->get_xpos()+p->w->get_w()>x &&
										  p->w->get_ypos()<y && p->w->get_ypos()+p->w->get_h()>y) {
								// Mouse is inside this window. 
								if(p->w->get_flags() != Window::FLAT && but==2) delete_window(p->w);
								if(p->w->handle_click(but, b, x-p->w->get_xpos(), y-p->w->get_ypos()) == INPUT_UNHANDLED) {
										  if(p->w->get_flags() != Window::FLAT && b && but==1) {					 
													 dragwin=p->w;
													 return INPUT_HANDLED;
										  }
								} else return INPUT_HANDLED;
					 }
		  }

		  return INPUT_UNHANDLED;
}
  

/** int User_Interface::handle_mm(const uint x, const uint y, const int xdiff, const int ydiff, const bool b1, const bool b2, void* a);
 *
 * This function cares for a mouse move over the windows. It's also responsible for dragging (read: moving) the
 * windows.
 *
 * Args:	x	x click on screen
 * 		y	y click on screen
 * 		xdiff	difference since last movement
 * 		ydiff	difference since last movement
 * 		b1, b2	true or false, depends if buttons are pressed or not
 * 		a	user defined argument
 * Returns: INPUT_HANDLED or -1
 */
int User_Interface::handle_mm(const uint x, const uint y, const int xdiff, const int ydiff, const bool b1, const bool b2, void* a) {
		  if(b1 && dragwin) {
					 move_window(dragwin, dragwin->get_xpos()+xdiff, dragwin->get_ypos()+ydiff);
					 return INPUT_HANDLED;
		  }

		  // Check every window
		  for(win_p* p=first; p &&  p->w; p=p->next) {
					 if(p->w->get_xpos()<x && p->w->get_xpos()+p->w->get_w()>x &&
										  p->w->get_ypos()<y && p->w->get_ypos()+p->w->get_h()>y) {
								// Mouse is inside this window. 
								if(p->w->handle_mm(x-p->w->get_xpos(), y-p->w->get_ypos(), b1, b2) == INPUT_UNHANDLED) {
										  if(p->w->get_flags() != Window::FLAT && b1) {					 
													 dragwin=p->w;
													 return INPUT_HANDLED;
										  }
								}
					 }
		  }

		  return -1;
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
		  
/** void User_Interface::move_window(Window* win, const unigned int x, const uint y) 
 *
 * This moves a window to the new given coordinates. This is done in User Interface since we have
 * to make sure, that the window won't move outside the screen
 *
 * Args:	win Window to move
 * 		x	new xpos
 * 		y	new ypos
 * Returns: Nothing
 */
void User_Interface::move_window(Window* win, const uint x, const uint y) {
		  int myx=x;
		  int myy=y;
		  uint mw=win->get_w();
		  uint mh=win->get_h();

		  if(myx+mw >= g_gr.get_xres()) myx=g_gr.get_xres()-mw;
		  if(myx<0) return;
		  if(myy+mh >= g_gr.get_yres()) myy=g_gr.get_yres()-mh;
		  if(myy<0) return;

		  win->set_pos(myx, myy);
}

/** Window* User_Interface::create_window(const uint x, const uint y, const uint w, 
 *   	const uint h, const Window::Flags f=Window::DEFAULT) 
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
Window*  User_Interface::create_window(const uint x, const uint y, const uint w, 
					 const uint h, const Window::Flags f) {
		  Window* win;
		  
		  uint mw=w;
		  uint mh=h;
		  
		  if(f!=Window::FLAT) {
					 uint get_b=Window::get_border();
					 mw+=get_b;
					 mh+=get_b;
		  }
		  
		  
		  int myx=x;
		  int myy=y;
		  if(myx+mw >= g_gr.get_xres()) myx=g_gr.get_xres()-mw;
		  if(myx<0) return 0;
		  if(myy+mh >= g_gr.get_yres()) myy=g_gr.get_yres()-mh;
		  if(myy<0) return 0;
		  
		  win=new Window(myx, myy, mw, mh, f);
		  

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

		  Graph::copy_pic(dp, txt, posx, yp+y, 0, 0, myw, h);
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7

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
		  if(funca) free(funca);
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
		  
