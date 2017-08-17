/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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

QuickNavigation::QuickNavigation(MapView* map_view) : map_view_(map_view), landmarks_(10) {
	map_view->changeview.connect([this] { view_changed(); });
	havefirst_ = false;
}

void QuickNavigation::view_changed() {
	current_ = map_view_->view();
	havefirst_ = true;
}

void QuickNavigation::set_landmark(size_t index, const MapView::View& view) {
	assert(index < landmarks_.size());
	landmarks_[index].view = view;
	landmarks_[index].set = true;
}

bool QuickNavigation::handle_key(bool down, SDL_Keysym key) {
	if (!havefirst_)
		return false;
	if (!down)
		return false;

	if (key.sym >= SDLK_1 && key.sym <= SDLK_9) {
		unsigned int which = key.sym - SDLK_0;
		assert(which < 10);

		if (key.mod & KMOD_CTRL) {
			set_landmark(which, current_);
		} else {
			if (landmarks_[which].set) {
				map_view_->set_view(landmarks_[which].view, MapView::Transition::Smooth);
			}
		}
		return true;
	}

	return false;
}
