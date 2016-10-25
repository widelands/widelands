/*
 * Copyright (C) 2010-2016 by the Widelands Development Team
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

#include "logic/editor_game_base.h"
#include "wlapplication.h"
#include "wui/mapviewpixelfunctions.h"

static const uint32_t MaxHistorySize = 32;

QuickNavigation::QuickNavigation(const Widelands::EditorGameBase& egbase, MapView* map_view)
   : egbase_(egbase), map_view_(map_view), landmarks_(10) {
	map_view->changeview.connect([this](const bool jump) { view_changed(jump); });
	havefirst_ = false;
	update_ = true;
	history_index_ = 0;
}

void QuickNavigation::setview(const View& view) {
	update_ = false;
	map_view_->set_zoom(view.zoom);
	map_view_->set_viewpoint(view.viewpoint, true);
	update_ = true;
}

void QuickNavigation::view_changed(bool jump) {
	const Rectf view_area = map_view_->get_view_area();
	if (havefirst_ && update_) {
		if (!jump) {
			// Check if the anchor is moved outside the screen. If that is the
			// case, we did jump.
			const Vector2f dist =
			   MapviewPixelFunctions::calc_pix_difference(egbase_.map(), anchor_, view_area.center());
			if (dist.x > view_area.w / 2.f || dist.y > view_area.w / 2.f) {
				jump = true;
			}
		}

		if (jump) {
			if (history_index_ < history_.size())
				history_.erase(history_.begin() + history_index_, history_.end());
			history_.push_back(current_);
			if (history_.size() > MaxHistorySize)
				history_.erase(history_.begin(), history_.end() - MaxHistorySize);
			history_index_ = history_.size();
		}
	}

	if (jump || !havefirst_) {
		anchor_ = view_area.center();
	}

	current_ = View{
	   map_view_->get_viewpoint(), map_view_->get_zoom(),
	};
	havefirst_ = true;
}

void QuickNavigation::set_landmark(size_t index, const View& view) {
	assert(index < landmarks_.size());
	landmarks_[index].view = view;
	landmarks_[index].set = true;
}

bool QuickNavigation::handle_key(bool down, SDL_Keysym key) {
	if (!havefirst_)
		return false;
	if (!down)
		return false;

	if (key.sym >= SDLK_0 && key.sym <= SDLK_9) {
		unsigned int which = key.sym - SDLK_0;
		assert(which < 10);

		bool ctrl = WLApplication::get()->get_key_state(SDL_SCANCODE_LCTRL) ||
		            WLApplication::get()->get_key_state(SDL_SCANCODE_RCTRL);
		if (ctrl) {
			set_landmark(which, current_);
		} else {
			if (landmarks_[which].set) {
				setview(landmarks_[which].view);
			}
		}
		return true;
	}

	if (key.sym == SDLK_COMMA) {
		if (history_index_ > 0) {
			if (history_index_ >= history_.size()) {
				history_.push_back(current_);
			}
			history_index_--;
			setview(history_[history_index_]);
		}
		return true;
	}

	if (key.sym == SDLK_PERIOD) {
		if (history_index_ + 1 < history_.size()) {
			history_index_++;
			setview(history_[history_index_]);
		}
		return true;
	}

	return false;
}
