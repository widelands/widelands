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
#include "map.h"

using std::swap;

Graphic *g_graphic = 0;

// stupid kludge: this function from system.cc must be imported to report
// resolution changes
// ideally, the Graphic setup code would go into system.cc, while the actual
// graphics code goes into different back-end code (e.g. OpenGL, software,
// and software+MMX)
void Sys_SetMaxMouseCoords(int x, int y);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 
// next are the two render triangle functions. they are very similar and were once only one function (render triangle)
// but I splitted it for speed and different texture maps. This makes the code longer and more 'stupid' (lots of repeatings), 
// but this is the function that is spent most of the time in, so speed is absolutly crucial!!
// points are ordered:
// first=left points
// second=right_bottom points
// third=left_bottom
// first is alway the toppest one
void render_bottom_triangle(Bitmap *dst, Point_with_bright* first, Point_with_bright* second, Point_with_bright* third, Pic* texture, int vpx, int vpy)
{
	int dstw = dst->get_w(); // cache
   int xd1_add=0, bd1_add=0, xd2_add=0, bd2_add=0, bd3_add=0, xd3_add=0;
   int xd1 = 0, xd2 = 0, xd3 = 0;
   int bd1 = 0, bd2 = 0, bd3 = 0;
   int temp;

   if (second->y > third->y)
   {
      swap<Point_with_bright*>(second, third);

      temp=(second->y-first->y);
      xd1_add=-(FIELD_WIDTH<<7)/temp;
      bd1_add=((second->b-first->b)<<8)/temp;

      temp=(third->y-first->y);
      xd2_add=+(FIELD_WIDTH<<7)/temp;
      bd2_add=((third->b-first->b)<<8)/temp;
      
      temp=(third->y-second->y);
      if(temp) {
         xd3_add=+(FIELD_WIDTH<<8)/temp;
         bd3_add=((third->b-second->b)<<8)/temp;
      }
   } else {
      temp=(second->y-first->y);
      xd1_add=+(FIELD_WIDTH<<7)/temp;
      bd1_add=((second->b-first->b)<<8)/temp;

      temp=(third->y-first->y);
      xd2_add=-(FIELD_WIDTH<<7)/temp;
      bd2_add=((third->b-first->b)<<8)/temp;

      temp=(third->y-second->y);
      if(temp) {
         xd3_add=-(FIELD_WIDTH<<8)/temp;
         bd3_add=((third->b-second->b)<<8)/temp;
      }
   }

   // upper part of triangle
   int xstart, bstart, bstop, xstop;
   long xdiff, bdiff, bd; // known to be 4 bytes
   int b, x;
   ushort* pix;
   int onscreen_y=(first->y)+1;
   while(onscreen_y<=(second->y)) {
      if(onscreen_y>=(int)dst->get_h()) {
         // we're already outside the screen
         return;
      } 
      if(onscreen_y<0) goto go_on1;

      xstart=first->x + (xd2>>8);
      xstop=first->x + (xd1>>8);

      bstart=(first->b<<8) + (bd2);
      bstop=(first->b<<8) + (bd1);

      if(xstart>xstop) {
         swap<int>(xstart,xstop);
         swap<int>(bstart,bstop);
      }
      // since our scanline algorythm is flawed, we add 1 to xstop, xdiff is therefore always >0. 
      ++xstop;

      // xdiff is always > 0
      xdiff=xstop-xstart;

      // bdiff is not always positiv
      bdiff = bstop-bstart; 

      b = bstart;
      bd = bdiff / xdiff;

      pix = dst->get_pixels() + (onscreen_y)*dst->get_pitch() + xstart;
      x=xstart;
      while(x!=xstop) {
         //     cerr << x << ":" << xstart << ":" << xstop << ":" << xdiff << ":" << add << endl;
         if(x<0) goto go_onx1;
         if(x>=dstw) goto go_onx1;
         *pix = bright_up_clr(*texture->get_pixels(), (b>>8));
go_onx1:
         ++pix;
         ++x; 
         b+=bd;
      } 

go_on1:
      xd1 += xd1_add;
      xd2 += xd2_add;
      bd1 += bd1_add;
      bd2 += bd2_add;
      ++onscreen_y;
   }

   // lower part
   while(onscreen_y<=(third->y)) {
      if(onscreen_y>=(int)dst->get_h()) {
         // we're already outside the screen
         return;
      } 
      if(onscreen_y<0) goto go_on2;

      xstart=first->x + (xd2>>8);
      xstop=second->x + (xd3>>8);

      bstart=(first->b<<8) + (bd2);
      bstop=(second->b<<8) + (bd3);

      if(xstart>xstop) {
         swap<int>(xstart,xstop);
         swap<int>(bstart,bstop);
      }

      // since our scanline algorythm is flawed, we add 1 to xstop. 
      ++xstop;

      // xdiff is always > 0
      xdiff=xstop-xstart;

      // bdiff is not always positiv
      bdiff = bstop-bstart; 
      b = bstart;
      bd = bdiff / xdiff ;

      pix = dst->get_pixels() + (onscreen_y)*dst->get_pitch() + xstart;
      x=xstart;
      while(x!=xstop) {
         //  cerr << x << ":" << xstart << ":" << xstop << ":" << xdiff << ":" << add << endl;
         if(x<0) goto go_onx2;
         if(x>=dstw) goto go_onx2;
         *pix = bright_up_clr(*texture->get_pixels(), b>>8);
go_onx2:
         ++pix;
         ++x; 
         b+=bd;
      } 

go_on2:
      xd2 += xd2_add;
      xd3 += xd3_add;
      bd2 += bd2_add;
      bd3 += bd3_add;
      ++onscreen_y;
   }
}

