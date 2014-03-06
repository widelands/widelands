/*
 * Copyright (C) 2002-2004, 2006, 2008-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef MINIMAP_H
#define MINIMAP_H

#include <boost/signals2.hpp>

#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

struct Interactive_Base;

struct MiniMap : public UI::UniqueWindow {
	struct Registry : public UI::UniqueWindow::Registry {
		int8_t flags; /**< Combination of \ref Layers flags */

		Registry() : flags(Terrn | Owner | Flags | Roads | Bldns) {}
	};

	MiniMap(Interactive_Base & parent, Registry *);

	boost::signals2::signal<void (int32_t, int32_t)> warpview;

	void set_view_pos(int32_t const x, int32_t const y) {
		m_view.set_view_pos(x, y);
	}

	enum Layers {Terrn = 1, Owner = 2, Flags = 4, Roads = 8, Bldns = 16, Zoom2 = 32};

private:
	void toggle(Layers);
	void update_button_permpressed();
	void resize();

	/**
	 * MiniMap::View is the panel that represents the pure representation of the
	 * map, without any borders or gadgets.
	 *
	 * If the size of MiniMapView is not the same as the size of the map itself,
	 * it will either show a subset of the map, or it will show the map more than
	 * once.
	 * The minimap always centers around the current viewpoint.
	 */
	struct View : public UI::Panel {
		View
			(UI::Panel & parent, int8_t * flags,
			 int32_t x, int32_t y, uint32_t w, uint32_t h,
			 Interactive_Base &);

		void set_view_pos(int32_t x, int32_t y);

		void draw(RenderTarget &) override;

		bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y) override;
		bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;

		void set_zoom(int32_t z);


	private:
		Interactive_Base & m_ibase;
		int32_t                m_viewx, m_viewy;
		const Image* m_pic_map_spot;
	public:
		int8_t * m_flags;
	};

	uint32_t number_of_buttons_per_row() const;
	uint32_t number_of_button_rows    () const;
	uint32_t but_w                    () const;
	uint32_t but_h                    () const;

	View     m_view;
	UI::Button button_terrn;
	UI::Button button_owner;
	UI::Button button_flags;
	UI::Button button_roads;
	UI::Button button_bldns;
	UI::Button button_zoom;
};

#endif
