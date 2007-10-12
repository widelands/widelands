/*
 * Copyright (C) 2002, 2004, 2006-2007 by the Widelands Development Team
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

#include "watchwindow.h"

#include "bob.h"
#include "game.h"
#include "graphic.h"
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

#include <vector>


#define NUM_VIEWS 5
#define REFRESH_TIME 5000

//Holds information for a view
struct WatchWindowView {
	Point view_point;
	Interactive_Player *parent;
	Object_Ptr tracking; //  if non-null, we're tracking a Bob
};

class WatchWindow : public UI::Window {
public:
	WatchWindow(Interactive_Player *parent, int32_t x, int32_t y, int32_t w, int32_t h, Coords coords, bool single_window=false);
	~WatchWindow();

	UI::Signal1<Point> warp_mainview;
   UI::Signal         closed;

	void start_tracking(Point pos);
	void toggle_tracking();
	void act_mainview_goto();

	void add_view(Coords coords);
	void next_view(bool first=false);
	void show_view(bool first=false);
	Point calc_coords(Coords coords);
	void save_coords();
	void set_view(int32_t index);
	void close_cur_view();
	void toggle_buttons();

protected:
	virtual void think();
	void stop_tracking_by_drag(int32_t x, int32_t y);

private:
	Game                           * m_game;
	Map_View                         m_mapview;
	bool m_single_window;
	uint32_t last_visit;
	int32_t m_cur_index;
	UI::Button<WatchWindow>          m_follow;
	UI::Button<WatchWindow>          m_goto;
	std::vector<WatchWindowView> m_views;
	UI::IDButton<WatchWindow, int32_t> * m_view_btns[NUM_VIEWS];
};


static WatchWindow *g_watch_window = NULL;

/*
===============
WatchWindow::WatchWindow

Initialize a watch window.
===============
*/
WatchWindow::WatchWindow(Interactive_Player *parent, int32_t x, int32_t y, int32_t w, int32_t h, Coords coords, bool single_window)
:
UI::Window(parent, x, y, w, h, _("Watch").c_str()),
m_game(parent->get_game()),
m_mapview(this, 0, 0, 200, 166, *parent),
m_single_window(single_window),
last_visit(m_game->get_gametime()),

	// UI::Buttons

m_follow
(this,
 0, h - 34, 34, 34,
 20,
 g_gr->get_picture(PicMod_UI, "pics/menu_watch_follow.png"),
 &WatchWindow::toggle_tracking, this,
 _("Follow")),

m_goto
(this,
 34, h - 34, 34, 34,
 21,
 g_gr->get_picture(PicMod_UI, "pics/menu_goto.png"),
 &WatchWindow::act_mainview_goto, this,
 _("Center mainview on this"))

{
	if (m_single_window) {
		for (Uint8 i = 0; i < NUM_VIEWS; ++i)
			m_view_btns[i] = new UI::IDButton<WatchWindow, int32_t>
				(this,
				 74 + (17 * i), 200 - 34, 17, 34,
				 0,
				 &WatchWindow::set_view, this, i,
				 "-");

		new UI::Button<WatchWindow>
			(this,
			 w - 34, h - 34, 34, 34,
			 22,
			 g_gr->get_picture(PicMod_UI, "pics/menu_abort.png"),
			 &WatchWindow::close_cur_view, this,
			 _("Close"));
	}
	m_mapview.fieldclicked.set(parent, &Interactive_Player::field_action);
	m_mapview.warpview.set(this, &WatchWindow::stop_tracking_by_drag);
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
	int32_t vx = coords.x * TRIANGLE_WIDTH;
	int32_t vy = coords.y * TRIANGLE_HEIGHT;


	return Point(vx - m_mapview.get_w() / 2, vy - m_mapview.get_h() / 2);
}

