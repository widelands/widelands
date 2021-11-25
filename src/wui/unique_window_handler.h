/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_WUI_UNIQUE_WINDOW_HANDLER_H
#define WL_WUI_UNIQUE_WINDOW_HANDLER_H

#include <map>

#include "base/macros.h"
#include "ui_basic/unique_window.h"

// Handles unique windows that should not close when their parents close. They
// must have a unique name and their registry is owned by this class.
// TODO(sirver): Eventually all unique windows should live here. A new button
// class could integrate with this code to get rid of all the ugly explicit
// handling of pressed button state when a Window is open.
class UniqueWindowHandler {
public:
	UniqueWindowHandler() = default;

	// Returns the registry for 'name'.
	UI::UniqueWindow::Registry& get_registry(const std::string& name);

private:
	std::map<std::string, UI::UniqueWindow::Registry> registries_;

	DISALLOW_COPY_AND_ASSIGN(UniqueWindowHandler);
};

#endif  // end of include guard: WL_WUI_UNIQUE_WINDOW_HANDLER_H
