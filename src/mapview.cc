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
		  draw_polygon(f->get_bln(), f, f->get_brn(), f->get_texd() );
		  draw_polygon(f, f->get_brn(), f->get_rn(), f->get_texr());
}

#define MAX2(a, b) ( (a)>(b) ? (a):  (b)>(a) ? (b):(a) )
#define MAX3(a, b, c) (MAX2(MAX2(a,b),MAX2(b,c)))
#define MIN2(a, b) (a<b?a:b<a?b:a)
#define MIN3(a, b, c) (MIN2(MIN2(a,b),MIN2(b,c)))
#include <iostream>
void Map_View::draw_polygon(Field* l, Field* r, Field* m, Pic* p) {
		  int ystart, ystop;
		  long xstart, xstop;
		  long xstart_h, xstop_h;
		  int x_d, y_d;
		  int temp;
		  long mh, h;

		  ystart=MIN3(l->get_ypix()-vpy, r->get_ypix()-vpy, m->get_ypix()-vpy);
		  ystop=MAX3(l->get_ypix()-vpy, r->get_ypix()-vpy, m->get_ypix()-vpy);

		  // ycheck
		  if(ystop < 0) return; 
		  if(ystart >= (int)g_gr.get_yres()) return;
		
		  get_starts(l,r, m, ystart, ystop);
		  ystop= ystop>= (int)g_gr.get_yres() ? (int)(g_gr.get_yres())-1 : ystop;

		  
		  p->get_fpixel();			 
		  for(y_d= ystart<0 ? 0 : ystart; y_d<ystop; y_d++) {
					 xstart=(long)g_starts[y_d-ystart].edge;
					 xstop=(long)g_stops[y_d-ystart].edge;
					 xstart_h=(long)g_starts[y_d-ystart].h;
					 xstop_h=(long)g_stops[y_d-ystart].h;

					 if(xstart>xstop) {
								temp=xstop;
								xstop=xstart;
								xstart=temp;
					 
								temp=xstop_h;
								xstop_h=xstart_h;
								xstart_h=temp;

					 } 

					 if(ystop-ystart) {
								mh=(xstop_h-xstart_h)<<16;
								mh/=(ystop-ystart);
					 }
					 h=xstart_h<<16;

					 if(xstart<0) {
								xstart= 0 ;
					 }

					 if(xstop>= (int)g_gr.get_xres()) {
								xstop= (int)(g_gr.get_xres())-1;
					 }
								
					 g_gr.set_cpixel(xstart-1, y_d);
					 p->set_cpixel((xstart+vpx) % (p->get_w()),(y_d+vpy) % (p->get_h()-2));
								//p->set_cpixel(xstart-vpx % (p->get_w()>>1), ystart-vpy % (p->get_h()>>1));
					 for(x_d=xstart; x_d<xstop; x_d++) {
								g_gr.set_npixel(Graph::bright_up_clr(p->get_npixel(), h>>(16-HEIGHT_CLR_FACTOR)));
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
		  long hslope=0, h;
		  int ystop;

		  // check, if this saves cycles
		  // no, it doesn't
		  //		  if(r->get_ypix()-vpy <0 && l->get_ypix()-vpy <0) return;
		  //		  if(r->get_ypix()-vpy >= (int)g_gr.get_yres() && l->get_ypix()-vpy >= (int)g_gr.get_yres()) return;

		  slope=(l->get_xpix()-r->get_xpix())<<16;
		  hslope=(l->get_height()-r->get_height())<<16;

		  if(r->get_ypix()-l->get_ypix()) {
					 slope/=(l->get_ypix()-r->get_ypix());
					 hslope/=(l->get_ypix()-r->get_ypix());
		  } else { 
					 slope=0;
					 hslope=0;
		  }

		  x=(r->get_xpix()-vpx)<<16; 
		  h=r->get_height()<<16;
		  
		  count=r->get_ypix()-vpy;

		  if(count-ystart<0) { x+=slope*(ystart-count); count=ystart; }
		  ystop=l->get_ypix()-vpy < (int) g_gr.get_yres()  ? l->get_ypix()-vpy : g_gr.get_yres();

		  while(count < ystop) {
					 start[count-ystart].edge=x>>16;
					 start[count-ystart].h=h>>16;
					 x+=slope;
					 h+=hslope;
					 count++; 
		  }

/*
		  const Field* left = MIN2(r->get_xpix(), l->get_xpix()) == l->get_xpix() ? l : r;
		  const Field* right = MAX2(r->get_xpix(), l->get_xpix()) == l->get_xpix() ? l : r;

		  assert(left->get_xpix() < right->get_xpix());

		  float d[4] = {
					 left->get_ln()->get_ypix()-vpy,
					 left->get_ypix()-vpy,
					 right->get_ypix()-vpy, 
					 right->get_rn()->get_ypix()-vpy
		  };
		  cerr << d[0] << ":" << d[1] << ":" << d[2] << ":" << d[3] << endl;

		  float x[4] = {
					 left->get_xpix()-FIELD_WIDTH-vpx,
					 left->get_xpix()-vpx,
					 left->get_xpix()+FIELD_WIDTH-vpx,
					 left->get_xpix()+FIELD_WIDTH+FIELD_WIDTH-vpx,
					 
		  };
		  
		  float a[4]; 
		  float t2;
		  float sub[4-1];
		  float diag[4-1];
		  float sup[4-1];
		  uint i;
		  float oldy, oldt;
		  float y;
		  float t; 
		  
		  for (i=1; i<=4-2; i++){
					 diag[i] = (FIELD_WIDTH*2)/3;
					 sup[i] = FIELD_WIDTH/6;
					 sub[i] = FIELD_WIDTH/6;
					 a[i] = (d[i+1]-d[i])/FIELD_WIDTH-(d[i]-d[i-1])/FIELD_WIDTH;
		  }
		  * solve linear system with tridiagonal n by n matrix a
			* using Gaussian elimination *without* pivoting
			* where   a(i,i-1) = sub[i]  for 2<=i<=n
			* a(i,i)   = diag[i] for 1<=i<=n
			* a(i,i+1) = sup[i]  for 1<=i<=n-1
			* (the values sub[1], sup[n] are ignored)
			* right hand side vector b[1:n] is overwritten with solution 
			* NOTE: 1...n is used in all arrays, 0 is unused 
			*

		  //    factorization and forward substitution 
		  for(i=2; i<=4-2; i++){
					 sub[i] = sub[i]/diag[i-1];
					 diag[i] = diag[i] - sub[i]*sup[i-1];
					 a[i] = a[i] - sub[i]*a[i-1];
		  }
		  a[4-2] = a[4-2]/diag[4-2];
		  for(i=(4-2)-1;i>=1;i--){
					 a[i] = (a[i] - sup[i]*a[i+1])/diag[i];
		  }

		  //		  solveTridiag(sub,diag,sup,a,np-2);

		  // note that a[0]=a[np-1]=0
		  // draw
		  oldt=x[0];
		  oldy=d[0];
		//  draw_line((int)oldt,(int)oldy,(int)oldt,(int)oldy, Graph::pack_rgb(0,255,0));
		  for (uint i=1; i<=4-1; i++) {   // loop over intervals between nodes
					 for (uint j=1; j<=FIELD_WIDTH; j++){
								t2 = FIELD_WIDTH - j;
								y = ((-a[i-1]/6*(t2+FIELD_WIDTH)*j+d[i-1])*t2 +
													 (-a[i]/6*(j+FIELD_WIDTH)*t2+d[i])*j)/FIELD_WIDTH;
								t=x[i-1]+j;
		//						draw_line((int)oldt,(int)oldy,(int)t,(int)y, Graph::pack_rgb(0,255,0));
								
								if((t>(left->get_xpix()-vpx)) && y>0 && (t<(right->get_xpix()-vpx)) && y<g_gr.get_yres())
										  g_gr.set_pixel(t-j, d[i-1], Graph::pack_rgb(0, 0, 255));
								oldt=t;
								oldy=y;
					 }
		  }



*/
}


void Map_View::set_viewpoint(uint x,  uint y) { 
		  vpx=x; vpy=y; 
		  while(vpx>FIELD_WIDTH*map->get_w())    	  vpx-=(FIELD_WIDTH*map->get_w());
		  while(vpy>(FIELD_HEIGHT*map->get_h())>>1)  vpy-=(FIELD_HEIGHT*map->get_h())>>1;
		  while(vpx< 0)  vpx+=(FIELD_WIDTH*map->get_w());
		  while(vpy< 0)  vpy+=(FIELD_HEIGHT*map->get_h())>>1;
}
