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

// 2002-02-10	sft+	renamed State members

#ifndef __S__GRAPHIC_H
#define __S__GRAPHIC_H

#define 	DEF_CLRKEY	Graph::pack_rgb(0,0,255)	

#include <SDL/SDL.h>
#include <stdlib.h>
#include "errors.h"
#include "singleton.h"
#include "mytypes.h"

namespace Graph {

		 
		  /** inline ushort Graph::pack_rgb(const uchar r, const uchar g, const uchar b);
			* 
			* This functions packs a RGB tribble into a short
			*
			* Args: r 	red value
			* 		g 	green value
			* 		b	blue value (SUPRISE!)
			* Returns: packed value
			*/
		  inline ushort pack_rgb(const uchar r, const uchar g, const uchar b) {
					 return ((b>>3) + ((g>>2)<<5)+ ((r>>3)<<11) );
		  }

		  /** inline void Graph::unpack_rgb(const ushort clr, uchar* r, uchar* g, uchar* b) ;
			*	
			* this unpacks a clr and returns the RGB tribble
			*
			* Args: 	clr	clr to unpack
		  *   		r		ptr to var to hold red value
			*			g		ptr to var to hold green value
			*			b		ptr to var to hold blue value
			* Returns: Nothing
			*/
		  inline void unpack_rgb(const ushort clr, uchar* r, uchar* g, uchar* b) {
					 *r= ((clr<<3)>>11);
					 *g= ((clr<<2)>>5);
					 *b= (clr<<3);
		  }

		  /** inline ushort Graph::bright_up_clr(const ushort clr, const ushort factor)
			*
			* This function brights a clr up.
			*
			* Args:	clr to bright up
			* 			factor	by how much
			* Returns: Brighter color
			*/
		  inline ushort bright_up_clr(const ushort clr, const ushort factor) {
					 uchar r, g, b;
					 r= ((clr<<3)>>11);
					 g= ((clr<<2)>>5);
					 b= (clr<<3);
					 r= ((long) r+factor) > 255 ? 255 : r+factor;
					 g= ((long) g+factor) > 255 ? 255 : g+factor;
					 b= ((long) b+factor) > 255 ? 255 : b+factor;
					 return pack_rgb(r, g, b);
		  }
		 
		  
		  /** class Pic
			* 
			* This class represents a picture  
			*/
		  class Pic {
					 public:
					 Pic(void) { pixels=NULL; w=h=lpix=clrkey=sh_clrkey=bhas_clrkey=0; }
					 ~Pic(void) { if(pixels) free(pixels); }

					 void set_size(const ushort, const ushort);
					 void set_clrkey(const ushort);
					 void set_clrkey(const uchar, const uchar, const uchar);
					 int  load(const char*);
					 int  create(const ushort, const ushort, ushort* data);
					 void clear_all(void);
					 Pic& operator=(const Pic&);
					 Pic(const Pic&);

					 /** inline uint Pic::get_w(void) const 
					  * 
					  * This function returns the width
					  * Args: none
					  * returns: width
					  */
					 inline uint get_w(void) const { return w; }

					 /** inline uint Pic::get_h(void) const
					  * 
					  * This function returns the height
					  * Args: none
					  * returns: height
					  */
					 inline uint get_h(void) const { return h; }

