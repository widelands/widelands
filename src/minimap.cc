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

#include "os.h"
#include "minimap.h"
#include "map.h"
#include "auto_pic.h"

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
	MiniMapView(Panel *parent, int x, int y, Map *m);

	UISignal2<int,int> warpview;

	void set_view_pos(int x, int y);

	// Drawing & event handling
	void draw(Bitmap *dst, int ofsx, int ofsy);

	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	static AutoPic map_spot;

	Map *_map;
	int _viewx, _viewy;
};

AutoPic MiniMapView::map_spot("map_spot.bmp", 0, 0, 255);

/** MiniMapView::MiniMapView(Panel *parent, int x, int y, Map *m)
 *
 * Initialize the minimap object
 */
MiniMapView::MiniMapView(Panel *parent, int x, int y, Map *m)
	: Panel(parent, x, y, m->get_w(), m->get_h())
{
	_map = m;

	_viewx = _viewy = 0;
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
	_viewx = x / FIELD_WIDTH;
	_viewy = y / (FIELD_HEIGHT>>1);
	update(0, 0, get_w(), get_h());
}

/** MiniMapView::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Redraw the view of the map
 */
void MiniMapView::draw(Bitmap *dst, int ofsx, int ofsy)
{
	int sx, sy;
	int ex, ey;

	sx = -ofsx;
	if (sx < 0)
		sx = 0;
	sy = -ofsy;
	if (sy < 0)
		sy = 0;

	ex = dst->get_w() - ofsx;
	if (ex > _map->get_w())
		ex = _map->get_w();
	ey = dst->get_h() - ofsy;
	if (ey > _map->get_h())
		ey = _map->get_h();
			
   ushort clr;
	Field* f;
   for(int y = sy; y < ey; y++)
	{
		ushort *pix = dst->get_pixels() + (y+ofsy)*dst->get_pitch() + (sx+ofsx);
			
      f = _map->get_field(sx, y);
		for(int x = sx; x < ex; x++)
		{

			clr = *f->get_texd()->get_pixels();
			clr = bright_up_clr2(clr, f->get_brightness());

			*pix++ = clr;
         f = _map->get_nfield();
		}
	}

	// draw the view pos marker
	int x = ofsx + _viewx - (map_spot.get_w()>>1);
	int y = ofsy + _viewy - (map_spot.get_h()>>1);
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
		if (x >= 0 && x < _map->get_w() && y > 0 && y < _map->get_h())
			warpview.call(x * FIELD_WIDTH, y * (FIELD_HEIGHT>>1));
	}

	return true;
}

/*
==============================================================================

MiniMap

==============================================================================
*/

/** MiniMap::MiniMap(Panel *parent, int x, int y, Map *m, MiniMap **reg)
 *
 * Initialize the minimap window. Dimensions will be set automatically
 * according to the map size.
 * A registry pointer is set to track the MiniMap object (only show one
 * minimap at a time).
 *
 * Args: parent	parent panel
 *       x		coordinates of the window
 *       y
 *       m		pointer to the map
 *       reg	registry pointer will be set by constructor and cleared by
 *       		destructor
 */
MiniMap::MiniMap(Panel *parent, int x, int y, Map *m, MiniMap **reg)
	: Window(parent, x, y, m->get_w(), m->get_h(), "Map")
{
	_registry = reg;
	if (_registry) {
		if (*_registry)
			delete *_registry;
		*_registry = this;
	}

	_view = new MiniMapView(this, 0, 0, m);
	_view->warpview.set(&warpview, &UISignal2<int,int>::call);

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
