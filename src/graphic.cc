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

#include "graphic.h"
#ifdef WIN32
#include <string.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// #include <iostream>
namespace Graph {

		  /** class Graphic
			*
			* This functions is responsible for displaying graphics and keeping them up to date
			*
			* It's little strange in it's interface, but it is optimzed for speed, not beauty
			* This is a singleton
			*/

		  /** Graphic::Graphic(void) 
			*
			* Default Constructor. Simple Inits
			*
			* Args: none
			* Returns: nothing
			*/
		  Graphic::Graphic(void) {
					 lpix=0;
					 sc=NULL;
					 pixels=NULL;
					 xres=yres=0;
					 st=STATE_NOT_INIT;
					 nupr=0;
					 bneeds_fs_update=false;
					 
					 SDL_Init(SDL_INIT_VIDEO);
		  }

		  /** Graphic::~Graphic(void) 
			*
			* simple cleanups. 
			*
			* Args: none
			* Returns: nothing
			*/
		  Graphic::~Graphic(void) {
					 lpix=0;
					 if(sc) {
								SDL_FreeSurface(sc);
								sc=NULL;
					 }
					 pixels=NULL;
					 xres=yres=0;
					 st=STATE_NOT_INIT;
					 
					 SDL_Quit();
		  }

		  /** void Graphic::set_mode(const unsigned short x, const unsigned short y, const Mode m)
			*
			* This function sets a new graphics mode.
			*
			* Args:	x	x resolution
			* 		y	y resolution
			* 		m	either windows or fullscreen
			* Returns: Nothing
			*/
		  void Graphic::set_mode(const unsigned short x, const unsigned short y, const Mode m) {
					 if(sc)
								SDL_FreeSurface(sc);

					 if(m==MODE_FS) {
								sc = SDL_SetVideoMode(x, y, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
					 } else {
								sc = SDL_SetVideoMode(x, y, 16, SDL_SWSURFACE);
					 }

					 mode=m;
					 xres=x; 
					 yres=y;
					 pixels=(unsigned short*) sc->pixels;

					 st=STATE_OK;
					 
					 bneeds_update=true;

					 return;
		  }

		  /** void Graphic::register_update_rect(const unsigned short x, const unsigned short y, const unsigned short w, const unsigned short h);
			*
			* This registers a rect of the screen for update 
			*
			* Args: 	x	upper left corner of rect
			* 			y  upper left corner of rect
			* 			w	width
			* 			h	height
			*/
		  void Graphic::register_update_rect(const unsigned short x, const unsigned short y, const unsigned short w, const unsigned short h) {
					 if(nupr>=MAX_RECTS) { 
								bneeds_fs_update=true; 
								return; 
					 }
					 
					upd_rects[nupr].x=x;
					upd_rects[nupr].y=y;
					upd_rects[nupr].w=w;
					upd_rects[nupr].h=h;
				
					++nupr;

					bneeds_update=true;
		  }

		  /** void Graphic::update(void) 
			*
			*	This function updates the registered rects on the screen
			*
			* Args: none
			* Returns: Nothing
			*/
		  void Graphic::update(void) {
					 if(bneeds_fs_update) {
								SDL_UpdateRect(sc, 0, 0, xres, yres);
					 } else {
	/*							cerr << "##########################" << endl;
								cerr << nupr << endl;
								for(unsigned int i=0; i<nupr; i++) 
										  cerr << upd_rects[i].x << ":" << upd_rects[i].y << ":" << 
													 upd_rects[i].w << ":" << upd_rects[i].h << endl;
								cerr << "##########################" << endl;
	*/							SDL_UpdateRects(sc, nupr, upd_rects);
					 }
					 nupr=0;
					 bneeds_fs_update=false;
					 bneeds_update=false;
		  }
		  
		  /** void draw_pic(Pic* p, const unsigned short d_x_pos, const unsigned short d_y_pos,  const unsigned short p_x_pos, 
			* 		const unsigned short p_y_pos, const unsigned short i_w, const unsigned short i_h)
			*
			* 	This functions plots a picture onto the current screen
			*	
			*	friend to class pic and class Graphic
			*
			* 	Args:	p 	picture to plot
			* 			d_x_pos	xpos on screen
			* 			d_y_pos	ypos on screen
			* 			p_x_pos	start xpos in picture
			* 			p_y_pos	start ypos in picture
			* 			i_w		width
			* 			i_h		height
			* 	returns: Nothing
			*/
		  void draw_pic(Pic* p, const unsigned short d_x_pos, const unsigned short d_y_pos,  const unsigned short p_x_pos, const unsigned short p_y_pos, 
								const unsigned short i_w, const unsigned short i_h) {
					 unsigned int clr;
					 unsigned int w=i_w;
					 unsigned int h=i_h; 

					 if(d_x_pos+w>g_gr.get_xres()) w=g_gr.get_xres()-d_x_pos;
					 if(d_y_pos+h>g_gr.get_yres()) h=g_gr.get_yres()-d_y_pos;

					 if(p->has_clrkey()) {
								// Slow blit, checking for clrkeys. This could probably speed up by copying
								// 2 pixels (==4bytes==register width)
								// in one rush. But this is a nontrivial task
								for(unsigned long  y=0; y<h; y++) {
										  clr=p->get_pixel(p_x_pos, p_y_pos+y);
										  if(clr != p->get_clrkey()) g_gr.set_pixel(d_x_pos, d_y_pos+y, clr);
										  else g_gr.set_cpixel(d_x_pos, d_y_pos+y);
										  for(unsigned long x=1; x<w; x++) {
													 clr=p->get_npixel();
													 if(clr != p->get_clrkey()) g_gr.set_npixel(clr);
													 else g_gr.npixel();
										  }
								}
					 } else {
								if(w == g_gr.get_xres() && h == g_gr.get_yres()) {
										  // one memcpy and we're settled
										  memcpy(g_gr.pixels, p->pixels, (p->get_w()*p->get_h()<<1));
								} else {
										  unsigned long poffs=p->get_w()*p_y_pos + p_x_pos;
										  unsigned long doffs=g_gr.get_xres()*d_y_pos + d_x_pos;

										  // fast blitting, using memcpy
										  for(unsigned long y=0; y<h; y++) {
													 memcpy(g_gr.pixels+doffs, p->pixels+poffs, w<<1); // w*sizeof(short) 
													 doffs+=g_gr.get_xres();
													 poffs+=p->get_w();
										  }
								}
					 }
		  }
											        
		  /** void copy_pic(Pic* dst, Pic* src, const unsigned short d_x_pos, const unsigned short d_y_pos,  const unsigned short p_x_pos, 
			* 		const unsigned short p_y_pos, const unsigned short i_w, const unsigned short i_h)
			*
			* 	This functions plots a picture into an other
			*
			*	friend to class pic
			*
			* 	Args:	src 	picture to plot
			* 			dst	picture to plot inside
			* 			d_x_pos	xpos on screen
			* 			d_y_pos	ypos on screen
			* 			p_x_pos	start xpos in picture
			* 			p_y_pos	start ypos in picture
			* 			i_w		width
			* 			i_h		height
			* 	returns: Nothing
			*/
		  void copy_pic(Pic* dst, Pic* src, const unsigned short d_x_pos, const unsigned short d_y_pos,  const unsigned short p_x_pos, 
								const unsigned short p_y_pos, const unsigned short i_w, const unsigned short i_h) {
					 unsigned short clr;
					 unsigned int w=i_w;
					 unsigned int h=i_h; 

					 if(d_x_pos+w>dst->get_w()) w=dst->get_w()-d_x_pos;
					 if(d_y_pos+h>dst->get_h()) h=dst->get_h()-d_y_pos;

					if(src->has_clrkey() && (dst->get_clrkey()!=src->get_clrkey())) {
								for(unsigned long  y=0; y<h; y++) {
										  clr=src->get_pixel(p_x_pos, p_y_pos+y);
										  if(clr != src->get_clrkey()) dst->set_pixel(d_x_pos, d_y_pos+y, clr);
										  else dst->set_cpixel(d_x_pos, d_y_pos+y);
										  for(unsigned long x=1; x<w; x++) {
													 clr=src->get_npixel();
													 if(clr != src->get_clrkey()) dst->set_npixel(clr);
													 else dst->npixel();
										  }
								}
					 } else {
								unsigned long soffs=src->get_w()*p_y_pos + p_x_pos;
								unsigned long doffs=dst->get_w()*d_y_pos + d_x_pos;

								// fast blitting, using memcpy
								for(unsigned long y=0; y<h; y++) {
										  memcpy(dst->pixels+doffs, src->pixels+soffs, w<<1); // w*sizeof(short) 
										  doffs+=dst->get_w();
										  soffs+=src->get_w();
								}
					 }		  
		  }
}
