/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "graphic.h"

#define PIXEL(x, y)		pixels[(y)*pitch+(x)]

/*
==========================================================================

Bitmap

==========================================================================
*/

/** Bitmap::draw_rect(uint rx, uint ry, uint rw, uint rh, ushort color)
 *
 * Draws the outline of a rectangle
 */
void Bitmap::draw_rect(uint rx, uint ry, uint rw, uint rh, ushort color)
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

/** Bitmap::fill_rect(uint rx, uint ry, uint rw, uint rh, ushort color)
 *
 * Draws a filled rectangle
 */
void Bitmap::fill_rect(uint rx, uint ry, uint rw, uint rh, ushort color)
{
   rw += rx;
   rh += ry;
   for (uint y=ry; y<rh; y++)
   {
      uint p = y * pitch + rx;
      for (uint x=rx; x<rw; x++)
         pixels[p++]= color;
   }
}

/** Bitmap::brighten_rect(uint rx, uint ry, uint rw, uint rh, int factor)
 *
 * Change the brightness of the given rectangle
 */
void Bitmap::brighten_rect(uint rx, uint ry, uint rw, uint rh, int factor)
{
   rw += rx;
   rh += ry;
   for (uint y=ry; y<rh; y++)
   {
      uint p = y * pitch + rx;
      for (uint x=rx; x<rw; x++)
         pixels[p++]= bright_up_clr(pixels[p], factor);
   }
}

/** Bitmap::set_clrkey(ushort dclrkey)
 *
 * sets the clrkey of this bitmap
 *
 * Args: dclrkey	The clrkey to use
 */
void Bitmap::set_clrkey(ushort dclrkey)
{
   sh_clrkey = dclrkey;
   bhas_clrkey=true;
}

/** Bitmap::set_clrkey(uchar r, uchar g, uchar b)
 *
 * sets the clrkey of this pic
 *
 * Args: r	red value of clrkey
 * 	     g	green value of clrkey
 * 	     b	blue value of clrkey
 */
void Bitmap::set_clrkey(uchar r, uchar g, uchar b)
{
   ushort dclrkey = pack_rgb(r,g,b);
   sh_clrkey=dclrkey;
   bhas_clrkey=true;
}

/** Bitmap::make_partof(const Bitmap *other, uint x, uint y, uint w, uint h,
*                      int *ofsx, int *ofsy)
 *
 * Makes this bitmap an alias of a part of another bitmap.
 * The given rectangle is automatically clipped. If x and y lie outside of
 * the bitmap, subsequent drawing operations need to be offset. The necessary
 * offset is returned in the provided pointers.
 *
 * Args: other	the source bitmap
*       x		coordinates of the rectangle you are going to use
*       y
*       nw
*       nh
*       ofsx	pointers to the offset that is going to be necessary
*       ofsy
 *
 * Returns: false if the resulting bitmap size is 0. In this case, this
*          bitmap may end up in an invalid state.
*/
bool Bitmap::make_partof(const Bitmap *other, int x, int y, uint nw, uint nh,
      int *ofsx, int *ofsy)
{
   pitch = other->pitch;
   sh_clrkey = other->sh_clrkey;
   bhas_clrkey = other->bhas_clrkey;

   // clipping
   w = nw;
   h = nh;
   *ofsx = 0;
   *ofsy = 0;
   if (x < 0) {
      if (-x >= (int)w)
         return false;
      *ofsx = x;
      w += x;
      x = 0;
   } else if (x > (int)other->w)
      return false;
   if (x+w > other->w)
      w = other->w - x;

   if (y < 0) {
      if (-y >= (int)h)
         return false;
      *ofsy = y;
      h += y;
      y = 0;
   } else if (y > (int)other->h)
      return false;
   if (y+h > other->h)
      h = other->h - y;

   pixels = other->pixels + y*pitch + x;
   return true;
}

/*
   ==========================================================================

   Pic

   ==========================================================================
   */

/** Pic& Pic::operator=(const Pic& p)
 *
 * Copy operator. Simple and slow. Don't use often
 *
 * Args: p 	pic to copy
 * returns: *this
 */
Pic& Pic::operator=(const Pic& p) {
   sh_clrkey=p.sh_clrkey;
   bhas_clrkey=p.bhas_clrkey;

   set_size(p.w, p.h);

   memcpy(pixels, p.pixels, sizeof(short)*w*h);

   return *this;
}

/** void Pic::set_size(const uint nw, const uint nh)
 *
 * This functions sets the new size of a pic
 *
 * Args: nw	= new width
 * 		 nh	= new height
 * Returns: nothinh
 */
void Pic::set_size(const uint nw, const uint nh) {
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
   pitch=nw;
   h=nh;
   
   clear_all();
}

/** Pic::clear_all(void) [private]
 *
 * Clears all pixels with the color of the color key.
 */
void Pic::clear_all(void)
{
   if(!bhas_clrkey) return;

   for(int i = w*h-1; i >= 0; i--)
      pixels[i] = sh_clrkey;
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
   for (uint y=0; y<h; y++)
   {
      uchar* bits = (uchar*)bmp->pixels + y * bmp->pitch;
      for (uint x=0; x<w; x++)
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



