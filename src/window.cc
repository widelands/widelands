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
#include "input.h"

#if 0
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

AutoPic Window::l_border("win_l_border.bmp", CORNER, MUST_HAVE_NPIX);
AutoPic Window::r_border("win_r_border.bmp", CORNER, MUST_HAVE_NPIX);
AutoPic Window::top("win_top.bmp", MUST_HAVE_NPIX, CORNER);
AutoPic Window::bot("win_bot.bmp", MUST_HAVE_NPIX, CORNER);
AutoPic Window::bg("win_bg.bmp");

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

		  ncheckbox=0;
		  checkbox=(Checkbox**) malloc(sizeof(Checkbox)*MAX_CHECKBOX);

		  nlistselect=0;
		  listselect=(Listselect**) malloc(sizeof(Listselect)*MAX_LISTSELECT);

		  nmultiline_textarea=0;
		  multiline_textarea=(Multiline_Textarea**) malloc(sizeof(Multiline_Textarea)*MAX_MULTILINE_TEXTAREA);

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

		  for(i=0; i< ncheckbox; i++)
					 delete checkbox[i];
		  free(checkbox);
		  
		  for(i=0; i<nlistselect; i++) 
					 delete listselect[i];
		  free(listselect);
		  
		  for(i=0; i<nmultiline_textarea; i++) 
					 delete  multiline_textarea[i]; 
		  free(multiline_textarea);

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
		  g_gr.register_update_rect(x, y, w, h);
		  x=posx; y=posy;
		  g_gr.register_update_rect(x, y, w, h);
}
					
/** Listselect* Window::create_listselect(const uint px, const uint py, const uint w, const uint h) 
 *
 * This creates a listselect at the given point
 *
 * Args: px, py	pos in window
 * 		w,h		dimensions
 * Returns: Pointer to listselect-box just created
 */
Listselect* Window::create_listselect(const uint px, const uint py, const uint w, const uint h) {
		  uint myw=w;
		  uint myh=h;
		  uint add=0;

		  if(myf!=FLAT) {
					 myw-=get_border();
					 myh-=get_border();
					 add=get_border()>>1;
		  }

		  listselect[nlistselect]=new Listselect(px, py, myw-20-Button::get_border(), myh, winpic, add, add);
		  listselect[nlistselect]->draw();

		  g_gr.register_update_rect(x+listselect[nlistselect]->get_xpos(), y+listselect[nlistselect]->get_ypos(),
								listselect[nlistselect]->get_w(), listselect[nlistselect]->get_h());

		  assert(nlistselect<MAX_TA);

		  // set the move button
		  Button* b=create_button(listselect[nlistselect]->get_xpos()+myw-20-Button::get_border(),
								listselect[nlistselect]->get_ypos(),
								20, 20, 1);
		  b->register_func(listselect_but_up, listselect[nlistselect]);
		  b->set_pic(g_fh.get_string("U", 0));
		  b=create_button(listselect[nlistselect]->get_xpos()+myw-20-Button::get_border(), 
								listselect[nlistselect]->get_ypos()+20+Button::get_border(),
								20, 20, 1);
		  b->set_pic(g_fh.get_string("D", 0));
		  b->register_func(listselect_but_down, listselect[nlistselect]);
								
		  nlistselect++;
		  
		  g_gr.register_update_rect(x+px, y+py, myw, myh);
		  
		  return listselect[nlistselect-1];
}

/** Multiline_Textarea* Window::create_multiline_textarea(const uint px, const uint py, const uint w, const uint h, 
 * 	const bool scrollable, const Multiline_Textarea::Align a);
 *
 * This creates a listselect at the given point
 *
 * Args: px, py	pos in window
 * 		w,h		dimensions
 * 		scrollable 	is this textarea scrollable?
 * 		a 			alignent of each line in this box
 * Returns: Pointer to listselect-box just created
 */
Multiline_Textarea*  Window::create_multiline_textarea(const uint px, const uint py, const uint w, const uint h, const bool scrollable,
					 const Multiline_Textarea::Align a) {
		  uint myw=w;
		  uint myh=h;
		  uint add=0;

		  if(myf!=FLAT) {
					 myw-=get_border();
					 myh-=get_border();
					 add=get_border()>>1;
		  }

		  if(!scrollable) {
					 multiline_textarea[nmultiline_textarea]=new Multiline_Textarea(px, py, myw, myh, a, winpic, add, add);
					 multiline_textarea[nmultiline_textarea]->draw();

					 g_gr.register_update_rect(x+multiline_textarea[nmultiline_textarea]->get_xpos(), y+multiline_textarea[nmultiline_textarea]->get_ypos(), 
										  multiline_textarea[nmultiline_textarea]->get_w(), multiline_textarea[nmultiline_textarea]->get_h());
					 assert(nmultiline_textarea<MAX_TA);
		  } else {
					 multiline_textarea[nmultiline_textarea]=new Multiline_Textarea(px, py, myw-20-Button::get_border(), myh, a, winpic, add, add);
					 multiline_textarea[nmultiline_textarea]->draw();

					 g_gr.register_update_rect(x+multiline_textarea[nmultiline_textarea]->get_xpos(), y+multiline_textarea[nmultiline_textarea]->get_ypos(), 
										  multiline_textarea[nmultiline_textarea]->get_w(), multiline_textarea[nmultiline_textarea]->get_h());

					 assert(nmultiline_textarea<MAX_TA);

					 // set the move button
					 Button* b=create_button(multiline_textarea[nmultiline_textarea]->get_xpos()+myw-20-Button::get_border(), 
										  multiline_textarea[nmultiline_textarea]->get_ypos(),
										  20, 20, 1);
					 b->register_func(multiline_textarea_but_up, multiline_textarea[nmultiline_textarea]);
					 b->set_pic(g_fh.get_string("U", 0));
					 b=create_button(multiline_textarea[nmultiline_textarea]->get_xpos()+myw-20-Button::get_border(),
										  multiline_textarea[nmultiline_textarea]->get_ypos()+20+Button::get_border(),
										  20, 20, 1);
					 b->set_pic(g_fh.get_string("D", 0));
					 b->register_func(multiline_textarea_but_down, multiline_textarea[nmultiline_textarea]);
		  }
		  nmultiline_textarea++;

		  g_gr.register_update_rect(x+px, y+py, myw, myh);

		  return multiline_textarea[nmultiline_textarea-1];

}

