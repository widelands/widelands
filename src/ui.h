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

#ifndef __S__UI_H
#define __S__UI_H

#include "graphic.h"
#include "singleton.h"
#include "font.h"

// predeclaration
class Window;

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
typedef void (*BUT_FUNC)(Window*, void*);

class Button {
		  Button(const Button&);
		  Button& operator=(const Button&);

		  
		  friend class Window;
		  
		  public:
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
					 void set_pic(Pic*);

					 /** static void Button::set_bg(Pic* p, uint n) 
					  *
					  * This function associates the picture with a background
					  *
					  * Args: p		picture to associate
					  * 		 n 	number of bg to associate p with
					  * Returns: Nothing
					  */
					 static void set_bg(Pic* p, uint n) { 
								assert(n<3); 
								Pic* ep=0;
								
								if(n==0) { bg0=*p; bg0e=*p; ep=&bg0e;}
								if(n==1) { bg1=*p; bg1e=*p; ep=&bg1e; }
								if(n==2) { bg2=*p; bg2e=*p; ep=&bg2e; }
								
							
								
								uint mx, my;
								ushort clr;
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

		  private:
					 Button(const uint, const uint, const uint, const uint, const uint, Pic*, const uint,
										  const uint);
					 ~Button();

					 int draw();

					 

					 /** static uint Button::get_border(void) 
					  *
					  * Returns the width of the borders in pixel
					  *
					  * Args: none
					  * Returns: width in pixel of button borders
					  */
					 static uint get_border(void) { return 4; }
					
					 

					 /** void run(Window* par) 
					  *
					  * This runs the registered button func (if any) 
					  *
					  * Args:	par	Parent window of this button
					  * Returns: Nothing
					  */
					 void run(Window* par) {
								if(func) 
										  func(par, funca);
					 }

					 // some functions to set informations and to get informations
					 inline void set_bright(const bool b) { if(benlighted!=b) { benlighted=b; needs_draw=true; }  }
					 inline void set_pressed(const bool b) { if(bpressed!=b) { bpressed=b; needs_draw=true ; } }
					 inline uint get_xpos(void) { return x+xp; }
					 inline uint get_ypos(void) { return y+yp; }
					 inline uint get_w(void) { return w; }
					 inline uint get_h(void) { return h; }
					 inline bool is_pressed(void) { return bpressed; } 
					 
		  private:
					 bool needs_draw;
					 bool bpressed;
					 bool	benlighted;
					 static Pic bg0, bg1, bg2, bg0e, bg1e, bg2e;
					 uint x, y, w, h, xp, yp;
					 BUT_FUNC func; 
					 void* funca;
					 
					 Pic* mybg, *myebg;
					 Pic* dp;
					 Pic* myp;
};

/** class Checkbox
 *
 * This defines a checkbox, which will be marked or unmarked, depending on their state
 * 
 * Depends:	g_gr
 * 			class Graph::Pic
 */
#define CHECKBOX_WIDTH 20
#define CHECKBOX_HEIGHT 20 
class Checkbox {
		  Checkbox( const Checkbox&);
		  Checkbox operator=(const Checkbox&);

		  friend class Window;

		  public:
					 // Returns the current state of the checkbox
					 bool get_state(void) const { return bstate; }

					 /** static void set_graph(Pic*);
					  * 
					  * Function to set the graphic.
					  * 
					  * It has to be of the following format: 
					  *  CHECKBOX_WIDTH (uncheked) CHECKBOX_WIDTH (cheked)
					  *  HEC
					  *  KBO
					  *  X
					  *  _
					  *  HEIGHT
					  * It also has a total for (CHECKBOX_WIDTH*2)*CHECKBOX_HEIGHT
					  */
					 static void set_graph(Pic* p) { 
								assert(p->get_w() == (CHECKBOX_WIDTH*2));
								assert(p->get_h() == CHECKBOX_HEIGHT);
								gr=*p; 
					 }


		  private:
					 Checkbox(const uint, const uint, const bool, Pic*, const uint, const uint);
					 ~Checkbox(void);
					 int draw(void);
					 void set_state(bool b) { bstate=b; }
					 // Information funcs
					 static inline uint get_w(void) { return CHECKBOX_WIDTH; }
					 static inline uint get_h(void) { return CHECKBOX_HEIGHT; }
					 inline uint get_xpos(void) { return x+xp; }
					 inline uint get_ypos(void) { return y+yp; }
					 
					 
					 bool bstate;
					 static Pic gr;
					 uint x, y;
					 uint xp, yp;
					 Pic* dp;
};

/** class Listselect
 *
 * This class defines a list-select box. 
 *
 * Depends: class Graph::Pic
 * 			g_fh
 * 			class Button
 */
#define MAX_LISTENTRYS	1024	// TODO: replace this with growablearray, as soon as it is faster
class Listselect {
		  Listselect(const Listselect&);
		  Listselect& operator=(const Listselect&);

		  friend class Window;
		  friend void listselect_but_up(Window*,   void*);
		  friend void listselect_but_down(Window*, void*);
					 