// render_right_triangle: render the right triangle
// The points given are
//  first==right point
//  second==left point
//  third==bottom_right_point
//  the third point is always the lowest one!
void render_right_triangle(Bitmap *dst, Point_with_bright* first, Point_with_bright* second, Point_with_bright* third, Pic* texture, int vpx, int vpy)
{
	int dstw = dst->get_w(); // cache
   int xd1_add=0, bd1_add=0, xd2_add=0, bd2_add=0, bd3_add=0, xd3_add=0;
   int xd1 = 0, xd2 = 0, xd3 = 0;
   int bd1 = 0, bd2 = 0, bd3 = 0;
   int temp;

   if (first->y > second->y)
   {
      swap<Point_with_bright*>(first, second);

      temp=(third->y - first->y);
      xd2_add= +(FIELD_WIDTH<<7)/temp;  // xdiff2_0 is always FIELD_WIDTH/2, and this shifted to the left 8 times
      bd2_add=((third->b - first->b)<<8)/temp;
      temp=(second->y - first->y);
      if(temp) {
         xd1_add=+(FIELD_WIDTH<<8)/temp;   // xdiff between first and second is always FIELD_WIDTH
         bd1_add=((second->b - first->b)<<8)/temp;
      } 
      temp=(third->y - second->y);
      xd3_add=-(FIELD_WIDTH<<7)/temp;
      bd3_add=((third->b - second->b)<<8)/temp;
   } else {
      temp=(third->y - first->y);
      xd2_add= -(FIELD_WIDTH<<7)/temp;  // xdiff2_0 is always FIELD_WIDTH/2, and this shifted to the left 8 times
      bd2_add= ((third->b - first->b)<<8)/temp;
      temp=(second->y - first->y);
      if(temp) {
         xd1_add=-(FIELD_WIDTH<<8)/temp;   // xdiff between first and second is always FIELD_WIDTH
         bd1_add=((second->b - first->b)<<8)/temp;
      } 
      temp=(third->y - second->y);
      xd3_add=+(FIELD_WIDTH<<7)/temp;
      bd3_add=((third->b - second->b)<<8)/temp;
   }

   // upper part of triangle
   int bstart, bstop, xstart, xstop;
   long xdiff, bdiff, bd; // known to be 4 bytes
   int b, x;
   ushort* pix;
   int onscreen_y=(first->y)+1;
   while(onscreen_y<=second->y) {
      if(onscreen_y>=(int)dst->get_h()) {
         // we're already outside the screen
         return;
      } 
      if(onscreen_y<0) goto go_on1;

      xstart=first->x + (xd2>>8);
      xstop=first->x + (xd1>>8);

      bstart=(first->b<<8) + (bd2);
      bstop=(first->b<<8) + (bd1);

      if(xstart>xstop) {
         swap<int>(xstart,xstop);
         swap<int>(bstart,bstop);
      }
      // since our scanline algorythm is flawed, we add 1 to xstop, xdiff is therefore always >0. 
      ++(xstop);

      // xdiff is always > 0
      xdiff=xstop-xstart;

      // bdiff is not always positiv
      bdiff = bstop-bstart; 

      b = bstart;
      bd = bdiff / xdiff;

      pix = dst->get_pixels() + (onscreen_y)*dst->get_pitch() + xstart;
      x=xstart;
      while(x!=xstop) {
         //     cerr << x << ":" << xstart << ":" << xstop << ":" << xdiff << ":" << add << endl;
         if(x<0) goto go_onx1;
         if(x>=dstw) goto go_onx1;
         *pix = bright_up_clr(*texture->get_pixels(), (b>>8));
go_onx1:
         ++pix;
         ++x; 
         b+=bd;
      } 

go_on1:
      xd1 += xd1_add;
      xd2 += xd2_add;
      bd1 += bd1_add;
      bd2 += bd2_add;
      ++onscreen_y;
   }

   // lower part
   while(onscreen_y<=third->y) {
      if(onscreen_y>=(int)dst->get_h()) {
         // we're already outside the screen
         return;
      } 
      if(onscreen_y<0) goto go_on2;

      xstart=first->x + (xd2>>8);
      xstop=second->x + (xd3>>8);

      bstart=(first->b<<8) + (bd2);
      bstop=(second->b<<8) + (bd3);

      if(xstart>xstop) {
         swap<int>(xstart,xstop);
         swap<int>(bstart,bstop);
      }

      // since our scanline algorythm is flawed, we add 1 to xstop. 
      ++xstop;

      // xdiff is always > 0
      xdiff=xstop-xstart;

      // bdiff is not always positiv
      bdiff = bstop-bstart; 
      b = bstart;
      bd = bdiff / xdiff ;

      pix = dst->get_pixels() + (onscreen_y)*dst->get_pitch() + xstart;
      x=xstart;
      while(x!=xstop) {
         //  cerr << x << ":" << xstart << ":" << xstop << ":" << xdiff << ":" << add << endl;
         if(x<0) goto go_onx2;
         if(x>=dstw) goto go_onx2;
         *pix = bright_up_clr(*texture->get_pixels(), b>>8);
go_onx2:
         ++pix;
         ++x; 
         b+=bd;
      } 

go_on2:
      xd2 += xd2_add;
      xd3 += xd3_add;
      bd2 += bd2_add;
      bd3 += bd3_add;
      ++onscreen_y;
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
	int dstw = dst->get_w();
	int dsth = dst->get_h();

	int ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int centery = start.y << 16;

	for(int x = start.x; x < end.x; x++, centery += ydiff) {
		if (x < 0 || x >= dstw)
			continue;
		
		int y = (centery >> 16) - 2;
		
		for(int i = 0; i < 5; i++, y++) {
			if (y < 0 || y >= dsth)
				continue;
			
	      ushort *pix = dst->get_pixels() + y*dst->get_pitch() + x;
			*pix = color;
		}
	}
}

void render_road_vert(Bitmap *dst, Point start, Point end, ushort color)
{
	int dstw = dst->get_w();
	int dsth = dst->get_h();

	int xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int centerx = start.x << 16;

	for(int y = start.y; y < end.y; y++, centerx += xdiff) {
		if (y < 0 || y >= dsth)
			continue;
		
		int x = (centerx >> 16) - 2;
		
		for(int i = 0; i < 5; i++, x++) {
			if (x < 0 || x >= dstw)
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
	Section *s = g_options.pull_section("global");
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
   screenbmp.m_pixels = 0;
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
		x = screenbmp.m_w;
		y = screenbmp.m_h;
	}
   if (screenbmp.m_w==x && screenbmp.m_h==y && mode==m)
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
   screenbmp.m_w = x;
   screenbmp.m_pitch = sc->pitch / (16/8);
   screenbmp.m_h = y;
   screenbmp.m_pixels = (ushort*) sc->pixels;

   st = STATE_OK;

	Sys_SetMaxMouseCoords(x, y);
	
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
void copy_animation_pic(RenderTarget* tgt, Animation* anim, uint time, int dst_x, int dst_y, const uchar *plrclrs)
{
	Bitmap* dst = static_cast<Bitmap*>(tgt); // HACKHACKHACK

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
                  if( (x<dst->get_w()) && (y<dst->get_h()) ) {
                     dst->m_pixels[y*dst->m_pitch + x]=clr; //pic->data[i];
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
                  if( (x<dst->get_w()) && (y<dst->get_h()) ) {
                     dst->m_pixels[y*dst->m_pitch + x]=clr; 
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

