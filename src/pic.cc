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

namespace Graph {
		  /** Pic::Pic(const Pic& p)
			*
			* Copy constructor. Slow and simple
			*
			* Args: p	pic to copy
			* Returns: Nothing
			*/
		  Pic::Pic(const Pic& p) {
					 *this=p;
		  }


		  /** Pic& Pic::operator=(const Pic& p) 
			*
			* Copy operator. Simple and slow. Don't use often
			*
			* Args: p 	pic to copy
			* returns: *this
			*/
		  Pic& Pic::operator=(const Pic& p) {
					 clrkey=p.clrkey;
					 sh_clrkey=p.sh_clrkey;
					 lpix=0;
					 bhas_clrkey=p.bhas_clrkey;

					 set_size(p.w, p.h);
					 
					 memcpy(pixels, p.pixels, sizeof(short)*w*h);
					 
					 return *this;
		  }

		  /** void Pic::set_size(const unsigned short nw, const unsigned short nh) 
			*
			* This functions sets the new size of a pic
			*
			* Args: nw	= new width
			* 		 nh	= new height
			* Returns: nothinh
			*/
		  void Pic::set_size(const unsigned short nw, const unsigned short nh) {
					 if(pixels) free(pixels);
					 w=nw;
					 h=nh;

					 // We make sure, that everything pic is aligned to 4bytes, so that clearing can 
					 // goes fast with dwords
					 if(w & 1) w++; // %2
					 if(h & 1) h++;  // %2
					 if(!w) w=2;
					 if(!h) h=2;

					 pixels=(unsigned short*) malloc(sizeof(short)*w*h);

					 w=nw;
					 h=nh;

					 clear_all();
		  }

		  /** void Pic::clear_all(void) 
			*
			* This function clear all the pixels and sets them to the clrkey, if defined
			*
			* Args: none
			* Returns: nothing
			*/
		  void Pic::clear_all(void) {
					 if(!clrkey) return;

					 for(unsigned int i=(w*h-2); i; i-=2) {
								pixels[i]=clrkey;
					 }
		  }

		  /** int Pic::load(const char* file)
			*
			* This function loads a bitmap from a file using the SDL functions 
			*
			* Args: file		Path to pic
			* Returns: RET_OK or ERR_FAILED
			*/
		  int Pic::load(const char* file) {
					 if(!file) return ERR_FAILED;
					 
					 SDL_Surface* bmp=NULL;
					 int x=0;
					 int y=0;
					 unsigned char* bits, R, G, B;

					 bmp=SDL_LoadBMP(file);
					 if(bmp == NULL) { return ERR_FAILED; }

					 set_size(bmp->w, bmp->h);

					 for(y=0; y<h; y++) {
								for(x=0; x<w; x++) {
										  bits=((Uint8*)bmp->pixels)+y*bmp->pitch+x*bmp->format->BytesPerPixel;
										  R= (Uint8) *((bits)+bmp->format->Rshift/8);
										  G= (Uint8) *((bits)+bmp->format->Gshift/8);
										  B= (Uint8) *((bits)+bmp->format->Bshift/8);
										  set_pixel(x, y, R, G, B);
								}
					 }

					 SDL_FreeSurface(bmp);

					 return RET_OK;
		  }

		  /** void Pic::set_clrkey(unsigned short dclrkey) 
			*
			* sets the clrkey of this pic
			*
			* Args: dclrkey	The clrkey to use
			* Returns: nothing
			*/
		  void Pic::set_clrkey(unsigned short dclrkey) {
					 sh_clrkey=dclrkey;
					 clrkey= (dclrkey<<16 | dclrkey);
					 bhas_clrkey=true;
		  }

		  /** void Pic::set_clrkey(unsigned char r, unsigned char g, unsigned char b) 
			*
			* sets the clrkey of this pic
			*
			* Args: r	red value of clrkey
			* 		  g	green value of clrkey
			* 		  b	blue value of clrkey
			* Returns: nothing
			*/
		  void Pic::set_clrkey(unsigned char r, unsigned char g, unsigned char b) { 
					 unsigned short dclrkey=pack_rgb(r,g,b);
					 sh_clrkey=dclrkey;
					 clrkey= (dclrkey<<16 & clrkey);
					 bhas_clrkey=true;
		  }	

}
