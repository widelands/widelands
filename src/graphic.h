/*
 * Copyright (C) 2002 by the Wide Lands Development Team
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

#define 	DEF_CLRKEY	pack_rgb(0,0,255)

#include "pic.h"

#include <SDL.h> // need SDL_Rect

/** inline ushort pack_rgb(const uchar r, const uchar g, const uchar b);
 *
 * This functions packs a RGB tribble into a short
 *
 * Args: r 	red value
 * 		g 	green value
 * 		b	blue value (SUPRISE!)
 * Returns: packed value
 */
inline ushort pack_rgb(const uchar r, const uchar g, const uchar b)
{
   return ((b>>3) + ((g>>2)<<5)+ ((r>>3)<<11) );
}

/** inline void unpack_rgb(const ushort clr, uchar* r, uchar* g, uchar* b) ;
 *
 * this unpacks a clr and returns the RGB tribble
 *
 * Args: 	clr	clr to unpack
*   		r		ptr to var to hold red value
 *			g		ptr to var to hold green value
 *			b		ptr to var to hold blue value
 * Returns: Nothing
 */
inline void unpack_rgb(const ushort clr, uchar* r, uchar* g, uchar* b)
{
   *r= ((clr<<3)>>11);
   *g= ((clr<<2)>>5);
   *b= (clr<<3);
}

/** inline ushort bright_up_clr(const ushort clr, const ushort factor)
 *
 * This function brights a clr up.
 *
 * Args:	clr to bright up
 * 			factor	by how much
 * Returns: Brighter color
 */
inline ushort bright_up_clr(const ushort clr, const ushort factor)
{
   uchar r, g, b;
   r= ((clr<<3)>>11);
   g= ((clr<<2)>>5);
   b= (clr<<3);
   r= ((long) r+factor) > 255 ? 255 : r+factor;
   g= ((long) g+factor) > 255 ? 255 : g+factor;
   b= ((long) b+factor) > 255 ? 255 : b+factor;
   return pack_rgb(r, g, b);
}

#if 1
// the voodoo version..., rev 2.
// why use goto?
inline uint bright_up_clr2(uint clr, int factor)
{
   int r = ((clr >> 11) << 3);
   int g = ((clr >> 5)  << 2) & 0xFF;
   int b = ((clr)       << 3) & 0xFF;

   r += factor;
   if (r & 0xFF00) r = (~r) >> 24;
   g += factor;
   if (g & 0xFF00) g = (~g) >> 24;
   b += factor;
   if (b & 0xFF00) b = (~b) >> 24;
   return pack_rgb(r, g, b);
}
#elif 1
// the voodoo version...
// it's quite a bit faster than the original on my CPU
inline uint bright_up_clr2(uint clr, int factor)
{
   int r = ((clr >> 11) << 3);
   int g = ((clr >> 5)  << 2) & 0xFF;
   int b = ((clr)       << 3) & 0xFF;

   r += factor;
   if (r & 0xFF00) goto fix_r;
end_r:
   g += factor;
   if (g & 0xFF00) goto fix_g;
end_g:
   b += factor;
   if (b & 0xFF00) goto fix_b;
   goto end;

fix_r: r = (~r) >> 24; goto end_r;
fix_g: g = (~g) >> 24; goto end_g;
fix_b: b = (~b) >> 24;
end:
       return pack_rgb(r, g, b);
}
#else
inline uint bright_up_clr2(uint clr, int factor)
{
   if (factor == 0)
      return clr;
   int r = ((clr >> 11) << 3);
   int g = ((clr >> 5)  << 2) & 0xFF;
   int b = ((clr)       << 3) & 0xFF;

   if (factor > 0)
   {
      r += factor;
      if (r > 255) r = 255;
      g += factor;
      if (g > 255) g = 255;
      b += factor;
      if (b > 255) b = 255;
   }
   else
   {
      r += factor;
      if (r < 0) r = 0;
      g += factor;
      if (g < 0) g = 0;
      b += factor;
      if (b < 0) b = 0;
   }
   return pack_rgb(r, g, b);
}
#endif

/** class Point
*/
struct Point
{
   int x;
   int y;
   Point()
   {
      x = y = 0;
   }
   Point(int px, int py)
   {
      x = px; y = py;
   }
};

// hm, floats...
// tried to be faster with fixed point arithmetics
// it was, but i'll try to find other opts first
class Vector
{
   public:
      float x;
      float y;
      float z;
      Vector()
      {
         x = y = z = 0;
      }
      Vector(float px, float py, float pz)
      {
         x = px; y = py; z = pz;
      }
      void normalize()
      {
         float f = (float)sqrt(x*x + y*y + z*z);
         if (f == 0)
            return;
         x /= f;
         y /= f;
         z /= f;
      }
};

// vector addition
inline Vector operator + (const Vector& a, const Vector& b)
{
   return Vector(a.x + b.x, a.y + b.y, a.z + b.z);
};

// inner product
inline float operator * (const Vector& a, const Vector& b)
{
   return a.x * b.x + a.y * b.y + a.z * b.z;
};

/** class Graphic
 *
 * This class is responsible for all graphics stuff. It's
 * modified/optimized to work only for 16bit colordepth and nothing else
 */
#define MAX_RECTS 20

class Graphic
{
   // forbidden functions
   Graphic(const Graphic&);
   Graphic& operator=(const Graphic&);

   public:
   enum Mode
   {
      MODE_FS,
      MODE_WIN
   };

   enum State
   {
      STATE_NOT_INIT,
      STATE_OK,
      STATE_ERROR
   };

   Graphic(void);
   ~Graphic(void);

   void set_mode(ushort, ushort, Mode);
   void register_update_rect(const ushort, const ushort, const ushort, const ushort);
   void update(void);
   void screenshot(const char*);

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
   inline uint get_xres(void) const { return screenbmp.get_w(); }

   /** inline uint Graphic::get_yres(void) const
    *
    * This function returns the Y Resoultion of the current screen
    * Args: none
    * returns: YRes
    */
   inline uint get_yres(void) const { return screenbmp.get_h(); }

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

   /** inline Bitmap *get_screenbmp()
    *
    * Return a Bitmap that contains the screen surface.
    *
    * Returns: a Bitmap representing the screen
    */
   inline Bitmap *get_screenbmp() { return &screenbmp; }

   private:
   Bitmap screenbmp;
   Mode mode;
   SDL_Surface* sc;
   State st;
   SDL_Rect upd_rects[MAX_RECTS];
   uint nupr;
   bool bneeds_fs_update;
   bool bneeds_update;
};

extern Graphic *g_graphic;

#define g_gr (*g_graphic)


void render_triangle(Bitmap *dst, Point* points, int *brightness, Pic* texture);
void render_road_horiz(Bitmap *dst, Point start, Point end, ushort color);
void render_road_vert(Bitmap *dst, Point start, Point end, ushort color);
void copy_pic(Bitmap *dst, Bitmap *src, int dst_x, int dst_y,
      uint src_x, uint src_y, int w, int h);

class Animation;

void copy_animation_pic(Bitmap* dst, Animation* anim, uint time, int dst_x, int dst_y, const uchar *plrclrs);

#endif /* __S__GRAPHIC_H */
