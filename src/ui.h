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

/** class Button
 *
 * This defines a button.
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */
#define BUTTON_EDGE_BRIGHT_FACTOR 60
#define MOUSE_OVER_BRIGHT_FACTOR  15
typedef void (*BUT_FUNC)(void*);

class Button {
		  Button(const Button&);
		  Button& operator=(const Button&);

		  public:
					 Button(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, Pic*, const unsigned int,
										  const unsigned int);
					 ~Button();

					 void set_pic(Pic*);
					 int draw();

					 /** static void Button::set_bg(Pic* p, unsigned int n) 
					  *
					  * This function associates the picture with a background
					  *
					  * Args: p		picture to associate
					  * 		 n 	number of bg to associate p with
					  * Returns: Nothing
					  */
					 static void set_bg(Pic* p, unsigned int n) { 
								assert(n<3); 
								Pic* ep;
								
								if(n==0) { bg0=*p; bg0e=*p; ep=&bg0e;}
								if(n==1) { bg1=*p; bg1e=*p; ep=&bg1e; }
								if(n==2) { bg2=*p; bg2e=*p; ep=&bg2e; }
								
							
								
								unsigned int mx, my;
								unsigned short clr;
								for(my=0; my<(p->get_h()); my++) {
										  mx=0;
										  clr=p->get_pixel(mx, my);
										  ep->set_pixel(mx,my, Graph::bright_up_clr(clr, MOUSE_OVER_BRIGHT_FACTOR));
										  for(mx=1; mx<p->get_w(); mx++) {
													 clr=p->get_npixel();
													 ep->set_npixel(Graph::bright_up_clr(clr, MOUSE_OVER_BRIGHT_FACTOR));
										  }
								}
					 }

					 /** static unsigned int Button::get_border(void) 
					  *
					  * Returns the width of the borders in pixel
					  *
					  * Args: none
					  * Returns: width in pixel of button borders
					  */
					 static unsigned int get_border(void) { return 4; }
					
					 /** void register_func(BUT_FUNC f, void* arg)
					  *
					  * This funtion registers the click func for this button
					  *
					  * Args:	f func to use
					  * 			a	user definde argument given to the function
					  * returns: Nothing
					  */
					 void register_func(BUT_FUNC f, void* arg) {
								func=f;
								funca=arg;
					 }

					 /** void run(void) 
					  *
					  * This runs the registered button func (if any) 
					  *
					  * Args:	none
					  * Returns: Nothing
					  */
					 void run(void) {
								if(func) 
										  func(funca);
					 }

					 // some functions to set informations and to get informations
					 inline void set_bright(const bool b) { if(benlighted!=b) { benlighted=b; needs_draw=true; }  }
					 inline void set_pressed(const bool b) { if(bpressed!=b) { bpressed=b; needs_draw=true ; } }
					 inline unsigned int get_xpos(void) { return x+xp; }
					 inline unsigned int get_ypos(void) { return y+yp; }
					 inline unsigned int get_w(void) { return w; }
					 inline unsigned int get_h(void) { return h; }
					 inline bool is_pressed(void) { return bpressed; } 
					 
		  private:
					 bool needs_draw;
					 bool bpressed;
					 bool	benlighted;
					 static Pic bg0, bg1, bg2, bg0e, bg1e, bg2e;
					 unsigned int x, y, w, h, xp, yp;
					 BUT_FUNC func; 
					 void* funca;
					 
					 Pic* mybg, *myebg;
					 Pic* dp;
					 Pic* myp;
};

/** class Textarea 
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: g_gr
 * 			class Graph::Pic
 * 			class Font_Handler
 */
class Textarea {
		  Textarea( const Textarea&);
		  Textarea& operator=(const Textarea&);

		  public:
					 enum Align {
								RIGHTA, 
								LEFTA, 
								CENTER
					 };
					
