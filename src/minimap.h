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

class Interactive_Base;
class Player;
class Map;
class MiniMapView;

/** class MiniMapView
 *
 * MiniMapView is the panel that represents the pure representation of the
 * map, without any borders or gadgets.
 */
class MiniMapView : public Panel {
public:
	MiniMapView(Panel *parent, int x, int y, Interactive_Base *plr, uint fx = 0, uint fy = 0);

	UISignal2<int,int> warpview;

	void set_view_pos(int x, int y);

	// Drawing & event handling
	void draw(RenderTarget* dst);

	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	Interactive_Base	*m_player;
	int						m_viewx, m_viewy;
	uint                  m_fx, m_fy;

	uint			m_pic_map_spot;
};


/* class MiniMap
 *
 * Provide a minimap view (eventually with all sorts of gadgets, e.g.
 * show/hide buildings)
 */

class MiniMap : public UniqueWindow {
public:
	MiniMap(Interactive_Base *parent, UniqueWindowRegistry *reg);
	~MiniMap();

   inline MiniMapView* get_minimapview(void) { return m_view; }

private:
	MiniMapView		*m_view;
};

#endif /* MINIMAP_H */
