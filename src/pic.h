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

#ifndef __S__PIC_H
#define __S__PIC_H

class Animation;

/** class Bitmap
 *
 * Rectangle of 16bit pixels, can be colorkeyed.
 *
 * This class provides a common interface for both loaded and run-time
 * generated pictures (i.e. class Pic) and the framebuffer.
 */
class Bitmap {
   public:
      Bitmap() { pixels = 0; w = pitch = h = sh_clrkey = bhas_clrkey = 0; }

      inline uint get_w() const { return w; }
      inline uint get_h() const { return h; }
      inline uint get_pitch() const { return pitch; }
      inline ushort *get_pixels() const { return pixels; }
      inline bool has_clrkey(void) const { return bhas_clrkey; }
      inline ushort get_clrkey(void) const
      {
         if (bhas_clrkey) return sh_clrkey;
         return 0;
      }

      void draw_rect(uint x, uint y, uint w, uint h, ushort color);
      void fill_rect(uint x, uint y, uint w, uint h, ushort color);
      void brighten_rect(uint x, uint y, uint w, uint h, int factor);

      void set_clrkey(const ushort);
      void set_clrkey(const uchar, const uchar, const uchar);

      bool make_partof(const Bitmap *other, int x, int y, uint nw, uint nh,
            int *ofsx, int *ofsy);

   protected:
      ushort *pixels;
      uint w, h;
      uint pitch; // every row in the bitmap is pitch pixels long
      ushort sh_clrkey;
      bool bhas_clrkey;

      friend void copy_pic(Bitmap *dst, Bitmap *src, int dst_x, int dst_y,
            uint src_x, uint src_y, int w, int h);
      friend void copy_animation_pic(Bitmap* dst, Animation* anim, uint time,
		      int dst_x, int dst_y, uint src_x, uint src_y, int w, int h);
      friend class Graphic;
};

/** class Pic
 *
 * Pic represents a picture.
 * It extends Bitmap by providing Load, Resize, etc... operations
 */
class Pic : public Bitmap
{
   public:
      Pic(void) : Bitmap() { }
      ~Pic(void) { if (pixels) free(pixels); }

      Pic(const Pic& p) { *this = p; }
      Pic& operator=(const Pic&);

      void set_size(const uint w, const uint h);
      int  load(const char*);
      int  create(const ushort, const ushort, ushort* data);

   private:
      void clear_all(void);
};



#endif 

