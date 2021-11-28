/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
		if (window->is_minimal()) {
			window->restore();
		}
		window->move_to_top();
	}
}

/**
 * Destroys the window, if it exists.
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
		// There is already a window. If it is minimal, restore it.
		if (window->is_minimal()) {
			window->restore();
			opened();
		} else {
			window->die();
			window = nullptr;
		}
	} else {
		open_window();
	}
}

/**
 * In order to avoid dangling pointers, we need to kill our contained window
 * here.
 */
UniqueWindow::Registry::~Registry() {
	if (window) {
		delete window;
		window = nullptr;
	}
}

/**
 * Register, position according to the registry information.
 */
UniqueWindow::UniqueWindow(Panel* const parent,
                           WindowStyle s,
                           const std::string& name,
                           UniqueWindow::Registry* const reg,
                           int32_t const w,
                           int32_t const h,
                           const std::string& title)
   : UniqueWindow(parent, s, name, reg, 0, 0, w, h, title) {
}

/**
 * Register, position according to the registry information.
 */
UniqueWindow::UniqueWindow(Panel* const parent,
                           WindowStyle s,
                           const std::string& name,
                           UniqueWindow::Registry* const reg,
                           int32_t const x,
                           int32_t const y,
                           int32_t const w,
                           int32_t const h,
                           const std::string& title)
   : Window(parent, s, name, x, y, w, h, title), registry_(reg), usedefaultpos_(true) {
	if (registry_) {
		delete registry_->window;
		registry_->window = this;
		if (registry_->valid_pos) {
			set_pos(Vector2i(registry_->x, registry_->y));
			usedefaultpos_ = false;
		}
		registry_->opened();
	}
}

/**
 * Unregister, save latest position.
 */
UniqueWindow::~UniqueWindow() {
	if (registry_ && registry_->window) {
		assert(registry_->window == this);

		registry_->window = nullptr;
		registry_->x = get_x();
		registry_->y = get_y();
		registry_->valid_pos = true;

		registry_->closed();
	}
}
}  // namespace UI
