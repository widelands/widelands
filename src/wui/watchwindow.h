/*
 * Copyright (C) 2002, 2004, 2008-2009, 2011-2013 by The Widelands Development Team
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

#ifndef WL_WUI_WATCHWINDOW_H
#define WL_WUI_WATCHWINDOW_H

#include "logic/widelands_geometry.h"

#include "ui_basic/button.h"
#include "ui_basic/window.h"
#include "wui/mapview.h"

class InteractiveGameBase;
namespace Widelands {
class Game;
}

struct WatchWindow : public UI::Window {
	WatchWindow(InteractiveGameBase& parent,
	            int32_t x,
	            int32_t y,
	            uint32_t w,
	            uint32_t h,
	            bool single_window_ = false);
	~WatchWindow() override;

	Notifications::Signal<const Vector2f&> warp_mainview;

	void add_view(Widelands::Coords);
	void follow(Widelands::Bob* bob);

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kWatchWindow;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&, Widelands::MapObjectLoader&);

private:
	static constexpr size_t kViews = 5;

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
	MapView map_view_;
	Time last_visit_;
	bool single_window_;
	uint8_t cur_index_;
	UI::Button* view_btns_[kViews];
	std::vector<WatchWindow::View> views_;
};

WatchWindow* show_watch_window(InteractiveGameBase&, const Widelands::Coords&);

#endif  // end of include guard: WL_WUI_WATCHWINDOW_H
