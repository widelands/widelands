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

#define PIXEL(x, y)		m_pixels[(y)*m_pitch+(x)]

/*
==========================================================================

Bitmap

==========================================================================
*/

/*
===============
Bitmap::Bitmap

Initialize a Bitmap.
===============
*/
Bitmap::Bitmap()
{
	m_pixels = 0;
	
	m_w = m_pitch = 0;
	m_h = 0;
	
	m_clrkey = 0;
	m_bhas_clrkey = false;
	
	m_ofsx = m_ofsy = 0;
}


/*
===============
Bitmap::draw_rect

Draws the outline of a rectangle
===============
*/
void Bitmap::draw_rect(int rx, int ry, int rw, int rh, uchar r, uchar g, uchar b)
{
	ushort color = pack_rgb(r, g, b);

	rx += m_ofsx;
	ry += m_ofsy;
	
   rw += rx;
   rh += ry;
	
	if (rx < 0)
		rx = 0;
	if (ry < 0)
		ry = 0;
	if (rw > (int)m_w)
		rw = m_w;
	if (rh > (int)m_h)
		rh = m_h;
	
	if (rx > rw || ry > rh)
		return;
	
   for (int x=rx+1; x<rw-1; x++)
   {
      PIXEL(x, ry) = color;
      PIXEL(x, rh-1) = color;
   }
   for (int y=ry; y<rh; y++)
   {
      PIXEL(rx, y) = color;
      PIXEL(rw-1, y) = color;
   }
}


/*
===============
Bitmap::fill_rect

Draws a filled rectangle
===============
*/
void Bitmap::fill_rect(int rx, int ry, int rw, int rh, uchar r, uchar g, uchar b)
{
	ushort color = pack_rgb(r, g, b);
   
	rx += m_ofsx;
	ry += m_ofsy;
	
	rw += rx;
   rh += ry;
	
	if (rx < 0)
		rx = 0;
	if (ry < 0)
		ry = 0;
	if (rw > (int)m_w)
		rw = m_w;
	if (rh > (int)m_h)
		rh = m_h;
	
	if (rx > rw || ry > rh)
		return;
	
   for (int y=ry; y<rh; y++)
   {
      uint p = y * m_pitch + rx;
      for (int x=rx; x<rw; x++)
         m_pixels[p++]= color;
   }
}

/*
===============
Bitmap::brighten_rect

Change the brightness of the given rectangle
===============
*/
void Bitmap::brighten_rect(int rx, int ry, int rw, int rh, int factor)
{
	rx += m_ofsx;
	ry += m_ofsy;
   
	rw += rx;
   rh += ry;
	
	if (rx < 0)
		rx = 0;
	if (ry < 0)
		ry = 0;
	if (rw > (int)m_w)
		rw = m_w;
	if (rh > (int)m_h)
		rh = m_h;
	
	if (rx > rw || ry > rh)
		return;
	
   for (int y=ry; y<rh; y++)
   {
      uint p = y * m_pitch + rx;
      for (int x=rx; x<rw; x++)
         m_pixels[p++]= bright_up_clr(m_pixels[p], factor);
   }
}


/*
===============
Bitmap::clear

Clear the entire bitmap to black
===============
*/
void Bitmap::clear()
{
	// this function clears the bitmap completly (draws it black)
	// it assumes, that: (2*w*h)%4 == 0.
	// This function is speedy
	if (m_w != m_pitch || (m_w & 1 && m_h & 1))
		fill_rect(0, 0, m_w, m_h, 0, 0, 0);
	else {
		int i=(m_w*m_h);
		assert(!(i&3));
		i>>=1;
		while(--i) *(((long*)(m_pixels))+i)=0; // faster than a memset?
		*m_pixels=0;
	}
}


/*
===============
Bitmap::blit

Blits a blitsource into this bitmap
===============
*/
void Bitmap::blit(int dstx, int dsty, BlitSource* src)
{
	src->blit_to_bitmap16(this, dstx + m_ofsx, dsty + m_ofsy);
}


/*
===============
Bitmap::blitrect

Blits a blitsource into this bitmap, using a source rectangle
===============
*/
void Bitmap::blitrect(int dstx, int dsty, BlitSourceRect* src, int srcx, int srcy, int w, int h)
{
	src->blit_to_bitmap16rect(this, dstx + m_ofsx, dsty + m_ofsy, srcx, srcy, w, h);
}


/*
===============
Bitmap::blit_to_bitmap16
===============
*/
void Bitmap::blit_to_bitmap16(Bitmap* dst, int dstx, int dsty)
{
	blit_to_bitmap16rect(dst, dstx, dsty, 0, 0, m_w, m_h);
}

