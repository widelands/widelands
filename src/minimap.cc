/*
 * Copyright (C) 2002 by The Widelands Development Team
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
#include "minimap.h"
#include "map.h"
#include "player.h"
#include "IntPlayer.h"

/*
==============================================================================

MiniMapView

==============================================================================
*/

/*
===============
MiniMapView::MiniMapView

Initialize the minimap object
===============
*/
MiniMapView::MiniMapView(Panel *parent, int x, int y, Interactive_Base *plr, uint fx, uint fy)
	: Panel(parent, x, y, 10, 10)
{
	m_player = plr;
	m_map = plr->get_map();

	m_viewx = m_viewy = 0;
	
	m_pic_map_spot = g_gr->get_picture(PicMod_Game, "pics/map_spot.bmp", RGBColor(0,0,255));
   m_fx=fx;
   m_fy=fy;

   if(m_fx==0) m_fx=m_map->get_width();
   if(m_fy==0) m_fy=m_map->get_height();
	
   set_size(m_fx, m_fy);

}

/** MiniMapView::set_view_pos(int x, int y)
 *
 * Set the view point marker to a new position.
 *
 * Args: x	new view point coordinates, in screen coordinates
 *       y
 */
void MiniMapView::set_view_pos(int x, int y)
{
   m_viewx = x / FIELD_WIDTH;
   m_viewy = y / (FIELD_HEIGHT>>1);

   if(get_w()!=(int)m_map->get_width() && get_h()!=(int)m_map->get_height()) {
      m_viewx=(int)(((float)m_viewx/(float)m_map->get_width())*get_w());
      m_viewy=(int)(((float)m_viewy/(float)m_map->get_height())*get_h());
   }
   update(0, 0, get_w(), get_h());
}

/*
===============
MiniMapView::draw

Redraw the view of the map
===============
*/
void MiniMapView::draw(RenderTarget* dst)
{
	int x, y;
	int w, h;

	dst->renderminimap(Point(0,0), m_player->get_maprenderinfo(), m_fx, m_fy);
	
	// draw the view pos marker
	g_gr->get_picture_size(m_pic_map_spot, &w, &h);
	
	x = m_viewx - (w>>1);
	y = m_viewy - (h>>1);
	dst->blit(x, y, m_pic_map_spot);

}

/** MiniMapView::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Left-click: warp the view point to the new position
 */
bool MiniMapView::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0)
		return false;


   if (down) {
      if(get_w()==(int)m_map->get_width() && get_h()==(int)m_map->get_height()) {
         // make sure x/y is within range
         if (x >= 0 && x < (int)m_map->get_width() && y > 0 && y < (int)m_map->get_height())
            warpview.call(MULTIPLY_WITH_FIELD_WIDTH(x), MULTIPLY_WITH_HALF_FIELD_HEIGHT(y));
      } else {
         if(x>=0 && x < get_w() && y>=0 && y < get_h()) {
            int mx, my;
            mx=(int)(((double)x/(double)get_w())*m_map->get_width());
            my=(int)(((double)y/(double)get_h())*m_map->get_height());
            warpview.call(MULTIPLY_WITH_FIELD_WIDTH(mx), MULTIPLY_WITH_HALF_FIELD_HEIGHT(my));
         }
      }
   }

	return true;
}

/*
==============================================================================

MiniMap

==============================================================================
*/

/*
=============== 
MiniMap::MiniMap

Initialize the minimap window. Dimensions will be set automatically
according to the map size.
A registry pointer is set to track the MiniMap object (only show one
minimap at a time).

reg, the registry pointer will be set by constructor and cleared by
destructor
===============
*/
MiniMap::MiniMap(Interactive_Base *plr, UniqueWindow *reg)
	: Window(plr, 200, 150, 10, 10, "Map")
{
	m_registry = reg;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}

	m_view = new MiniMapView(this, 0, 0, plr);
//	m_view->warpview.set(&warpview, &UISignal2<int,int>::call);
	
	set_inner_size(m_view->get_w(), m_view->get_h());

	//set_cache(false); // testing
}

/** MiniMap::~MiniMap()
 *
 * Cleanup the minimap; unregister from the registry pointer
 */
MiniMap::~MiniMap()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}
