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

/*
==============================================================================

IMPLEMENTATIONS

==============================================================================
*/

/** class Bitmap
 *
 * Rectangle of 16bit pixels, can be colorkeyed.
 *
 * This class provides a common interface for both loaded and run-time
 * generated pictures (i.e. class Pic) and the framebuffer.
 */
class Bitmap : public RenderTarget, public BlitSourceRect {
   public:
      Bitmap();

      inline int get_w() const { return m_w; }
      inline int get_h() const { return m_h; }
      inline uint get_pitch() const { return m_pitch; }
      inline ushort *get_pixels() const { return m_pixels; }
      inline bool has_clrkey(void) const { return m_bhas_clrkey; }
      inline ushort get_clrkey(void) const
      {
         if (m_bhas_clrkey) return m_clrkey;
         return 0;
      }

      void set_clrkey(const ushort);
      void set_clrkey(const uchar, const uchar, const uchar);

		// interface RenderTarget
		RenderTarget* enter_window(int x, int y, int w, int h);
		void leave_window();
      
		void draw_rect(int x, int y, int w, int h, RGBColor clr);
      void fill_rect(int x, int y, int w, int h, RGBColor clr);
      void brighten_rect(int x, int y, int w, int h, int factor);
      void clear(void);
      
		void blit(int dstx, int dsty, BlitSource* src);
		void blitrect(int dstx, int dsty, BlitSourceRect* src, int srcx, int srcy, int w, int h);

	private:
		// interface BlitSource
		void blit_to_bitmap16(Bitmap* dst, int dstx, int dsty);
		void blit_to_bitmap16rect(Bitmap* dst, int dstx, int dsty, int srcx, int srcy, int w, int h);

   protected:
      ushort *m_pixels;
      uint m_w, m_h;
      uint m_pitch; // every row in the bitmap is pitch pixels long
      ushort m_clrkey;
      bool m_bhas_clrkey;
		int m_ofsx, m_ofsy; // added to every destination position (necessary when used as RenderTarget)

      friend void copy_animation_pic(RenderTarget* dst, Animation* anim, uint time,
		      int dst_x, int dst_y, const uchar *plrclrs);
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
      virtual ~Pic(void) { if (m_pixels) free(m_pixels); }

      Pic(const Pic& p) { *this = p; }
      Pic& operator=(const Pic&);

      void set_size(const uint w, const uint h);
      int  load(const char*);
      int  create(const ushort, const ushort, ushort* data);

   private:
      void clear_all(void);
};


/**/


#endif 

