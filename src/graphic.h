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

#define TEXTURE_W          64
#define TEXTURE_H          TEXTURE_W   // texture have a fixed size and are squares, TEXTURE_H is just defined for easier understandement of the code


/*
==============================================================================

COLOR HANDLING

==============================================================================
*/

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

// the voodoo version...
// it's quite a bit faster than the original on my CPU
inline ushort bright_up_clr(ushort clr, int factor)
{
   if(!factor) return clr;
   
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

/*
class RGBColor
*/
class RGBColor {
	uchar m_r, m_g, m_b;
	
public:
	inline RGBColor() { }
	inline RGBColor(uchar r, uchar g, uchar b) : m_r(r), m_g(g), m_b(b) { }
	
	inline void set(uchar r, uchar g, uchar b) { m_r = r; m_g = g; m_b = b; }
	
	inline uchar r() const { return m_r; }
	inline uchar g() const { return m_g; }
	inline uchar b() const { return m_b; }
	
	inline ushort pack16() const {
		return ((m_b>>3) + ((m_g>>2)<<5)+ ((m_r>>3)<<11) );
	}
	inline void unpack16(ushort clr) {
		m_r = ((clr<<3)>>11);
		m_g = ((clr<<2)>>5);
		m_b = (clr<<3);
	}
};


/*
==============================================================================

class Point

==============================================================================
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

/** class Point_with_bright
 * this class is like a point, but with additional bright factor
 * bright is an int to make it possible to directly save shifted values (8.8 fixed or so)
 */
struct Point_with_bright : public Point {
   int b;
   Point_with_bright() : Point(0,0) { b=0; }
   Point_with_bright(int px, int py, int pb) : Point(px, py) { b=pb; }
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


/*
==============================================================================

Graphics object interfaces

==============================================================================
*/

class BlitSource;
class BlitSourceRect;

/*
class RenderTarget

This abstract class represents anything that can be rendered to.

enter_window() is used to obtain a RenderTarget that can be used to draw into
the given rectangle whle clipping and so on. It returns the new target only if
the window is actually visible.
After you're finished rendering into that window, call leave_window() on the
RenderTarget that was returned from enter_window().
*/
class RenderTarget {
public:
	virtual RenderTarget* enter_window(int x, int y, int w, int h) = 0;
	virtual void leave_window() = 0;

	virtual int get_w() const = 0;
	virtual int get_h() const = 0;
	
	virtual void draw_rect(int x, int y, int w, int h, RGBColor clr) = 0;
   virtual void fill_rect(int x, int y, int w, int h, RGBColor clr) = 0;
   virtual void brighten_rect(int x, int y, int w, int h, int factor) = 0;
	virtual void clear() = 0;

	virtual void blit(int dstx, int dsty, BlitSource* src) = 0;
	virtual void blitrect(int dstx, int dsty, BlitSourceRect* src, 
	                      int srcx, int srcy, int w, int h) = 0;
};

/*
class BlitSource

This abstract class represents any kind of picture that can be copied into a
RenderTarget.

TODO: Add a blit-offset feature here (makes sense for animations)?
*/
class Bitmap;

class BlitSource {
	friend class Bitmap;

public:
	virtual int get_w() const = 0;
	virtual int get_h() const = 0;
	
private:
	virtual void blit_to_bitmap16(Bitmap* dst, int dstx, int dsty) = 0;
};

/*
class BlitSourceRect

This is a more general source for blits into RenderTarget which allows a
source rectangle to be specified.
*/
class BlitSourceRect : public BlitSource {
	friend class Bitmap;

private:
	virtual void blit_to_bitmap16rect(Bitmap* dst, int dstx, int dsty, int srcx, int srcy, int w, int h) = 0;
};



/*
==============================================================================

class Graphic

==============================================================================
*/

/*
class Graphic

This interface represents the framebuffer / screen.
*/
class Graphic {
public:
	virtual ~Graphic() { }

	virtual int get_xres() = 0;
	virtual int get_yres() = 0;
	virtual RenderTarget* get_render_target() = 0;
	virtual void update_fullscreen() = 0;
	virtual void update_rectangle(int x, int y, int w, int h) = 0;
	virtual bool need_update() = 0;
	virtual void refresh() = 0;
	
	virtual void screenshot(const char* fname) = 0;
};

extern Graphic* g_gr;


// TODO: get rid of the following
class Bitmap;
class Pic;

void render_right_triangle(Bitmap*, Point_with_bright*, Point_with_bright*, Point_with_bright*, Pic*, int, int);
void render_bottom_triangle(Bitmap*, Point_with_bright*, Point_with_bright*, Point_with_bright*, Pic*, int, int);
void render_road_horiz(Bitmap *dst, Point start, Point end, ushort color);
void render_road_vert(Bitmap *dst, Point start, Point end, ushort color);

class Animation;

void copy_animation_pic(RenderTarget* dst, Animation* anim, uint time, int dst_x, int dst_y, const uchar *plrclrs);

#endif /* __S__GRAPHIC_H */