					 /** inline ushort get_clrkey(void) const
					  *
					  * this returns the current colorkey
					  *
					  * Args: none
					  * returns: clrkey
					  */
					 inline ushort get_clrkey(void) const {
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
					 inline ushort get_pixel(const uint x, const uint y) {
								lpix=(y)*(w) + (x);
								return pixels[lpix];
					 }
					 inline ushort get_npixel(void) {
								return pixels[++lpix];
					 }
					 inline ushort get_ppixel(void) {
								return pixels[--lpix];
					 }
					 inline ushort get_fpixel(void) {
								lpix=0;
								return pixels[0];
					 }

					 // rewinding or forwarding without change
					 inline void npixel(void) { ++lpix; }
					 inline void ppixel(void) { --lpix; }
					 inline void fpixel(void) { lpix=0; }
					 inline void set_cpixel(const uint x, const uint y) { lpix=y*w + x; }

					 // optimized pixel set functions declared inline
					 // All those functions take either a rgb tribble or 3 chars
					 // And they are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline void set_pixel(const uint x, const uint y, const uchar r, const uchar g, const uchar b) {
								lpix=(y)*(w) + (x);
								pixels[lpix] = pack_rgb(r, g, b);
					 }
					 inline void set_pixel(const uint x, const uint y, const ushort clr) {
								lpix=(y)*(w) + (x);
								pixels[lpix] = clr; 
					 }
					 inline void set_npixel(const ushort clr) {
								pixels[++lpix] = clr;
					 }
					 inline void set_npixel(const uchar r, const uchar g, const uchar b) {
								pixels[++lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_ppixel(const ushort clr) {
								pixels[--lpix]= clr;
					 }
					 inline void set_ppixel(const uchar r, const uchar g, const uchar b) {
								pixels[--lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_fpixel(const ushort clr) {
								lpix=0;
								pixels[0] = clr;
					 }
					 inline void set_fpixel(const uchar r, const uchar g, const uchar b) {
								lpix=0;
								pixels[0] = pack_rgb(r, g, b);
					 }
					 
					 // this function really needs faaast blitting
					 friend	void copy_pic(Pic*, Pic*, const ushort, const ushort,  const ushort, const ushort, 
										  const ushort, const ushort);
					 friend  void draw_pic(Pic*, const ushort, const ushort,  const ushort, const ushort, 
										  const ushort, const ushort);

					 private:
					 bool bhas_clrkey;
					 ulong clrkey;
					 ushort sh_clrkey;
					 ushort* pixels;
					 ushort w, h;
					 uint lpix;
		  };

		  /** class Graphic 
			*
			* This class is responsible for all graphics stuff. It's
			* modified/optimized to work only for 16bit colordepth and nothing else
			* 
			* It's a singleton
			*/
		  #define MAX_RECTS 20
		  
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
								STATE_NOT_INIT,
								STATE_OK,
								STATE_ERROR
					 };

					 Graphic(void);
					 ~Graphic(void);

					 void set_mode(const ushort, const ushort, const Mode);
					 void register_update_rect(const ushort, const ushort, const ushort, const ushort);
					 void update(void);

					 /** Graphic::State Graphic::get_state(void) 
					  *
					  * returns the current state of the graphics class
					  *
					  * Args: none
					  * Returns: nothing
					  */
					 State get_state(void) const { return st; }

					 /** inline Mode Graphic::get_mode(void)
					  *
					  * return the current mode (fs or window) 
					  *
					  * Args: none
					  * Returns: the current mode
					  */
					 inline Mode get_mode(void) {  st=STATE_OK; return mode; }

					 /** inline uint Graphic::get_xres(void) const
					  * 
					  * This function returns the X Resoultion of the current screen
					  * Args: none
					  * returns: XRes
					  */
					 inline uint get_xres(void) const { return xres; }

					 /** inline uint Graphic::get_yres(void) const
					  * 
					  * This function returns the Y Resoultion of the current screen
					  * Args: none
					  * returns: YRes
					  */
					 inline uint get_yres(void) const { return yres; }

					 /** inline void Graphics::needs_fs_update(void) 
					  *
					  * This functions tells the graphic that it should redraw the whole screen
					  *
					  * Args: None
					  * returns: Nothing
					  */
					 inline void needs_fs_update(void) { bneeds_fs_update=bneeds_update=true; }

					 /** inline bool does_need_update(void) 
					  *
					  * This returns if the object needs to be updated
					  *
					  * Args: None
					  * Returns: true if the screen should be redrawn
					  */
					 inline bool does_need_update(void) { return bneeds_update; }

					 // optimized pixel get functions
					 // They are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline ushort get_pixel(const uint x, const uint y) {
								lpix=(y)*(xres>>1) + (x);
								return *((Uint16*) sc->pixels + lpix);
					 }
					 inline ushort get_npixel(void) {
								return *((Uint16*) sc->pixels + ++lpix);
					 }
					 inline ushort get_ppixel(void) {
								return *((Uint16*) sc->pixels + --lpix);
					 }
					 inline ushort get_fpixel(void) {
								lpix=0;
								return *((Uint16*) sc->pixels);
					 }

					 // rewinding or forwarding without change
					 inline void npixel(void) { ++lpix; }
					 inline void ppixel(void) { --lpix; }
					 inline void fpixel(void) { lpix=0; }
					 inline void set_cpixel(const uint x, const uint y) { lpix=y*xres + x; }

					 // optimized pixel set functions declared inline
					 // All those functions take either a rgb tribble or 3 chars
					 // And they are rather simple,
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline void set_pixel(const uint x, const uint y, const uchar r, const uchar g, const uchar b) {
								lpix=(y)*(xres) + (x);
								pixels[lpix] = pack_rgb(r, g, b);
					 }
					 inline void set_pixel(const uint x, const uint y, const ushort clr) {
								lpix=(y)*(xres) + (x);
								pixels[lpix] = clr; 
					 }
					 inline void set_npixel(const ushort clr) {
								pixels[++lpix] = clr;
					 }
					 inline void set_npixel(const uchar r, const uchar g, const uchar b) {
								pixels[++lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_ppixel(const ushort clr) {
								pixels[--lpix]= clr;
					 }
					 inline void set_ppixel(const uchar r, const uchar g, const uchar b) {
								pixels[--lpix]= pack_rgb(r, g, b);
					 }
					 inline void set_fpixel(const ushort clr) {
								lpix=0;
								pixels[0] = clr;
					 }
					 inline void set_fpixel(const uchar r, const uchar g, const uchar b) {
								lpix=0;
								pixels[0] = pack_rgb(r, g, b);
					 }
		
					 
					 // this function really needs faaast blitting
					 friend	void draw_pic(Pic*, const ushort, const ushort,  const ushort, const ushort, 
										  const ushort, const ushort);

					 private:
					 ushort* pixels;
					 ushort xres, yres;
					 uint lpix;
					 Mode mode;
					 SDL_Surface* sc;
					 State st;
					 SDL_Rect upd_rects[MAX_RECTS];
					 uint nupr;
					 bool bneeds_fs_update;
					 bool bneeds_update;
		  };

		  void draw_pic(Pic*, const ushort, const ushort,  const ushort, const ushort, 
								const ushort, const ushort);
		  void copy_pic(Pic*, Pic*, const ushort, const ushort,  const ushort, const ushort, 
								const ushort, const ushort);
}

using 	Graph::Graphic;
using 	Graph::Pic;
#define 	g_gr 	Graph::Graphic::get_singleton()

#endif /* __S__GRAPHIC_H */
