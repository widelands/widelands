/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "wui/fileview.h"

#include "graphic/graphic.h"

FileViewWindow::FileViewWindow
	(UI::Panel& parent, UI::UniqueWindow::Registry& reg, const std::string& title)
	:
	  UI::UniqueWindow(&parent, "file_view", &reg, 0, 0, title),
	  tabs_(this, 0, 0, 560, 340, g_gr->images().get("images/ui_basic/but4.png"))
 {
	set_inner_size(560, 340);

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void FileViewWindow::add_tab(const std::string& lua_script) {
	tabs_.add_tab(lua_script);
}
