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

#include <stdint.h>
#include "ui_button.h"
#include "ui_unique_window.h"

class Interactive_Base;

struct MiniMap : public UI::UniqueWindow {
	MiniMap(Interactive_Base & parent, UI::UniqueWindow::Registry *);

	UI::Signal2<int32_t, int32_t> warpview;

	void set_view_pos(const int32_t x, const int32_t y) throw ()
	{m_view.set_view_pos(x, y);}

	enum Layers {Terrn = 1, Owner = 2, Flags = 4, Roads = 8, Bldns = 16};

private:
	void toggle(Layers);

	/**
	 * MiniMapView is the panel that represents the pure representation of the
	 * map, without any borders or gadgets.
	 *
	 * If the size of MiniMapView is not the same as the size of the map itself,
	 * it will either show a subset of the map, or it will show the map more than
	 * once.
	 * The minimap always centers around the current viewpoint.
	 */
	struct View : public UI::Panel {
		View
			(UI::Panel & parent,
			 const  int32_t x, const  int32_t y,
			 const uint32_t w, const uint32_t h,
			 Interactive_Base &);

		void set_view_pos(const int32_t x, const int32_t y);

		void draw(RenderTarget* dst);

		bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
		bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);

	private:
		Interactive_Base & m_iabase;
		int32_t                m_viewx, m_viewy;
		uint32_t               m_pic_map_spot;
	public:
		int8_t flags;
	};

	uint32_t number_of_buttons_per_row() const throw ();
	uint32_t number_of_button_rows    () const throw ();
	uint32_t but_w                    () const throw ();
	uint32_t but_h                    () const throw ();

	View     m_view;
	UI::IDButton<MiniMap, Layers> button_terrn;
	UI::IDButton<MiniMap, Layers> button_owner;
	UI::IDButton<MiniMap, Layers> button_flags;
	UI::IDButton<MiniMap, Layers> button_roads;
	UI::IDButton<MiniMap, Layers> button_bldns;
   int8_t     m_flags;
};

#endif /* MINIMAP_H */
