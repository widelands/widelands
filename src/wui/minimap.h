/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "graphic/minimap_renderer.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

class InteractiveBase;

class MiniMap : public UI::UniqueWindow {
public:
	struct Registry : public UI::UniqueWindow::Registry {
		MiniMapLayer minimap_layers;
		MiniMapType minimap_type;

		Registry()
		   : minimap_layers(MiniMapLayer::Terrain | MiniMapLayer::Owner | MiniMapLayer::Flag |
		                    MiniMapLayer::Road | MiniMapLayer::Building),
		     minimap_type(MiniMapType::kStaticViewWindow) {
		}

		MiniMap* get_window() const {
			return dynamic_cast<MiniMap*>(window);
		}
	};

	MiniMap(InteractiveBase& parent, Registry*);

	Notifications::Signal<const Vector2f&> warpview;

	void set_view(const Rectf& rect) {
		view_.set_view(rect);
	}
	void check_boundaries();

	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kMinimap;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;

	void toggle(MiniMapLayer);
	void update_button_permpressed();
	void resize();

	/**
	 * MiniMap::View is the panel that represents the pure representation of the
	 * map, without any borders or gadgets.
	 */
	struct View : public UI::Panel {
		View(UI::Panel& parent,
		     MiniMapLayer* minimap_layers,
		     MiniMapType* minimap_type,
		     int32_t x,
		     int32_t y,
		     uint32_t w,
		     uint32_t h,
		     InteractiveBase&);

		// Set the currently viewed area in map pixel space.
		void set_view(const Rectf&);

		// Delete the intermediate texture, causing a full redraw on the next draw().
		void reset();

		void draw(RenderTarget&) override;

		bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;

		void set_zoom(bool zoom);

		bool can_zoom();

	private:
		InteractiveBase& ibase_;
		Rectf view_area_;
		const Image* pic_map_spot_;

		// Intermediate texture, cached between frames.
		std::unique_ptr<Texture> minimap_image_static_;
		uint16_t rows_drawn_;

		// This needs to be owned since it will be rendered by the RenderQueue
		// later, so it must be valid for the whole frame.
		std::unique_ptr<Texture> minimap_image_final_;

	public:
		MiniMapLayer* minimap_layers_;
		MiniMapType* minimap_type_;
	};

	uint32_t number_of_buttons_per_row() const;
	uint32_t number_of_button_rows() const;
	uint32_t but_w() const;
	uint32_t but_h() const;

	InteractiveBase& ibase_;
	View view_;
	UI::Button button_terrn;
	UI::Button button_owner;
	UI::Button button_flags;
	UI::Button button_roads;
	UI::Button button_bldns;
	UI::Button button_zoom;
};

#endif  // end of include guard: WL_WUI_MINIMAP_H
