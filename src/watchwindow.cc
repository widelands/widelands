/*
 * Copyright (C) 2002, 2004, 2006 by the Widelands Development Team
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

#include "bob.h"
#include "game.h"
#include "i18n.h"
#include "interactive_player.h"
#include "map.h"
#include "mapview.h"
#include "mapviewpixelconstants.h"
#include "mapviewpixelfunctions.h"
#include "profile.h"
#include "ui_button.h"
#include "ui_signal.h"
#include "ui_window.h"
#include "watchwindow.h"
#include <vector>

#define NUM_VIEWS 5
#define REFRESH_TIME 5000

/*
==============================================================================

WatchWindow

==============================================================================
*/
//Holds information for a view
struct WatchWindowView {
	Point view_point;
	Interactive_Player *parent;
	Object_Ptr tracking;		// if non-null, we're tracking a Bob
};

class WatchWindow : public UIWindow {
public:
	WatchWindow(Interactive_Player *parent, int x, int y, int w, int h, Coords coords, bool single_window=false);
	~WatchWindow();

	UISignal1<Point> warp_mainview;
   UISignal         closed;

	void start_tracking(Point pos);
	void toggle_tracking();
	void act_mainview_goto();

	void add_view(Coords coords);
	void next_view(bool first=false);
	void show_view(bool first=false);
	Point calc_coords(Coords coords);
	void save_coords();
	void set_view(int index);
	void close_cur_view();
	void toggle_buttons();

protected:
	virtual void think();
	void stop_tracking_by_drag(int x, int y);

private:
	Game*				m_game;
	Map_View*		m_mapview;
	bool m_single_window;
	uint last_visit;
	int m_cur_index;
	std::vector<WatchWindowView> m_views;
	UIButton* m_view_btns[NUM_VIEWS];
};


static WatchWindow *g_watch_window = NULL;

/*
===============
WatchWindow::WatchWindow

Initialize a watch window.
===============
*/
WatchWindow::WatchWindow(Interactive_Player *parent, int x, int y, int w, int h, Coords coords, bool single_window)
	: UIWindow(parent, x, y, w, h, _("Watch").c_str())
{
	UIButton* btn;

	m_game = parent->get_game();
	last_visit = m_game->get_gametime();
	m_single_window = single_window;

	// UIButtons
	btn = new UIButton(this, 0, h - 34, 34, 34, 20);
	btn->set_pic(g_gr->get_picture( PicMod_UI,  "pics/menu_watch_follow.png" ));
	btn->clicked.set(this, &WatchWindow::toggle_tracking);
	btn->set_tooltip(_("Follow").c_str());

	btn = new UIButton(this, 34, h - 34, 34, 34, 21);
	btn->set_pic(g_gr->get_picture( PicMod_UI,  "pics/menu_goto.png" ));
	btn->clicked.set(this, &WatchWindow::act_mainview_goto);
	btn->set_tooltip(_("Center mainview on this").c_str());

	if (m_single_window) {
		for (int i=0;i<NUM_VIEWS;i++) {
			btn = new UIButton(this, 74 + (17 * i), 200 - 34, 17, 34, 0, i);
			btn->set_title("-");
			btn->clickedid.set(this, &WatchWindow::set_view);
			m_view_btns[i] = btn;
		}

		btn = new UIButton(this, w-34, h - 34, 34, 34, 22);
		btn->set_pic(g_gr->get_picture( PicMod_UI,  "pics/menu_abort.png" ));
		btn->clicked.set(this, &WatchWindow::close_cur_view);
		btn->set_tooltip(_("Close").c_str());
	}
	m_mapview = new Map_View(this, 0, 0, 200, 166, parent);
	m_mapview->fieldclicked.set(parent, &Interactive_Player::field_action);
	m_mapview->warpview.set(this, &WatchWindow::stop_tracking_by_drag);
	warp_mainview.set(parent, &Interactive_Base::move_view_to_point);

	add_view(coords);
	next_view(true);
	set_cache(false);
}

//Add a view to a watchwindow, if there is space left
void WatchWindow::add_view(Coords coords) {
	if (m_views.size() >= NUM_VIEWS)
		return;
	WatchWindowView view;

	view.tracking = 0;
	view.view_point = calc_coords(coords);

	m_views.push_back(view);
	if (m_single_window)
		toggle_buttons();
}

//Calc point on map from coords
Point WatchWindow::calc_coords(Coords coords) {
	// Initial positioning
	int vx = coords.x * TRIANGLE_WIDTH;
	int vy = coords.y * TRIANGLE_HEIGHT;

	Point p (vx - m_mapview->get_w()/2, vy - m_mapview->get_h()/2);
	return p;
}

//Switch to next view
void WatchWindow::next_view(bool first) {
	if (!first && m_views.size() == 1)
		return;
	if (!first)
		save_coords();
	if (first || (static_cast<uint>(m_cur_index) == m_views.size()-1 && m_cur_index != 0))
		m_cur_index = 0;
	else if (static_cast<uint>(m_cur_index) < m_views.size()-1)
		m_cur_index++;
	show_view(first);
}

