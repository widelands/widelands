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

#ifndef __S__GRAPHIC_H
#define __S__GRAPHIC_H

#define 	DEF_CLRKEY	Graph::pack_rgb(0,0,255)	

#include <SDL/SDL.h>
#include <stdlib.h>
#include "errors.h"
#include "singleton.h"

namespace Graph {

		  /** inline unsigned short Graph::pack_rgb(const unsigned char r, const unsigned char g, const unsigned char b);
			* 
			* This functions packs a RGB tribble into a short
			*
			* Args: r 	red value
			* 		g 	green value
			* 		b	blue value (SUPRISE!)
			* Returns: packed value
			*/
		  inline unsigned short Graph::pack_rgb(const unsigned char r, const unsigned char g, const unsigned char b) {
					 return ((b>>3) + ((g>>2)<<5)+ ((r>>3)<<11) );
		  }

		  /** inline void Graph::unpack_rgb(const unsigned short clr, unsigned char* r, unsigned char* g, unsigned char* b) ;
			*	
			* this unpacks a clr and returns the RGB tribble
			*
			* Args: 	clr	clr to unpack
		  *   		r		ptr to var to hold red value
			*			g		ptr to var to hold green value
			*			b		ptr to var to hold blue value
			* Returns: Nothing
			*/
		  inline void Graph::unpack_rgb(const unsigned short clr, unsigned char* r, unsigned char* g, unsigned char* b) {
					 *r= ((clr<<3)>>11);
					 *g= ((clr<<2)>>5);
					 *b= (clr<<3);
		  }

		  
		  /** class Pic
			* 
			* This class represents a picture  
			*/
		  class Pic {
					 public:
					 Pic(void) { pixels=NULL; w=h=lpix=clrkey=sh_clrkey=bhas_clrkey=0; }
					 ~Pic(void) { if(pixels) free(pixels); }

					 void set_size(const unsigned short, const unsigned short);
					 void set_clrkey(const unsigned short);
					 void set_clrkey(const unsigned char, const unsigned char, const unsigned char);
					 int  load(const char*);
					 void clear_all(void);
					 Pic& operator=(const Pic&);
					 Pic(const Pic&);

					 /** inline unsigned int Pic::get_w(void) const 
					  * 
					  * This function returns the width
					  * Args: none
					  * returns: width
					  */
					 inline unsigned int get_w(void) const { return w; }

					 /** inline unsigned int Pic::get_h(void) const
					  * 
					  * This function returns the height
					  * Args: none
					  * returns: height
					  */
					 inline unsigned int get_h(void) const { return h; }

					 /** inline unsigned short get_clrkey(void) const
					  *
					  * this returns the current colorkey
					  *
					  * Args: none
					  * returns: clrkey
					  */
					 inline unsigned short get_clrkey(void) const {
								if(bhas_clrkey) return sh_clrkey;
								return 0;
					 }

					 /** inline bool has_clrkey(void) const
					  *
					  * this indicates if the pixel has a valid clrkey
					  *
					  * Args: none
					  * Returns: if the pixel has a clrkey or not
					  */
					 inline bool has_clrkey(void) { return bhas_clrkey; }
					 
					 // optimized pixel get functions
					 // They are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline unsigned short get_pixel(const unsigned int x, const unsigned int y) {
								lpix=(y)*(w) + (x);
								return pixels[lpix];
					 }
					 inline unsigned short get_npixel(void) {
								return pixels[++lpix];
					 }
					 inline unsigned short get_ppixel(void) {
								return pixels[--lpix];
					 }
					 inline unsigned short get_fpixel(void) {
								lpix=0;
								return pixels[0];
					 }

					 // rewinding or forwarding without change
					 inline void npixel(void) { ++lpix; }
					 inline void ppixel(void) { --lpix; }
					 inline void fpixel(void) { lpix=0; }
					 inline void set_cpixel(const unsigned int x, const unsigned int y) { lpix=y*w + x; }

