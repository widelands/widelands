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
#include "auto_pic.h"
#include "player.h"
#include "IntPlayer.h"

/*
==============================================================================

MiniMapView

==============================================================================
*/

/** class MiniMapView
 *
 * MiniMapView is the panel that represents the pure representation of the
 * map, without any borders or gadgets.
 */
class MiniMapView : public Panel {
public:
	MiniMapView(Panel *parent, int x, int y, Interactive_Player *plr);

	UISignal2<int,int> warpview;

	void set_view_pos(int x, int y);

	// Drawing & event handling
	void draw(Bitmap *dst, int ofsx, int ofsy);

	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	static AutoPic map_spot;
	
	Interactive_Player	*m_player;
	Map						*m_map;
	int						m_viewx, m_viewy;
};

AutoPic MiniMapView::map_spot("map_spot.bmp", 0, 0, 255);

/*
===============
MiniMapView::MiniMapView

Initialize the minimap object
===============
*/
MiniMapView::MiniMapView(Panel *parent, int x, int y, Interactive_Player *plr)
	: Panel(parent, x, y, 10, 10)
{
	m_player = plr;
	m_map = plr->get_game()->get_map();

	m_viewx = m_viewy = 0;

	set_size(m_map->get_w(), m_map->get_h());
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
	update(0, 0, get_w(), get_h());
}

/** MiniMapView::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the view of the map
 */
void MiniMapView::draw(Bitmap *dst, int ofsx, int ofsy)
{
	bool use_see_area = !m_player->get_ignore_shadow();
	Player *player = m_player->get_player();
	int sx, sy;
	int ex, ey;
	int x, y;

	sx = -ofsx;
	if (sx < 0)
		sx = 0;
	sy = -ofsy;
	if (sy < 0)
		sy = 0;

	ex = dst->get_w() - ofsx;
	if (ex > (int)m_map->get_w())
		ex = m_map->get_w();
	ey = dst->get_h() - ofsy;
	if (ey > (int)m_map->get_h())
		ey = m_map->get_h();
			
	ushort clr;
	Field* f;
	for(y = sy; y < ey; y++)
	{
		ushort *pix = dst->get_pixels() + (y+ofsy)*dst->get_pitch() + (sx+ofsx);

		f = m_map->get_field(sx, y);
		for(x = sx; x < ex; x++, f++)
		{
         if (!use_see_area || player->is_field_seen(x, y)) {
				clr = *f->get_terd()->get_texture()->get_pixels();
				clr = bright_up_clr2(clr, f->get_brightness());
         
            *pix++ = clr;
         } else {
            *pix++ =  pack_rgb(0, 0, 0); // make black
         }
		}
	}

	// draw the view pos marker
	x = ofsx + m_viewx - (map_spot.get_w()>>1);
	y = ofsy + m_viewy - (map_spot.get_h()>>1);
	copy_pic(dst, &map_spot, x, y, 0, 0, map_spot.get_w(), map_spot.get_h());
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
		// make sure x/y is within range
		if (x >= 0 && x < (int)m_map->get_w() && y > 0 && y < (int)m_map->get_h())
			warpview.call(x * FIELD_WIDTH, y * (FIELD_HEIGHT>>1));
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
MiniMap::MiniMap(Panel *parent, int x, int y, MiniMap **reg, Interactive_Player *plr)
	: Window(parent, x, y, 10, 10, "Map")
{
	_registry = reg;
	if (_registry) {
		if (*_registry)
			delete *_registry;
		*_registry = this;
	}

	_view = new MiniMapView(this, 0, 0, plr);
	_view->warpview.set(&warpview, &UISignal2<int,int>::call);
	
	set_inner_size(_view->get_w(), _view->get_h());

	//set_cache(false); // testing
}

/** MiniMap::~MiniMap()
 *
 * Cleanup the minimap; unregister from the registry pointer
 */
MiniMap::~MiniMap()
{
	if (_registry)
		*_registry = 0;
}

/** MiniMap::set_view_pos(int x, int y)
 *
 * Forward the viewpos change to the actual view.
 *
 * Args: x	new view pos screen coordinates
 *       y
 */
void MiniMap::set_view_pos(int x, int y)
{
	_view->set_view_pos(x, y);
}
