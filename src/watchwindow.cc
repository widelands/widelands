/*
 * Copyright (C) 2002, 2004 by The Widelands Development Team
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

#include <vector>
#include "widelands.h"
#include "ui.h"
#include "game.h"
#include "map.h"
#include "mapview.h"
#include "IntPlayer.h"
#include "watchwindow.h"


/*
==============================================================================

WatchWindow

==============================================================================
*/

class WatchWindow : public Window {
public:
	WatchWindow(Interactive_Player *parent, int x, int y, int w, int h, Coords coords);

	UISignal1<Point> warp_mainview;

	void start_tracking(Point pos);
	void toggle_tracking();
	void act_mainview_goto();

protected:
	virtual void think();

	void stop_tracking_by_drag(int x, int y);

private:
	Game*				m_game;
	Map_View*		m_mapview;
	Object_Ptr		m_tracking;		// if non-null, we're tracking a Bob
};


/*
===============
WatchWindow::WatchWindow

Initialize a watch window.
===============
*/
WatchWindow::WatchWindow(Interactive_Player *parent, int x, int y, int w, int h, Coords coords)
	: Window(parent, x, y, w, h, "Watch")
{
	Button* btn;

	m_game = parent->get_game();

	m_mapview = new Map_View(this, 0, 0, w, h - 34, parent);
	m_mapview->fieldclicked.set(parent, &Interactive_Player::field_action);
	m_mapview->warpview.set(this, &WatchWindow::stop_tracking_by_drag);

	warp_mainview.set(parent, &Interactive_Base::move_view_to_point);

	// Buttons
	btn = new Button(this, 0, h - 34, 34, 34, 0);
	btn->set_pic(g_gr->get_picture(PicMod_UI, "pics/menu_watch_follow.png", RGBColor(0,0,255)));
	btn->clicked.set(this, &WatchWindow::toggle_tracking);

	btn = new Button(this, 34, h - 34, 34, 34, 0);
	btn->set_pic(g_gr->get_picture(PicMod_UI, "pics/menu_goto.png", RGBColor(0,0,255)));
	btn->clicked.set(this, &WatchWindow::act_mainview_goto);

	// Initial positioning
	int vx = MULTIPLY_WITH_FIELD_WIDTH(coords.x);
	int vy = MULTIPLY_WITH_HALF_FIELD_HEIGHT(coords.y);

	m_mapview->set_viewpoint(Point(vx - m_mapview->get_w()/2, vy - m_mapview->get_h()/2));

	start_tracking(Point(vx, vy));

	// don't cache: animations will always enforce redraw
	set_cache(false);
}


/*
===============
WatchWindow::start_tracking

Find the nearest bob. Other objects cannot move and are therefore not of
interest.

point is *not* a coordinate, but a map-global position in pixels.
===============
*/
void WatchWindow::start_tracking(Point pos)
{
	Map* map = m_game->get_map();
	std::vector<Bob*> bobs;
	Coords center;
	int radius;

	map->normalize_pix(&pos);
	center = map->calc_coords(pos);

	// Scan progressively larger circles around the given position for suitable bobs
	for(radius = 2; radius <= 32; radius <<= 1) {
		if (map->find_bobs(center, radius, &bobs))
			break;
	}

	// Find the bob closest to us
	int closest_dist = -1;
	Bob* closest = 0;

	for(uint i = 0; i < bobs.size(); i++) {
		Bob* bob = bobs[i];
		Point p;
		int dist;

		map->get_pix(bob->get_position(), &p.x, &p.y);
		bob->calc_drawpos(m_game, p, &p);

		dist = map->calc_pix_distance(p, pos);

		if (!closest || closest_dist > dist) {
			closest = bob;
			closest_dist = dist;
		}
	}

	m_tracking = closest;
}


/*
===============
WatchWindow::toggle_tracking

If we're currently tracking, stop tracking.
Otherwise, start tracking the nearest bob from our current position.
===============
*/
void WatchWindow::toggle_tracking()
{
	Map_Object* obj = m_tracking.get(m_game);

	if (obj)
		m_tracking = 0;
	else
		start_tracking(m_mapview->get_viewpoint() +
					Point(m_mapview->get_w()/2, m_mapview->get_h()/2));
}


/*
===============
WatchWindow::act_mainview_goto

Cause the main mapview to jump to our current position.
===============
*/
void WatchWindow::act_mainview_goto()
{
	Point p = m_mapview->get_viewpoint() + Point(m_mapview->get_w()/2, m_mapview->get_h()/2);

	warp_mainview.call(p);
}


/*
===============
WatchWindow::think

Update the mapview if we're tracking something.
===============
*/
void WatchWindow::think()
{
	Window::think();

	Map_Object* obj = m_tracking.get(m_game);

	if (obj) {
		Bob* bob = (Bob*)obj;
		Point pos;

		assert(obj->get_type() == Map_Object::BOB);

		m_game->get_map()->get_pix(bob->get_position(), &pos.x, &pos.y);
		bob->calc_drawpos(m_game, pos, &pos);

		m_mapview->set_viewpoint(pos - Point(m_mapview->get_w()/2, m_mapview->get_h()/2));
	}
}


/*
===============
WatchWindow::stop_tracking_by_drag

When the user drags the mapview, we stop tracking.
===============
*/
void WatchWindow::stop_tracking_by_drag(int x, int y)
{
	if (m_mapview->is_dragging())
		m_tracking = 0;
}


/*
===============
show_watch_window

Open a watch window.
===============
*/
void show_watch_window(Interactive_Player *parent, Coords coords)
{
	new WatchWindow(parent, 250, 150, 200, 200, coords);
}