					 Textarea(const unsigned int, const unsigned int, const char* , const Align, const unsigned int, const unsigned int, Pic*, const unsigned int,
										  const unsigned int);
					 Textarea(const unsigned int, const unsigned int, const unsigned int, const Align, Pic*, const unsigned int, const unsigned int);
					 ~Textarea(void);
					 
					 void set_text(const char*);
					 void draw(void) const ;
					 
					 /** static void set_font(unsigned int n)
					  * This function sets the font to use for textareas
					  * defaults to zero
					  *
					  * Args: n 	font to use
					  * Returns:	nothing
					  */
					 static void set_font(unsigned int n) { nfont=n; }
			
					 // some information funcs
					 inline unsigned int get_xpos(void) { return x+xp; }
					 inline unsigned int get_ypos(void) { return y+yp; }
					 inline unsigned int get_w(void) { return w; }
					 inline unsigned int get_h(void) { return h; }
	
					 
		  private: 
					 static unsigned int nfont;
					 unsigned int x, y, w, h, xp, yp;
					 Align al;
					 
					 Pic* txt;
					 Pic* dp;
};

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
 * 			User_Interface
 */

// widht/height the graphs above must have
#define MUST_HAVE_NPIX	100
// width/height to use as the corner
#define CORNER			20
#define MIDDLE			(MUST_HAVE_NPIX-(CORNER*2))

#define MAX_BUT 100  // these values don't get checked. you better don't ignore them
#define MAX_TA   40	 

class Window {
		  // Copy is non trivial and shouldn't be needed
		  Window(const Window&);
		  Window& operator=(const Window&);
		  
		  public:
					 enum Flags {
								DEFAULT, 
								FLAT // No clicks, no moves, no borders.
					 };

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
					 inline const static unsigned int get_border(void) { return (CORNER<<1); }

					 // inline functions to get some informations
					 inline unsigned int get_xpos(void) { return x; }
					 inline unsigned int get_ypos(void) { return y; }
					 inline unsigned int get_w(void) { return w; }
					 inline unsigned int get_h(void) { return h; }
					 inline Flags get_flags(void) { return myf; }					 
					 
					 int handle_click(const unsigned int, const bool, const unsigned int, const unsigned int);
					 int handle_mm(const unsigned int, const unsigned int, const bool, const bool);
					 void draw(void);	
					 void set_new_bg(Pic* p);
					 
					 // creation functions
					 Textarea* create_textarea(const unsigned int, const unsigned int, const unsigned int, const Textarea::Align = Textarea::LEFTA);
					 Textarea* create_textarea(const unsigned int, const unsigned int, const char* ,  Textarea::Align = Textarea::LEFTA);
					 Button*   create_button(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);

					 //					 void set_closefunc(...)


		  friend class User_Interface;

		  private:
					 // Only friends can create and destroy and move us!
					 void set_pos(const unsigned int, const unsigned int); 
					 Window(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const Flags); 
					 ~Window();
					 void redraw_win(void);
					 
					 unsigned int x, y, w, h;
					 Pic* winpic;
					 Pic* own_bg;
					 Flags myf;
				
					 // for textareas
					 unsigned int nta;
					 Textarea** ta;
					
					 // for buttons
					 unsigned int nbut;
					 Button** but;

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
		  User_Interface(void);
		  ~User_Interface(void);
		  
		  Window* create_window(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const Window::Flags=Window::DEFAULT); 
		  void delete_window(Window*); 
		  void move_window(Window*, const unsigned int, const unsigned int);
		  void draw(void);
		  int handle_mm(const unsigned int, const unsigned int, const int, const int, const bool, const bool, void*);
		  int handle_click(const unsigned int, const bool, const unsigned int, const unsigned int, void* );

		  private:
		  struct win_p {
					 win_p* next; 
					 win_p* prev; 
					 Window* w;
		  };
		  Window* dragwin;
		  
		  win_p* first;
		  win_p* last;
};


#define g_ui	User_Interface::get_singleton()
	  
#endif /* __S__UI_H */
