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
void Map_View::draw(void)
{
/*	g_gr.set_pixel(0, 0, 0);
	for (int z=0; z<640*480; z++)
		g_gr.set_npixel(0);*/
	Field *f;
	static bool xtrans;
	static bool ytrans;

	f=map->get_ffield();
	if( (f->get_rn()->get_xpix()-vpx >=0) && (f->get_bln()->get_xpix()-vpx < (int)g_gr.get_xres()) ) 
		draw_field(f); 

	for(int i=(map->get_w()*(map->get_h()-1)); --i; )
	{
		f=map->get_nfield();
		// X-check
		if(f->get_rn()->get_xpix()-vpx <0) continue;
		if(f->get_bln()->get_xpix()-vpx >= (int)g_gr.get_xres()) continue;
		draw_field(f); 
	} 

	if(!xtrans && (uint)vpx> map->get_w()*FIELD_WIDTH-g_gr.get_xres())
	{
		int ovpx=vpx;
		vpx-=map->get_w()*FIELD_WIDTH;
		xtrans=true;
		draw();
		xtrans=false;
		vpx=ovpx; 
	}

	if(!ytrans && (uint)vpy> (((map->get_h()-1)*FIELD_HEIGHT)>>1)-g_gr.get_yres())
	{
		int ovpy=vpy;
		vpy-=(((map->get_h()+1)*FIELD_HEIGHT)>>1);
		ytrans=true;
		draw();
		ytrans=false;
		vpy=ovpy; 
	}
}
					 
void Map_View::draw_field(Field* f)
{
	// for plain terrain, this param order will avoid swapping in
	// Graphic::render_triangle
	draw_polygon(f, f->get_bln(), f->get_brn(), f->get_texd());
	draw_polygon(f, f->get_rn(), f->get_brn(), f->get_texr());
}

inline void Map_View::draw_polygon(Field* l, Field* r, Field* m, Pic* pic)
{
	Graph::Point p[3];
	p[0] = Graph::Point(l->get_xpix()-vpx, l->get_ypix()-vpy);
	p[1] = Graph::Point(r->get_xpix()-vpx, r->get_ypix()-vpy);
	p[2] = Graph::Point(m->get_xpix()-vpx, m->get_ypix()-vpy);
	Vector n[3];
	n[0] = l->get_normal();
	n[1] = r->get_normal();
	n[2] = m->get_normal();
	g_gr.render_triangle(p, n, pic);
}

void Map_View::set_viewpoint(uint x,  uint y)
{ 
	vpx=x; vpy=y; 
	while(vpx>FIELD_WIDTH*map->get_w())			vpx-=(FIELD_WIDTH*map->get_w());
	while(vpy>(FIELD_HEIGHT*map->get_h())>>1)	vpy-=(FIELD_HEIGHT*map->get_h())>>1;
	while(vpx< 0)  vpx+=(FIELD_WIDTH*map->get_w());
	while(vpy< 0)  vpy+=(FIELD_HEIGHT*map->get_h())>>1;
}