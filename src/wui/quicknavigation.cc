/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#include "wui/quicknavigation.h"

#include "wlapplication_options.h"

QuickNavigation::QuickNavigation(MapView* map_view)
   : map_view_(map_view), location_jumping_started_(false) {
	map_view->changeview.connect([this] { view_changed(); });
	map_view->jump.connect([this] { jumped(); });
	havefirst_ = false;
}

inline void QuickNavigation::insert_if_applicable(std::list<MapView::View>& l) {
	if (l.empty() || !l.back().view_roughly_near(current_)) {
		l.push_back(current_);
	}
}

void QuickNavigation::jumped() {
	if (!location_jumping_started_) {
		assert(previous_locations_.empty());
		assert(next_locations_.empty());
		location_jumping_started_ = true;
		return;
	}
	next_locations_.clear();
	insert_if_applicable(previous_locations_);
}

void QuickNavigation::view_changed() {
	current_ = map_view_->view();
	havefirst_ = true;
}

void QuickNavigation::set_landmark(size_t index, const MapView::View& view) {
	assert(index < kQuicknavSlots);
	landmarks_[index].view = view;
	landmarks_[index].set = true;
}

bool QuickNavigation::handle_key(bool down, SDL_Keysym key) {
	if (!havefirst_ || !down) {
		return false;
	}

	auto check_landmark = [this, key](const uint8_t i) {
		// This function assumes that the shortcut entries are ordered
		// Set1,Goto1,Set2,Goto2,Set3,Goto3,etc
		if (matches_shortcut(static_cast<KeyboardShortcut>(
		                        static_cast<uint16_t>(KeyboardShortcut::kInGameQuicknavSet1) + 2 * i),
		                     key)) {
			set_landmark(i, current_);
			return true;
		}
		if (landmarks_[i].set &&
		    matches_shortcut(
		       static_cast<KeyboardShortcut>(
		          static_cast<uint16_t>(KeyboardShortcut::kInGameQuicknavGoto1) + 2 * i),
		       key)) {
			map_view_->set_view(landmarks_[i].view, MapView::Transition::Smooth);
			return true;
		}
		return false;
	};
	for (uint8_t i = 0; i < 9; ++i) {
		if (check_landmark(i)) {
			return true;
		}
	}

	if (matches_shortcut(KeyboardShortcut::kCommonQuicknavPrev, key) &&
	    !previous_locations_.empty()) {
		// go to previous location
		insert_if_applicable(next_locations_);
		map_view_->set_view(previous_locations_.back(), MapView::Transition::Smooth);
		previous_locations_.pop_back();
		return true;
	}
	if (matches_shortcut(KeyboardShortcut::kCommonQuicknavNext, key) && !next_locations_.empty()) {
		// go to next location
		insert_if_applicable(previous_locations_);
		map_view_->set_view(next_locations_.back(), MapView::Transition::Smooth);
		next_locations_.pop_back();
		return true;
	}

	return false;
}
