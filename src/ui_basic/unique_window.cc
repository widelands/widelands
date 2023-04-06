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
void UniqueWindow::Registry::create() const {
	if (window == nullptr) {
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
void UniqueWindow::Registry::destroy() const {
	if (window != nullptr) {
		window->save_position();
		window->die();
	}
}

/**
 * Either destroys or creates the window.
 */
void UniqueWindow::Registry::toggle() {
	if (window != nullptr) {
		// There is already a window. If it is minimal, restore it.
		if (window->is_minimal()) {
			window->restore();
			opened();
		} else {
			window->save_position();
			window->die();
			closed();
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
	if (window != nullptr) {
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
	if (registry_ != nullptr) {
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
	if ((registry_ != nullptr) && (registry_->window != nullptr)) {
		assert(registry_->window == this);

		save_position();
		registry_->window = nullptr;
		registry_->closed();
	}
}

void UniqueWindow::save_position() {
	if ((registry_ != nullptr) && (registry_->window != nullptr)) {
		assert(registry_->window == this);

		registry_->x = get_x();
		registry_->y = get_y();
		registry_->valid_pos = true;
	}
}

void UniqueWindow::set_pos(Vector2i new_pos) {
	UI::Panel::set_pos(new_pos);
	save_position();
}

/**
 * Restore latest position, or move so that it is inside the parent panel.
 */
void UniqueWindow::move_inside_parent() {
	const Panel* parent = get_parent();
	if (parent == nullptr) {
		return;
	}

	if (registry_->valid_pos) {
		constexpr int32_t kMinVisible = 100;

		const bool top_visible = registry_->y < parent->get_inner_h() - kMinVisible;
		const bool bottom_visible = registry_->y + get_h() > kMinVisible;
		const bool left_visible = registry_->x < parent->get_inner_w() - kMinVisible;
		const bool right_visible = registry_->x + get_w() > kMinVisible;

		if (top_visible && bottom_visible && left_visible && right_visible) {
			set_pos(Vector2i(registry_->x, registry_->y));
			return;
		}
	}
	Window::move_inside_parent();
}

}  // namespace UI
