/*
 * Copyright (C) 2002 by Holger Rapp 
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

#include "mapview.h"
#include "graphic.h"

/* class Map_View 
 *
 * this implements a view of a card. it's used
 * to render a valid card on the screen
 *
 * Depends: class Map
 * 			g_gr
 */

/** Map_View::Map_View(Map* m) 
 *
 * Init
 *
 * Args: m 	map to use
 */
Map_View::Map_View(Map* m) {
		  vpx=vpy=0;
		  map=m;
		
		  // TEMP
		  tmpg.set_size(50, 50);
		  tmpg.set_clrkey(Graph::pack_rgb(51, 114, 44));
		  tmpg.clear_all();

		  tmpr.set_size(50, 50);
		  tmpr.set_clrkey(Graph::pack_rgb(255, 0, 0));
		  tmpr.clear_all();
		  // TEMP
}

/** Map_View::~Map_View(void) 
 *
 * Cleanups
 */
Map_View::~Map_View(void) {

}

/** void Map_View::draw(void) 
 *
 * This is the guts!! this function draws the whole
 * card the user can see. we spend a lot of time
 * in this function
 *
 * Args: None
 * Returns: Nothing
 */
void Map_View::draw(void) {
		  Field *f;
		  static bool xtrans;
		  static bool ytrans;
		  
		  f=map->get_ffield();
		  if( (f->get_rn()->get_xpix()-vpx >=0) && (f->get_bln()->get_xpix()-vpx < (int)g_gr.get_xres()) ) 
					 draw_field(f); 
		  for(int i=(map->get_w()*(map->get_h()-1)); --i; )  {
					 f=map->get_nfield();
					 // X-check
					 if(f->get_rn()->get_xpix()-vpx <0) continue;
					 if(f->get_bln()->get_xpix()-vpx >= (int)g_gr.get_xres()) continue;
					 draw_field(f); 
		  } 

		  if(!xtrans && (uint)vpx> map->get_w()*FIELD_WIDTH-g_gr.get_xres()) {
					 int ovpx=vpx;
					 vpx-=map->get_w()*FIELD_WIDTH;
					 xtrans=true;
					 draw();
					 xtrans=false;
					 vpx=ovpx; 
		  }

		  if(!ytrans && (uint)vpy> (((map->get_h()-1)*FIELD_HEIGHT)>>1)-g_gr.get_yres()) {
					 int ovpy=vpy;
					 vpy-=(((map->get_h()+1)*FIELD_HEIGHT)>>1);
					 ytrans=true;
					 draw();
					 ytrans=false;
					 vpy=ovpy; 
		  }
}
					 
void Map_View::draw_field(Field* f) {
		  draw_polygon(f->get_bln(), f, f->get_brn(), &tmpr);
		  draw_polygon(f, f->get_brn(), f->get_rn(), &tmpg);
}

#define MAX2(a, b) ( (a)>(b) ? (a):  (b)>(a) ? (b):(a) )
#define MAX3(a, b, c) (MAX2(MAX2(a,b),MAX2(b,c)))
#define MIN2(a, b) (a<b?a:b<a?b:a)
#define MIN3(a, b, c) (MIN2(MIN2(a,b),MIN2(b,c)))

void Map_View::draw_polygon(Field* l, Field* r, Field* m, Pic* p) {
		  int ystart, ystop;
		  long xstart, xstop;
		  int x_d, y_d;
		  int temp;
		  
		  ystart=MIN3(l->get_ypix()-vpy, r->get_ypix()-vpy, m->get_ypix()-vpy);
		  ystop=MAX3(l->get_ypix()-vpy, r->get_ypix()-vpy, m->get_ypix()-vpy);

		  // ycheck
		  if(ystop < 0) return; 
		  if(ystart >= (int)g_gr.get_yres()) return;
					 
		  get_starts(l,r, m, ystart, ystop);
		  ystop= ystop>= (int)g_gr.get_yres() ? (int)(g_gr.get_yres())-1 : ystop;
					 
		  for(y_d= ystart<0 ? 0 : ystart; y_d<ystop; y_d++) {
					 xstart=(long)g_starts[y_d-ystart].border1;
					 xstop=(long)g_stops[y_d-ystart].border1;

					 if(xstart>xstop) {
								temp=xstop;
								xstop=xstart;
								xstart=temp;
					 } 

					 xstart= xstart<0 ? 0 : xstart;
					 xstop= xstop>= (int)g_gr.get_xres() ? (int)(g_gr.get_xres())-1 : xstop;
								
					 g_gr.set_cpixel(xstart-1, y_d);
					 for(x_d=xstart; x_d<xstop; x_d++) {
								g_gr.set_npixel(p->get_fpixel());
					 }

		  }


}

