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

#include "widelands.h"
#include "options.h"
#include "graphic.h"
#include "bob.h"

Graphic *g_graphic = 0;

/* apparently deprecated -- Nicolai
// wireframe or filled triangles?
#define SHADING_FLAT		1
#define SHADING_GOURAUD	2
#define SHADING				SHADING_GOURAUD
#define FILL_TRIANGLES
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// bei mir gibt es diese funktion auch im header 'Xutility'
	// den header gibts aber glaub ich beim linux nicht
	// egal, inline kostet nichts
// florian
/* this comes for free from the STL headers (if it doesn't in a non-glibc implementation,
   please add the necessary header(s) to widelands.h)

  it does NOT come from stl in my win32 (i wouldn't have added it, would i?)
  well i'll leave it here, as this part of code will hopefully disappear soon
  */
#ifdef WIN32
template <typename T> inline void swap(T& a, T& b)
{
   T temp = a;
   a = b;
   b = temp;
}
#endif

struct _go
{
   int x;
   int b; // 16.16 fixed point
};

int make_triangle_lines(Point* points, int* bright, _go* starts, _go* ends)
{
	int ydiff1 = points[1].y - points[0].y;
	int ydiff2 = points[2].y - points[0].y;
	int ydiff3 = points[2].y - points[1].y;
	int xdiff1 = points[1].x - points[0].x;
	int xdiff2 = points[2].x - points[0].x;
	int xdiff3 = points[2].x - points[1].x;
	int bdiff1 = bright[1] - bright[0];
	int bdiff2 = bright[2] - bright[0];
	int bdiff3 = bright[2] - bright[1];

	if (!ydiff2)
		// triangle has height 0
		return 0;

	// calculate x for line b at height of point B
	int midx = points[0].x + (xdiff2 * ydiff1) / ydiff2;
	// is B left from b?
	if (points[1].x < midx)
		// arrays have to be swapped
		swap<_go*>(starts, ends);

   int y;
   int xd1 = 0, xd2 = 0, xd3 = 0;
   int bd1 = 0, bd2 = 0, bd3 = 0;
   // upper part of triangle
   for (y=0; y<ydiff1; y++)
   {
      starts[y].x = points[0].x + xd2 / ydiff2;
      starts[y].b = bright[0] + bd2 / ydiff2;
      ends[y].x = points[0].x + xd1 / ydiff1;
      ends[y].b = bright[0] + bd1 / ydiff1;
      xd1 += xdiff1;
      xd2 += xdiff2;
      bd1 += bdiff1;
      bd2 += bdiff2;
   }
   // lower part
   for (y=ydiff1; y<ydiff2; y++)
   {
      starts[y].x = points[0].x + xd2 / ydiff2;
      starts[y].b = bright[0] + bd2 / ydiff2;
      ends[y].x = points[1].x + xd3 / ydiff3;
      ends[y].b = bright[1] + bd3 / ydiff3;
      xd2 += xdiff2;
      xd3 += xdiff3;
      bd2 += bdiff2;
      bd3 += bdiff3;
   }
   return ydiff2;
}

// render_triangle for gouraud shading
void render_triangle(Bitmap *dst, Point* points, int* bright, Pic* texture)
{
   if (points[0].y > points[1].y)
   {
      swap<Point>(points[0], points[1]);
      swap<int>(bright[0], bright[1]);
   }
   if (points[1].y > points[2].y)
   {
      swap<Point>(points[1], points[2]);
      swap<int>(bright[1], bright[2]);
   }
   if (points[0].y > points[1].y)
   {
      swap<Point>(points[0], points[1]);
      swap<int>(bright[0], bright[1]);
   }

   _go starts[200];		// FEAR!!
   _go ends[200];			// don't use to high triangles
   bright[0] <<= 16; // convert to 16.16 fixed point
   bright[1] <<= 16;
   bright[2] <<= 16;

   int ymax = make_triangle_lines(points, bright, starts, ends);
   int ystart = points[0].y < 0 ? -points[0].y : 0;
   ymax = ymax + points[0].y <= (int)dst->get_h() ? ymax : dst->get_h()-points[0].y;
   for (int y=ystart; y<ymax; y++)
   {
      if (starts[y].x >= (int)dst->get_w())
         continue;
      if (ends[y].x < 0)
         continue;

      int xdiff = ends[y].x - starts[y].x;
      if (!xdiff)
         xdiff = 1;
      int bdiff = ends[y].b - starts[y].b;
      int b = starts[y].b;
      int bd = bdiff / xdiff;

      int end = ends[y].x < (int)dst->get_w() ? ends[y].x : dst->get_w()-1;
      int start = starts[y].x;
      if (start < 0) {
         b -= bd * start;
         start = 0;
      }

      ushort *pix = dst->get_pixels() + (points[0].y + y)*dst->get_pitch() + start;
      ushort *texp = texture->get_pixels() + (y % texture->get_h())*texture->get_w();
      uint tp = start - starts[y].x;

      for(int cnt = end-start; cnt >= 0; cnt--)
      {
         //*pix++ = pack_rgb((b >> 16) + 128, (b >> 16) + 128, (b >> 16) + 128); // shading test
         *pix++ = bright_up_clr2(texp[tp], b >> 16);
         b += bd;
         tp++;

         // replacing tp %= texture->w with the following conditional
         // makes this _entire function_ two times faster on my Athlon
         if (tp == texture->get_w())
            tp = 0;
      }
   }
}

