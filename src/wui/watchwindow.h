/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_WATCHWINDOW_H
#define WL_WUI_WATCHWINDOW_H

#include <string>

#include "logic/widelands_geometry.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"
#include "wui/mapview.h"

class InteractiveGameBase;
namespace Widelands {
class Game;
}  // namespace Widelands

struct WatchWindow : public UI::UniqueWindow {
	WatchWindow(InteractiveGameBase& parent,
	            const std::string& name,
	            uint16_t id,
	            int32_t x,
	            int32_t y,
	            uint32_t w,
	            uint32_t h,
	            bool init_single_window = false);
	~WatchWindow() override;

	Notifications::Signal<const Vector2f&> warp_mainview;

	void add_view(Widelands::Coords);
	void follow(Widelands::Bob* bob);

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kWatchWindow;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&, Widelands::MapObjectLoader&);

	bool handle_key(bool down, SDL_Keysym code) override {
		return map_view_.handle_key(down, code);
	}

private:
	static constexpr size_t kViews = 5;

	// Specialization of MapView
	class WatchWindowMapView : public MapView {
	public:
		WatchWindowMapView(WatchWindow* parent, const Widelands::Map& map);
		bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
		bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
		bool
		handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;

	private:
		WatchWindow* parent_window_;
		bool view_moved_{false};
		uint32_t dragging_start_time_{0};
	};

	// Holds information for a view
	struct View {
		MapView::View view;
		Widelands::ObjectPointer tracking;  //  if non-null, we're tracking a Bob
	};

	Widelands::Game& game() const;

	void think() override;
	void stop_tracking_by_drag();
	void draw(RenderTarget&) override;
	void save_coords();
	void next_view();
	void close_cur_view();
	void toggle_buttons();

	void do_follow();
	void do_goto();
	void view_button_clicked(uint8_t index);
	void set_current_view(uint8_t idx, bool save_previous = true);

	InteractiveGameBase& parent_;
	WatchWindowMapView map_view_;
	Time last_visit_;
	bool single_window_;
	uint8_t cur_index_{0U};
	UI::Button* view_btns_[kViews];
	std::vector<WatchWindow::View> views_;
	uint16_t id_;
};

WatchWindow* show_watch_window(InteractiveGameBase&, const Widelands::Coords&);

#endif  // end of include guard: WL_WUI_WATCHWINDOW_H