/*
===============
Bitmap::blit_to_bitmap16rect

Blit this bitmap to the given destination, clipping as necessary.
===============
*/
void Bitmap::blit_to_bitmap16rect(Bitmap* dst, int dstx, int dsty, int srcx, int srcy, int w, int h)
{
	// Clip the rectangle
	if (dstx < 0) {
		srcx -= dstx;
		w += dstx;
		dstx = 0;
	}
	if (dstx + w > (int)dst->m_w)
		w = dst->m_w - dstx;
	if (w <= 0)
		return;
	
	if (dsty < 0) {
		srcy -= dsty;
		h += dsty;
		dsty = 0;
	}
	if (dsty + h > (int)dst->m_h)
		h = dst->m_h - dsty;
	if (h <= 0)
		return;
	
	// Actual blitting
	ushort* dstpixels = dst->m_pixels + dsty*dst->m_pitch + dstx;
	ushort* srcpixels = m_pixels + srcy*m_pitch + srcx;
	
	if (m_bhas_clrkey)
   {
		uint dstskip = dst->m_pitch - w;
		uint srcskip = m_pitch - w;
		
      // Slow blit, checking for clrkeys. This could probably speed up by copying
      // 2 pixels (==4bytes==register width)
      // in one rush. But this is a nontrivial task
      // This could also use MMX assembly on targets that support it...
      for (int i = h; i; i--, dstpixels += dstskip, srcpixels += srcskip) {
         for (int j = w; j; j--, dstpixels++, srcpixels++) {
            ushort clr = *srcpixels;
            if (clr != m_clrkey)
               *dstpixels = clr;
         }
      }
   }
   else
   {
      if (w == (int)dst->m_pitch && w == (int)m_pitch)
      {
         // copying entire rows, so it can be all done in a single memcpy
			memcpy(dstpixels, srcpixels, (w*h) << 1);
      }
      else
      {
         // fast blitting, using one memcpy per row
			int bw = w << 1;
			
			for(int i = h; i; i--, dstpixels += dst->m_pitch, srcpixels += m_pitch)
				memcpy(dstpixels, srcpixels, bw);
      }
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
   m_clrkey = dclrkey;
   m_bhas_clrkey=true;
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
   m_clrkey=dclrkey;
   m_bhas_clrkey=true;
}


/*
===============
Bitmap::enter_window

Create a new bitmap that represents the requested subset of this bitmap.
Clipping is ensured as appropriate.

Returns zero if the requested subwindow is not visible at all.

TODO: Can we avoid all this allocating and deleting of objects somehow?
===============
*/
RenderTarget* Bitmap::enter_window(int x, int y, int w, int h)
{
	int newofsx = 0;
	int newofsy = 0;

	x += m_ofsx;
	y += m_ofsy;
	
	// Clipping
	if (x < 0) {
		newofsx = x;
		w += x;
		x = 0;
	}
	if (x + w > (int)m_w)
		w = m_w - x;
	if (w <= 0)
		return 0;
	
	if (y < 0) {
		newofsy = y;
		h += y;
		y = 0;
	}
	if (y + h > (int)m_h)
		h = m_h - y;
	if (h <= 0)
		return 0;
	
	// Create the new bitmap
	Bitmap* subset = new Bitmap;
	
	subset->m_pixels = m_pixels + y*m_pitch + x;
	subset->m_pitch = m_pitch;
	subset->m_w = w;
	subset->m_h = h;
	subset->m_ofsx = newofsx;
	subset->m_ofsy = newofsy;
	
	subset->m_bhas_clrkey = m_bhas_clrkey;
	subset->m_clrkey = m_clrkey;
	
	return subset;
}

/*
===============
Bitmap::leave_window

Finish using this sub-window
===============
*/
void Bitmap::leave_window()
{
	delete this;
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
Pic& Pic::operator=(const Pic& p)
{
   m_clrkey=p.m_clrkey;
   m_bhas_clrkey=p.m_bhas_clrkey;

   set_size(p.m_w, p.m_h);

   memcpy(m_pixels, p.m_pixels, sizeof(short)*m_w*m_h);

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
void Pic::set_size(const uint nw, const uint nh)
{
   if (m_pixels)
		free(m_pixels);
	
   m_w = nw;
   m_h = nh;
	m_pitch = nw;

	m_pixels = (ushort*)malloc(m_w*m_h*sizeof(ushort));
   
   clear_all();
}

/** Pic::clear_all(void) [private]
 *
 * Clears all pixels with the color of the color key.
 */
void Pic::clear_all(void)
{
   if(!m_bhas_clrkey) return;

	ushort* dst = m_pixels;
   for(int i = m_w*m_h; i; i--)
      *dst++ = m_clrkey;
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

   for (uint y=0; y<m_h; y++)
   {
      uchar* bits = (uchar*)bmp->pixels + y * bmp->pitch;
		ushort* dst = m_pixels + y*m_pitch;
      for (uint x=0; x<m_w; x++, dst++)
      {
         uchar r = *(bits + (bmp->format->Rshift >> 3));
         uchar g = *(bits + (bmp->format->Gshift >> 3));
         uchar b = *(bits + (bmp->format->Bshift >> 3));
			*dst = pack_rgb(r, g, b);
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
   set_size(mw, mh);
	
	for(int y = 0; y < mh; y++, data += mw)
		memcpy(m_pixels + y*m_pitch, data, mw*sizeof(ushort));
	
   return RET_OK;
}

