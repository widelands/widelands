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
#include "graphic.h"

// wireframe or filled triangles?
#define SHADING_FLAT		1
#define SHADING_GOURAUD	2
#define SHADING				SHADING_GOURAUD
#define FILL_TRIANGLES

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// bei mir gibt es diese funktion auch im header 'Xutility'
	// den header gibts aber glaub ich beim linux nicht
	// egal, inline kostet nichts
// florian
template <typename T> inline void swap(T& a, T& b)
{
   T temp = a;
   a = b;
   b = temp;
}


#define V3	(float)0.57735

#if SHADING == SHADING_FLAT || !defined(FILL_TRIANGLES)
/* int make_triangle_lines(Vector* points, int* starts, int* ends)
 * fills arrays with horizontal start- and end-points of a triangle
 * returns number of lines written to the arrays
 *
 * points are expected to be sorted top-down
 * in comments, points 0 1 2 will be 'A' 'B' 'C', the opposite edge for
 * point 'A' will be 'a' etc as common in math, but: the points ABC are
 * not always labeled in counterclockwise order (as it would be in math),
 * they are labeled top-down.
 */
inline int make_triangle_lines(Point* points, int* starts, int* ends)
{
   int ydiff1 = points[1].y - points[0].y;
   int ydiff2 = points[2].y - points[0].y;
   int ydiff3 = points[2].y - points[1].y;
   int xdiff1 = points[1].x - points[0].x;
   int xdiff2 = points[2].x - points[0].x;
   int xdiff3 = points[2].x - points[1].x;

   if (!ydiff2)
      // triangle has height 0
      return 0;

   // calculate x for line b at height of point B
   int midx = points[0].x + (xdiff2 * ydiff1) / ydiff2;
   // is B left from b?
   if (points[1].x < midx)
      // arrays have to be swapped
      swap<int*>(starts, ends);

   int y;
   // upper part of triangle
   for (y=0; y<ydiff1; y++)
   {
      // calculate x for line b at height y
      starts[y] = points[0].x + (xdiff2 * y) / ydiff2;
      // calculate x for line c at height y
      ends[y] = points[0].x + (xdiff1 * y) / ydiff1;
   }
   // lower part
   for (y=0; y<ydiff3; y++)
   {
      // calculate x for line b at height y
      starts[ydiff1 + y] = points[0].x + (xdiff2 * (y + ydiff1)) / ydiff2;
      // calculate x for line a at height y
      ends[ydiff1 + y] = points[1].x + (xdiff3 * y) / ydiff3;
   }
   return ydiff2;
}