		  public:
					 // Function to set the font
					 static void set_font(uint n) { nfont=n; }
					 static void set_clrs(ushort b, ushort f, ushort s) { bgclr=b; frameclr=f; selclr=s; }
					 void add_entry(const char*, const char* =0) ;
					 inline const char* get_selection(void) {
								if(cursel==-1) return 0;
								return ent[cursel].value;
					 }

		  
		  private:
					 Listselect(const uint, const uint, const uint, const uint, Pic*, const uint, const uint);
					 ~Listselect(void);
					 int draw(void);
					 void move_up(uint i) { firstvis-=i; if(firstvis<0) firstvis=0; draw(); }
					 void move_down(uint i) { firstvis+=i; if(firstvis+h > nent) firstvis=nent-h; if(firstvis<0) firstvis=0; draw(); }   
					 void select(uint);
					 
					 // Information funcs
					 inline uint get_w(void) { return w; }
					 inline uint get_h(void) { return (g_fh.get_fh(nfont)+2)*h; }
					 inline uint get_xpos(void) { return x+xp; }
					 inline uint get_ypos(void) { return y+yp; }


					 // Vars
					 struct Entry {
								Pic* p;
								char value[255];
					 };
					 
					 static uint nfont;
					 static ushort bgclr, frameclr, selclr;
					 
					 Entry ent[MAX_LISTENTRYS];
					 uint nent;

					 int firstvis;
					 int cursel;
					 uint w, h;
					 uint x, y;
					 uint xp, yp;
					 Pic* dp;
					 
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

		  friend class Window;

		  public:
					 enum Align {
								RIGHTA, 
								LEFTA, 
								CENTER
					 };
					 void set_text(const char*);
		
					 /** static void set_font(uint n)
					  * This function sets the font to use for textareas
					  * defaults to zero
					  *
					  * Args: n 	font to use
					  * Returns:	nothing
					  */
					 static void set_font(uint n) { nfont=n; }
	
		  private:
				
					 Textarea(const uint, const uint, const char* , const Align, const uint, const uint, Pic*, const uint,
										  const uint);
					 Textarea(const uint, const uint, const uint, const Align, Pic*, const uint, const uint);
					 ~Textarea(void);
					 
					 void draw(void) const ;
					 		
					 // information funcs
					 static inline ushort get_fh(void) { return g_fh.get_fh(nfont); }

					 // some information funcs
					 inline uint get_xpos(void) { return x+xp; }
					 inline uint get_ypos(void) { return y+yp; }
					 inline uint get_w(void) { return w; }
					 inline uint get_h(void) { return h; }
	
					 
		  private: 
					 static uint nfont;
					 uint x, y, w, h, xp, yp;
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
#define MAX_CHECKBOX   10
#define MAX_LISTSELECT   2

class Window {
		  // Copy is non trivial and shouldn't be needed
		  Window(const Window&);
		  Window& operator=(const Window&);
		 
		 
		  public:
					 enum Flags {
								DEFAULT, 
								FLAT // No clicks, no moves, no borders.
					 };

					 // creation functions
					 Textarea* create_textarea(const uint, const uint, const uint, const Textarea::Align = Textarea::LEFTA);
					 Textarea* create_textarea(const uint, const uint, const char* ,  Textarea::Align = Textarea::LEFTA);
					 Button*   create_button(const uint, const uint, const uint, const uint, const uint);
					 Checkbox*   create_checkbox(const uint, const uint, const bool b);
					 Listselect*   create_listselect(const uint, const uint, const uint, const uint);
					 void set_new_bg(Pic* p);

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
					

					 // inline functions to get some informations
					 inline uint get_xpos(void) { return x; }
					 inline uint get_ypos(void) { return y; }
					 inline uint get_w(void) { return w; }
					 inline uint get_h(void) { return h; }
					 inline Flags get_flags(void) { return myf; }					 

		  private:
					  // The next two functions are there to get the different between asked window size and given window size
					 // Ex: you want a 100x100 window. Now, the User_Interface class makes sure that border widths and top,bottom heights
					 // are added to the size and still the window musn't leave the screen on any edges.
					 inline const static uint get_border(void) { return (CORNER<<1); }

					 					 
					 int handle_click(const uint, const bool, const uint, const uint);
					 int handle_mm(const uint, const uint, const bool, const bool);
					 void draw(void);	
					 
										 //					 void set_closefunc(...)


		  friend class User_Interface;

		  private:
					 // Only friends can create and destroy and move us!
					 void set_pos(const uint, const uint); 
					 Window(const uint, const uint, const uint, const uint, const Flags); 
					 ~Window();
					 void redraw_win(void);
					 
					 uint x, y, w, h;
					 Pic* winpic;
					 Pic* own_bg;
					 Flags myf;
				
					 // for textareas
					 uint nta;
					 Textarea** ta;
					
					 // for buttons
					 uint nbut;
					 Button** but;

					 // for checkboxes
					 uint ncheckbox;
					 Checkbox** checkbox;
					
					 // for listselects
					 uint nlistselect;
					 Listselect** listselect;

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
		  
		  Window* create_window(const uint, const uint, const uint, const uint, const Window::Flags=Window::DEFAULT); 
		  void delete_window(Window*); 
		  void move_window(Window*, const uint, const uint);
		  void draw(void);
		  int handle_mm(const uint, const uint, const int, const int, const bool, const bool, void*);
		  int handle_click(const uint, const bool, const uint, const uint, void* );

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