//Switch to next view
void WatchWindow::next_view(bool first) {
	if (!first && m_views.size() == 1)
		return;
	if (!first)
		save_coords();
	if (first || (static_cast<uint32_t>(m_cur_index) == m_views.size()-1 && m_cur_index != 0))
		m_cur_index = 0;
	else if (static_cast<uint32_t>(m_cur_index) < m_views.size()-1)
		++m_cur_index;
	show_view(first);
}

//Sets the current view to index and resets timeout
void WatchWindow::set_view(int32_t index) {
	save_coords();
	m_cur_index = index;
	last_visit = m_game->get_gametime();
	show_view();
}

//Saves the coordinates of a view if it was already shown (and possibly moved)
void WatchWindow::save_coords()
{m_views[m_cur_index].view_point = m_mapview.get_viewpoint();}

//Closes current view and disables button
void WatchWindow::close_cur_view() {
	if (m_views.size() == 1) {
		delete this;
		return;
	}

	int32_t old_index = m_cur_index;
	next_view();

	std::vector<WatchWindowView>::iterator view_it = m_views.begin();

	for (int32_t i = 0; i < old_index; ++i)
		++view_it;

	m_view_btns[m_cur_index]->set_enabled(false);
	m_views.erase(view_it);
	toggle_buttons();
}

//Enables/Disables buttons for views
void WatchWindow::toggle_buttons() {
	for (uint32_t i = 0; i < NUM_VIEWS; ++i) {
		if (i<m_views.size()) {
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%i", i + 1);
			m_view_btns[i]->set_title(buffer);
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
	m_mapview.set_viewpoint(m_views[m_cur_index].view_point);
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

	MapviewPixelFunctions::normalize_pix(map, pos);

	// Scan progressively larger circles around the given position for suitable bobs
	for
		(Area<FCoords> area
		 (map.get_fcoords
		  (MapviewPixelFunctions::calc_node_and_triangle(map, pos.x, pos.y).node),
		  2); area.radius <= 32; area.radius *= 2)
		if (map.find_bobs(area, &bobs)) break;

	// Find the bob closest to us
	int32_t closest_dist = -1;
	Bob* closest = 0;

	for (uint32_t i = 0; i < bobs.size(); ++i) {
		Bob* bob = bobs[i];
		Point p;

		MapviewPixelFunctions::get_pix(map, bob->get_position(), p.x, p.y);
		p = bob->calc_drawpos(*m_game, p);

		const int32_t dist = MapviewPixelFunctions::calc_pix_distance(map, p, pos);

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
	else start_tracking
		(m_mapview.get_viewpoint()
		 +
		 Point(m_mapview.get_w() / 2, m_mapview.get_h() / 2));
}


/*
===============
WatchWindow::act_mainview_goto

Cause the main mapview to jump to our current position.
===============
*/
void WatchWindow::act_mainview_goto() {
	warp_mainview.call
		(m_mapview.get_viewpoint()
		 +
		 Point(m_mapview.get_w() / 2, m_mapview.get_h() / 2));
}


/*
===============
WatchWindow::think

Update the mapview if we're tracking something.
===============
*/
void WatchWindow::think()
{
	UI::Window::think();

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

		m_mapview.set_viewpoint
			(pos - Point(m_mapview.get_w() / 2, m_mapview.get_h() / 2));
	}

   // make sure that the view gets updated
	m_mapview.need_complete_redraw();
}


/*
===============
WatchWindow::stop_tracking_by_drag

When the user drags the mapview, we stop tracking.
===============
*/
void WatchWindow::stop_tracking_by_drag(int32_t, int32_t) {
	//Disable switching while dragging
	if (m_mapview.is_dragging()) {
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
   if (s->get_bool("single_watchwin", false)) {
		if (g_watch_window != NULL)
			g_watch_window->add_view(coords);
		else
			g_watch_window = new WatchWindow(parent, 250, 150, 200, 200, coords, true);
      win = g_watch_window;
	}
	else
		win = new WatchWindow(parent, 250, 150, 200, 200, coords, false);
   win->closed.set(parent, &Interactive_Player::need_complete_redraw);

}