// render_triangle for flat shading
void render_triangle(Bitmap *dst, Point* points, Vector* normals, Pic* texture)
{
   static Vector sun = Vector(V3, -V3, -V3);	// |sun| = 1

   //don't need to swap normals here, they are summed up anyway
   if (points[0].y > points[1].y)
      swap<Point>(points[0], points[1]);
   if (points[1].y > points[2].y)
      swap<Point>(points[1], points[2]);
   if (points[0].y > points[1].y)
      swap<Point>(points[0], points[1]);

   Vector normal = normals[0] + normals[1] + normals[2];
   normal.normalize();
   float b = normal * sun;
   int lfactor = (int)(b * LIGHT_FACTOR);

   int starts[200];		// FEAR!!
   int ends[200];			// don't use to high triangles

   int ymax = make_triangle_lines(points, starts, ends);
   int ystart = points[0].y < 0 ? -points[0].y : 0;
   ymax = ymax + points[0].y <= yres ? ymax : yres-points[0].y;
   for (int y=ystart; y<ymax; y++)
   {
      if (starts[y] >= xres)
         continue;
      if (ends[y] < 0)
         continue;

      int start = starts[y] < 0 ? 0 : starts[y];
      int end = ends[y] < xres ? ends[y] : xres-1;
#ifdef FILL_TRIANGLES
      int p = (points[0].y + y) * xres + start;
      for (int x=start; x<=end; x++)
      {
         //				pixels[(points[0].y + y)*xres + x] = pack_rgb(clr, clr, clr);
         uint tp = (y % texture->h)*texture->w + (x-starts[y])%texture->w;
         //				uint tp = ((points[0].y + y) % texture->h)*texture->w + x%texture->w;
         //				pixels[p++] = bright_up_clr2(texture->pixels[tp], -lfactor);
         pixels[p++] = bright_up_clr2(texture->pixels[tp], -lfactor);
      }
#else
      if (y == 0 || y == ymax-1)
         for (int x=start; x<=end; x++)
         {
            uint p = (y % texture->h)*texture->w + (x-starts[y])%texture->w;
            pixels[(points[0].y + y)*xres + x] = texture->pixels[p];
         }
      else
      {
         uint p = (y % texture->h)*texture->w + (start-starts[y])%texture->w;
         pixels[(points[0].y + y)*xres + start] = texture->pixels[p];
         p = (y % texture->h)*texture->w + (end-starts[y])%texture->w;
         pixels[(points[0].y + y)*xres + end] = texture->pixels[p];
      }
#endif

   }
}
#else
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
   int xd1=0, xd2=0, xd3=0;
   int bd1=0, bd2=0, bd3=0;
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
   for (y=0; y<ydiff3; y++)
   {
      starts[ydiff1 + y].x = points[0].x + xd2 / ydiff2;
      starts[ydiff1 + y].b = bright[0] + bd2 / ydiff2;
      ends[ydiff1 + y].x = points[1].x + xd3 / ydiff3;
      ends[ydiff1 + y].b = bright[1] + bd3 / ydiff3;
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
#endif

/** class Graphic
 *
 * This functions is responsible for displaying graphics and keeping them up to date
 *
 * It's little strange in it's interface, but it is optimzed for speed, not beauty
 * This is a singleton
 */

/** Graphic::Graphic(void)
 *
 * Default Constructor. Simple Inits
 *
 * Args: none
 * Returns: nothing
 */
Graphic::Graphic(void) {
   sc=NULL;
   st=STATE_NOT_INIT;
   nupr=0;
   bneeds_fs_update=false;

   SDL_Init(SDL_INIT_VIDEO);
}

/** Graphic::~Graphic(void)
 *
 * simple cleanups.
 *
 * Args: none
 * Returns: nothing
 */
Graphic::~Graphic(void) {
   if(sc) {
      SDL_FreeSurface(sc);
      sc=NULL;
   }
   screenbmp.pixels = 0;
   st=STATE_NOT_INIT;

   SDL_Quit();
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
void Graphic::set_mode(ushort x, ushort y, Mode m) {
   if(!x && !y) { mode=m; return; }
   if(screenbmp.w==x && screenbmp.h==y && mode==m) return;
   if(sc)
      SDL_FreeSurface(sc);
   sc=0;

   if(m==MODE_FS) {
      sc = SDL_SetVideoMode(x, y, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
   } else {
      sc = SDL_SetVideoMode(x, y, 16, SDL_SWSURFACE);
   }
   if (!sc) {
      char buf[256];
	  sprintf(buf, "Couldn't set video mode: %s", SDL_GetError());
      tell_user(buf);
	  exit(0);
   }
   mode=m;
   screenbmp.w=x;
   screenbmp.pitch=x;
   screenbmp.h=y;
   screenbmp.pixels=(ushort*) sc->pixels;

   st=STATE_OK;

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
void Graphic::register_update_rect(const ushort x, const ushort y, const ushort w, const ushort h) {
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
void Graphic::screenshot(const char* f) {
   SDL_SaveBMP(sc, f);
}

/** void Graphic::update(void)
 *
 *	This function updates the registered rects on the screen
 *
 * Args: none
 * Returns: Nothing
 */
void Graphic::update(void) {
   if(bneeds_fs_update) {
      SDL_UpdateRect(sc, 0, 0, get_xres(), get_yres());
   } else {
      /*								cerr << "##########################" << endl;
                              cerr << nupr << endl;
                              for(uint i=0; i<nupr; i++)
                              cerr << upd_rects[i].x << ":" << upd_rects[i].y << ":" <<
                              upd_rects[i].w << ":" << upd_rects[i].h << endl;
                              cerr << "##########################" << endl;
                              */								SDL_UpdateRects(sc, nupr, upd_rects);
   }
   nupr=0;
   bneeds_fs_update=false;
   bneeds_update=false;
}

/** 
 * This function copys from a bob picture
 *
 * TODO: it needs to much information (players etc. etc.)
 */
#include "instances.h"
#include "bob.h"

void copy_animation_pic(Bitmap* dst, Instance* inst, int dst_x, int dst_y, uint src_x, uint src_y, int w, int h) {

   int x, y;
   Animation_Pic* pic=inst->get_cur_pic();
   Animation* bob=pic->parent;
   ushort cmd;
   ushort count;
   ushort i=0;
   ushort clr;

   // TODO: proper hotspot implementation
   dst_x-=(int) ((bob->get_w()>>1)-inst->get_rel_pos_x());
   dst_y-=(int) ((bob->get_h()>>1)-inst->get_rel_pos_y());
   
   x=dst_x;
   for(y=dst_y; y<dst_y+bob->get_h(); ) {
//      for(x=0; x<bob->get_w(); x++) {
         cmd=((pic->data[i]>> 14) & 0x3);
         count=pic->data[i] & 0x3fff;

//         cerr << "ALIVE:" << hex << pic->data[i] << ":"  << cmd << ":" << count << endl;

         i++;
         if(cmd==0) {
//            cerr << "Should draw " << count << " pixels"; 
            //   cerr << count << ":" <<  x << ":" << y << endl;
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
               if(x==dst_x+bob->get_w()) { 
                  ++y; 
                  x=dst_x; 
                  if(y==dst_y+bob->get_h()) break;
               }
            }
            i+=count;
            continue;
         } else if(cmd==1) {
//            cerr << "Should skip " << count << " pixels!" << endl;
            while(count) {
               ++x;
                  
               if(x==dst_x+bob->get_w()) { ++y; x=dst_x; if(y==dst_y+bob->get_h()) break;}
               --count;
            }
            continue;
         } else if(cmd==2) {
//            cerr << "Should draw " << count << " pixels in player color!" << endl;
            // Player Color. Skip for the moment: TODO!
           /* while(count) {
               ++x;
               if(x==bob->get_w()) { ++y; x=0; }
               --count;
            }
           */ i++;
            continue;
         } else if(cmd==3) {
            // Shadow. Skip for the moment TODO!
//            cerr << "Should draw " << count << " pixels as shadow!" << endl;
           /* while(count) {
               ++x;
               if(x==bob->get_w()) { ++y; x=0; }
               --count;
            }
           */ continue;
  //        }
      
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