#define MAXC2Y(a, b) ( (a->get_ypix()-vpy) > (b->get_ypix()-vpy) ? (a) :  (b->get_ypix()-vpy)>(a->get_ypix()-vpy) ? (b):(a) )
#define MINC2Y(a, b) ( (a->get_ypix()-vpy) < (b->get_ypix()-vpy) ? (a) :  (b->get_ypix()-vpy)<(a->get_ypix()-vpy) ? (b):(b) )
void Map_View::get_starts(const Field* l, const Field* r, const Field* m, int ystart, int ystop) {
		  if((l->get_ypix()-vpy==ystart && m->get_ypix()-vpy==ystop) ||
								(l->get_ypix()-vpy==ystop && m->get_ypix()-vpy==ystart) )  {
					 // m.y -> l.y == start;
					 // l->r && r->m == g_stops;
					 scanconv(MINC2Y(l, m), MAXC2Y(l, m), g_starts, ystart);
					 scanconv(MINC2Y(l, r), MAXC2Y(l, r), g_stops, ystart);
					 scanconv(MINC2Y(r, m), MAXC2Y(r, m), g_stops, ystart);
					 return;
		  } else if((r->get_ypix()-vpy==ystart && m->get_ypix()-vpy==ystop) ||
								(r->get_ypix()-vpy==ystop && m->get_ypix()-vpy==ystart) )  {
					 // m.y -> r.y == g_stops;
					 // l->r && m->l == g_starts;
					 scanconv(MINC2Y(r, l), MAXC2Y(r, l), g_starts, ystart);
					 scanconv(MINC2Y(l, m), MAXC2Y(l, m), g_starts, ystart);
					 scanconv(MINC2Y(r, m), MAXC2Y(r, m), g_stops, ystart);
					 return; 
		  } else if((r->get_ypix()-vpy==ystart && l->get_ypix()-vpy==ystop)) {
					 scanconv(MINC2Y(r, m), MAXC2Y(r, m), g_starts, ystart);
					 scanconv(MINC2Y(l, m), MAXC2Y(l, m), g_starts, ystart);
					 scanconv(MINC2Y(r, l), MAXC2Y(r, l), g_stops, ystart);
					 return;
		  } else if((l->get_ypix()-vpy==ystart && r->get_ypix()-vpy==ystop)) {
					 scanconv(MINC2Y(r, l), MAXC2Y(r, l), g_starts, ystart);
					 scanconv(MINC2Y(r, m), MAXC2Y(r, m), g_stops, ystart);
					 scanconv(MINC2Y(l, m), MAXC2Y(l, m), g_stops, ystart);
					 return;
		  }

		  // Can never ever get here!
		  assert(0);
}
					 

void Map_View::scanconv(const Field* r, const Field* l, __starts* start, int ystart) {
		  long slope=0, x;
		  long count=0;   
		  int ystop;

		  // check, if this saves cycles
		  // no, it doesn't
//		  if(r->get_ypix()-vpy <0 && l->get_ypix()-vpy <0) return;
//		  if(r->get_ypix()-vpy >= (int)g_gr.get_yres() && l->get_ypix()-vpy >= (int)g_gr.get_yres()) return;

		  slope=((l->get_xpix()-vpx)-(r->get_xpix()-vpx))<<16;

		  if((r->get_ypix()-vpy)-(l->get_ypix()-vpy)) {
					 slope/=(l->get_ypix()-vpy)-(r->get_ypix()-vpy);
		  } else { 
					 slope=0;
		  }
		  
		  x=(r->get_xpix()-vpx)<<16; 
		  count=r->get_ypix()-vpy;

		  if(count-ystart<0) { x+=slope*(ystart-count); count=ystart; }
		  ystop=l->get_ypix()-vpy < (int) g_gr.get_yres()  ? l->get_ypix()-vpy : g_gr.get_yres();

		  while(count < ystop) {
					 start[count-ystart].border1=x>>16;
					 x+=slope;
					 count++; 
		  }

}
					 

void Map_View::set_viewpoint(uint x,  uint y) { 
		  vpx=x; vpy=y; 
		  while(vpx>FIELD_WIDTH*map->get_w())    	  vpx-=(FIELD_WIDTH*map->get_w());
		  while(vpy>(FIELD_HEIGHT*map->get_h())>>1)  vpy-=(FIELD_HEIGHT*map->get_h())>>1;
		  while(vpx< 0)  vpx+=(FIELD_WIDTH*map->get_w());
		  while(vpy< 0)  vpy+=(FIELD_HEIGHT*map->get_h())>>1;
}
