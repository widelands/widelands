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

#ifndef __S__UI_H
#define __S__UI_H

#include "graphic.h"
#include "singleton.h"


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
class Window {
		  // Copy is non trivial and shouldn't be needed
		  Window(const Window&);
		  Window& operator=(const Window&);
		  
		  public:
					 enum Flags {
								DEFAULT, 
								FLAT // No clicks, no moves, no borders.
					 };

					 // widht/height the graphs above must have
					 const static unsigned short MUST_HAVE_NPIX=100;
					 // width/height to use as the corner
					 const static unsigned short CORNER=20;
					 const static unsigned short MIDDLE=MUST_HAVE_NPIX-(CORNER*2);

					 // static Functions to set the standart graphics
					 /** static void Window::set_l_border(Pic* p) 
					  *
					  * This represents also the other function following.
					  * This makes sure, that the standart l_border pic is of 
					  * valid size and sets it
					  *
					  * Agrs: p 	pic to set
					  * Returns: Nothing
					  */
					 static void set_l_border(Pic* p) { assert(p->get_h()==MUST_HAVE_NPIX && p->get_w()==CORNER
										  && "l_border doesn't have the default height!"); Window::l_border=*p; }
					 static void set_r_border(Pic* p) { assert(p->get_h()==MUST_HAVE_NPIX && p->get_w()==CORNER 
										  && "r_border doesn't have the default height!"); Window::r_border=*p; }
					 static void set_top(Pic* p) { assert(p->get_w()==MUST_HAVE_NPIX && p->get_h()==CORNER 
										  && "top doesn't have the default width!"); Window::top=*p; }
					 static void set_bot(Pic* p) { assert(p->get_w()==MUST_HAVE_NPIX && p->get_h()==CORNER 
										  && "bot doesn't have the default width!"); Window::bot=*p; }
					 static void set_bg(Pic* p) { Window::bg=*p; }
					 // The next two functions are there to get the different between asked window size and given window size
					 // Ex: you want a 100x100 window. Now, the User_Interface class makes sure that border widths and top,bottom heights
					 // are added to the size and still the window musn't leave the screen on any edges.
					 inline unsigned int get_borderw(void) { return (l_border.get_w()+r_border.get_w()); }
					 inline unsigned int get_borderh(void) { return (top.get_h()+bot.get_h()); }

					 // inline functions to get some informations
					 inline unsigned int get_xpos(void) { return x; }
					 inline unsigned int get_ypos(void) { return y; }
					 inline unsigned int get_w(void) { return w; }
					 inline unsigned int get_h(void) { return h; }
					 inline unsigned int get_id(void) { return id ; }
					 
					 
					 void handle_click(const unsigned int, const unsigned int); // TODO
					 void handle_mmove(const unsigned int, const unsigned int); // TODO
					 void draw(void);	
					 void set_pos(const unsigned int, const unsigned int);  // TODO
					 void set_new_bg(Pic* p);
//					 void set_closefunc(...)

					 Window(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const Flags); 
					 ~Window();

		  private:
					 unsigned int x, y, w, h;
					 Pic* own_bg;
					 unsigned int id;
					 Flags myf;
					
					 //closefunc dfkj;
					 static Pic l_border;
					 static Pic r_border;
					 static Pic top;
					 static Pic bot;
					 static Pic bg;
};


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
 * 			global Object: g_gr
 */
class User_Interface : public Singleton<User_Interface> {
		  User_Interface(const User_Interface&);
		  User_Interface& operator=(const User_Interface&);

		  public:
		  static const unsigned int MAX_WINS=20;

		  User_Interface(void);
		  ~User_Interface(void);
		  
		  unsigned int create_window(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const Window::Flags=Window::DEFAULT); 
		  void delete_window(const unsigned int); // TODO
		  void set_win_bg(const unsigned int, Pic*);
		  void draw(void);
					 
		  private:
		  Window **wins;
		  unsigned int nwins;

		  /** inline Window* get_window(const unsigned int id)
			*
			* This function returns the window with the given id. It's inlined for speeds sake
			*
			* PRIVATE FUNCTION
			*
			* Args: id	id to find
			* Returns: window or NULL
			*/
		  inline Window* get_window(const unsigned int id) {
					 for(unsigned int i=0; i<nwins; i++) 
								if(wins[i]->get_id()==id) return wins[i];
					 return NULL;
		  }
};


#define g_ui	User_Interface::get_singleton()
	  
#endif /* __S__UI_H */
