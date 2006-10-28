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

#ifndef MINIMAP_H
#define MINIMAP_H


#include "ui_panel.h"
#include "ui_signal.h"
#include "ui_unique_window.h"

class Interactive_Base;
class MiniMapView;

enum {
   Minimap_Terrain = 1,       // display terrain
   Minimap_PlayerColor = 2,   // color of owner
   Minimap_Flags = 4,         // show flags
   Minimap_Roads = 8,         // show roads
   Minimap_Buildings = 16,    // show buildings
};

/** class MiniMapView
 *
 * MiniMapView is the panel that represents the pure representation of the
 * map, without any borders or gadgets.
 *
 * If the size of MiniMapView is not the same as the size of the map itself,
 * it will either show a subset of the map, or it will show the map more than
 * once.
 * The minimap always centers around the current viewpoint.
 */
class MiniMapView : public UIPanel {
public:
	MiniMapView(UIPanel *parent, int x, int y, int w, int h, Interactive_Base *plr);

	UISignal2<int,int> warpview;

	void set_view_pos(int x, int y);

	// Drawing & event handling
	void draw(RenderTarget* dst);

	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

   inline void set_flags(char flags) { m_flags = flags; }

private:
	Interactive_Base*	m_player;
	int					m_viewx, m_viewy;
	uint					m_pic_map_spot;
   char 					m_flags;
};


/* class MiniMap
 *
 * Provide a minimap view (eventually with all sorts of gadgets, e.g.
 * show/hide buildings)
 */
class MiniMap : public UIUniqueWindow {
public:
	MiniMap(Interactive_Base *parent, UIUniqueWindowRegistry *reg);
	~MiniMap();

   inline MiniMapView* get_minimapview(void) { return m_view; }

   //void toggle_MapFlags(char new_flag);
   void toggle_color();
   void toggle_ownedBy();
   void toggle_flags();
   void toggle_roads();
   void toggle_buildings();

private:
	MiniMapView		*m_view;
   char 				m_flags;
};

#endif /* MINIMAP_H */
