/*
 * Copyright (C) 2002-20116 by the Widelands Development Team
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

#ifndef WL_WUI_MINIMAP_H
#define WL_WUI_MINIMAP_H

#include <memory>

#include <boost/signals2.hpp>

#include "graphic/minimap_renderer.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

class InteractiveBase;

struct MiniMap : public UI::UniqueWindow {
	struct Registry : public UI::UniqueWindow::Registry {
		MiniMapLayer flags; /**< Combination of \ref MiniMapLayer flags */

		Registry()
		   : flags(MiniMapLayer::Terrain | MiniMapLayer::Owner | MiniMapLayer::Flag |
		           MiniMapLayer::Road | MiniMapLayer::Building) {
		}
	};

	MiniMap(InteractiveBase& parent, Registry*);

	boost::signals2::signal<void(const Point&)> warpview;

	void set_view(const FloatRect& rect) {
		view_.set_view(rect);
	}

private:
	void toggle(MiniMapLayer);
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
		View(UI::Panel& parent,
		     MiniMapLayer* flags,
		     int32_t x,
		     int32_t y,
		     uint32_t w,
		     uint32_t h,
		     InteractiveBase&);

		// Set the currently viewed area in map pixel space.
		void set_view(const FloatRect&);

		void draw(RenderTarget&) override;

		bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
		bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;

		void set_zoom(int32_t z);

	private:
		InteractiveBase& ibase_;
		FloatRect view_area_;
		const Image* pic_map_spot_;

		// This needs to be owned since it will be rendered by the RenderQueue
		// later, so it must be valid for the whole frame.
		std::unique_ptr<Texture> minimap_image_;

	public:
		MiniMapLayer* flags_;
	};

	uint32_t number_of_buttons_per_row() const;
	uint32_t number_of_button_rows() const;
	uint32_t but_w() const;
	uint32_t but_h() const;

	View view_;
	UI::Button button_terrn;
	UI::Button button_owner;
	UI::Button button_flags;
	UI::Button button_roads;
	UI::Button button_bldns;
	UI::Button button_zoom;
};

#endif  // end of include guard: WL_WUI_MINIMAP_H
