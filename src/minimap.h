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

#ifndef MINIMAP_H
#define MINIMAP_H

#include "ui.h"
class Player;
class Map;

/* class MiniMap
 *
 * Provide a minimap view (eventually with all sorts of gadgets, e.g.
 * show/hide buildings)
 */
class MiniMapView;

class MiniMap : public Window {
public:
	MiniMap(Panel *parent, int x, int y, Map *m, MiniMap **reg, Player* ply);
	~MiniMap();

	UISignal2<int,int> warpview; // screen coordinates of viewport

	void set_view_pos(int x, int y);

private:
	MiniMap **_registry;
	MiniMapView *_view;
};

#endif /* MINIMAP_H */