/** Textarea* Window::create_textarea(const uint px, const uint py, const char* t ,  Textarea::Align a = Textarea::LEFTA)
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

		  // Draw listselects
		  for(i=0; i< nlistselect; i++)
					 listselect[i]->draw();

		  // Draw multiline textareas
		  for(i=0; i<nmultiline_textarea; i++)
					 multiline_textarea[i]->draw();

		  // Draw textareas
		  for(i=0 ; i< nta; i++)
					 ta[i]->draw();

		  // Draw Buttons
		  for(i=0; i< nbut; i++)
					 but[i]->draw();

		  // Draw checkboxes
		  for(i=0; i< ncheckbox; i++)
					 checkbox[i]->draw();

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


/** Checkbox* Window::create_checkbox(const uint px, const uint py, const bool bstate)
 *
 * This funcion creates a checkbox in the window
 *
 * Args: px, py	position
 * 		bstate	initial state of checkbox
 * Returns: The created checkbox
 */
Checkbox* Window::create_checkbox(const uint px, const uint py, const bool bstate) {
		  uint add=0;
		  if(myf!=FLAT) add=get_border();

		  int mypx=px;
		  int mypy=py;
		  uint myw=Checkbox::get_w();
		  uint myh=Checkbox::get_h();


		  if(px+add+myw > w) mypx=w-(add+myw);
		  if(mypx<0) return NULL;
		  if(py+add+myh > h) mypy=h-(add+myh);



		  checkbox[ncheckbox]=new Checkbox(mypx, mypy, bstate, winpic, add>>1, add>>1);
		  checkbox[ncheckbox]->draw();
		  g_gr.register_update_rect(x+checkbox[ncheckbox]->get_xpos(),
								y+checkbox[ncheckbox]->get_ypos(), checkbox[ncheckbox]->get_w(), checkbox[ncheckbox]->get_h());
		  ncheckbox++;

		  assert(ncheckbox<MAX_CHECKBOX);

		  return checkbox[ncheckbox-1];
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

		  // We do not care for checkboxes, since they only react on clicks
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
										  but[i]->run(this);
								}
								but[i]->set_pressed(b);
					 } else {
								but[i]->set_pressed(false);
					 }

					 if(but[i]->draw()) g_gr.register_update_rect(this->x+but[i]->get_xpos(), this->y+but[i]->get_ypos(), but[i]->get_w(), but[i]->get_h());
		  }

		  // Check for checkboxes
		  if(b) {
					 for(i=0; i<ncheckbox; i++) {
								if(checkbox[i]->get_xpos()<x && checkbox[i]->get_xpos()+checkbox[i]->get_w()>x &&
													 checkbox[i]->get_ypos()<y && checkbox[i]->get_ypos()+checkbox[i]->get_h()>y) {
										  // is inside!
										  checkbox[i]->set_state(!checkbox[i]->get_state());
								}

								if(checkbox[i]->draw()) g_gr.register_update_rect(this->x+checkbox[i]->get_xpos(),
													 this->y+checkbox[i]->get_ypos(), checkbox[i]->get_w(), checkbox[i]->get_h());
					 }
		  }
		  
		  // Check for select boxes
		  if(b) {
					 for(i=0; i<nlistselect; i++) {
								if(listselect[i]->get_xpos()<x && listselect[i]->get_xpos()+listselect[i]->get_w()>x &&
													 listselect[i]->get_ypos()<y && listselect[i]->get_ypos()+listselect[i]->get_h()>y) {
										  // is inside!
										  listselect[i]->select(y-listselect[i]->get_ypos());
								}

								if(listselect[i]->draw()) g_gr.register_update_rect(this->x+listselect[i]->get_xpos(), 
													 this->y+listselect[i]->get_ypos(), listselect[i]->get_w(), listselect[i]->get_h());
					 }
		  }
	
		  // we do not care for textareas, they are unresponsive to clicks
		  return INPUT_UNHANDLED;
}

#endif