//Sets the current view to index and resets timeout
void WatchWindow::set_view(int index) {
	save_coords();
	m_cur_index = index;
	last_visit = m_game->get_gametime();
	show_view();
}

//Saves the coordinates of a view if it was already shown (and possibly moved)
void WatchWindow::save_coords() {
	m_views[m_cur_index].view_point = m_mapview->get_viewpoint();
}

//Closes current view and disables button
void WatchWindow::close_cur_view() {
	if (m_views.size() == 1) {
		delete this;
		return;
	}

	int old_index = m_cur_index;
	next_view();

	std::vector<WatchWindowView>::iterator view_it = m_views.begin();

	for (int i=0;i<old_index;i++)
		view_it++;

	m_view_btns[m_cur_index]->set_enabled(false);
	m_views.erase(view_it);
	toggle_buttons();
}

//Enables/Disables buttons for views
void WatchWindow::toggle_buttons() {
	for (uint i=0;i<NUM_VIEWS;i++) {
		if (i<m_views.size()) {
			char buf[32];
			snprintf(buf, sizeof(buf), "%i", i+1);
			m_view_btns[i]->set_title(buf);
			m_view_btns[i]->set_enabled(true);
		}
		else {
			m_view_btns[i]->set_title("-");
			m_view_btns[i]->set_enabled(false);
		}
	}
}

//Draws the current view
void WatchWindow::show_view(bool) {
	m_mapview->set_viewpoint(m_views[m_cur_index].view_point);
	//Tracking turned of by default
	//start_tracking(m_views[m_cur_index].view_point);
}

WatchWindow::~WatchWindow() {
	g_watch_window = NULL;
   closed.call();
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
	Map & map = *m_game->get_map();
	std::vector<Bob*> bobs;
	int radius;

	MapviewPixelFunctions::normalize_pix(map, pos);
	const Coords center =
		MapviewPixelFunctions::calc_node_and_triangle(map, pos.x, pos.y).node;

	// Scan progressively larger circles around the given position for suitable bobs
	for(radius = 2; radius <= 32; radius <<= 1) {
		if (map.find_bobs(center, radius, &bobs))
			break;
	}

	// Find the bob closest to us
	int closest_dist = -1;
	Bob* closest = 0;

	for(uint i = 0; i < bobs.size(); i++) {
		Bob* bob = bobs[i];
		Point p;

		MapviewPixelFunctions::get_pix(map, bob->get_position(), p.x, p.y);
		p = bob->calc_drawpos(*m_game, p);

		const int dist = MapviewPixelFunctions::calc_pix_distance(map, p, pos);

		if (!closest || closest_dist > dist) {
			closest = bob;
			closest_dist = dist;
		}
	}
	m_views[m_cur_index].tracking = closest;
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

	Map_Object* obj = m_views[m_cur_index].tracking.get(m_game);

	if (obj)
		m_views[m_cur_index].tracking = 0;
	else {
		start_tracking(m_mapview->get_viewpoint() +
					Point(m_mapview->get_w()/2, m_mapview->get_h()/2));
	}
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
	UIWindow::think();

	Map_Object* obj = m_views[m_cur_index].tracking.get(m_game);

	if ((m_game->get_gametime() - last_visit) > REFRESH_TIME) {
		last_visit = m_game->get_gametime();
		next_view();
		return;
	}

	if (obj) {
		Bob* bob = (Bob*)obj;
		Point pos;

		assert(obj->get_type() == Map_Object::BOB);

		MapviewPixelFunctions::get_pix
			(*m_game->get_map(), bob->get_position(), pos.x, pos.y);
		pos = bob->calc_drawpos(*m_game, pos);

		m_mapview->set_viewpoint(pos - Point(m_mapview->get_w()/2, m_mapview->get_h()/2));
	}

   // make sure that the view gets updated
   m_mapview->need_complete_redraw();
}


/*
===============
WatchWindow::stop_tracking_by_drag

When the user drags the mapview, we stop tracking.
===============
*/
void WatchWindow::stop_tracking_by_drag(int, int) {
	//Disable switching while dragging
	if (m_mapview->is_dragging()) {
		last_visit = m_game->get_gametime();
		m_views[m_cur_index].tracking = 0;
	}
}


/*
===============
show_watch_window

Open a watch window.
===============
*/
void show_watch_window(Interactive_Player *parent, Coords coords)
{
	Section *s = g_options.pull_section("global");
	WatchWindow* win;
   if (s->get_bool("single_watchwin",false)) {
		if (g_watch_window != NULL)
			g_watch_window->add_view(coords);
		else
			g_watch_window = new WatchWindow(parent, 250, 150, 200, 200, coords,true);
      win = g_watch_window;
	}
	else
		win = new WatchWindow(parent, 250, 150, 200, 200, coords,false);
   win->closed.set( parent, &Interactive_Player::need_complete_redraw);

}
