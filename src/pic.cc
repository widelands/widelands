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

// 2002-02-10	sft+	minor speedup in Pic::clear_all

#include "graphic.h"
#include "fileloc.h"
#include "output.h"

#ifdef WIN32
#include <string.h>
#endif

#define PIXEL(x, y)		pixels[(y)*w+(x)]

namespace Graph
{
	void Pic::draw_rect(uint rx, uint ry, uint rw, uint rh, uint color)
	{
		rw += rx;
		rh += ry;
		for (uint x=rx+1; x<rw-1; x++)
		{
			PIXEL(x, ry) = color;
			PIXEL(x, rh-1) = color;
		}
		for (uint y=ry; y<rh; y++)
		{
			PIXEL(rx, y) = color;
			PIXEL(rw-1, y) = color;
		}
	}

	void Pic::fill_rect(uint rx, uint ry, uint rw, uint rh, uint color)
	{
		rw += rx;
		rh += ry;
		for (uint y=ry; y<rh; y++)
		{
			uint p = y * w + rx;
			for (uint x=rx; x<rw; x++)
				pixels[p++]= color;
		}
	}

	void Pic::brighten_rect(uint rx, uint ry, uint rw, uint rh, int factor)
	{
		rw += rx;
		rh += ry;
		for (uint y=ry; y<rh; y++)
		{
			uint p = y * w + rx;
			for (uint x=rx; x<rw; x++)
				pixels[p++]= bright_up_clr(pixels[p], factor);
		}
	}

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
					 bhas_clrkey=p.bhas_clrkey;

					 set_size(p.w, p.h);

					 memcpy(pixels, p.pixels, sizeof(short)*w*h);

					 return *this;
		  }

		  /** void Pic::set_size(const ushort nw, const ushort nh)
			*
			* This functions sets the new size of a pic
			*
			* Args: nw	= new width
			* 		 nh	= new height
			* Returns: nothinh
			*/
		  void Pic::set_size(const ushort nw, const ushort nh) {
					 if(pixels) free(pixels);
					 w=nw;
					 h=nh;

					 // We make sure, that everything pic is aligned to 4bytes, so that clearing can
					 // goes fast with dwords
					 if(w & 1) w++; // %2
					 if(h & 1) h++;  // %2
					 if(!w) w=2;
					 if(!h) h=2;

					 pixels=(ushort*) malloc(sizeof(short)*w*h);

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
		  void Pic::clear_all(void)
		  {
				if(!bhas_clrkey) return;

				for(int i=w*h-1; i>=0; i--)
						  pixels[i]=sh_clrkey;

//				for(int i=w*h-2; i>=0; i-=2)
//						  pixels[i]=clrkey;
		  }

		  /** int Pic::load(const char* file)
			*
			* This function loads a bitmap from a file using the SDL functions
			*
			* Args: file		Path to pic
			* Returns: RET_OK or ERR_FAILED
			*/
	int Pic::load(const char* file)
	{
		if(!file) return ERR_FAILED;

		SDL_Surface* bmp = SDL_LoadBMP(file);
		if(bmp == NULL)
			return ERR_FAILED;

		set_size(bmp->w, bmp->h);

		int i=0;
		for (int y=0; y<h; y++)
		{
			uchar* bits = (uchar*)bmp->pixels + y * bmp->pitch;
			for (int x=0; x<w; x++)
			{
				uchar r = *(bits + (bmp->format->Rshift >> 3));
				uchar g = *(bits + (bmp->format->Gshift >> 3));
				uchar b = *(bits + (bmp->format->Bshift >> 3));
				pixels[i++] = pack_rgb(r, g, b);
				bits += bmp->format->BytesPerPixel;
			}
		}

		SDL_FreeSurface(bmp);

		return RET_OK;
	}

		/** void Pic::create(const ushort w, const ushort h, ushort* data)
		  *
		  * This function creates the picture described by the args
		  *
		  * Args:	mw, mh	widht/height of image
		  *			data	the image data
		  * Returns:		RET_OK or ERR_FAILED
		  */
		int Pic::create(const ushort mw, const ushort mh, ushort* data)
		{
			if (!mw || !mh)
				return ERR_FAILED;
			this->set_size(mw, mh);
			memcpy(pixels, data, mw*mh*sizeof(ushort));
			return RET_OK;
		}

		  /** void Pic::set_clrkey(ushort dclrkey)
			*
			* sets the clrkey of this pic
			*
			* Args: dclrkey	The clrkey to use
			* Returns: nothing
			*/
		  void Pic::set_clrkey(ushort dclrkey) {
					 sh_clrkey=dclrkey;
					 clrkey= (dclrkey<<16 | dclrkey);
					 bhas_clrkey=true;
		  }

		  /** void Pic::set_clrkey(uchar r, uchar g, uchar b)
			*
			* sets the clrkey of this pic
			*
			* Args: r	red value of clrkey
			* 		  g	green value of clrkey
			* 		  b	blue value of clrkey
			* Returns: nothing
			*/
		  void Pic::set_clrkey(uchar r, uchar g, uchar b) {
					 ushort dclrkey=pack_rgb(r,g,b);
					 sh_clrkey=dclrkey;
					 clrkey= (dclrkey<<16 & clrkey);
					 bhas_clrkey=true;
		  }

	/*
	==========================================================================

	AutoPic

	==========================================================================
	*/

	AutoPic *AutoPic::first = 0;

	/** AutoPic::load_all()
	 *
	 * Loads all AutoPics into memory.
	 */
	void AutoPic::load_all()
	{
		while(first) {
			char buf[1024];
			const char *effname = g_fileloc.locate_file(first->filename, TYPE_PIC);
			if (!effname) {
				sprintf(buf, "%s: File not found. Check your installation and --searchdir.", first->filename);
				tell_user(buf);
				exit(0);
			}
			first->load(effname);
			if ((first->desw && first->desw != (int)first->get_w()) ||
			    (first->desh && first->desh != (int)first->get_h())) {
				char buf[1024];
				sprintf(buf, "%s: should be %ix%i pixels.", first->filename, first->desw, first->desh);
				tell_user(buf);
				exit(0);
			}
			first = first->next;
		}
	}
}
