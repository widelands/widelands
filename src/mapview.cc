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
		  
		  for(int y=3; y<map->get_h()-1; y++) {
					 for(int x=3; x<map->get_w()-1; x++) {
								f=map->get_field(x,y);
								if(f->get_ypix()-vpy < (int)g_gr.get_yres() &&
									f->get_xpix()-vpx < (int)g_gr.get_yres()) 
										  draw_field(f); 
					 }
		  }

}
					 
void Map_View::draw_field(Field* f) {
		  draw_polygon(f->get_tln(), f->get_trn(), f, &tmpr);
		  draw_polygon(f->get_tln(), f, f->get_ln(), &tmpg);
}

#define MAX2(a, b) ( (a)>(b) ? (a):  (b)>(a) ? (b):(a) )
#define MAX3(a, b, c) (MAX2(MAX2(a,b),MAX2(b,c)))
#define MIN2(a, b) (a<b?a:b<a?b:a)
#define MIN3(a, b, c) (MIN2(MIN2(a,b),MIN2(b,c)))

#include <iostream>
void Map_View::draw_polygon(Field* l, Field* r, Field* m, Pic* p) {
		  int ystart, ystop;
		  long xstart, xstop;
		  int x_d, y_d;
		  int temp;
		  
		  ystart=MIN3(l->get_ypix()-vpy, r->get_ypix()-vpy, m->get_ypix()-vpy);
		  ystop=MAX3(l->get_ypix()-vpy, r->get_ypix()-vpy, m->get_ypix()-vpy);

		  get_starts(l,r, m, ystart, ystop);

		  for(y_d=ystart; y_d<ystop; y_d++) {
					 if(y_d <0) continue;
					 if(y_d>=(int)g_gr.get_yres()) continue;

					 xstart=(long)g_starts[y_d-ystart].border1;
					 xstop=(long)g_stops[y_d-ystart].border1;

					 if(xstart>xstop) {
								temp=xstop;
								xstop=xstart;
								xstart=temp;
					 }

					 xstart= xstart<0 ? 0 : xstart;
					 xstop= xstop>=(int)g_gr.get_xres() ? g_gr.get_xres()-1 : xstop;

					 for(x_d=xstart; x_d<xstop; x_d++) {
								g_gr.set_pixel(x_d, y_d, p->get_pixel(0, 0));
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

		  slope=((l->get_xpix()-vpx)-(r->get_xpix()-vpx))<<16;

		  if((r->get_ypix()-vpy)-(l->get_ypix()-vpy)) {
					 slope/=(l->get_ypix()-vpy)-(r->get_ypix()-vpy);
		  } else { 
					 slope=0;
		  }

		  x=(r->get_xpix()-vpx)<<16;  
		  for(count=r->get_ypix()-vpy; count <= l->get_ypix()-vpy; count++) {
					 // cerr << count << ":" << l->get_ypix()  << endl;
					 start[count-ystart].border1=x>>16;
					 x+=slope;
		  }

}

