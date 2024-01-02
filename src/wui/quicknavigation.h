/*
 * Copyright (C) 2010-2024 by the Widelands Development Team
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

#ifndef WL_WUI_QUICKNAVIGATION_H
#define WL_WUI_QUICKNAVIGATION_H

#include <memory>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"
#include "wui/mapview.h"

class InteractiveBase;

constexpr uint16_t kQuicknavSlots = 9;

struct NoteQuicknavChangedEvent {
	CAN_BE_SENT_AS_NOTE(NoteId::QuicknavChangedEvent)
};

/** Provide quick navigation shortcuts and landmarks. */
struct QuickNavigation {
	struct Landmark {
		MapView::View view;
		bool set{false};
		std::string name;

		Landmark() = default;
	};

	explicit QuickNavigation(MapView* map_view);

	// Set the landmark for 'index' to 'view'. A landmark with the given index must already exist.
	void set_landmark(size_t index, const MapView::View& view);
	inline void set_landmark_to_current(size_t index) {
		set_landmark(index, current_);
	}
	void unset_landmark(size_t index);
	void add_landmark();
	void remove_landmark(size_t index);

	/** Returns the vector of all landmarks. */
	[[nodiscard]] const std::vector<Landmark>& landmarks() const {
		return landmarks_;
	}
	std::vector<Landmark>& landmarks() {
		return landmarks_;
	}

	/** Go to the previous/next location or a landmark. */
	void goto_prev();
	void goto_next();
	void goto_landmark(int index);
	[[nodiscard]] bool can_goto_prev() const;
	[[nodiscard]] bool can_goto_next() const;

	bool handle_key(bool down, SDL_Keysym key);

private:
	void view_changed();
	void jumped();

	MapView* map_view_;

	bool havefirst_;
	MapView::View current_;

	// Landmarks that were set explicitly by the player, mapped on the 1-9 keys.
	std::vector<Landmark> landmarks_;

	// navigation with ',' and '.'
	std::list<MapView::View> previous_locations_;
	std::list<MapView::View> next_locations_;
	// Ignore the initial (0,0,1×) view
	bool location_jumping_started_{false};
	void insert_if_applicable(std::list<MapView::View>&);
};

/** A window with all landmarks and quick navigation UI. */
class QuickNavigationWindow : public UI::UniqueWindow {
public:
	QuickNavigationWindow(InteractiveBase& ibase, UI::UniqueWindow::Registry& r);

	void think() override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kQuicknav;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	void rebuild();

	InteractiveBase& ibase_;
	UI::Box main_box_, buttons_box_;
	UI::Button prev_, next_, new_;
	std::unique_ptr<UI::Box> content_box_;

	std::unique_ptr<Notifications::Subscriber<NoteQuicknavChangedEvent>> subscriber_;
};

#endif  // end of include guard: WL_WUI_QUICKNAVIGATION_H