/*
===============
render_road_horiz
render_road_vert

Render a road. This is really dumb right now, not using a texture
===============
*/
void render_road_horiz(Bitmap *dst, Point start, Point end, ushort color)
{
	int ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int centery = start.y << 16;

	for(int x = start.x; x < end.x; x++, centery += ydiff) {
		if (x < 0 || x >= dst->get_w())
			continue;
		
		int y = (centery >> 16) - 2;
		
		for(int i = 0; i < 5; i++, y++) {
			if (y < 0 || y >= dst->get_h())
				continue;
			
	      ushort *pix = dst->get_pixels() + y*dst->get_pitch() + x;
			*pix = color;
		}
	}
}

void render_road_vert(Bitmap *dst, Point start, Point end, ushort color)
{
	int xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int centerx = start.x << 16;

	for(int y = start.y; y < end.y; y++, centerx += xdiff) {
		if (y < 0 || y >= dst->get_h())
			continue;
		
		int x = (centerx >> 16) - 2;
		
		for(int i = 0; i < 5; i++, x++) {
			if (x < 0 || x >= dst->get_w())
				continue;
			
			ushort *pix = dst->get_pixels() + y*dst->get_pitch() + x;
			*pix = color;
		}
	}
}


/** class Graphic
 *
 * This functions is responsible for displaying graphics and keeping them up to date
 *
 * It's little strange in it's interface, but it is optimzed for speed, not beauty
 */

/** Graphic::Graphic(void)
 *
 * Default Constructor. Simple Inits
 *
 * Args: none
 * Returns: nothing
 */
Graphic::Graphic(void)
{
	Section *s = g_options.get_safe_section("global");
	Mode mode;
   
	sc=NULL;
   st=STATE_NOT_INIT;
   nupr=0;
   bneeds_fs_update=false;
	
	if (s->get_bool("fullscreen", true))
		mode = Graphic::MODE_FS;
	else
		mode = Graphic::MODE_WIN;
	
	set_mode(640, 480, mode);
}

/** Graphic::~Graphic(void)
 *
 * simple cleanups.
 *
 * Args: none
 * Returns: nothing
 */
Graphic::~Graphic(void)
{
   if(sc) {
      SDL_FreeSurface(sc);
      sc=NULL;
   }
   screenbmp.pixels = 0;
   st = STATE_NOT_INIT;
}

/** void Graphic::set_mode(ushort x, ushort y, Mode m)
 *
 * This function sets a new graphics mode.
 *	if x==0 and y==0: ignore resolution, just set the mode (won't create a window)
 *
 * Args:	x	x resolution
 * 		y	y resolution
 * 		m	either windows or fullscreen
 * Returns: Nothing
 */
