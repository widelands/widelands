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

#ifndef WL_UI_BASIC_UNIQUE_WINDOW_H
#define WL_UI_BASIC_UNIQUE_WINDOW_H

#include <functional>

#include "ui_basic/window.h"

namespace UI {
class Panel;

/**
 * Can only be created once, when it is requested to
 * open a second one, it will implicitly kill the old one
 */
struct UniqueWindow : public Window {
	struct Registry {
		UniqueWindow* window;

		// Whenever Registry::toggle() is called, a window will be created using
		// 'open_window' or if one is open already, the existing one will be
		// closed.
		std::function<void()> open_window;

		// Triggered when the window opens
		Notifications::Signal<> opened;
		// Triggered when the window closes
		Notifications::Signal<> closed;

		void create();
		void destroy();
		void toggle();

		int32_t x, y;
		bool valid_pos;

		Registry() : window(nullptr), x(0), y(0), valid_pos(false) {
		}
		~Registry();
	};

	UniqueWindow(Panel* parent,
	             WindowStyle,
	             const std::string& name,
	             Registry*,
	             int32_t x,
	             int32_t y,
	             int32_t w,
	             int32_t h,
	             const std::string& title);
	UniqueWindow(Panel* parent,
	             WindowStyle,
	             const std::string& name,
	             Registry*,
	             int32_t w,
	             int32_t h,
	             const std::string& title);
	~UniqueWindow() override;

	bool get_usedefaultpos() {
		return usedefaultpos_;
	}

private:
	Registry* registry_;
	bool usedefaultpos_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_UNIQUE_WINDOW_H