					 // optimized pixel set functions declared inline
					 // All those functions take either a rgb tribble or 3 chars
					 // And they are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline void set_pixel(const unsigned int x, const unsigned int y, const unsigned char r, const unsigned char g, const unsigned char b) {
								lpix=(y)*(w) + (x);
								pixels[lpix] = pack_rgb(r, g, b);
					 }
					 inline void set_pixel(const unsigned int x, const unsigned int y, const unsigned short clr) {
								lpix=(y)*(w) + (x);
								pixels[lpix] = clr; 
					 }
					 inline void set_npixel(const unsigned short clr) {
								pixels[++lpix] = clr;
					 }
					 inline void set_npixel(const unsigned char r, const unsigned char g, const unsigned char b) {
								pixels[++lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_ppixel(const unsigned short clr) {
								pixels[--lpix]= clr;
					 }
					 inline void set_ppixel(const unsigned char r, const unsigned char g, const unsigned char b) {
								pixels[--lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_fpixel(const unsigned short clr) {
								lpix=0;
								pixels[0] = clr;
					 }
					 inline void set_fpixel(const unsigned char r, const unsigned char g, const unsigned char b) {
								lpix=0;
								pixels[0] = pack_rgb(r, g, b);
					 }
					 
					 // this function really needs faaast blitting
					 friend	void copy_pic(Pic*, Pic*, const unsigned short, const unsigned short,  const unsigned short, const unsigned short, 
										  const unsigned short, const unsigned short);
					 friend  void draw_pic(Pic*, const unsigned short, const unsigned short,  const unsigned short, const unsigned short, 
										  const unsigned short, const unsigned short);

					 private:
					 bool bhas_clrkey;
					 unsigned long clrkey;
					 unsigned short sh_clrkey;
					 unsigned short* pixels;
					 unsigned short w, h;
					 unsigned int lpix;
		  };

		  /** class Graphic 
			*
			* This class is responsible for all graphics stuff. It's
			* modified/optimized to work only for 16bit colordepth and nothing else
			* 
			* It's a singleton
			*/
		  class Graphic : public Singleton<Graphic> {
					 // forbidden functions
					 Graphic(const Graphic&);
					 Graphic& operator=(const Graphic&);

					 public:
					 enum Mode {
								MODE_FS,
								MODE_WIN
					 };

					 enum State {
								NOT_INIT,
								OK,
								ERROR
					 };

					 Graphic(void);
					 ~Graphic(void);

					 void set_mode(const unsigned short, const unsigned short, const Mode);
					 void update_screen(void);
					 void update_rect(const unsigned short, const unsigned short, const unsigned short, const unsigned short);
					 void update_quarter(void);

					 /** Graphic::State Graphic::get_state(void) 
					  *
					  * returns the current state of the graphics class
					  *
					  * Args: none
					  * Returns: nothing
					  */
					 State get_state(void) const { return st; }

					 /** inline bool Graphic::is_fs(void)
					  *
					  * graphic mode is fullscreen?
					  *
					  * Args: none
					  * Returns: true if fs, false if window
					  */
					 inline bool is_fs(void) { return bis_fs; st=OK; }

					 /** inline unsigned int Graphic::get_xres(void) const
					  * 
					  * This function returns the X Resoultion of the current screen
					  * Args: none
					  * returns: XRes
					  */
					 inline unsigned int get_xres(void) const { return xres; }

					 /** inline unsigned int Graphic::get_yres(void) const
					  * 
					  * This function returns the Y Resoultion of the current screen
					  * Args: none
					  * returns: YRes
					  */
					 inline unsigned int get_yres(void) const { return yres; }


					 // optimized pixel get functions
					 // They are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline unsigned short get_pixel(const unsigned int x, const unsigned int y) {
								lpix=(y)*(xres>>1) + (x);
								return *((Uint16*) sc->pixels + lpix);
					 }
					 inline unsigned short get_npixel(void) {
								return *((Uint16*) sc->pixels + ++lpix);
					 }
					 inline unsigned short get_ppixel(void) {
								return *((Uint16*) sc->pixels + --lpix);
					 }
					 inline unsigned short get_fpixel(void) {
								lpix=0;
								return *((Uint16*) sc->pixels);
					 }

					 // rewinding or forwarding without change
					 inline void npixel(void) { ++lpix; }
					 inline void ppixel(void) { --lpix; }
					 inline void fpixel(void) { lpix=0; }
					 inline void set_cpixel(const unsigned int x, const unsigned int y) { lpix=y*xres + x; }

					 // optimized pixel set functions declared inline
					 // All those functions take either a rgb tribble or 3 chars
					 // And they are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline void set_pixel(const unsigned int x, const unsigned int y, const unsigned char r, const unsigned char g, const unsigned char b) {
								lpix=(y)*(xres) + (x);
								pixels[lpix] = pack_rgb(r, g, b);
					 }
					 inline void set_pixel(const unsigned int x, const unsigned int y, const unsigned short clr) {
								lpix=(y)*(xres) + (x);
								pixels[lpix] = clr; 
					 }
					 inline void set_npixel(const unsigned short clr) {
								pixels[++lpix] = clr;
					 }
					 inline void set_npixel(const unsigned char r, const unsigned char g, const unsigned char b) {
								pixels[++lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_ppixel(const unsigned short clr) {
								pixels[--lpix]= clr;
					 }
					 inline void set_ppixel(const unsigned char r, const unsigned char g, const unsigned char b) {
								pixels[--lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_fpixel(const unsigned short clr) {
								lpix=0;
								pixels[0] = clr;
					 }
					 inline void set_fpixel(const unsigned char r, const unsigned char g, const unsigned char b) {
								lpix=0;
								pixels[0] = pack_rgb(r, g, b);
					 }
		
					 
					 // this function really needs faaast blitting
					 friend	void draw_pic(Pic*, const unsigned short, const unsigned short,  const unsigned short, const unsigned short, 
										  const unsigned short, const unsigned short);

					 private:
					 unsigned short* pixels;
					 unsigned short xres, yres;
					 unsigned int lpix;
					 bool	bis_fs;
					 SDL_Surface* sc;
					 State st;
		  };

		  void draw_pic(Pic*, const unsigned short, const unsigned short,  const unsigned short, const unsigned short, 
								const unsigned short, const unsigned short);
		  void copy_pic(Pic*, Pic*, const unsigned short, const unsigned short,  const unsigned short, const unsigned short, 
								const unsigned short, const unsigned short);
}

using 	Graph::Graphic;
using 	Graph::Pic;
#define 	g_gr 	Graph::Graphic::get_singleton()

#endif /* __S__GRAPHIC_H */