void Graphic::set_mode(ushort x, ushort y, Mode m)
{
   if (!x && !y) {
		x = screenbmp.w;
		y = screenbmp.h;
	}
   if (screenbmp.w==x && screenbmp.h==y && mode==m)
		return;
   if(sc)
      SDL_FreeSurface(sc);
   sc=0;

	st = STATE_NOT_INIT;

   if (m == MODE_FS) {
      sc = SDL_SetVideoMode(x, y, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
   } else {
      sc = SDL_SetVideoMode(x, y, 16, SDL_SWSURFACE);
   }
   if (!sc) {
      critical_error("Couldn't set video mode: %s", SDL_GetError());
		return;
	}
	
   mode=m;
   screenbmp.w = x;
   screenbmp.pitch = sc->pitch / (16/8);
   screenbmp.h = y;
   screenbmp.pixels = (ushort*) sc->pixels;

   st = STATE_OK;

	g_sys.set_max_mouse_coords(x, y);
	
   bneeds_fs_update=true;

   return;
}

/** void Graphic::register_update_rect(const ushort x, const ushort y, const ushort w, const ushort h);
 *
 * This registers a rect of the screen for update
 *
 * Args: 	x	upper left corner of rect
 * 			y  upper left corner of rect
 * 			w	width
 * 			h	height
 */
void Graphic::register_update_rect(const ushort x, const ushort y, const ushort w, const ushort h)
{
   if(nupr>=MAX_RECTS) {
      bneeds_fs_update=true;
      return;
   }

   upd_rects[nupr].x=x;
   upd_rects[nupr].y=y;
   upd_rects[nupr].w=w;
   upd_rects[nupr].h=h;

   ++nupr;

   bneeds_update=true;
}

/** void Graphic::screenshot(const char* f)
 *
 * This makes a screenshot of a the current screen
 *
 * Args: f 	Filename to use
 * Returns: Nothing
 */
void Graphic::screenshot(const char* f)
{
	// TODO: this is incorrect; it bypasses the files code
   SDL_SaveBMP(sc, f);
}

/** void Graphic::update(void)
 *
 *	This function updates the registered rects on the screen
 *
 * Args: none
 * Returns: Nothing
 */
void Graphic::update(void)
{
//   if(bneeds_fs_update) {
      SDL_UpdateRect(sc, 0, 0, get_xres(), get_yres());
//    } else {
//       /*								cerr << "##########################" << endl;
//                               cerr << nupr << endl;
//                               for(uint i=0; i<nupr; i++)
//                               cerr << upd_rects[i].x << ":" << upd_rects[i].y << ":" <<
//                               upd_rects[i].w << ":" << upd_rects[i].h << endl;
//                               cerr << "##########################" << endl;
//                               */
// 		SDL_UpdateRects(sc, nupr, upd_rects);
//    }
   nupr=0;
   bneeds_fs_update=false;
   bneeds_update=false;
}

/*
===============
copy_animation_pic
 
This function renders a frame of an Animation.
time is in milliseconds into the animation.
dst_x/dst_y are the coordinates of the animation's hotspot.
plrclrs is an array of R/G/B values for the 4 playercolors:
	plrclrs[0] = darkest_r;
	plrclrs[1] = darkest_g;
	plrclrs[3] = dark_r;
	plrclrs[11] = brightest_b;
===============
*/
void copy_animation_pic(Bitmap* dst, Animation* anim, uint time, int dst_x, int dst_y, const uchar *plrclrs)
{
   int x, y;
	Animation_Pic* pic = anim->get_time_pic(time);
	ushort cmd;
	ushort count;
	ushort i;
	ushort clr;
	ushort plrclrs_packed[4];

	// build packed player colors
	if (!plrclrs) {
		static uchar ownerless_playercolor[12] = {
			10,   79,  54,
			15,  110,  75,
			28,  208, 142,
			35,  255, 174
		};
		plrclrs = ownerless_playercolor;
	}
	
	for(i = 0; i < 4; i++)
		plrclrs_packed[i] = pack_rgb(plrclrs[i*3+0], plrclrs[i*3+1], plrclrs[i*3+2]);
	
	// get destination relative to top-left corner
	dst_x -= anim->get_hsx();
	dst_y -= anim->get_hsy();
	   
	i = 0;
   x = dst_x;
   for(y = dst_y; y<dst_y+anim->get_h(); ) {
//      for(x=0; x<bob->get_w(); x++) {
         cmd=((pic->data[i]>> 14));
         count=pic->data[i] & 0x3fff;

//         cerr << "ALIVE:" << hex << pic->data[i] << ":"  << cmd << ":" << count << endl;

         i++;
         if(cmd==0) {
            // Normal color pixels
            for(uint z=0; z<count; z++) {
               clr=pic->data[i+z];
   //            cerr << count << ":" << hex << clr << endl;
//               clr=pic->data[i];
               if(x>=0 && y>=0) {
                  if( (((uint)x)<dst->get_w()) && (((uint)y)<dst->get_h()) ) {
                     dst->pixels[y*dst->pitch + x]=clr; //pic->data[i];
                  }
               }
               ++x;
               if(x==dst_x+anim->get_w()) { 
                  ++y; 
                  x=dst_x; 
                  if(y==dst_y+anim->get_h()) break;
               }
            }
            i+=count;
            continue;
         } else if(cmd==1) {
            // Skip pixels
            while(count) {
               ++x;
                  
               if(x==dst_x+anim->get_w()) { ++y; x=dst_x; if(y==dst_y+anim->get_h()) break;}
               --count;
            }
            continue;
         } else if(cmd==2) {
				// Should draw player color pixels.
            clr = pic->data[i];
				assert(clr < 4);
				clr = plrclrs_packed[clr];
            
            while(count) {
             if(x>=0 && y>=0) {
                  if( (((uint)x)<dst->get_w()) && (((uint)y)<dst->get_h()) ) {
                     dst->pixels[y*dst->pitch + x]=clr; 
                  }
               }
               ++x;
               if(x==dst_x+anim->get_w()) { 
                  ++y; 
                  x=dst_x; 
                  if(y==dst_y+anim->get_h()) break;
               }     
               if(x==dst_x+anim->get_w()) { ++y; x=dst_x; if(y==dst_y+anim->get_h()) break;}
               --count;
            }
            ++i;
            continue;
         } else if(cmd==3) {
            // Shadow. Skip for the moment TODO!
            // Skip pixels
            while(count) {
               ++x;
                  
               if(x==dst_x+anim->get_w()) { ++y; x=dst_x; if(y==dst_y+anim->get_h()) break;}
               --count;
            }
             
            continue;
      
      }
   }
//   cerr << y << "Should write an animation_pic something" << endl;

//   assert(0);
}

/** void copy_pic(Bitmap +dst, Bitmap *src, const int dst_x, const int dst_y,
*                const uint src_x, const uint src_y, const uint w, const uint h)
 *
 * Copy an area of the source bitmap to the destination bitmap, using
 * source colorkey if necessary.
 *
 * Assumes a valid source rectangle!
 * Destination clipping is performed
 *
 * Args: dst	destination bitmap
*       src	source bitmap
*       dst_x	destination coordinates
*       dst_y
*       src_x	source coordinates
*       src_y
*       w		width
*       h		height
*/
void copy_pic(Bitmap *dst, Bitmap *src, int dst_x, int dst_y,
      uint src_x, uint src_y, int w, int h)
{
   if (dst_x < 0) {
      w += dst_x;
      src_x -= dst_x;
      dst_x = 0;
   }
   if (dst_x+w > (int)dst->w)
      w = dst->w - dst_x;
   if (w <= 0)
      return;

   if (dst_y < 0) {
      h += dst_y;
      src_y -= dst_y;
      dst_y = 0;
   }
   if (dst_y+h > (int)dst->h)
      h = dst->h - dst_y;
   if (h <= 0)
      return;

   if (src->has_clrkey())
   {
      // Slow blit, checking for clrkeys. This could probably speed up by copying
      // 2 pixels (==4bytes==register width)
      // in one rush. But this is a nontrivial task
      // This could also use MMX assembly on targets that support it...
      for (int y=0; y<h; y++)
      {
         int sp = (src_y++)*src->pitch + src_x;
         int dp = (dst_y++)*dst->pitch + dst_x;
         for (int x=0; x<w; x++)
         {
            ushort clr = src->pixels[sp++];
            if (clr != src->get_clrkey())
               dst->pixels[dp] = clr;
            dp++;
         }
      }
   }
   else
   {
      if (w == (int)dst->pitch && w == (int)src->pitch)
      {
         // copy entire rows, so it can be all done in a single memcpy
         uint soffs = src_y * src->w;
         uint doffs = dst_y * dst->w;
         memcpy(dst->pixels+doffs, src->pixels+soffs, (w*h) << 1);
      }
      else
      {
         // fast blitting, using one memcpy per row
         uint soffs = src_y * src->pitch + src_x;
         uint doffs = dst_y * dst->pitch + dst_x;
         int bw = w << 1; // w*sizeof(short)
         for (int y=0; y<h; y++)
         {
            memcpy (dst->pixels+doffs, src->pixels+soffs, bw);
            soffs += src->pitch;
            doffs += dst->pitch;
         }
      }
   }
}


