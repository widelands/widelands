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
// note that this is not used by the new renderer anymore
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
	
   Point() { }
   Point(int px, int py) : x(px), y(py) { }
};

struct Rect : public Point {
	int w;
	int h;
	
	Rect() { }
	Rect(int px, int py, int pw, int ph) : Point(px, py), w(pw), h(ph) { }
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

/** struct Vertex
 *
 * This replaces Point_with_bright for use with the new texture mapping renderer.
 *
 * This struct is like a point, but with an additional bright factor and texture coordinates.
 */
struct Vertex:public Point {
	int b,tx,ty;
	Vertex (): Point (0,0) { b=tx=ty=0; }
	Vertex (int vx,int vy,int vb,int vtx,int vty): Point (vx,vy)
	{ b=vb; tx=vtx; ty=vty; }
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

MapRenderInfo

==============================================================================
*/

/*
struct MapRenderInfo

This structure contains all the information that is needed by the renderer.

It includes the map itself as well as overlay data (build symbols, road 
building symbols, ...)
*/
class Editor_Game_Base;
class Map;

enum {
	Overlay_Frontier_Base = 0,	// add the player number to mark a border field
	Overlay_Frontier_Max = 15,
	
	Overlay_Build_Flag = 16,
	Overlay_Build_Small,
	Overlay_Build_Medium,
	Overlay_Build_Big,
	Overlay_Build_Mine,
	
	Overlay_Build_Min = Overlay_Build_Flag,
	Overlay_Build_Max = Overlay_Build_Mine,
};

class Game;

struct MapRenderInfo {
	Editor_Game_Base*    egbase;
   Map*		map;
	Coords	fieldsel; // field selection marker, moved by cursor
	uchar*	overlay_basic; // borders and build help
	uchar*	overlay_roads; // needs to be ORed with logical road info
	bool		show_buildhelp;

	std::vector<bool>*		visibility; // array of fields, true if the field can be seen
	                     // can be 0, in which case the whole map is visible
};


/*
==============================================================================

Graphics object interfaces

==============================================================================
*/

/*
class RenderTarget

This abstract class represents anything that can be rendered to.

It supports windows, which are composed of a clip rectangle and a drawing
offset:
The drawing offset will be added to all coordinates that are passed to drawing
routines. Therefore, the offset is usually negative. Then the coordinates are 
interpreted as relative to the clip rectangle and the primitives are clipped
accordingly.
enter_window() can be used to enter a sub-window of the current window. When 
you're finished drawing in the window, restore the old window by calling
set_window() with the values stored in previous and prevofs.
Note: If the sub-window would be empty/invisible, enter_window() returns false
and doesn't change the window state at all.
*/
class RenderTarget {
public:
	virtual void get_window(Rect* rc, Point* ofs) const = 0;
	virtual void set_window(const Rect& rc, const Point& ofs) = 0;
	virtual bool enter_window(const Rect& rc, Rect* previous, Point* prevofs) = 0;

	virtual int get_w() const = 0;
	virtual int get_h() const = 0;
	
	virtual void draw_rect(int x, int y, int w, int h, RGBColor clr) = 0;
   virtual void fill_rect(int x, int y, int w, int h, RGBColor clr) = 0;
   virtual void brighten_rect(int x, int y, int w, int h, int factor) = 0;
	virtual void clear() = 0;

	virtual void blit(int dstx, int dsty, uint picture) = 0;
	virtual void blitrect(int dstx, int dsty, uint picture,
	                      int srcx, int srcy, int w, int h) = 0;

	virtual void rendermap(const MapRenderInfo* mri, Point viewofs) = 0;
	virtual void renderminimap(Point pt, const MapRenderInfo* mri) = 0;
	
	virtual void drawanim(int dstx, int dsty, uint animation, uint time, const RGBColor* plrclrs) = 0;
};


/*
==============================================================================

class Graphic

==============================================================================
*/

enum { // picture module flags
	PicMod_UI = 1,
	PicMod_Menu = 2,
	PicMod_Game = 4,
};

/*
class Graphic

This interface represents the framebuffer / screen.

Picture IDs can be allocated using get_picture() and used in RenderTarget::blit().
Pictures are only loaded from disk once and thrown out of memory when the 
graphics system is unloaded, or when flush() is called with the appropriate
module flag.
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
	
	virtual void flush(int mod) = 0;
	virtual uint get_picture(int mod, const char* fname) = 0;
	virtual uint get_picture(int mod, const char* fname, RGBColor clrkey) = 0;
	virtual void get_picture_size(uint pic, int* pw, int* ph) = 0;
	virtual uint create_surface(int w, int h) = 0;
	virtual uint create_surface(int w, int h, RGBColor clrkey) = 0;
	virtual void free_surface(uint pic) = 0;
	virtual RenderTarget* get_surface_renderer(uint pic) = 0;
	
	virtual uint get_maptexture(const char* fnametempl, uint frametime) = 0;
	virtual void animate_maptextures(uint time) = 0;
	
	virtual void load_animations() = 0;

	virtual void screenshot(const char* fname) = 0;
	
	// HACK: needed to load the old font
	virtual uint get_picture(int mod, int w, int h, const ushort* data, RGBColor clrkey) = 0;
};

extern Graphic* g_gr;


#endif /* __S__GRAPHIC_H */
