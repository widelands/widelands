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
		  
		  dragwin=0;
		  
		  // Check every window
		  for(win_p* p=last->prev; p &&  p->w; p=p->prev) {
					 if(p->w->get_xpos()<x && p->w->get_xpos()+p->w->get_w()>x &&
										  p->w->get_ypos()<y && p->w->get_ypos()+p->w->get_h()>y) {
								// Mouse is inside this window. 
								if(p->w->get_flags() != Window::FLAT && b && but==2) {
										  delete_window(p->w);
										  return INPUT_HANDLED;
								}
								// make this the topmost window
								if(p->w->get_flags() != Window::FLAT) {
										  if(p==first) {
													 p->next->prev=0;
													 first=p->next;
													 last->prev->next=p; 
													 p->prev=last->prev;
													 p->next=last;
													 last->prev=p;
													 g_gr.register_update_rect(p->w->get_xpos(), p->w->get_ypos(),
																		  p->w->get_w(), p->w->get_h());
										  } else if(p!=last->prev) {
													 // p is not topmost
													 p->next->prev=p->prev;
													 p->prev->next=p->next;
													 last->prev->next=p; 
													 p->prev=last->prev;
													 p->next=last;
													 last->prev=p;
													 g_gr.register_update_rect(p->w->get_xpos(), p->w->get_ypos(),
																		  p->w->get_w(), p->w->get_h());
										  }
								}

								if(p->w->handle_click(but, b, x-p->w->get_xpos(), y-p->w->get_ypos()) == INPUT_UNHANDLED) {
										  if(p->w->get_flags() != Window::FLAT && b && but==1) {					 
													 dragwin=p->w;
													 return INPUT_HANDLED;
										  }
										  return INPUT_UNHANDLED;
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
		  
		  dragwin=0;

		  // Check every window
		  for(win_p* p=last->prev; p &&  p->w; p=p->prev) {
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
		  if(myx<0) myx=0; 
		  if(myy+mh >= g_gr.get_yres()) myy=g_gr.get_yres()-mh;
		  if(myy<0) myy=0; 

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
		 
/** void User_Interface::delete_all_windows(void) 
 *
 * This function cleans the whole screen from windows. It's used
 * to clean up after each menue and after each game.
 *
 * Args: none
 * Returns: Nothing
 */
void User_Interface::delete_all_windows(void) {
		  win_p* p;
		  win_p* temp;
		  for(p=last->prev; p; p=temp) {
					 temp=p->prev;
					 delete p->w;
					 delete p;
		  }
		  last->prev=0;
		  last->next=0;
		  last->w=0;
		  first=last;
		  dragwin=0;
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

