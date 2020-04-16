/*
 * Copyright (C) 2010-2020 by the Widelands Development Team
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

#include "wui/quicknavigation.h"

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

	if (key.sym >= SDLK_1 && key.sym <= SDLK_9) {
		int which = key.sym - SDLK_1;
		assert(which >= 0);
		assert(which < kQuicknavSlots);

		if (key.mod & KMOD_CTRL) {
			set_landmark(which, current_);
		} else if (landmarks_[which].set) {
			map_view_->set_view(landmarks_[which].view, MapView::Transition::Smooth);
		}
	} else if (key.sym >= SDLK_KP_1 && key.sym <= SDLK_KP_9) {
		if (!(key.mod & KMOD_NUM)) {
			return false;
		}
		int which = key.sym - SDLK_KP_1;
		assert(which >= 0);
		assert(which < kQuicknavSlots);

		if (key.mod & KMOD_CTRL) {
			set_landmark(which, current_);
		} else if (landmarks_[which].set) {
			map_view_->set_view(landmarks_[which].view, MapView::Transition::Smooth);
		}
	} else if (key.sym == SDLK_COMMA && !previous_locations_.empty()) {
		// go to previous location
		insert_if_applicable(next_locations_);
		map_view_->set_view(previous_locations_.back(), MapView::Transition::Smooth);
		previous_locations_.pop_back();
	} else if (key.sym == SDLK_PERIOD && !next_locations_.empty()) {
		// go to next location
		insert_if_applicable(previous_locations_);
		map_view_->set_view(next_locations_.back(), MapView::Transition::Smooth);
		next_locations_.pop_back();
	} else {
		return false;
	}

	return true;
}
