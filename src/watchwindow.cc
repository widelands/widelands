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
	WatchWindow(Interactive_Player *parent, int x, int y, int w, int h, int fx, int fy);

private:
	Map_View *mapview;
};

/** WatchWindow(Panel *parent, int x, int y, int w, int h, int fx, int fy)
 *
 * Initialize a watch window.
 *
 * Args: parent	the parent panel
 *       x		initial on-screen coordinates
 *       y
 *       w		size of the MapView
 *       h
 *       fx		field coordinates to center on
 *       fy
 */
WatchWindow::WatchWindow(Interactive_Player *parent, int x, int y, int w, int h, int fx, int fy)
	: Window(parent, x, y, w, h, "Watch")
{
	mapview = new Map_View(this, 0, 0, w, h, parent->get_game());
	mapview->fieldclicked.set(parent, &Interactive_Player::field_action);

	// only an approximation (odd rows shifted), but so what
	int vx = fx * FIELD_WIDTH;
	int vy = fy * (FIELD_HEIGHT>>1);
	mapview->set_viewpoint(vx - w/2, vy - h/2);

	// don't cache: animations will always enforce redraw
	set_cache(false);
}

/** show_watch_window(Interactive_Player *parent, int fx, int fy)
 *
 * Open a watch window.
 *
 * Args: parent	the associated player
 *       fx		the field coordinates to center on
 *       fy
 */
void show_watch_window(Interactive_Player *parent, int fx, int fy)
{
	new WatchWindow(parent, 250, 150, 200, 200, fx, fy);
}
