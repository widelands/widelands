/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "ui_basic/unique_window.h"

#include <boost/bind.hpp>

namespace UI {
/*
==============================================================================

UniqueWindow IMPLEMENTATION

==============================================================================
*/

/**
 * Creates the window, if it does not exist.
*/
void UniqueWindow::Registry::create() {
	if (!window) {
		open_window();
	} else {
		if (window->is_minimal())
			window->restore();
		window->move_to_top();
	}
}

/**
 * Destroys the window, if it eixsts.
*/
void UniqueWindow::Registry::destroy() {
	if (window) {
		window->die();
	}
}

/**
 * Either destroys or creates the window.
*/
void UniqueWindow::Registry::toggle() {
	if (window) {
		window->die();
	} else {
		open_window();
	}
}

/**
 * In order to avoid dangling pointers, we need to kill our contained window
 * here.
*/
UniqueWindow::Registry::~Registry() {
	delete window;
}

void UniqueWindow::Registry::assign_toggle_button(UI::Button* button) {
	assert(!on_create);
	assert(!on_delete);
	on_create = boost::bind(&UI::Button::set_visual_state, button, UI::Button::VisualState::kPermpressed);
	on_delete = boost::bind(&UI::Button::set_visual_state, button, UI::Button::VisualState::kRaised);
	if (window) {
		button->set_visual_state(UI::Button::VisualState::kPermpressed);
	}
}

void UniqueWindow::Registry::unassign_toggle_button() {
	on_create = 0;
	on_delete = 0;
}

/**
 * Register, position according to the registry information.
*/
UniqueWindow::UniqueWindow(Panel* const parent,
                           const std::string& name,
                           UniqueWindow::Registry* const reg,
                           int32_t const w,
                           int32_t const h,
                           const std::string& title)
   : Window(parent, name, 0, 0, w, h, title.c_str()), registry_(reg), usedefaultpos_(true) {
	if (registry_) {
		delete registry_->window;

		registry_->window = this;
		if (registry_->valid_pos) {
			set_pos(Vector2i(registry_->x, registry_->y));
			usedefaultpos_ = false;
		}
		if (registry_->on_create) {
			registry_->on_create();
		}
	}
}

/**
 * Unregister, save latest position.
*/
UniqueWindow::~UniqueWindow() {
	if (registry_) {
		assert(registry_->window == this);

		registry_->window = nullptr;
		registry_->x = get_x();
		registry_->y = get_y();
		registry_->valid_pos = true;

		if (registry_->on_delete) {
			registry_->on_delete();
		}
	}
}
}
