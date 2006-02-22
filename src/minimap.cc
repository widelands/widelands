/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

 // just testing - philipp

#include "graphic.h"
#include "interactive_base.h"
#include "map.h"
#include "mapviewpixelconstants.h"
#include "minimap.h"
#include "rendertarget.h"
#include "ui_button.h"

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
MiniMapView::MiniMapView(UIPanel *parent, int x, int y, int w, int h, Interactive_Base *plr)
	: UIPanel(parent, x, y, 10, 10)
{
	m_player = plr;
	m_viewx = m_viewy = 0;
   m_flags = Minimap_Terrain;

	m_pic_map_spot = g_gr->get_picture( PicMod_Game,  "pics/map_spot.png" );

   if (!w)
		w = m_player->get_map()->get_width();
   if (!h)
		h = m_player->get_map()->get_height();

   set_size(w, h);
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
	m_viewx = x / TRIANGLE_WIDTH;
	m_viewy = y / TRIANGLE_HEIGHT;

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
   dst->renderminimap(m_player->get_egbase(), m_player->get_visibility(),
			Coords(m_viewx - get_w()/2, m_viewy - get_h()/2), m_flags);

/*
	// draw the view pos marker
	int x, y;
	int w, h;

	g_gr->get_picture_size(m_pic_map_spot, &w, &h);
	x = m_viewx - (w>>1);
	y = m_viewy - (h>>1);
	dst->blit(((int)m_player->get_map()->get_width()/2) - (w>>1), ((int)m_player->get_map()->get_height()/2) - (h>>1), m_pic_map_spot);
*/
}


/*
===============
MiniMapView::handle_mouseclick

Left-click: warp the view point to the new position
===============
*/
bool MiniMapView::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0)
		return false;

	if (down) {
		Coords c;

		// calculates the coordinates corresponding to the mouse position
      c.x = m_viewx + 1 - (get_w() / 2) + x;
      c.y = m_viewy + 1 - (get_h() / 2) + y;

		m_player->get_map()->normalize_coords(&c);

		warpview.call(c.x * TRIANGLE_WIDTH, c.y * TRIANGLE_HEIGHT);
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
MiniMap::MiniMap(Interactive_Base *plr, UIUniqueWindowRegistry *reg)
	: UIUniqueWindow(plr, reg, 10, 10, _("Map"))
{
	m_view = new MiniMapView(this, 0, 0, 0, 0, plr);
   int button_width = (int) (m_view->get_w()/3);
   int button_height = 20;

	m_flags = Minimap_Terrain;
//	m_view->warpview.set(&warpview, &UISignal2<int,int>::call);

	set_inner_size(m_view->get_w(), m_view->get_h() + 2 * button_height);

	//set_cache(false); // testing

	UIButton* b=new UIButton(this, 0, m_view->get_h(), button_width, button_height, 0);
	b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/button_color.png" ));
	//b->set_title(_("col"));
	b->clicked.set(this, &MiniMap::toggle_color);

	b=new UIButton(this, button_width, m_view->get_h(), button_width, button_height, 0);
	b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/button_ownedBy.png" ));
	//b->set_title(_("own"));
	b->clicked.set(this, &MiniMap::toggle_ownedBy);

	b=new UIButton(this, 2*button_width, m_view->get_h(), button_width, button_height, 0);
	b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/button_flags.png" ));
	//b->set_title(_("Flags"));
	b->clicked.set(this, &MiniMap::toggle_flags);

	b=new UIButton(this, 0, m_view->get_h()+button_height, button_width, button_height, 0);
	b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/button_roads.png" ));
	//b->set_title(_("Roads"));
	b->clicked.set(this, &MiniMap::toggle_roads);

	b=new UIButton(this, button_width, m_view->get_h()+button_height, button_width, button_height, 0);
	b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/button_building.png" ));
	//b->set_title(_("Buildings"));
	b->clicked.set(this, &MiniMap::toggle_buildings);

	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
MiniMap::toggle_color

Toggles "show Map Color"
===============
*/
void MiniMap::toggle_color()
{
   m_flags = m_flags ^ Minimap_Terrain;
   m_view->set_flags(m_flags );
}

/*
===============
MiniMap::toggle_ownedBy

Toggles "show by who the land is owned"
===============
*/
void MiniMap::toggle_ownedBy()
{
   m_flags = m_flags ^ Minimap_PlayerColor;
   m_view->set_flags(m_flags );
}

/*
===============
MiniMap::toggle_flags

Toggles "show flags
===============
*/
void MiniMap::toggle_flags()
{
   m_flags = m_flags ^ Minimap_Flags;
   m_view->set_flags(m_flags );
}

/*
===============
MiniMap::toggle_roads

Toggles "show roads"
===============
*/
void MiniMap::toggle_roads()
{
   m_flags = m_flags ^ Minimap_Roads;
   m_view->set_flags(m_flags );
}

/*
===============
MiniMap::toggle_buildings

Toggels "show buildings"
===============
*/
void MiniMap::toggle_buildings()
{
   m_flags = m_flags ^ Minimap_Buildings;
   m_view->set_flags(m_flags );
}

/** MiniMap::~MiniMap()
 *
 * Cleanup the minimap; unregister from the registry pointer
 */
MiniMap::~MiniMap()
{
   delete m_view;
}
