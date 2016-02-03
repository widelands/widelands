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

QuickNavigation::QuickNavigation
	(const Widelands::EditorGameBase & egbase,
	 uint32_t screenwidth, uint32_t screenheight)
: egbase_(egbase)
{
	screenwidth_ = screenwidth;
	screenheight_ = screenheight;

	havefirst_ = false;
	update_ = true;
	history_index_ = 0;
}

void QuickNavigation::set_setview(const QuickNavigation::SetViewFn & fn)
{
	setview_ = fn;
}

void QuickNavigation::setview(Point where)
{
	update_ = false;
	setview_(where);
	update_ = true;
}

void QuickNavigation::view_changed(Point newpos, bool jump)
{
	if (havefirst_ && update_) {
		if (!jump) {
			Point delta =
				MapviewPixelFunctions::calc_pix_difference
					(egbase_.map(), newpos, anchor_);

			if
				(static_cast<uint32_t>(abs(delta.x)) > screenwidth_ ||
			    	 static_cast<uint32_t>(abs(delta.y)) > screenheight_)
				jump = true;
		}

		if (jump) {
			if (history_index_ < history_.size())
				history_.erase
					(history_.begin() + history_index_,
					 history_.end());
			history_.push_back(current_);
			if (history_.size() > MaxHistorySize)
				history_.erase
					(history_.begin(),
					 history_.end() - MaxHistorySize);
			history_index_ = history_.size();
		}
	}

	if (jump || !havefirst_) {
		anchor_ = newpos;
	}

	current_ = newpos;
	havefirst_ = true;
}

bool QuickNavigation::handle_key(bool down, SDL_Keysym key)
{
	if (!havefirst_)
		return false;
	if (!down)
		return false;

	if (key.sym >= SDLK_0 && key.sym <= SDLK_9) {
		unsigned int which = key.sym - SDLK_0;
		assert(which < 10);

		bool ctrl =
			WLApplication::get()->get_key_state(SDL_SCANCODE_LCTRL) ||
			WLApplication::get()->get_key_state(SDL_SCANCODE_RCTRL);
		if (ctrl) {
			landmarks_[which].point = current_;
			landmarks_[which].set = true;
		} else {
			if (landmarks_[which].set)
				setview_(landmarks_[which].point);
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
